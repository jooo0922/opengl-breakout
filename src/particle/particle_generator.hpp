#ifndef PARTICLE_GENERATOR_HPP
#define PARTICLE_GENERATOR_HPP

#include <vector>

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

/**
 * ParticleGenerator 클래스
 *
 *
 * 매 프레임마다 일정 수명을 갖는 particle 재생성, 제거, 렌더링을 담당하는 클래스
 *
 * 참고로, 개별 particle 들은 GameObject 로 렌더링하지 않고,
 * ParticleGenerator 내에서 자체 계산된 정점 데이터를 사용.
 */
class ParticleGenerator
{
public:
  // 생성자 (particle 렌더링에 사용할 쉐이더 객체, 텍스쳐 객체, 오브젝트 풀에서 관리할 전체 particle 개수)
  ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);

  // 매 프레임마다 particle 업데이트 (particle 재생성 및 각 particle property 업데이트)
  void Update(float dt, GameObejct &object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

  // 수명이 남아있는 particle 렌더링
  void Draw();

private:
  std::vector<Particle> particles; // 정해진 개수의 particle 들을 관리하는 컨테이너 -> Object Pool
  unsigned int amount;             // 오브젝트 풀에 담긴 전체 particle 개수
  Shader shader;                   // particle 렌더링에 사용할 쉐이더 객체
  Texture2D texture;               // particle 렌더링에 사용할 텍스쳐 객체
  unsigned int VAO;                // particle 렌더링에 사용할 정점 데이터가 바인딩된 VAO 객체

  // particle 렌더링에 사용할 정점 데이터 및 버퍼 객체 초기화
  void init();

  // 가장 먼저 수명이 다해서 대기 상태에 있는 particle 탐색 -> 대기 상태에 있는 particle respawn 목적
  unsigned int firstUnusedParticle();

  // 대기 상태에 있는 particle 를 재사용할 수 있도록 respawn
  void respawnParticle(Particle &particle, GameObejct &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};

#endif /* PARTICLE_GENERATOR_HPP */
