#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game/game.hpp"

#include <iostream>

/** 콜백함수 전방 선언 */

// GLFW 윈도우 resizing 콜백함수
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

// GLFW 윈도우 키 입력 콜백함수
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

/** 스크린 해상도 선언 */
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

// Game 클래스 인스턴스 전역 스코프 생성 -> main 함수 외에 콜백함수 접근을 위해 전역 선언
Game Breakout(SCREEN_WIDTH, SCREEN_HEIGHT);

int main()
{
  return 0;
}

/** 콜백함수 구현부 */

// GLFW 윈도우 resizing 콜백함수
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

// GLFW 윈도우 키 입력 콜백함수
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  // ESC 키 입력 시 렌더링 루프 및 프로그램 종료
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, true);
  }

  // 나머지 키 입력 시, Game 클래스의 키 입력 플래그 변경
  if (key >= 0 && key < 1024)
  {
    if (action == GLFW_PRESS)
    {
      // key press 시, 해당 키 입력 플래그 활성화
      Breakout.Keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
      // key release 시, 해당 키 입력 플래그 비활성화
      Breakout.Keys[key] = false;
    }
  }
}
