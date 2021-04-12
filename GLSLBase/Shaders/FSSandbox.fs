#version 450

layout(location=0) out vec4 FragColor; // 사용자가 정의한 출력값

varying vec4 v_Color; // VS랑 핀트 맞춰주기

const vec3 Circle = vec3(0.5, 0.5, 0.0); // 원점

void main()
{
	vec4 color = vec4(0);

	// length() : 벡터의 크기를 리턴해주는 함수
	float tempLength = length(v_Color.rgb - Circle); // 프래그먼트의 좌표는 v_Color로 넘어옴
	if ( tempLength > 0.49 && tempLength < 0.5 )
	{
		color = vec4(1,1,1,1);
	}
	else
	{
		color = vec4(0,0,0,0);
	}
	FragColor = color;
}
