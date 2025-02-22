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

#endif /* TEXT_RENDERER_HPP */
