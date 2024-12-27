#include "ball_object.hpp"

BallObject::BallObject()
    : GameObejct(), Radius(12.5f), Stuck(true) {};

BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite)
    // 멤버 초기화 리스트에서 부모 클래스 GameObject 생성자 함수 호출하여 상속받은 멤버변수들도 같이 초기화함.
    : GameObejct(pos, glm::vec2(radius * 2.0f, radius * 2.0f), sprite, glm::vec3(1.0f), velocity), Radius(radius), Stuck(true) {};

glm::vec2 BallObject::Move(float dt, unsigned int window_width)
{
  // player paddle 에 고정되어 있지 않은 상태에서만 이동 로직 수행
  if (!this->Stuck)
  {
    // 프레임 당 ball 이동거리 계산 (프레임 당 이동 거리 = 속도 * delta time)
    this->Position += this->Velocity * dt;

    if (this->Position.x <= 0.0f)
    {
      /** ball 왼쪽 모서리 충돌 시 처리 */
      this->Velocity.x = -this->Velocity.x; // ball x축 이동 방향 뒤집기
      this->Position.x = 0.0f;              // ball x축 위치값 초기화
    }
    else if (this->Position.x + this->Size.x >= window_width)
    {
      /** ball 오른쪽 모서리 충돌 시 처리 */
      this->Velocity.x = -this->Velocity.x;           // ball x축 이동 방향 뒤집기
      this->Position.x = window_width - this->Size.x; // ball x축 위치값 초기화
    }
    if (this->Position.y <= 0.0f)
    {
      /** ball 위쪽 모서리 충돌 시 처리 */
      this->Velocity.y = -this->Velocity.y; // ball y축 이동 방향 뒤집기
      this->Position.y = 0.0f;              // ball y축 위치값 초기화
    }
  }
  /**
   * 참고로, ball 아래쪽 모서리 충돌 시 game over 이므로,
   * 해당 로직은 게임 상태 변경과 관련되어 있음.
   *
   * 따라서, BallObject 클래스가 아닌 uber 클래스인 Game 에
   * 별도로 구현 예정.
   */

  // 최종적으로 이동한 ball 위치 반환
  return this->Position;
};

void BallObject::Reset(glm::vec2 position, glm::vec2 velocity) {};
