#include "game.hpp"
#include "../manager/resource_manager.hpp"
#include "../renderer/sprite_renderer.hpp"

/** 게임 관련 상태 변수들 전역 선언(가급적 전역 변수 사용 지양...) */
SpriteRenderer *Renderer;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
  // 동적 할당된 게임 상태 변수(전역 선언)들 메모리 반납
  delete Renderer;
}

void Game::Init()
{
  // 2D Sprite 쉐이더 객체 생성
  ResourceManager::LoadShader("resources/shaders/sprite.vs", "resources/shaders/sprite.fs", nullptr, "sprite");

  // 2D Sprite 에 적용할 orthogonal projection 행렬 계산
  // 2D Quad 정점 데이터 및 위치를 직관적인 screen space 좌표계로 다루기 위해, screen size 해상도로 left, right, top, bottom 정의
  // 아래와 같이 orthogonal 투영행렬을 정의하면 world space 좌표를 screen space 와 동일하게 다루어도 알아서 [-1, 1] 범위의 NDC 좌표계로 변환해 줌.
  // https://github.com/jooo0922/opengl-text-rendering/blob/main/src/main.cpp 참고
  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

  // 2D Sprite 쉐이더에 uniform 변수 전송
  ResourceManager::GetShader("sprite").Use().SetInt("image", 0);
  ResourceManager::GetShader("sprite").SetMat4("projection", projection);

  // 생성된 2D Sprite 쉐이더 객체를 넘겨줘서 SpriteRenderer 인스턴스 동적 할당 생성
  Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

  // 2D Sprite 에 적용할 텍스쳐 객체 생성
  ResourceManager::LoadTexture("resources/textures/awesomeface.png", true, "face");
}

void Game::Update(float dt)
{
}

void Game::ProcessInput(float dt)
{
}

void Game::Render()
{
}
