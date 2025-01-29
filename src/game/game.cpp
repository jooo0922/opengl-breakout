#include <cmath>
#include "game.hpp"
#include "../manager/resource_manager.hpp"
#include "../renderer/sprite_renderer.hpp"
#include "../game_object/game_object.hpp"
#include "../game_object/ball_object.hpp"
#include "../particle/particle_generator.hpp"
#include "../postprocess/post_processor.hpp"

/** 게임 관련 상태 변수들 전역 선언(가급적 전역 변수 사용 지양...) */
SpriteRenderer *Renderer;
GameObejct *Player;
BallObject *Ball;
ParticleGenerator *Particles;
PostProcessor *Effects;

// solid collision 발생 시 reset 되는 shake effect 활성화 지속시간 전역 변수로 선언
float ShakeTime = 0.0f;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
  // 동적 할당된 게임 상태 변수(전역 선언)들 메모리 반납
  delete Renderer;
  delete Player;
  delete Ball;
  delete Particles;
  delete Effects;
}

void Game::Init()
{
  // 2D Sprite 쉐이더 객체 생성
  ResourceManager::LoadShader("resources/shaders/sprite.vs", "resources/shaders/sprite.fs", nullptr, "sprite");
  ResourceManager::LoadShader("resources/shaders/particle.vs", "resources/shaders/particle.fs", nullptr, "particle");

  // 2D Sprite 에 적용할 orthogonal projection 행렬 계산
  // 2D Quad 정점 데이터 및 위치를 직관적인 screen space 좌표계로 다루기 위해, screen size 해상도로 left, right, top, bottom 정의
  // 아래와 같이 orthogonal 투영행렬을 정의하면 world space 좌표를 screen space 와 동일하게 다루어도 알아서 [-1, 1] 범위의 NDC 좌표계로 변환해 줌.
  // https://github.com/jooo0922/opengl-text-rendering/blob/main/src/main.cpp 참고
  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

  // 2D Sprite 쉐이더에 uniform 변수 전송
  ResourceManager::GetShader("sprite").Use().SetInt("image", 0);
  ResourceManager::GetShader("sprite").SetMat4("projection", projection);
  ResourceManager::GetShader("particle").Use().SetInt("sprite", 0);
  ResourceManager::GetShader("particle").SetMat4("projection", projection);

  // 2D Sprite 에 적용할 텍스쳐 객체 생성
  ResourceManager::LoadTexture("resources/textures/awesomeface.png", true, "face");
  ResourceManager::LoadTexture("resources/textures/background.jpg", false, "background");
  ResourceManager::LoadTexture("resources/textures/block.png", false, "block");
  ResourceManager::LoadTexture("resources/textures/block_solid.png", false, "block_solid");
  ResourceManager::LoadTexture("resources/textures/paddle.png", true, "paddle");
  ResourceManager::LoadTexture("resources/textures/particle.png", true, "particle");

  // 생성된 2D Sprite 쉐이더 객체를 넘겨줘서 SpriteRenderer 인스턴스 동적 할당 생성
  Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

  // 생성된 Particle 쉐이더 객체를 넘겨줘서 ParticleGenerator 인스턴스 동적 할당 생성
  Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);

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

  // 매 프레임마다 각 particle 재생성 및 업데이트
  Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

  // ball 아래쪽 모서리 충돌 시 game over -> 게임 리셋 처리
  if (Ball->Position.y >= this->Height)
  {
    this->ResetLevel();
    this->ResetPlayer();
  }
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
    // multisampled 프레임버퍼에 scene 요소 렌더링 직전 처리
    Effects->BeginRender();

    // 배경을 2D Sprite 로 렌더링
    Renderer->DrawSprite(
        ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

    // 현재 게임 level 에 대응되는 GameLevel draw call 호출
    this->Levels[this->Level].Draw(*Renderer);

    // playder paddle draw call 호출
    Player->Draw(*Renderer);

    // particle draw call 호출 -> particle 은 ball 을 따라다니는 잔상 효과이므로, 다른 오브젝트들보다는 위에 그리지만, ball 을 가리지 않도록 그보다는 먼저 그림
    Particles->Draw();

    // ball draw call 호출
    Ball->Draw(*Renderer);

    // multisampled 프레임버퍼에 렌더링된 결과를 intermediate 프레임버퍼에 blit 으로 복사
    Effects->EndRender();

    // intermediate 프레임버퍼 렌더링 결과에 post processing 적용 후 2D Quad 렌더링
    Effects->Render(glfwGetTime());
  }
}

void Game::ResetLevel()
{
  // 현재 게임 level 에 대응되는 .lvl 파일을 다시 로드하여 GameLevel::Bricks 컨테이너를 초기화함
  if (this->Level == 0)
  {
    this->Levels[0].Load("resources/levels/one.lvl", this->Width, this->Height / 2);
  }
  else if (this->Level == 1)
  {
    this->Levels[1].Load("resources/levels/two.lvl", this->Width, this->Height / 2);
  }
  else if (this->Level == 2)
  {
    this->Levels[2].Load("resources/levels/three.lvl", this->Width, this->Height / 2);
  }
  else if (this->Level == 3)
  {
    this->Levels[3].Load("resources/levels/four.lvl", this->Width, this->Height / 2);
  }
};

void Game::ResetPlayer()
{
  // Player 및 Ball 의 멤버변수와 상태값을 모두 초기화함(Game::Init() 함수 참고)
  Player->Size = PLAYER_SIZE;
  Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
  Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
};

// collision detection 관련 함수 전방선언
bool checkCollision(GameObejct &one, GameObejct &two);
Collision checkCollision(BallObject &one, GameObejct &two);
Direction VectorDirection(glm::vec2 target);

void Game::DoCollisions()
{
  // 현재 Level 의 모든 Brick 들을 순회하면서 Ball 과의 충돌 검사
  for (GameObejct &box : this->Levels[this->Level].Bricks)
  {
    // 아직 파괴되지 않은 Brick 들에 대해서만 충돌 검사
    if (!box.Destroyed)
    {
      Collision collision = checkCollision(*Ball, box);
      if (std::get<0>(collision)) // std::get<n>(std::tuple) -> 현재 tuple 데이터쌍에서 n번째 요소를 읽음.
      {
        // 현재 Brick 이 non-solid brick 인 경우에만 파괴 상태 업데이트
        if (!box.IsSolid)
        {
          box.Destroyed = true;
        }
        else
        {
          // solid block collision 발생 시, shake effect 활성화 및 지속시간 reset
          ShakeTime = 0.05f;
          Effects->Shake = true;
        }

        // -> 왜 solid brick 도 충돌 검사를 할까? solid brick 과 충돌 시 처리할 것도 있으니까!(ex> 이동방향 전환 등)
        Direction dir = std::get<1>(collision);         // 충돌 방향
        glm::vec2 diff_vector = std::get<2>(collision); // circle 중점 ~ 가장 가까운 점 P 사이의 거리

        // 충돌 방향에 따른 충돌 처리
        if (dir == LEFT || dir == RIGHT) // 수평 방향 충돌 처리
        {
          // 공의 수평 이동방향 뒤집기
          Ball->Velocity.x = -Ball->Velocity.x;

          // Circle - AABB 충돌 시, 수평 방향으로 AABB 내부로 침투한 거리(level of penetration) 계산
          float penetration = Ball->Radius - std::abs(diff_vector.x);

          // 충돌 방향에 따라 반대 방향으로 침투 거리만큼 relocate -> 충돌한 공이 AABB 내부에 들어가지 못하도록 위치를 재조정한 것!
          if (dir == LEFT)
          {
            Ball->Position.x += penetration;
          }
          else
          {
            Ball->Position.x -= penetration;
          }
        }
        else // 수직 방향 충돌 처리
        {
          // 공의 수직 이동방향 뒤집기
          Ball->Velocity.y = -Ball->Velocity.y;

          // Circle - AABB 충돌 시, 수직 방향으로 AABB 내부로 침투한 거리(level of penetration) 계산
          float penetration = Ball->Radius - std::abs(diff_vector.y);

          // 충돌 방향에 따라 반대 방향으로 침투 거리만큼 relocate -> 충돌한 공이 AABB 내부에 들어가지 못하도록 위치를 재조정한 것!
          if (dir == UP)
          {
            Ball->Position.y -= penetration;
          }
          else
          {
            Ball->Position.y += penetration;
          }
        }
      }
    }
  }

  // Ball - Player Paddle 충돌 검사
  Collision result = checkCollision(*Ball, *Player);
  if (!Ball->Stuck && std::get<0>(result))
  {
    /** Ball - Paddle 충돌 처리 */
    // Ball 충돌 지점이 Paddle 중심에서 떨어진 거리의 비율값(percentage) 계산 -> Paddle 끝부분에 가까울수록 1, 중심에 가까울수록 0
    float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
    float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
    float percentage = distance / (Player->Size.x / 2.0f);

    // 원래 속도 벡터를 복사해 둠.
    glm::vec2 oldVelocity = Ball->Velocity;

    // Ball 충돌 지점이 Paddle 중심에서 멀수록 x축 속도를 증가시킴
    float strength = 2.0f;
    Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;

    // Paddle 과 충돌할 경우, 수직 이동방향이 항상 위쪽을 향하도록 계산 (수직 이동방향을 뒤집지 않는 이유 하단 필기)
    Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);

    // 속'력'은 일정하게 유지하도록 속도 벡터의 길이를 원래 속도 벡터와 동일하게 맞춤. -> Ball 충돌 지점에 따라 속도 벡터의 방향만 변경되겠군!
    Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
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
Collision checkCollision(BallObject &one, GameObejct &two)
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
  if (glm::length(difference) < one.Radius)
  {
    // 더 자세한 충돌 정보를 사용자 정의 타입 Collision 으로 파싱하여 반환
    return std::make_tuple(true, VectorDirection(difference), difference);
  }
  else
  {
    return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
  }
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

/**
 * Ball - Player Paddle 충돌 시, 수직 이동방향은 뒤집지 않고 UP 방향으로 고정하는 이유
 *
 *
 * //Ball->Velocity.y = -Ball->Velocity.y;
 * Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
 *
 * Ball - Player Paddle 충돌 시 수직 이동방향 변경을 위와 같이 처리한 것을 볼 수 있음.
 * 이는 LearnOpenGL 본문에서 'sticky paddle' 이라고 불리는 issue 를 해결하기 위한 목적임.
 *
 * paddle 이 너무 빠르게 공을 향해 움직이다 보면, 공이 paddle AABB 내부에 들어가는 케이스가 발생함.
 * 이럴 경우, 공이 AABB 안쪽에 갇혀 있으니 매 프레임마다 충돌 검사 결과가 true 로 감지될 것임.
 *
 * 공이 AABB 안에 갇혀서 계속 충돌되면, Ball→Velocity.y 를 계속 뒤집어버리다 보니,
 * 공이 paddle AABB 를 빨리 벗어나지 못하고 일시적으로 AABB 내부를 맴돌게 됨.
 * → why? 공의 수직 이동 방향이 위쪽으로 변경되는 게 아니라 위>아래>위>아래>… 이런 식으로 계속 뒤집어지니까!
 *
 * 이러한 문제를 해결하기 위해, 공의 paddle 충돌 방향은 항상 UP 으로 가정
 * (즉, 공이 paddle 에 충돌할 때에는 무조건 top of the paddle 에서만 충돌한다고 가정)함으로써,
 * 공의 수직 이동 방향 변경 시, 항상 위쪽 방향으로 향하도록
 * Ball→Velocity.y = -1.0f * abs(Ball→Velocity.y); 와 같이 고정된 방향으로 계산해줘야 함.
 *
 * 이렇게 해야 공이 paddle AABB 내부를 맴돌지 않고
 * 빠르게 위쪽 방향으로 이동방향을 전환해서 벗어날 수 있음.
 */