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
  // GLFW 초기화 및 윈도우 설정 구성
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // 현재 운영체제가 macos 일 경우, 미래 버전의 OpenGL 을 사용해서 GLFW 창을 생성하여 버전 호환성 이슈 해결
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // GLFW 윈도우 크기 조정 비활성화
  glfwWindowHint(GLFW_RESIZABLE, false);

  // GLFW 윈도우 생성 및 현재 OpenGL 컨텍스트로 등록
  GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // GLAD 를 사용하여 OpenGL 표준 API 호출 시 사용할 현재 그래픽 드라이버에 구현된 함수 포인터 런타임 로드
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    // 함수 포인터 로드 실패
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // GLFW 윈도우 콜백함수 등록
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  /** OpenGL 전역 상태 설정 */
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  // 투명 처리를 위한 blending mode 활성화
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /** rendering loop */
  while (!glfwWindowShouldClose(window))
  {
    /* code */
  }

  // GLFW 종료 및 메모리 반납
  glfwTerminate();

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
