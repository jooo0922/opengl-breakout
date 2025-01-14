#include "particle_generator.hpp"

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
