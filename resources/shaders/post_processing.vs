#version 330 core

// <vec2 pos, vec2 tex> 정점 데이터가 하나로 묶여서 전송되는 attribute 변수
layout(location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform bool chaos;   // chaos 효과 활성화 상태
uniform bool confuse; // confuse 효과 활성화 상태
uniform bool shake;   // shake 효과 활성화 상태
uniform float time;   // glfwGetTime() 함수가 반환하는 경과시간(elapsed time)

void main() {
  // screen-size 2D Quad 의 clip space 좌표값을 동차좌표계로 맞춰서 그대로 정점 출력 변수에 할당
  gl_Position = vec4(vertex.xy, 0.0, 1.0);

  // uv 좌표값을 로컬 변수에 swizzling 하여 복사
  vec2 texture = vertex.zw;

  if(chaos) {
    /*
      chaos 효과 활성화 시,
      기존 uv 좌표값에 경과시간에 따라 [-0.3, 0.3] 범위의 값을 offset 으로 변위시킴.

      -> sin(), cos() 함수로 offset 값을 계산하는 것으로 보아,
      텍스쳐 샘플링에 사용할 uv 좌표값에 회전 효과를 적용하려는 것이겠지!
      즉, scene 요소가 렌더링된 텍스쳐를 회전시켜서 scene 전체가 회전하는 효과를 구현.
    */
    float strength = 0.3;
    vec2 pos = vec2(texture.x + sin(time) * strength, texture.y + cos(time) * strength);
    TexCoords = pos;
  } else if(confuse) {
    /*
      confuse 효과 활성화 시,
      기존 uv 좌표값을 반전시켜서 샘플링함.

      -> scene 전체가 수평/수직 방향으로 뒤집어질 것임.
      (+ 분기문에 의해 chaos 와 confuse 효과는 동시 적용 불가 -> 두 상태값 동시 활성화 시, chaos 효과 우선 적용)
    */
    TexCoords = vec2(1.0 - texture.x, 1.0 - texture.y);
  } else {
    /*
      나머지 효과(shake) 활성화 또는 모든 효과 비활성화 시,
      uv 좌표값을 변경하지 않고 그대로 다음 파이프라인으로 출력
    */
    TexCoords = texture;
  }

  if(shake) {
    /*
      shake 효과 활성화 시,
      screen size 2D Quad 의 정점 좌표값을 일정 주기 동안 [-0.01, 0.01] 사이의 값만큼 이동시킴

      -> 경과시간(time)에 따라 2D Quad 가 상하좌우로 약간씩 왔다갔다 하므로,
      scene 전체가 흔들리는 듯한 효과를 줌.

      strength 값이 클수록 더 많이 흔들리고,
      경과시간(time)에 곱해주는 값이 클수록 cos() 함수 주기가 짧아져서
      더 빨리 흔들리게 됨.
    */
    float strength = 0.01;
    gl_Position.x += cos(time * 10) * strength;
    gl_Position.y += cos(time * 15) * strength;
  }
}
