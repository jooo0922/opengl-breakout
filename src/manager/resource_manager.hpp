#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <map>
#include <string>

#include <glad/glad.h>

#include "../utils/shader.hpp"
#include "../utils/texture.hpp"

/**
 * Shader, Texture 등의 리소스 객체 생성, 저장, file system 인터페이스 호출을 담당하는 singleton class
 * -> static 선언된 멤버들을 인스턴스 생성 없이 전역으로 접근 가능.
 */
class ResourceManager
{
public:
  // 생성된 리소스들을 std::pair<std::string name, 리소스 객체> 타입으로 저장할 std::map 컨테이너
  static std::map<std::string, Shader> Shaders;
  static std::map<std::string, Texture2D> Textures;

  // 파일 경로를 입력하면 리소스를 로드 및 생성하여 std::map 컨테이너에 저장하는 함수들 -> 캡슐화된 resource loading 함수들을 내부에서 호출.
  static Shader LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name);
  static Texture2D LoadTexture(const char *file, bool alpha, std::string name);

  // 각 리소스 name 을 통해 std::map 컨테이너에 저장된 리소스를 검색 및 반환하는 getter
  static Shader GetShader(std::string name);
  static Texture2D GetTexture(std::string name);

  // std::map 컨테이너에 저장된 리소스들 메모리 반납
  void Clear();

private:
  // singleton 클래스는 인스턴스 생성이 불필요하므로, 생성자 함수 캡슐화
  ResourceManager() {};

  // file system 인터페이스 호출을 통해 resource loading 처리 함수 캡슐화
  static Shader loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile = nullptr);
  static Texture2D loadTextureFromFile(const char *file, bool alpha);
};

#endif /* RESOURCE_MANAGER_HPP */
