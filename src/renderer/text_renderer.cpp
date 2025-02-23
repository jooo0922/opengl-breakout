#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "text_renderer.hpp"
#include "../manager/resource_manager.hpp"

TextRenderer::TextRenderer(unsigned int width, unsigned int height)
{
  // 텍스트 렌더링 시 바인딩할 쉐이더 객체 생성 및 uniform 변수 전송
  this->TextShader = ResourceManager::LoadShader("resources/shaders/text.vs", "resources/shaders/text.fs", nullptr, "text");

  // 2D 텍스트 렌더링 시 적용할 orthogonal projection 행렬 계산 및 전송 (관련 필기 하단 참고)
  this->TextShader.SetMat4("projection", glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f), true);

  // 각 glyph 텍스쳐를 바인딩할 0번 texture unit 위치값 전송
  this->TextShader.SetInt("text", 0);

  /** 2D Quad 의 VAO, VBO 객체 생성 및 설정 */
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glBindVertexArray(this->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  // 2D Quad 는 각 glyph metrices 에 따라 매 프레임마다 정점 데이터가 자주 변경되므로, GL_DYNAMIC_DRAW 모드로 정점 데이터 버퍼의 메모리를 예약함.
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};

void TextRenderer::Load(std::string font, unsigned int fontSize) {

};

void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {

};

/**
 * 2D 텍스트 렌더링 시 적용할 orthogonal projection 행렬 계산
 *
 * https://github.com/jooo0922/opengl-text-rendering/blob/main/src/main.cpp 예제에서는
 * bottom 을 0.0f 로 지정해서 y축 좌표가 bottom -> top 방향으로 증가했지만,
 * breakout 게임에서는 orthogonal projection 행렬의 top 을 0.0f 로 지정하므로, y축 좌표가 top -> bottom 방향으로 증가함.
 *
 * 따라서, 투영행렬 적용 시 y축 방향이 뒤집어지므로,
 * glyph metrices 수직방향 offset 과 2D Quad 정점 좌표 계산 방법이
 * opengl-text-rendering 예제와 달라짐.
 */
