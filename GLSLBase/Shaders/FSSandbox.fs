#version 450

layout(location=0) out vec4 FragColor; // ����ڰ� ������ ��°�

varying vec4 v_Color; // VS�� ��Ʈ �����ֱ�

const vec3 Circle = vec3(0.5, 0.5, 0.0); // ����

void main()
{
	vec4 color = vec4(0);

	// length() : ������ ũ�⸦ �������ִ� �Լ�
	float tempLength = length(v_Color.rgb - Circle); // �����׸�Ʈ�� ��ǥ�� v_Color�� �Ѿ��
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
