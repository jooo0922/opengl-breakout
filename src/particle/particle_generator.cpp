#include "particle_generator.hpp"
#include <cstdlib>

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount)
    : shader(shader), texture(texture), amount(amount)
{
  this->init();
}

void ParticleGenerator::Update(float dt, GameObejct &object, unsigned int newParticles, glm::vec2 offset)
{
  // 매 프레임마다 newParticles 개수만큼 particle respawn
  for (unsigned int i = 0; i < newParticles; i++)
  {
    // 가장 먼저 수명이 다해서 대기 상태에 있는 particle 탐색
    int unusedParticle = this->firstUnusedParticle();

    // 탐색된 대기 상태의 particle 재사용을 위해 오브젝트 풀에서 꺼내 respawn
    this->respawnParticle(this->particles[unusedParticle], object, offset);
  }

  // 오브젝트 풀에 저장된 모든 particle 객체들을 순회하며 데이터 업데이트
  for (unsigned int i = 0; i < this->amount; i++)
  {
    /**
     * 매 프레임마다의 delta time 값만큼 수명 감소
     *
     * -> 따라서, 만약 어떤 Particle 의 수명이 1.0f 라면,
     * 해당 Particle 은 1초 뒤에 수명이 다할 것이고,
     * 수명에 2.0, 3.0 등 스칼라 곱하면 수명이 2초, 3초와 같이 결정됨.
     */
    Particle &p = this->particles[i];
    p.Life -= dt;

    // 아직 수명이 남아있는 Particle 들의 위치와 색상 업데이트
    if (p.Life > 0.0f)
    {
      p.Position -= p.Velocity * dt; // object 중심을 향해 천천히 이동
      p.Color.a -= dt * 2.5f;        // alpha 값을 감소시켜 서서히 없어지는 것처럼 보이도록 함.
    }
  }
};

void ParticleGenerator::Draw()
{
  // particle 이 겹칠 때 glowy effect 를 주기 위해 blending function 을 additive blending(가산 혼합)으로 설정
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // particle 렌더링 시 사용할 쉐이더 객체 바인딩
  this->shader.Use();

  // 오브젝트 풀에 저장된 particle 을 순회하며 렌더링
  for (Particle particle : this->particles)
  {
    // 수명이 남아있는 particle 만 렌더링
    if (particle.Life > 0.0f)
    {
      this->shader.SetVec2("offset", particle.Position);
      this->shader.SetVec4("color", particle.Color);

      // 0번 texture unit 활성화 및 전달받은 텍스쳐 객체 바인딩
      glActiveTexture(GL_TEXTURE0);
      this->texture.Bind();

      glBindVertexArray(this->VAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindVertexArray(0);
    }
  }

  // 렌더링 완료 후 blending function 을 default 로 원복
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
  /**
   * particle 2D Quad 렌더링 시 사용할 VBO, VAO 객체 생성 및 바인딩
   * (SpriteRenderer::initRenderData() 함수와 정점 데이터 설정 동일)
   */
  // 실제 정점 데이터 바인딩 시, VAO 객체만 바인딩하면 되므로, VBO ID 값은 멤버변수로 들고있지 않아도 됨.
  unsigned int VBO;
  float particle_quad[] = {
      // pos      // tex
      0.0f, 1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,

      0.0f, 1.0f, 0.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(this->VAO);

  // 2D Quad 정점 데이터를 VBO 객체에 write
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

  // 2D Quad 의 pos, uv 데이터가 vec4 로 묶인 0번 attribute 변수 활성화 및 데이터 해석 방식 정의
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  // 정점 데이터 설정 완료 후 VBO, VAO 바인딩 해제
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // this->amount 에 해당하는 개수만큼 오브젝트 풀에 particle 객체를 미리 생성
  for (unsigned int i = 0; i < this->amount; i++)
  {
    this->particles.push_back(Particle());
  }
};

unsigned int lastUsedParticle = 0; // ParticleGenerator::firstUnusedParticle() 함수를 통해 가장 최근에 탐색되어 respawn 된 particle index 를 전역변수로 저장
unsigned int ParticleGenerator::firstUnusedParticle()
{
  // 첫 번째 탐색 전략 -> 가장 최근에 탐색된 particle 이후부터 탐색해본다. (탐색 전략 하단 필기 참고)
  for (unsigned int i = lastUsedParticle; i < this->amount; i++)
  {
    if (this->particles[i].Life <= 0.0f)
    {
      /**
       * 오브젝트 풀에서 수명이 다한 particle 을 찾았다면,
       * 1. 가장 최근에 탐색된 particle index 를 업데이트하고,
       * 2. 해당 particle index 를 반환한다.
       */
      lastUsedParticle = i;
      return i;
    }
  }

  // 두 번째 탐색 전략 -> 첫 번째 탐색에서 못찾았다면, 가장 최근에 탐색된 particle 이전에도 마저 탐색해본다.
  for (unsigned int i = 0; i < lastUsedParticle; i++)
  {
    if (this->particles[i].Life <= 0.0f)
    {
      lastUsedParticle = i;
      return i;
    }
  }

  // 수명이 다한 particle 을 찾지 못했다면, 첫 번째 particle index 를 반환해서 respawn(정확히는 property override) 하도록 한다.
  lastUsedParticle = 0;
  return 0;
};

void ParticleGenerator::respawnParticle(Particle &particle, GameObejct &object, glm::vec2 offset)
{
  float random = ((std::rand() % 100) - 50) / 10.0f;    // [-5.0, 4.9] 범위 난수 생성 -> Particle position 랜덤 조정 목적
  float rColor = 0.5f + ((std::rand() % 100) / 100.0f); // [0.5, 1.49] 범위 난수 생성 -> Particle color 랜덤 조정 목적

  /** 대기 상태의 particle 재사용을 위해 property update */
  particle.Position = object.Position + random + offset;    // ball 위치(object.Position)에서 약간 떨어트린(offset) 뒤, slightly random 하게(random) 재조정
  particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f); // 각 particle 마다 랜덤한 색상 부여
  particle.Life = 1.0f;                                     // particle 수명 1초로 초기화
  particle.Velocity = object.Velocity * 0.1f;               // ball 속도(object.Velocity)와 방향을 맞추되, '속력'은 0.1배로 줄임.
};

/**
 * 수명이 다한 particle 탐색 전략
 *
 *
 * ParticleGenerator::firstUnusedParticle() 함수에서는
 * 기본적으로 linear search(선형 탐색, 순차 탐색) 방식으로 배열의 처음부터 순차적으로 요소를 탐색하는 구조임.
 *
 * 탐색 시점에 수명이 다한 particle 이 배열에 여러 개 존재하더라도,
 * 순차 탐색의 원리 상 '가장 먼저 마주친 죽은 particle' 을 반환하게 되므로,
 * 다음 번 순차 탐색을 할 때에는 이전에 찾은 죽은 particle index (= lastUsedParticle) 이후에
 * 나머지 죽은 particle 이 남아있을 가능성이 훨씬 높음.
 *
 * 따라서, 첫 번째 탐색 전략은 lastUsedParticle 이후의 particle 들을 우선적으로 탐색해야
 * 죽은 particle 을 더 빠르게 찾을 수 있을 것임.
 *
 * 첫 번째 탐색에서 죽은 particle 을 찾지 못했다면,
 * lastUsedParticle 이전에 아직 살펴보지 않은 particle 들을 마저 찾아보는 게
 * 두 번째 탐색 전략!
 */
