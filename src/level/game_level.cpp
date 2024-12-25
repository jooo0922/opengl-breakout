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

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight) {};
