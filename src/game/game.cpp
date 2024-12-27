#include "game.hpp"
#include "../manager/resource_manager.hpp"
#include "../renderer/sprite_renderer.hpp"
#include "../game_object/game_object.hpp"
#include "../game_object/ball_object.hpp"

/** 게임 관련 상태 변수들 전역 선언(가급적 전역 변수 사용 지양...) */
SpriteRenderer *Renderer;
GameObejct *Player;
BallObject *Ball;

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
  ResourceManager::LoadTexture("resources/textures/background.jpg", false, "background");
  ResourceManager::LoadTexture("resources/textures/block.png", false, "block");
  ResourceManager::LoadTexture("resources/textures/block_solid.png", false, "block_solid");
  ResourceManager::LoadTexture("resources/textures/paddle.png", true, "paddle");

  // .lvl 파일을 로드하여 각 단계별 GameLevel 인스턴스 생성 및 컨테이너에 추가(= 인스턴스 복사)
  GameLevel one;
  GameLevel two;
  GameLevel three;
  GameLevel four;
  one.Load("resources/levels/one.lvl", this->Width, this->Height / 2);
  two.Load("resources/levels/two.lvl", this->Width, this->Height / 2);
  three.Load("resources/levels/three.lvl", this->Width, this->Height / 2);
  four.Load("resources/levels/four.lvl", this->Width, this->Height / 2);
  this->Levels.push_back(one);
  this->Levels.push_back(two);
  this->Levels.push_back(three);
  this->Levels.push_back(four);

  // 현재 게임 level 을 1단계(one)로 초기화
  this->Level = 0;

  // player paddle 시작 위치가 화면 하단 중앙에 오도록 screen space 기준 2D Sprite 좌상단 위치값 계산
  // -> player paddle 객체를 GameObject 인스턴스로 동적 할당 생성
  glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
  Player = new GameObejct(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

  // ball 시작 위치가 player paddle 중앙 윗쪽에 오도록 screen space 기준 2D Sprite 좌상단 위치값 계산
  // -> ball 객체를 BallObject 인스턴스로 동적 할당 생성
  glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
  Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::Update(float dt)
{
  Ball->Move(dt, this->Width);
}

void Game::ProcessInput(float dt)
{
  // 현재 게임 상태가 GAME_ACTIVE 인 경우에만 사용자 입력 처리 수행
  if (this->State == GAME_ACTIVE)
  {
    /**
     * 프레임 당 player paddle 이동거리 계산
     *
     * 클라이언트마다 framerate 이 달라도 일정한 이동 속도를 유지하려면,
     * '속도 * 매 프레임마다의 delta time(= 시간 간격)' 을 곱해줘야
     * 임의의 framerate 을 갖는 각 클라이언트에서 현재 프레임에서 이동해야 할 거리값이 나오겠지!
     *
     * -> 즉, '거리 = 속력 * 시간' 이라는
     * 기초적인 물리 공식을 활용한 예시
     */
    float velocity = PLAYER_VELOCITY * dt;

    // A 키 입력 시 player paddle 좌측 이동
    if (this->Keys[GLFW_KEY_A])
    {
      // 좌측 이동 시, player paddle 이 화면 왼쪽 모서리를 넘어가지 않도록 x축 위치값(= 2D Sprite 좌상단 정점의 x좌표값) 범위 제한
      if (Player->Position.x >= 0.0f)
      {
        Player->Position.x -= velocity;

        // ball 이 player paddle 에 고정되어 있을 경우, player paddle 을 따라가도록 이동
        if (Ball->Stuck)
        {
          Ball->Position.x -= velocity;
        }
      }
    }
    // D 키 입력 시 player paddle 우측 이동
    if (this->Keys[GLFW_KEY_D])
    {
      // 우측 이동 시, player paddle 이 화면 오른쪽 모서리를 넘어가지 않도록 x축 위치값(= 2D Sprite 좌상단 정점의 x좌표값) 범위 제한
      if (Player->Position.x <= this->Width - Player->Size.x)
      {
        Player->Position.x += velocity;

        // ball 이 player paddle 에 고정되어 있을 경우, player paddle 을 따라가도록 이동
        if (Ball->Stuck)
        {
          Ball->Position.x += velocity;
        }
      }
    }
    // Space 키 입력 시 ball 을 player paddle 에서 분리
    if (this->Keys[GLFW_KEY_SPACE])
    {
      Ball->Stuck = false;
    }
  }
}

void Game::Render()
{
  // 현재 게임 상태가 GAME_ACTIVE 인 경우에만 렌더링 로직 수행
  if (this->State == GAME_ACTIVE)
  {
    // 배경을 2D Sprite 로 렌더링
    Renderer->DrawSprite(
        ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

    // 현재 게임 level 에 대응되는 GameLevel draw call 호출
    this->Levels[this->Level].Draw(*Renderer);

    // playder paddle draw call 호출
    Player->Draw(*Renderer);

    // ball draw call 호출
    Ball->Draw(*Renderer);
  }
}
