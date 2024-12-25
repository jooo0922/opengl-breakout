#ifndef GAME_HPP
#define GAME_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../level/game_level.hpp"

// 현재 게임 상태를 enum 으로 정의
enum GameState
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_WIN
};

/**
 * Game 클래스
 *
 * 게임/그래픽 로직을 추상화하여 windowing library 로직과 de-coupling 한 클래스.
 * 게임의 전반적인 구성 요소를 한데 모아 관리하는 uber class
 *
 * (참고로, 'uber' 는 독일어로 '모든 것을 아우르는, 포괄적인, 최상위' 라는 의미를 가짐.)
 */
class Game
{
public:
  GameState State;            // 게임 상태
  bool Keys[1024];            // 키 입력 플래그
  unsigned int Width, Height; // 게임 창 resolution

  std::vector<GameLevel> Levels; // 각 단계별 GameLevel 인스턴스 저장 컨테이너
  unsigned int Level;            // 현재 게임 level

  Game(unsigned int width, unsigned int height);
  ~Game();

  /** 게임 라이프사이클 함수 정의 */
  void Init();                 // 초기화 라이프사이클 (shader/texture/levels 등의 resource loading)
  void ProcessInput(float dt); // 사용자 입력 처리 라이프사이클 -> delta time 전달받음.
  void Update(float dt);       // 업데이트 라이프사이클 (플레이어, 공 이동 업데이트 등) -> delta time 전달받음.
  void Render();               // 렌더링 라이프사이클
};

#endif /* GAME_HPP */
