#version 330 core

// <vec2 pos, vec2 tex> 정점 데이터가 하나로 묶여서 전송되는 attribute 변수
layout(location = 0) in vec4 vertex;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform vec2 offset;
uniform vec4 color;

void main() {
  // 각 particle quad 의 scale 을 10배 늘림
  float scale = 10.0;

  // uv 및 color 데이터를 프래그먼트 쉐이더로 출력하여 보간
  TexCoords = vertex.zw;
  ParticleColor = color;

  // scale 및 offset 값으로 particle quad 정점 변환 -> 모델 행렬로 처리하는 변환을 대체
  gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);
}
