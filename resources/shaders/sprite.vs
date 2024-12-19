#version 330 core

// <vec2 pos, vec2 tex> 정점 데이터가 하나로 묶여서 전송되는 attribute 변수
layout(location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main() {
  // uv 좌표값을 프래그먼트 쉐이더로 출력하여 보간
  TexCoords = vertex.zw;

  // Breakout 은 single-scene 게임으로, 카메라 이동이 없음. -> view 행렬 변환 과정 생략 가능!
  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
