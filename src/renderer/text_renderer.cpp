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

void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
  // shader 객체 바인딩 및 색상값 전송
  this->TextShader.Use();
  this->TextShader.SetVec3("textColor", color);

  // grayscale bitmap 텍스쳐(glyph 텍스쳐)를 바인딩할 0번 texture unit 활성화
  glActiveTexture(0);

  // glyph 텍스쳐를 적용할 2D Quad 정점 데이터 VAO 객체 바인딩
  glBindVertexArray(this->VAO);

  /** 주어진 문자열 컨테이너 std::string 을 순회하며 각 문자에 대응되는 glyph 를 2D Quad 에 렌더링  */
  // std::string 컨테이너를 순회하는 '읽기 전용' 이터레이터 선언 (하단 필기 참고)
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++)
  {
    // 현재 순회 중인 char 타입 문자에 대응되는 glyph metrices 를 가져옴
    Character ch = Characters[*c];

    // 현재 문자를 렌더링할 glyph 의 위치(= 2D Quad 의 좌상단 정점의 좌표값) 계산 (하단 필기 참고)
    float xpos = x + ch.Bearing.x * scale;
    float ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

    // 현재 문자를 렌더링할 glyph 의 크기(= 2D Quad 의 width, height) 계산
    float w = ch.Size.x * scale;
    float h = ch.Size.y * scale;

    // glyph 의 위치(= 2D Quad 좌상단 정점)와 크기(= 2D Quad 의 width, height)를 가지고 2D Quad 정점 데이터 계산
    // (이때, text-rendering 예제와 달리 orthogonal projection 행렬에 의해 위아래가 뒤집혔으므로, 정점 순서를 변경해서 2D Quad 의 앞/뒷면이 뒤집어지지 않도록 함.)
    float vertices[6][4] = {
        // position      // uv
        {xpos, ypos + h, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 0.0f},
        {xpos, ypos, 0.0f, 0.0f},

        {xpos, ypos + h, 0.0f, 1.0f},
        {xpos + w, ypos + h, 1.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 0.0f}};

    // 2D Quad 에 적용할 grayscale bitmap 텍스쳐 버퍼 바인딩
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);

    // 재계산된 2D Quad 정점 데이터를 VBO 객체에 덮어쓰기
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 2D Quad draw call
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /**
     * 현재 glyph 원점에서 Advance 만큼 떨어진 다음 glyph 원점의 x 좌표값 계산
     *
     * 이 때, FreeType 라이브러리의 Advance 값은 1/64 px 단위로 계산되기 때문에,
     * 이것을 1px 단위로 변환해서 사용해야 함.
     *
     * 이를 위해 1/2^6(= 1/64)제곱값을 구해서 Advance 에 곱할 수도 있으나,
     * >> 6, 즉, right bit shift 연산을 6번 수행하면 1/2 를 6제곱하는 것과 동일함.
     *
     * 심지어, bit shift 연산이 거듭제곱보다 더 빠르기 때문에, 성능 최적화에 유리함.
     */
    x += (ch.Advance >> 6) * scale;
  }

  // std::string 컨테이너에 저장된 모든 문자열 렌더링 완료 후, 텍스쳐 및 VAO 객체 바인딩 해제
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
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

/**
 * std::string::const_iterator c
 *
 *
 * 읽기 전용 이터레이터 c 에는 연속 메모리 블록으로 저장되는 std::string 컨테이너 내부의
 * 각 char 타입의 문자들이 저장된 메모리 블록 주소값이 담겨 있음.
 *
 * 따라서, *c (= de-referencing)을 통해 해당 메모리에 저장된 실제 char 타입 문자에 접근 가능.
 *
 * 이때, for 문 내에서 c++ 연산자를 사용하면 현재 가리키고 있는 메모리 주소로부터
 * 포인터가 가리키는 데이터 타입(= char)의 크기(= 1 byte)만큼 더해서 다음 메모리 블록에 저장된
 * char 타입 문자를 가리키는 주소값을 얻을 수 있음.
 *
 * -> 즉, 연속 메모리 블록 형태로 저장된 std::string 컨테이너의 각 char 타입 값을
 * 하나씩 순차적으로 접근할 수 있도록 for 문을 구성한 것.
 */

/**
 * glyph 의 위치(= 2D Quad 의 좌상단 정점의 좌표값) 계산
 *
 *
 * 참고로, 로컬 변수 x, y 에는 현재 렌더링할 glyph 좌상단 원점이 저장됨.
 *
 * text-rendering 예제와 달리, orthographic projection 행렬이
 * 'top = 0, bottom = window height' 로 뒤집혀서 계산되므로,
 * 현재 glyph 좌상단 원점에 Bearing 값을 더해 glyph 를 얼만큼 아래로 떨어트릴 지 결정함.
 *
 * 그러나, FreeType 라이브러리는 baseline(즉, glyph 원점이 포함된 수평선)에서
 * 얼만큼 위쪽으로 떨어졌는지에 대한 offset 값만 Bearing 으로 제공하고 있을 뿐,
 * glyph 의 천장에서 얼만큼 아래쪽으로 떨어졌는지에 대한 offset 은 제공하지 않음.
 *
 * 따라서, glyph 의 천장에서 얼만큼 떨어트릴지의 offset 은 직접 계산해야 되는데,
 * 이때, 'H', 'T', 'X' 처럼 천장과의 간격이 0인 glyph 들의 Bearing.y 는
 * 해당 폰트의 최대 높이로 가정한다면, 이 glyph 들의 높이값을 이용할 수 있음.
 *
 * 이 최대 높이값에 g, j, p, j 처럼 glyph 일부가 baseline(즉, glyph 원점이 포함된 수평선) 하단에 내려오는 글꼴들의,
 * Bearing.y 값을 뺀다면, g, j, p, j 같은 glyph 들을 천장에서 아래쪽으로 얼만큼 떨어트려야 할 지 알 수 있음!
 *
 * 그래서 아래쪽으로 떨어트릴 offset 을
 * y + (this->Characters['H'].Bearing.y - ch.Bearing.y) 와 같이 계산한 것!
 */
