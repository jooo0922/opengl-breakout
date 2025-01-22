#ifndef POST_PROCESSOR_HPP
#define POST_PROCESSOR_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../utils/texture.hpp"
#include "../utils/shader.hpp"
#include "../renderer/sprite_renderer.hpp"

/**
 * PostProcessor 클래스
 *
 * 현재 게임 scene 요소들이 렌더링된 텍스쳐 버퍼를 샘플링하여 post processing 처리하는 클래스
 *
 * -> PostProcessor::BeginRender() 와 PostProcessor::EndRender() 호출 사이에 렌더링된 scene 요소들을
 * PostProcessor::Render() 호출 시 post processing 하여 2D Quad 에 렌더링한다.
 */
class PostProcessor
{
public:
  Shader PostProcessingShader; // post processing 적용 쉐이더 객체
  Texture2D Texture;           // intermediate 프레임버퍼의 color attachment 로 사용할 텍스쳐 (blit 으로 복사된 렌더링 결과 저장 목적)
  unsigned int Width, Height;  // 현재 window 크기
  bool Confuse, Chaos, Shake;  // 각 post processing effect 활성화 상태

  PostProcessor(Shader shader, unsigned int width, unsigned int height);

  // scene 요소 렌더링 직전 호출 -> scene 요소를 렌더링할 multisampled 프레임버퍼 바인딩
  void BeginRender();
  // scene 요소 렌더링 직후 호출 -> multisampled 프레임버퍼에 렌더링된 결과를 intermediate 프레임버퍼에 blit 으로 복사
  void EndRender();
  // intermediate 프레임버퍼에 복사된 렌더링 결과가 저장된 텍스쳐 버퍼를 샘플링하여 post processing 적용 후 2D Quad 렌더링
  void Render(float time);

private:
  unsigned int MSFBO, FBO; // multisampled 프레임버퍼, intermediate 프레임버퍼 id (MSFBO -> FBO 로 blit 하여 렌더링 결과 복사)
  unsigned int RBO;        // multisampled 프레임버퍼의 color attachment 로 사용할 renderbuffer (하단 필기 참고)
  unsigned int VAO;        // 2D Quad 정점 데이터가 기록된 버퍼 객체들이 바인딩된 VAO 객체 id

  // 2D Quad 정점 데이터 저장 및 VBO, VAO 객체 설정
  void initRenderData();
};

/**
 * multisampled 프레임버퍼의 color attachment 로 renderbuffer 를 사용하는 이유
 *
 *
 * 원래 renderbuffer 는 주로 depth buffer, stencil buffer 로 사용되는 버퍼 객체임.
 * (https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Framebuffers_Exercise_1/framebuffers_exercise_1.cpp 참고)
 *
 * renderbuffer 를 사용하기 좋은 케이스는
 * '데이터 쓰기(write), 복사(blit)가 빈번하지만 샘플링(read)이 불필요한 경우' 였었지?
 * (https://learnopengl.com/Advanced-OpenGL/Framebuffers 참고)
 *
 * 그런데 어째서 이 예제에서는 renderbuffer 를
 * multisampled 프레임버퍼의 color attachment 로 사용하려는 걸까?
 *
 * 오히려 multisampled 프레임버퍼의 color attachment 는
 * scene 요소들을 직접 렌더링(write)하고, 그 결과를 intermediate 프레임버퍼에 매 프레임마다 빠르게 복사(blit)해야 하고,
 * 쉐이더 내에서 직접 샘플링(read)하지 않으므로, renderbuffer 를 사용하기 아주 좋음.
 *
 * multisampled 프레임버퍼의 역할을 생각해보면 오히려 텍스쳐 버퍼보다
 * renderbuffer 를 사용하는 게 맞음!
 */

#endif /* POST_PROCESSOR_HPP */
