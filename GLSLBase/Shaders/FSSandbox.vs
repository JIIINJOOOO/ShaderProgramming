#version 450

in vec3 a_Position;

varying vec4 v_Color; // FS�� ����ϴ°� �ǹ�


void main()
{
	gl_Position = vec4(a_Position, 1); // openGL ������ ��°�

	//v_Color = vec4(a_Position.x+ 0.5, a_Position.y + 0.5, 0.0, 0.0); // +0.5 ���ذ� 0~1���� �� ����� ����-> �ؽ��� �����Ҷ� �ʿ�
	v_Color = vec4(a_Position.x, a_Position.y, 0.0, 0.0); // ���͸� 0,0����
}
