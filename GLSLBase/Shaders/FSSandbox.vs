#version 450

in vec3 a_Position;

varying vec4 v_Color; // FS로 출력하는걸 의미


void main()
{
	gl_Position = vec4(a_Position, 1); // openGL 고유의 출력값
	v_Color = vec4(a_Position.x+ 0.5, a_Position.y + 0.5, 0.0, 0.0); // +0.5 해준건 0~1사이 값 만들기 위함
}
