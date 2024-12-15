#ifndef SHADER_HPP
#define SHADER_HPP

/*
  #ifndef ~ #endif 전처리기는
  헤더파일의 헤더가드를 처리하여 중복 include 방지해 줌!
*/

#include <glad/glad.h> // OpenGL 함수를 초기화하기 위한 헤더
#include <string>      // std::string
#include <glm/glm.hpp> // glm 라이브러리
#include <glm/gtc/type_ptr.hpp>

/*
  Shader 클래스

  쉐이더 파일 파싱, 컴파일, 컴파일 에러 예외처리,
  쉐이더 프로그램 생성 및 관리, uniform 변수에 데이터 전송 등

  쉐이더와 관련된 모든 작업들을 관리하는 클래스!

  즉, 기존 쉐이더 관련 코드들을 별도의 클래스로 추출하는
  리팩토링을 했다고 보면 됨!

  + file loading 작업은 ResourceManager 클래스로 추출함으로써,
  이 클래스에서는 순수한 Shader 관련 작업만 수행하도록 리팩토링함. (단일 책임 원칙)
*/
class Shader
{
public:
  unsigned int ID; // 생성된 ShaderProgram의 참조 ID

  // Shader 클래스 생성자
  Shader() {};

  // ShaderProgram 객체 활성화
  Shader &Use();

  // 주어진 shader 문자열로 Shader 생성 및 컴파일 (geometry shader 문자열은 optional)
  void Compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource = nullptr);

  // 유니폼 변수 관련 유틸리티
  void SetBool(const char *name, bool value, bool useShader = false);
  void SetInt(const char *name, int value, bool useShader = false);
  void SetFloat(const char *name, float value, bool useShader = false);
  void SetVec2(const char *name, const glm::vec2 &value, bool useShader = false);
  void SetVec2(const char *name, float x, float y, bool useShader = false);
  void SetVec3(const char *name, const glm::vec3 &value, bool useShader = false);
  void SetVec3(const char *name, float x, float y, float z, bool useShader = false);
  void SetVec4(const char *name, const glm::vec4 &value, bool useShader = false);
  void SetVec4(const char *name, float x, float y, float z, float w, bool useShader = false);
  void SetMat2(const char *name, const glm::mat2 &mat, bool useShader = false);
  void SetMat3(const char *name, const glm::mat3 &mat, bool useShader = false);
  void SetMat4(const char *name, const glm::mat4 &mat, bool useShader = false);

private:
  // 쉐이더 객체 및 쉐이더 프로그램 객체의 컴파일 및 링킹 에러 대응
  void checkCompileErrors(unsigned int shader, std::string type);
};

#endif /* SHADER_HPP */
