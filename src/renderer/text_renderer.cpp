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

void TextRenderer::Load(std::string font, unsigned int fontSize)
{
  // 기존에 파싱해서 컨테이너에 저장해 둔 glyph metrices 초기화 (레드-블랙트리 노드 데이터만 초기화. 노드에 할당된 메모리는 유지)
  this->Characters.clear();

  /** FreeType 라이브러리 초기화 */
  FT_Library ft;
  if (FT_Init_FreeType(&ft))
  {
    // FreeType 라이브러리 초기화 실패 -> FreeType 함수들은 에러 발생 시 0 이 아닌 값을 반환.
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
  }

  /** FT_Face 인터페이스로 .ttf 파일 로드 */
  FT_Face face;
  if (FT_New_Face(ft, font.c_str(), 0, &face))
  {
    // .ttf 파일 로드 실패
    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
  }

  // .ttf 파일 로드 성공 시 작업들 처리

  // .ttf 파일로부터 렌더링할 glyph 들의 pixel size 설정 -> height 값만 설정하고 width 는 각 glyph 형태에 따라 동적으로 계산하도록 0 으로 지정
  FT_Set_Pixel_Sizes(face, 0, fontSize);

  // glyph 가 렌더링된 grayscale bitmap 의 텍스쳐 데이터 정렬 단위 변경 (하단 필기 참고)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // 128 개의 ASCII 문자들의 glyph 들을 8-bit grayscale bitmap 텍스쳐 버퍼에 렌더링
  for (unsigned char c = 0; c < 128; c++)
  {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
      // 각 ASCII 문자에 해당하는 glyph 로드 실패
      std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
      continue;
    }

    // 각 glyph grayscale bitmap 텍스쳐 생성 및 bitmap 데이터 복사
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows, // bitmap 버퍼 줄 수 = bitmap 버퍼 height
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer);

    // 텍스쳐 파라미터 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 로드된 glyph metrices 를 커스텀 자료형으로 파싱
    Character character = {
        texture,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        static_cast<unsigned int>(face->glyph->advance.x)};
    // std::map 컨테이너는 std::pair 를 node 로 삼아 key-value 쌍을 추가함. (참고로, std::map 은 red-black tree 기반의 컨테이너)
    Characters.insert(std::pair<char, Character>(c, character));
  }
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

/**
 * glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
 *
 * GL_UNPACK_ALIGNMENT 란, GPU 가 텍스쳐 버퍼에 저장된 데이터들을 한 줄(row) 단위로 읽을 때,
 * (-> 참고로 여기서 텍스쳐 데이터의 한 줄은 텍스쳐 width 길이만큼과 동일함.)
 * 각 줄의 데이터를 몇 byte 단위로 정렬되도록 할 것인지 지정하는 OpenGL 상태값이라고 보면 됨.
 *
 * 이것의 기본값은 4인데,
 * 그 이유는 OpenGL 에서 다루는 대부분의 텍스쳐 포맷은 GL_RGBA 이므로,
 * 텍스쳐 버퍼 한 줄의 크기는 4 bytes(= r, g, b, a 각각 1 byte 씩) 의 배수로 맞아 떨어짐.
 *
 * 이렇게 되면, 텍스쳐 버퍼의 각 줄(= width)을 GPU 로 전송할 때,
 * 데이터가 기본적으로 4 bytes 단위로 정렬된 상태라고 가정하고 데이터를 읽음.
 * 그래서 일반적으로는 기본값을 그대로 적용해서 GPU 로 텍스쳐를 전송하면 아무런 문제가 안됨.
 *
 * 그러나, 이 예제에서 FreeType 라이브러리가 렌더링해주는 glyph 텍스쳐 버퍼의 포맷은
 * GL_RED 타입의 grayscale bitmap 이므로, 텍스쳐 버퍼의 각 줄의 크기는 1 byte 의 배수로 떨어짐.
 *
 * 이럴 경우, GL_UNPACK_ALIGNMENT 상태값을 1로 변경해서
 * GPU 에게 전송하려는 grayscale bitmap 데이터의 각 줄이 1 byte 단위로 정렬된 상태임을 알려야 함.
 *
 * 이렇게 하지 않으면 소위 Segmentation Fault 라고 하는
 * 허용되지 않은 에모리 영역을 침범하는 memory violation 에러가 런타임에 발생할 수 있음.
 */
