#ifndef PARTICLE_GENERATOR_HPP
#define PARTICLE_GENERATOR_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../utils/shader.hpp"
#include "../utils/texture.hpp"
#include "../game_object/game_object.hpp"

// Particle 구조체 정의
struct Particle
{
  glm::vec2 Position, Velocity; // Particle 위치 및 속도
  glm::vec4 Color;              // Particle 색상
  float Life;                   // Particle 수명 (0.0f 에 도달하면 Particle 제거됨.)

  Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {};
};

#endif /* PARTICLE_GENERATOR_HPP */
