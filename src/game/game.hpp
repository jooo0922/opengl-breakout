#ifndef GAME_HPP
#define GAME_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// 현재 게임 상태를 enum 으로 정의
enum GameState
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_WIN
};

#endif /* GAME_HPP */
