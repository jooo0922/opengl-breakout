#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glad/glad.h> // OpenGL 함수를 초기화하기 위한 헤더

class Texture
{
public:
  // 생성된 텍스쳐 객체 ID
  unsigned int ID;

  // 텍스쳐 버퍼 해상도
  unsigned int Width;
  unsigned int Height;

  // 텍스쳐 객체 pixel 포맷
  unsigned int Internal_Format;
  unsigned int Image_Format;

  // 텍스쳐 객체 설정 속성
  unsigned int Wrap_S;
  unsigned int Wrap_T;
  unsigned int Filter_Min;
  unsigned int Filter_Max;

  Texture();

  // 생성된 텍스쳐 객체에 메모리 할당 및 이미지 데이터 write
  void Generate(unsigned int width, unsigned int height, unsigned char *data);

  // 텍스쳐 객체 바인딩
  void Bind() const;
};

#endif /* TEXTURE_HPP */
