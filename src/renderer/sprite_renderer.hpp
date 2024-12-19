#ifndef SPRITE_RENDERER_HPP
#define SPRITE_RENDERER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../utils/texture.hpp"
#include "../utils/shader.hpp"

/**
 * SpriteRenderer 클래스
 *
 * 2D Sprite 렌더링에 필요한 쉐이더, 정점 데이터 등을 멤버로 갖는 추상화된 renderer 클래스.
 * 2D Sprite 렌더링에 필요한 정점 데이터 초기화 및 draw call 호출 가능
 *
 * 여러 개의 2D Sprite 들을 단일 SpriteRenderer 인스턴스를 통해
 * 초기화된 resource 를 재사용하여 렌더링함.
 */
class SpriteRenderer
{
public:
  SpriteRenderer(Shader &shader);
  ~SpriteRenderer();

  // sprite draw call
  void DrawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f));

private:
  // 2D Sprite 렌더링 시 바인딩할 쉐이더
  Shader shader;

  // 2D Sprite 렌더링 시 바인딩할 2D Quad 정점 데이터를 바인딩하는 VAO 객체 ID
  unsigned int quadVAO;

  // 2D Sprite 정점 데이터 초기화
  void initRenderData();
};

#endif /* SPRITE_RENDERER_HPP */
