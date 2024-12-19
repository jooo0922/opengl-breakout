#include "sprite_renderer.hpp"

SpriteRenderer::SpriteRenderer(Shader &shader)
{
  this->shader = shader;
  this->initRenderData();
};

SpriteRenderer::~SpriteRenderer()
{
  // 소멸자 함수 내에서 2D Quad VAO 객체 메모리 반납
  glDeleteVertexArrays(1, &this->quadVAO);
};

void SpriteRenderer::DrawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color)
{
  // 2D Sprite 렌더링 시 적용할 쉐이더 객체 바인딩
  this->shader.Use();

  // 2D Sprite 모델행렬 초기화
  glm::mat4 model = glm::mat4(1.0f);

  // scale -> rotate -> translate 순으로 변환행렬 적용
  // (참고로, 행렬 합성은 우측 -> 좌측 방향이므로, 곱셈 순서는 변환행렬 적용의 역순인 translate -> rotate -> scale 순으로!)
  model = glm::translate(model, glm::vec3(position, 0.0f));

  // 2D Sprite pivot 변경 후 rotate 적용 (하단 필기 참고)
  model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));   // pivot 을 2D Sprite 가운데로 이동
  model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));  // 회전 변환 적용
  model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // pivot 을 2D Sprite 좌상단으로 원복

  // scale 변환 적용
  model = glm::scale(model, glm::vec3(size, 1.0f));

  // 모델행렬 및 색상값 쉐이더로 전송
  this->shader.SetMat4("model", model);
  this->shader.SetVec3("spriteColor", color);

  // 0번 texture unit 활성화 및 전달받은 텍스쳐 객체 바인딩
  glActiveTexture(GL_TEXTURE0);
  texture.Bind();

  // 2D Quad VAO 객체 바인딩 후 draw call
  glBindVertexArray(this->quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
};

void SpriteRenderer::initRenderData()
{
  /** 2D Quad 렌더링 시 사용할 VBO, VAO 객체 생성 및 바인딩 */

  // 실제 정점 데이터 바인딩 시, VAO 객체만 바인딩하면 되므로, VBO ID 값은 멤버변수로 들고있지 않아도 됨.
  unsigned int VBO;
  float vertices[] = {
      // pos      // tex
      0.0f, 1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,

      0.0f, 1.0f, 0.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &this->quadVAO);
  glGenBuffers(1, &VBO);

  // 2D Quad 정점 데이터를 VBO 객체에 write
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(this->quadVAO);

  // 2D Quad 의 pos, uv 데이터가 vec4 로 묶인 0번 attribute 변수 활성화 및 데이터 해석 방식 정의
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  // 정점 데이터 설정 완료 후 VBO, VAO 바인딩 해제
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

/**
 * 2D Sprite pivot 변경 후 rotate 적용
 *
 *
 * initRenderData() 함수를 보면, 현재 2D Quad 의 좌상단 정점이 vec2(0.0f, 0.0f) 로 정의되어 있음.
 * 즉, 현재 2D Sprite 의 pivot 이 2D Quad 좌상단으로 설정되어 있다는 뜻.
 *
 * 이렇게 되면 회전 변환 시
 * 변환의 중심이 2D Quad 좌상단(= pivot)으로 설정되어 의도치 않은 변환 결과가 나옴.
 *
 * 이러한 문제를 해결하기 위해, 회전 변환을 적용하기 전,
 * pivot 을 2D Quad 의 가운데 지점으로 옮긴 뒤에 rotation 처리를 해야 함.
 * 그리고 나서 rotation 적용 완료되면 다시 2D Quad 의 pivot 을 좌상단으로 원상복구 시키면 됨.
 *
 * 이를 위해, 현재 2D Quad 크기의 절반(0.5f * size.x, 0.5f * size.y)만큼 pivot 을 이동시킨 후,
 * 회전 변환을 적용한 다음, 다시 pivot 이동을 원래 위치로 돌려놓는 계산을 처리한 것!
 */
