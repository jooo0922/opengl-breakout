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
