#include "post_processor.hpp"
#include <iostream>

PostProcessor::PostProcessor(Shader shader, unsigned int width, unsigned int height)
    : PostProcessingShader(shader), Texture(), Width(width), Height(height), Confuse(false), Chaos(false), Shake(false)
{
  /** framebuffer 및 renderbuffer 생성 */
  glGenFramebuffers(1, &this->MSFBO);
  glGenFramebuffers(1, &this->FBO);
  glGenRenderbuffers(1, &this->RBO);

  /** multisampled 프레임버퍼 color buffer 로 사용할 renderbuffer attach */
  // (**MSAA 설정은 대부분의 그래픽 드라이버에 기본 활성화되어 있으므로, glEnable(GL_MULTISAMPLE) 중복 활성화 생략.)
  glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
  // multisampled buffer 를 지원하는 Renderbuffer 의 경우, glRenderbufferStorageMultisample() 함수를 이용해서 메모리 할당
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, this->RBO);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::POSTPROCESSOR: Failed to initialize MSFBO" << std::endl;
  }

  /** intermediate 프레임버퍼 color buffer 로 사용할 texture attach */
  glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
  this->Texture.Generate(width, height, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Texture.ID, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::POSTPROCESSOR: Failed to initialize FBO" << std::endl;
  }

  // 프레임버퍼 설정 완료 후 기본 프레임버퍼로 바인딩 초기화
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
};