#include "resource_manager.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/**
 * singleton 클래스는 인스턴스를 별도로 초기화하지 않으므로,
 * std::map 컨테이너같은 정적 멤버변수들에 저장 공간을 할당하기 위해서는
 * .cpp 같은 구현부에서 이 컨테이너에 정적 메모리를 반드시 초기화해줘야 함.
 *
 * 그렇지 않으면 컴파일러가 해당 정적 멤버 변수의 공간을 할당하지 않아 링크 에러가 발생함.
 */
std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture2D> ResourceManager::Textures;

Shader ResourceManager::LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name)
{
  // 로드 및 생성된 Shader 객체를 std:map 컨테이너에 저장
  Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
  return Shaders[name];
};

Texture2D ResourceManager::LoadTexture(const char *file, bool alpha, std::string name)
{
  // 로드 및 생성된 Texture2D 객체를 std:map 컨테이너에 저장
  Textures[name] = loadTextureFromFile(file, alpha);
  return Textures[name];
};

Shader ResourceManager::GetShader(std::string name)
{
  return Shaders[name];
};

Texture2D ResourceManager::GetTexture(std::string name)
{
  return Textures[name];
};

void ResourceManager::Clear()
{
  // 각 std::map 컨테이너를 for-each 문으로 순회하며 메모리 반납
  // (참고로, std::map 컨테이너의 각 노드는 std::pair<first(key), second(value)> 타입으로 저장되어 있음.)
  for (auto iter : Shaders)
  {
    glDeleteProgram(iter.second.ID);
  }
  for (auto iter : Textures)
  {
    glDeleteTextures(1, &iter.second.ID);
  }
};

Shader ResourceManager::loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile)
{
  // 쉐이더 코드를 std::string 타입으로 파싱하여 저장할 변수 선언
  std::string vertexCode;
  std::string fragmentCode;
  std::string geometryCode;

  try
  {
    // std::ifstream 생성자 함수를 직접 호출하면 내부에서 std::ifstream::open() 실행을 통해 파일을 연다.
    std::ifstream vertexShaderFile(vShaderFile);
    std::ifstream fragmentShaderFile(fShaderFile);

    // std::ifstream 을 통해 읽은 파일 내용을 복사할 std::stringstream 선언
    std::stringstream vShaderStream, fShaderStream;

    // 스트림 버퍼에 임시 저장된 파일 내용을 std::stringstream 의 메모리 기반 버퍼에 복사 (관련 내용 하단 필기)
    vShaderStream << vertexShaderFile.rdbuf();
    fShaderStream << fragmentShaderFile.rdbuf();

    // 파일 스트림 객체 닫기
    vertexShaderFile.close();
    fragmentShaderFile.close();

    // std::stringstream 의 메모리 기반 버퍼에 저장된 데이터를 std::string 컨테이너에 복사하여 반환
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    // 지오메트리 쉐이더 파일 경로를 입력받은 경우, 파일 로드 및 std::string 타입으로 파싱
    if (gShaderFile != nullptr)
    {
      std::ifstream geometryShaderFile(gShaderFile);
      std::stringstream gShaderStream;
      gShaderStream << geometryShaderFile.rdbuf();
      geometryShaderFile.close();
      geometryCode = gShaderStream.str();
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "ERROR::SHADER:: Failed to read shader files" << std::endl;
  }

  // std::string 을 c-style 문자열로 변환
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();
  const char *gShaderCode = geometryCode.c_str();

  // 쉐이더 객체 생성 및 컴파일
  Shader shader;
  shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
  return shader;
};

Texture2D ResourceManager::loadTextureFromFile(const char *file, bool alpha)
{
  // Texture2D 객체 생성
  Texture2D texture;

  // alpha 매개변수에 따라 텍스쳐 internal format 변경
  if (alpha)
  {
    texture.Internal_Format = GL_RGBA;
    texture.Image_Format = GL_RGBA;
  }

  // stb_image 라이브러리로 이미지 데이터 로드
  int width, height, nrChannels;
  unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);

  // 텍스쳐 생성 및 이미지 데이터 write
  texture.Generate(width, height, data);

  // 이미지 데이터 메모리 반납
  stbi_image_free(data);

  return texture;
};

/**
 * 스트림 객체와 스트림 버퍼
 *
 *
 * std::ifstream::open() 실행했을 때,
 * 열린 파일 내용을 '스트림 버퍼(std::filebuf)'에 임시로 저장하게 되는데,
 * 이 스트림 버퍼의 주소값을 std::ifstream::rdbuf() 를 통해 반환받음.
 *
 * std::stringstream << std::filebuf* 로 스트림 버퍼 주소값을 흘려보내면,
 * std::stringstream 가 관리하는 일반적인 '메모리 기반 버퍼'에
 * 스트림 버퍼(std::filebuf)의 내용을 복사하여 저장함.
 *
 * 이를 통해, 파일 내용을 일반적인 문자열처럼 자유롭게 다룰 수 있도록 함.
 *
 * -> 참고로, '스트림 버퍼'란
 * 스트림 객체가 데이터를 읽고 쓸 때 효율적으로 관리할 수 있도록
 * 별도로 동적 할당되는 메모리 영역임.
 *
 * 스트림 버퍼는 주로 버퍼링된 입출력(I/O 작업)을 최적화하기 위해 존재하는
 * '중간 버퍼' 이므로, 스트림 객체와 연결되어
 * 스트림 객체를 통한 데이터 읽기/쓰기를 최적화하는 역할을 수행함.
 *
 * 스트림 객체는 데이터를 일정한 단위로 하나씩 순차적으로 읽어들이는 '버퍼링'을 수행함.
 *
 * 그러므로, 스트림 객체가 순차적으로 읽어들일 일부 데이터들을
 * 파일이 저장된 디스크에서 한 번에 가져와서
 * 임시로 저장하는 공간이 '스트림 버퍼' 라고 보면 됨.
 *
 * (디스크 접근으로 인한 오버헤드를 줄이기 위해,
 * 가급적 많은 양의 데이터를 한 번에 가져와서 임시로 저장해 둠.)
 */
