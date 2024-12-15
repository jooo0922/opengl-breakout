#include "shader.hpp"
#include <iostream> // 콘솔 입출력을 위한 헤더

Shader &Shader::Use()
{
  glUseProgram(this->ID);
  return *this;
};

void Shader::Compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource = nullptr)
{
  // 생성된 쉐이더 객체 ID 할당받을 변수 선언
  unsigned int sVertex, sFragment, gShader;

  // 버텍스 쉐이더 생성 및 컴파일
  sVertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(sVertex, 1, &vertexSource, NULL);
  glCompileShader(sVertex);
  checkCompileErrors(sVertex, "VERTEX");

  // 프래그먼트 쉐이더 생성 및 컴파일
  sFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(sFragment, 1, &fragmentSource, NULL);
  glCompileShader(sFragment);
  checkCompileErrors(sFragment, "FRAGMENT");

  // 지오메트리 쉐이더 소스를 입력받았을 경우, 지오메트리 쉐이더 생성 및 컴파일
  if (geometrySource != nullptr)
  {
    gShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(gShader, 1, &geometrySource, NULL);
    glCompileShader(gShader);
    checkCompileErrors(gShader, "GEOMETRY");
  }

  // 쉐이더 프로그램 객체 생성 및 쉐이더 객체 연결
  this->ID = glCreateProgram();
  glAttachShader(this->ID, sVertex);
  glAttachShader(this->ID, sFragment);
  if (geometrySource != nullptr)
  {
    glAttachShader(this->ID, gShader);
  }
  glLinkProgram(this->ID);
  checkCompileErrors(this->ID, "PROGRAM");

  // 쉐이더 객체 삭제
  glDeleteShader(sVertex);
  glDeleteShader(sFragment);
  if (geometrySource != nullptr)
  {
    glDeleteShader(gShader);
  }
};

void Shader::SetBool(const char *name, bool value, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform1i(glGetUniformLocation(this->ID, name), (int)value);
};

void Shader::SetInt(const char *name, int value, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform1i(glGetUniformLocation(this->ID, name), value);
};

void Shader::SetFloat(const char *name, float value, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform1f(glGetUniformLocation(this->ID, name), value);
};

void Shader::SetVec2(const char *name, const glm::vec2 &value, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform2fv(glGetUniformLocation(this->ID, name), 1, &value[0]);
};

void Shader::SetVec2(const char *name, float x, float y, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform2f(glGetUniformLocation(this->ID, name), x, y);
};

void Shader::SetVec3(const char *name, const glm::vec3 &value, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform3fv(glGetUniformLocation(this->ID, name), 1, &value[0]);
};

void Shader::SetVec3(const char *name, float x, float y, float z, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
};

void Shader::SetVec4(const char *name, const glm::vec4 &value, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform4fv(glGetUniformLocation(this->ID, name), 1, &value[0]);
};

void Shader::SetVec4(const char *name, float x, float y, float z, float w, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
};

void Shader::SetMat2(const char *name, const glm::mat2 &mat, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniformMatrix2fv(glGetUniformLocation(this->ID, name), 1, GL_FALSE, &mat[0][0]);
};

void Shader::SetMat3(const char *name, const glm::mat3 &mat, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniformMatrix3fv(glGetUniformLocation(this->ID, name), 1, GL_FALSE, &mat[0][0]);
};

void Shader::SetMat4(const char *name, const glm::mat4 &mat, bool useShader = false)
{
  if (useShader)
  {
    this->Use();
  }
  glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, GL_FALSE, &mat[0][0]);
};

void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
  int success;
  char infoLog[1024];

  if (type != "PROGRAM")
  {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
  }
  else
  {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
  }
};
