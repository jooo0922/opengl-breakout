#ifndef POWER_UP_HPP
#define POWER_UP_HPP

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.hpp"
#include "../utils/texture.hpp"

// PowerUp 관련 상수 전역 선언
const glm::vec2 POWERUP_SIZE(60.0f, 20.0f);
const glm::vec2 VELOCITY(0.0f, 150.0f);

/**
 * PowerUp 아이템 클래스를 GameObject 를 상속받아 구현
 */
class PowerUp : public GameObejct
{
  // powerup 상태 변수
  std::string Type; // powerup 아이템 유형
  float Duration;   // powerup 아이템에 의한 게임 상태 변경 지속시간
  bool Activated;   // powerup 아이템에 의한 게임 상태 변경 여부

  PowerUp(std::string type, glm::vec3 color, float duration, glm::vec2 position, Texture2D texture)
      : GameObejct(position, POWERUP_SIZE, texture, color, VELOCITY), Type(type), Duration(duration), Activated() {};
};

#endif /* POWER_UP_HPP */
