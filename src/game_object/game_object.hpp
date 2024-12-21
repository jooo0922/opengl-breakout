#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../utils/texture.hpp"
#include "../renderer/sprite_renderer.hpp"

/**
 * GameObject 클래스
 *
 * 현재 게임의 Scene 내에 노출되는 모든 object 들의 기본 Entity 를 정의한 클래스
 * -> 게임 내의 모든 object 들은 GameObject 로 구현되거나, GameObject 를 상속받아 구현됨.
 */
class GameObejct
{
public:
  // object 상태 변수
  glm::vec2 Position, Size, Velocity;
  glm::vec3 Color;
  float Rotation;
  bool IsSolid;   // object 파괴 가능 여부
  bool Destroyed; // object  파괴 여부

  // object 를 Sprite 로 렌더링할 때 사용할 텍스쳐 객체
  Texture2D Sprite;

  // 생성자 함수들
  GameObejct();                                                                                                                               // 기본 생성자 -> 멤버변수들을 기본값으로 초기화
  GameObejct(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f), glm::vec2 velocity = glm::vec2(0.0f, 0.0f)); // 멤버변수들의 값을 외부에서 정의할 수 있는 생성자 오버로딩

  // draw call (자식 클래스에서 override 할 수 있도록 가상함수로 정의)
  virtual void Draw(SpriteRenderer &renderer);
};

#endif /* GAME_OBJECT_HPP */
