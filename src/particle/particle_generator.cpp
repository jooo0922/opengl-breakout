#include "particle_generator.hpp"

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount)
    : shader(shader), texture(texture), amount(amount)
{
  this->init();
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
