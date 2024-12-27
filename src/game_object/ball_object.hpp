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

  // ball 이동 및 bouncing 구현 -> Game::Update() 라이프 사이클에서 호출
  glm::vec2 Move(float dt, unsigned int window_width);

  // 화면을 벗어난 ball 의 위치와 속도 초기화
  void Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif /* BALL_OBJECT_HPP */
