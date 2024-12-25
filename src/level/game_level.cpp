#include "game_level.hpp"

#include <fstream>
#include <sstream>

void GameLevel::Load(const char *file, unsigned int levelWidth, unsigned int levelHeight)
{
  // 이전 Bricks 데이터 제거
  this->Bricks.clear();

  // std::ifstream 생성자 함수를 호출하여 .lvl 파일 열기
  unsigned int tileCode;
  std::string line;
  std::ifstream fstream(file);

  // .lvl 파일에서 읽은 레벨 데이터를 파싱하여 저장할 tileData 컨테이너 선언
  std::vector<std::vector<unsigned int>> tileData;

  if (fstream)
  {
    // .lvl 파일을 한 줄씩 읽으면서 파싱
    while (std::getline(fstream, line))
    {
      // .lvl 파일의 각 줄을 '문자열 입력 스트림'으로 초기화 -> 각 문자열의 내용을 순차적으로 읽거나 분석하기 위해!
      std::istringstream sstream(line);

      // .lvl 파일에서 각 줄의 level 값을 읽어서 저장할 컨테이너 선언
      std::vector<unsigned int> row;

      // 더 이상 읽을 데이터가 없을 때까지 공백을 기준으로 문자열 스트림 버퍼에 저장된 level 값을 하나씩 읽어서 tileCode 에 복사함.
      // 참고로, std::istringstream >> std::string (= 스트림 추출 연산)을 할 경우, 한 번 읽은 데이터는 스트림 버퍼에서 제거됨.
      while (sstream >> tileCode)
      {
        row.push_back(tileCode);
      }

      // 각 줄을 순회하며 스트림 추출 연산을 완료했다면, 각 줄의 level 값이 저장된 컨테이너를 tileData 컨테이너에 동적으로 추가
      tileData.push_back(row);
    }

    // .lvl 파일이 파싱된 tileData 컨테이너를 init 함수의 파라미터로 전달하여 실행
    if (tileData.size() > 0)
    {
      this->init(tileData, levelWidth, levelHeight);
    }
  }
};

void GameLevel::Draw(SpriteRenderer &renderer) {};

bool GameLevel::IsCompleted() {};

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
{
  // tileData 의 행과 열 수를 계산
  unsigned int rows = tileData.size();
  unsigned int columns = tileData[0].size();

  // 각 block 의 pixel 단위 크기 계산
  float unit_width = levelWidth / static_cast<float>(columns);
  float unit_height = levelHeight / static_cast<float>(rows);

  // tileData 를 순회하며 각 Brick 에 대응되는 GameObject 인스턴스 생성
  for (unsigned int y = 0; y < rows; ++y)
  {
    for (unsigned int x = 0; x < columns; ++x)
    {
      if (tileData[y][x] == 1)
      {
        /** Brick 타입이 solid 인 경우 */
        // 현재 tile 의 행과 열을 기반으로 Brick 의 위치(= 2D Sprite 의 좌상단 좌표값) 및 크기 계산
        glm::vec2 pos(unit_width * x, unit_height * y);
        glm::vec2 size(unit_width, unit_height);

        // 현재 Brick 에 대응되는 GameObject 인스턴스 생성 및 컨테이너에 추가
        GameObejct obj(pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
        obj.IsSolid = true;
        this->Bricks.push_back(obj);
      }
      else if (tileData[y][x] > 1)
      {
        /** Brick 타입이 solid 가 아닌 경우 */
        // 현재 tile 값에 따라 서로 다른 색상으로 렌더링되도록 색상값 계산
        glm::vec3 color = glm::vec3(1.0f);
        if (tileData[y][x] == 2)
        {
          color = glm::vec3(0.2f, 0.6f, 1.0f);
        }
        else if (tileData[y][x] == 3)
        {
          color = glm::vec3(0.0f, 0.7f, 0.0f);
        }
        else if (tileData[y][x] == 4)
        {
          color = glm::vec3(0.8f, 0.8f, 0.4f);
        }
        else if (tileData[y][x] == 5)
        {
          color = glm::vec3(1.0f, 0.5f, 0.0f);
        }

        // 현재 tile 의 행과 열을 기반으로 Brick 의 위치(= 2D Sprite 의 좌상단 좌표값) 및 크기 계산
        glm::vec2 pos(unit_width * x, unit_height * y);
        glm::vec2 size(unit_width, unit_height);

        // 현재 Brick 에 대응되는 GameObject 인스턴스 생성 및 컨테이너에 추가
        this->Bricks.push_back(
            GameObejct(pos, size, ResourceManager::GetTexture("block"), color));
      }
    }
  }
};
