#version 450

layout(location=0) out vec4 FragColor; // 사용자가 정의한 출력값


//uniform vec4 u_Color;

in vec4 v_Color; // VS의 출력값과 보간된 값이 자동으로 들어온다

void main()
{
	//FragColor = vec4(1,1,1,1);
	FragColor = v_Color;
}
