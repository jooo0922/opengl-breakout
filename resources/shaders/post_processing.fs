#version 330 core

// 버텍스 쉐이더에서 보간되어 전송된 입력 변수 선언
in vec2 TexCoords;

// 최종 색상 출력 변수 선언
out vec4 color;

uniform sampler2D scene;      // scene 요소가 렌더링된 텍스쳐 버퍼 (intermediate 프레임버퍼의 color attachment)
uniform vec2 offsets[9];      // 현재 uv 좌표를 중심으로 주변 uv 좌표 계산을 위해 각 방향마다 더해줄 offset 값
uniform int edge_kernel[9];   // chaos 효과 적용을 위한 3*3 edge kernel (= convolution matrix)
uniform float blur_kernel[9]; // shake 효과 적용을 위한 3*3 blur kernel (= convolution matrix)

uniform bool chaos;   // chaos 효과 활성화 상태
uniform bool confuse; // confuse 효과 활성화 상태
uniform bool shake;   // shake 효과 활성화 상태

void main() {
  // 최종 색상 출력 변수 초기화
  color = vec4(0.0);

  // 현재 텍셀과 그 주변 텍셀들을 샘플링하여 저장할 동적 배열 선언
  vec3 sample[9];

  // convolution matrix 사용하는 효과 활성화 시, 현재 텍셀과 그 주변 텍셀들을 샘플링하여 동적 배열에 복사해 둠
  if(chaos || shake) {
    for(int i = 0; i < 9; i++) {
      sample[i] = vec3(texture(scene, TexCoords.st + offsets[i]));
    }
  }

  if(chaos) {
    // chaos 효과 활성화 시 처리
    for(int i = 0; i < 9; i++) {
      // 샘플링한 텍셀들에 edge kernel 에 정의된 가중치를 적용하여 합산(convolution)
      color += vec4(sample[i] * edge_kernel[i], 0.0);
    }
    color.a = 1.0;
  } else if(confuse) {
    // confuse 효과 활성화 시 처리
    // 샘플링한 텍셀 색상 반전
    color = vec4(1.0 - texture(scene, TexCoords).rgb, 1.0);
  } else if(shake) {
    // shake 효과 활성화 시 처리
    for(int i = 0; i < 9; i++) {
      // 샘플링한 텍셀들에 blur kernel 에 정의된 가중치를 적용하여 합산(convolution)
      color += vec4(sample[i] * blur_kernel[i], 0.0);
    }
    color.a = 1.0;
  } else {
    // 모든 효과 비활성화 시 샘플링한 텍셀 그대로 출력
    color = texture(scene, TexCoords);
  }
}
