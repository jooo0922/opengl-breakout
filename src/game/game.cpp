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

  // 매 프레임마다 ball 과의 충돌 검사
  this->DoCollisions();
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

// collision detection 관련 함수 전방선언
bool checkCollision(GameObejct &one, GameObejct &two);
bool checkCollision(BallObject &one, GameObejct &two);
Direction VectorDirection(glm::vec2 target);

void Game::DoCollisions()
{
  // 현재 Level 의 모든 Brick 들을 순회하면서 Ball 과의 충돌 검사
  for (GameObejct &box : this->Levels[this->Level].Bricks)
  {
    // 아직 파괴되지 않은 Brick 들에 대해서만 충돌 검사
    if (!box.Destroyed)
    {
      if (checkCollision(*Ball, box))
      {
        // 현재 Brick 이 non-solid brick 인 경우에만 파괴 상태 업데이트
        if (!box.IsSolid)
        {
          box.Destroyed = true;
        }

        // -> 왜 solid brick 도 충돌 검사를 할까? solid brick 과 충돌 시 처리할 것도 있으니까!(ex> 이동방향 전환 등)
      }
    }
  }
};

// AABB - AABB 간 충돌 검사
bool checkCollision(GameObejct &one, GameObejct &two)
{
  // 두 GameObject 의 AABB 간 x축 방향 충돌 검사(= 수평 방향 overlap 검사)
  bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                    two.Position.x + two.Size.x >= one.Position.x;

  // 두 GameObject 의 AABB 간 y축 방향 충돌 검사(= 수직 방향 overlap 검사)
  bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                    two.Position.y + two.Size.y >= one.Position.y;

  // x축, y축 방향 모두 충돌 시, 두 AABB 가 충돌한 것으로 판정
  return collisionX && collisionY;
};

// Circle - AABB 간 충돌 검사 (docs/nodes.md 참고)
bool checkCollision(BallObject &one, GameObejct &two)
{
  // BallObject 의 circle 중점 계산
  glm::vec2 center(one.Position + one.Radius);

  // 두 번째 GameObject 의 AABB 절반 크기 및 중점 계산 계산
  glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
  glm::vec2 aabb_center(
      two.Position.x + aabb_half_extents.x,
      two.Position.y + aabb_half_extents.y);

  // Circle 과 AABB 중점 사이의 vector D 계산
  glm::vec2 difference = center - aabb_center;

  // [-aabb_half_extents, aabb_half_extents] 범위 내로 vector D clamping
  glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

  // AABB edges 와 clamp 된 vector D 가 교차하는 임의의 점 P 계산 -> circle 중점에서 가장 가까운 점 P
  glm::vec2 closest = aabb_center + clamped;

  // circle 중점 ~ 가장 가까운 점 P 사이의 거리 계산
  difference = closest - center;

  // 'circle 중점 ~ 가장 가까운 점 P 사이의 거리' 가 circle 반지름보다 작다면, Circle 과 AABB 가 충돌한 것으로 판정
  return glm::length(difference) < one.Radius;
};

// Circle - AABB 충돌 방향 계산 함수 전방선언
Direction VectorDirection(glm::vec2 target)
{
  // 충돌 방향벡터 정의
  glm::vec2 compass[] = {
      glm::vec2(0.0f, 1.0f),  // up
      glm::vec2(1.0f, 0.0f),  // right
      glm::vec2(0.0f, -1.0f), // down
      glm::vec2(-1.0f, 0.0f)  // left
  };

  float max = 0.0f;
  unsigned int best_match = -1;

  /**
   * 충돌 방향벡터를 순회하며 target 벡터와의 내적값이 가장 큰(= 가장 1에 가까운) 벡터
   * (= 가장 방향이 비슷한 충돌 방향벡터)를 선택하여 best_match 에 갱신
   *
   * -> best_match 에 저장된 index 에 대응되는 Direction 타입으로 casting 후 반환
   */
  for (unsigned int i = 0; i < 4; i++)
  {
    float dot_product = glm::dot(glm::normalize(target), compass[i]);
    if (dot_product > max)
    {
      max = dot_product;
      best_match = i;
    }
  }
  return (Direction)best_match;
};
