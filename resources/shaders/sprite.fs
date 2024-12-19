#version 330 core

in vec2 TexCoords;
out vec4 color; // 최종 색상 출력 변수 선언

uniform sampler2D image; // sprite 텍스쳐 이미지
uniform vec3 spriteColor;

void main() {
  // uniform 변수로 전달된 색상값과 sprite 텍스쳐 이미지 색상을 곱해 최종 색상 출력
  color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
}
