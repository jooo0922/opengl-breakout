#include "ball_object.hpp"

BallObject::BallObject()
    : GameObejct(), Radius(12.5f), Stuck(true) {};

BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite)
    // 멤버 초기화 리스트에서 부모 클래스 GameObject 생성자 함수 호출하여 상속받은 멤버변수들도 같이 초기화함.
    : GameObejct(pos, glm::vec2(radius * 2.0f, radius * 2.0f), sprite, glm::vec3(1.0f), velocity), Radius(radius), Stuck(true) {};

glm::vec2 BallObject::Move(float dt, unsigned int window_width) {};

void BallObject::Reset(glm::vec2 position, glm::vec2 velocity) {};