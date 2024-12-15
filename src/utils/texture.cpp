#include "texture.hpp"
#include <iostream> // 콘솔 입출력을 위한 헤더

Texture2D::Texture2D() : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT),
                         Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR) // 텍스쳐 파라미터 멤버변수 초기화
{
  // 텍스쳐 생성 후 ID 값 할당받기
  glGenTextures(1, &this->ID);
};

// 생성된 텍스쳐 객체에 메모리 할당 및 이미지 데이터 write
void Texture2D::Generate(unsigned int width, unsigned int height, unsigned char *data)
{
  // 입력받은 텍스쳐 버퍼 크기로 변경
  this->Width = width;
  this->Height = height;

  // 생성된 텍스쳐 바인딩 후 이미지 데이터 write
  glBindTexture(GL_TEXTURE_2D, this->ID);
  glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, this->Width, this->Height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);

  // 텍스쳐 파라미터 설정
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);

  // 텍스쳐 바인딩 해제
  glBindTexture(GL_TEXTURE_2D, 0);
};

// 텍스쳐 객체 바인딩
void Texture2D::Bind() const
{
  glBindTexture(GL_TEXTURE_2D, this->ID);
};
