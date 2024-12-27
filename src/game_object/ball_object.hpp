#ifndef BALL_OBJECT_HPP
#define BALL_OBJECT_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.hpp"
#include "../utils/texture.hpp"

/**
 * GameObject 를 상속받아 구현된 BallObject 클래스
 */
class BallObject : public GameObejct
{
public:
  // ball 상태 변수
  float Radius; // ball 반지름
  bool Stuck;   // player paddle 에 고정되었는 지 여부

  BallObject();
  BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);

  glm::vec2 Move(float dt, unsigned int window_width);
  void Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif /* BALL_OBJECT_HPP */
