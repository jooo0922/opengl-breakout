#version 330 core

// 버텍스 쉐이더에서 보간되어 전송된 입력 변수 선언
in vec2 TexCoords;
in vec4 ParticleColor;

// 최종 색상 출력 변수 선언
out vec4 color;

// particle quad 에 적용할 sprite 텍스쳐 sampler 변수 선언
uniform sampler2D sprite;

void main() {
  // 보간되어 전송된 정점 color 와 sprite 텍스쳐에서 샘플링한 texel 을 곱해 최종 색상 출력
  color = (texture(sprite, TexCoords) * ParticleColor);
}
