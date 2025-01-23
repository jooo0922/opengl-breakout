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

  // screen-size 2D Quad 정점 데이터 버퍼(VAO, VBO) 설정
  this->initRenderData();

  /** post processing 쉐이더에 uniform 변수 전송 */
  // scene 요소가 렌더링된 텍스쳐를 바인딩할 0번 texture unit 위치값 전송
  this->PostProcessingShader.SetInt("scene", 0);
  // 프래그먼트 쉐이더에서 scene 요소가 렌더링된 텍스쳐 샘플링 시, 현재 uv 좌표를 중심으로 주변 uv 좌표 계산을 위한 offset 값 전송
  // (https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Framebuffers/MyShaders/framebuffers_screen_kernel_blur.fs 참고)
  float offset = 1.0f / 300.0f;
  float offsets[9][2] = {
      {-offset, offset},  // top-left
      {0.0f, offset},     // top-center
      {offset, offset},   // top-right
      {-offset, 0.0f},    // center-left
      {0.0f, 0.0f},       // center-center
      {offset, 0.0f},     // center - right
      {-offset, -offset}, // bottom-left
      {0.0f, -offset},    // bottom-center
      {offset, -offset}   // bottom-right
  };
  glUniform2fv(glGetUniformLocation(this->PostProcessingShader.ID, "offsets"), 9, (float *)offsets);
  // chaos 효과 적용을 위한 3*3 edge kernel (= convolution matrix) 전송
  // (https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Framebuffers/MyShaders/framebuffers_screen_kernel_edge_detection.fs 참고)
  int edge_kernel[9] = {
      // row-major
      -1, -1, -1, // row1
      -1, 8, -1,  // row2
      -1, -1, -1  // row3
  };
  glUniform1iv(glGetUniformLocation(this->PostProcessingShader.ID, "edge_kernel"), 9, edge_kernel);
  // shake 효과 적용을 위한 3*3 blur kernel (= convolution matrix) 전송
  // (https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Framebuffers/MyShaders/framebuffers_screen_kernel_blur.fs 참고)
  float blur_kernel[9] = {
      // row-major
      1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, // row1
      2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, // row2
      1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f  // row3
  };
  glUniform1fv(glGetUniformLocation(this->PostProcessingShader.ID, "blur_kernel"), 9, blur_kernel);
};

void PostProcessor::BeginRender()
{
  // scene 요소를 렌더링할 multisampled 프레임버퍼 바인딩 및 초기화
  glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
};

void PostProcessor::EndRender()
{
  // multisampled 프레임버퍼에 렌더링된 결과를 intermediate 프레임버퍼에 blit 으로 복사
  // (**multisampled 프레임버퍼 blit 관련 하단 필기 참고)
  glBindFramebuffer(GL_READ_FRAMEBUFFER, this->MSFBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO);
  glBlitFramebuffer(0, 0, this->Width, this->Height, 0, 0, this->Width, this->Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  // blit 을 마친 후 기본 프레임버퍼로 바인딩 원상복구
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

void PostProcessor::Render(float time) {
  // intermediate 프레임버퍼에 복사된 렌더링 결과가 저장된 텍스쳐 버퍼를 샘플링하여 post processing 적용 후 2D Quad 렌더링
};

void PostProcessor::initRenderData()
{
  /**
   * screen-size 2D Quad 렌더링 시 사용할 VBO, VAO 객체 생성 및 바인딩
   */
  // 실제 정점 데이터 바인딩 시, VAO 객체만 바인딩하면 되므로, VBO ID 값은 멤버변수로 들고있지 않아도 됨.
  unsigned int VBO;
  // screen quad 는 screen 크기와 같아야 하므로, clip space 좌표계의 최솟값, 최댓값인 -1.0, 1.0 로 정점 좌표값 정의
  float vertices[] = {
      // pos        // tex
      -1.0f, -1.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 0.0f, 1.0f,

      -1.0f, -1.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f};

  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(this->VAO);

  // 2D Quad 정점 데이터를 VBO 객체에 write
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // 2D Quad 의 pos, uv 데이터가 vec4 로 묶인 0번 attribute 변수 활성화 및 데이터 해석 방식 정의
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  // 정점 데이터 설정 완료 후 VBO, VAO 바인딩 해제
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};

/*
  Blitting multisampled buffer


  여러 개의 subsample 이 있는 multisampled buffer 는
  한 픽셀에 대해 여러 개의 색상 값을 가지고 있음.

  이는 일반적인 off-screen framebuffer 와는 다른 특성이며,
  이러한 특성으로 인해 multisampled buffer 는
  shader 내에서의 텍스쳐 샘플링같은 작업에 직접적으로 사용되지는 못함.

  이를 해결하기 위해, multisampled buffer 를
  일반적인 off-screen framebuffer 로 변환하는 작업이 필요한데,
  이를 'Blitting' 이라고 함.


  'Blitting'은 "Block Image Transfer"의 줄임말로,
  이미지나 버퍼와 같은 데이터 블록을 다른 위치로 전송하는 과정을 뜻함.

  'Blitting' 과정을 자세히 설명하자면,
  먼저 'resolve' 라는 과정을 거쳐야 함.

  각 subsample 에서 계산된 색상 값들을 적절한 방법으로 결합하거나 처리하여
  최종적으로 하나의 픽셀에 대한 색상 값을 얻게 되는데,
  이 과정을 LearnOpenGL 본문에서는 "resolve"라고 부름.

  이후 별도의 non-multisampled 색상 버퍼(== 일반적인 off-screen framebuffer)로
  그 값을 복사하는 것이 일반적임.


  OpenGL에서 glBlitFramebuffer() 함수는
  framebuffer 의 특정 영역을 다른 framebuffeer 로 블릿(blit)하는 데 사용됨.

  이 함수를 통해 multisampled buffer 에서 일반(non-multisampled) framebuffer 로
  이미지를 복사하면서 동시에 multisampled 데이터를
  'resolve' 하는 것으로 보면 됨.
*/

/*
  framebuffer 바인딩 시,
  GL_FRAMEBUFFER vs GL_READ_FRAMEBUFFER vs GL_DRAW_FRAMEBUFFER


  OpenGL 에서 glBindFramebuffer() 함수를 사용해서
  프레임버퍼 객체를 바인딩할 때, 위와 같이 3개의 상태값에 바인딩할 수 있음.

  첫 번째 상태값인 GL_FRAMEBUFFER 에 프레임버퍼를 바인딩하면,
  해당 프레임버퍼는 GL_READ_FRAMEBUFFER 와 GL_DRAW_FRAMEBUFFER 모두에
  바인딩된 것과 동일하게 작동함.

  이는 바인딩된 해당 프레임버퍼가
  '프레임버퍼 읽기 관련 연산'과 '프레임버퍼 쓰기 관련 연산' 모두에
  영향을 받게 됨을 의미함.

  두 번째 상태값인 GL_READ_FRAMEBUFFER 에 프레임버퍼를 바인딩하면,
  해당 프레인버퍼는 '프레임버퍼 읽기 관련 연산에만' 영향을 받게 됨.

  세 번째 상태값인 GL_DRAW_FRAMEBUFFER 에 프레임버퍼를 바인딩하면,
  해당 프레인버퍼는 '프레임버퍼 쓰기 관련 연산에만' 영향을 받게 됨.


  ex>
  glReadPixels() 같은 함수는 '프레임버퍼 읽기 관련 연산'에 해당하므로,
  GL_READ_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼 또는
  GL_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼로부터 pixel 값을 읽어올 것임!

  반면, glClear() 같은 함수는 '프레임버퍼 쓰기 관련 연산'에 해당하므로,
  GL_DRAW_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼 또는
  GL_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼를 초기화할 것임!


  반대로, 어떤 함수는 '읽기 전용 프레임버퍼'와 '쓰기 전용 프레임버퍼'를
  각각 필요로하는 케이스도 있는데, 그 대표적인 케이스가 바로 glBlitFramebuffer() 라고 보면 됨.

  glBlitFramebuffer() 함수를 호출하면,
  해당 함수를 호출한 시점에

  GL_READ_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼는
  Blitting 과정에서 데이터를 읽어올 source framebuffer 로 인식하며,

  GL_DRAW_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼는
  Blitting 과정에서 읽어온 데이터를 복사할 target framebuffer 로 인식함.
*/
