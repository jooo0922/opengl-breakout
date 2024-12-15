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

Shader ResourceManager::loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile) {};

Texture2D ResourceManager::loadTextureFromFile(const char *file, bool alpha) {};
