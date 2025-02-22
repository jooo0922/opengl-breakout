#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../utils/texture.hpp"
#include "../utils/shader.hpp"

/** FreeType 라이브러리로 로드한 glyph metrices(각 글꼴의 크기, 위치, baseline 등)를 파싱할 자료형 정의 */
struct Character
{
  unsigned int TextureID; // FreeType 내부에서 렌더링된 각 glyph 의 텍스쳐 버퍼 ID
  glm::ivec2 Size;        // glyph 크기
  glm::ivec2 Bearing;     // glyph 원점에서 x축, y축 방향으로 각각 떨어진 offset
  unsigned int Advance;   // 현재 glyph 원점에서 다음 glyph 원점까지의 거리 (1/64px 단위로 정의되어 있으므로, 값 사용 시 1px 단위로 변환해야 함.)
};

/**
 * TextRenderer 클래스
 *
 * FreeType 라이브러리 기반 텍스트 렌더링 관련 코드를 추상화한 클래스
 *
 * 아래 텍스트 렌더링 관련 코드들 재사용하여 구현
 * https://github.com/jooo0922/opengl-text-rendering/blob/main/src/main.cpp
 */
class TextRenderer
{
public:
  // 각 글꼴별 로드된 glyph metrices 를 파싱하여 std::map 컨테이너에 저장 -> key 값을 기준으로 레드-블랙트리 구조에 정렬하므로, 이진 탐색으로 데이터를 읽음
  std::map<char, Character> Character;

  // 텍스트 렌더링 시 바인딩할 쉐이더
  Shader TextShader;

  TextRenderer(unsigned int width, unsigned int height);

  // FreeType 라이브러리 초기화 및 .ttf 파일 로드
  void Load(std::string font, unsigned int fontSize);

  // 주어진 std::string 문자열을 주어진 위치, 크기, 색상으로 렌더링
  void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));

private:
  // 텍스트 렌더링 시 바인딩할 2D Quad 정점 데이터 버퍼 객체 ID
  unsigned int VAO, VBO;
};

#endif /* TEXT_RENDERER_HPP */
