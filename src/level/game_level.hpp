#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../game_object/game_object.hpp"
#include "../renderer/sprite_renderer.hpp"
#include "../manager/resource_manager.hpp"

class GameLevel
{
public:
  // game level 을 구성하는 각 Brick 들을 GameObject 클래스 인스턴스로 생성해서 저장할 std::vector 컨테이너
  // (참고로, 가급적 std::vector 컨테이너에는 인스턴스 자체를 복사하여 추가하는 방식보다는, 스마트 포인터로 주소값을 추가하는 방식이 더 나을 것임.)
  std::vector<GameObejct> Bricks;

  GameLevel() {};

  // .lvl 파일을 로드하여 tileData 로 파싱하는 함수
  void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);

  // draw call 호출 함수
  void Draw(SpriteRenderer &renderer);

  // non-solid bricks 파괴 완료 여부 (= 게임 클리어를 뜻함.)
  bool IsCompleted();

private:
  // 파싱된 tileData 를 전달받아 각 Brick 들을 GameObject 클래스 인스턴스로 생성하여 컨테이너에 저장하는 함수 -> GameLevel::Load() 함수 내부에서 호출
  void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

#endif /* GAME_LEVEL_HPP */
