#version 450

// ����ڰ� ������ �Է� ��
// location ���� => ���������� �� �����̼� �˾Ƽ� �Ҵ��
// in: VS ��ü�� �Է�
//in vec3 a_Position1;
in vec3 a_Position;  // float position 3��
in vec3 a_Velocity;	 // float velocity 3��
in float a_EmitTime; // float emittime 1��

uniform float u_Time; // �����ð�

const vec3 c_Gravity = vec3(0, -2.8, 0); // �߷� ���ӵ�

// uniform: ���������� ������ �Է� -> glDrawArrays�� ȣ��ǰ� ���ؽ��� �� ó���� �� ������ ����
//uniform float u_Scale; 
// Ư���� ��ǥ�� �ָ� �� ���� �������� �ﰢ���� �׷�������
//uniform vec3 u_Position; // ��� vertex�� ������ ���� ����ɶ� ���. �� �ٸ����� ����Ǿ�� �Ҷ� attribute������ ���� �����. 

// ��������� �����̼� �ִ� �� => ���� ����
//layout (location = 0) in vec3 a_Position; 
//layout (location = 1) in vec3 a_Position1; // ppt�� ���� -> �������� �Է��� �ϳ��� ���ؽ� ���·� ����
// => �� ���ؽ��� ������ ����: �� float 3�� + �Ʒ� float 3�� = �Է�: �� 6�� floats
// �Է��� ������ �޴� ���: �����̼��� �ٸ��� �༭ �������� �ڷᱸ���� �ϳ��� ���ؽ��� ������� �Է�

void main()
{
	//vec3 temp = a_Position; // ���� �ԷµǴ� position -> in == const => �ٲܼ� ����
	//temp = temp + u_Position;

	float newTime = u_Time - a_EmitTime; // �����ð��� ����ؼ�

	vec3 newPos = a_Position;

	// emit�� �� ���� ���
	if (newTime < 0.0) // �����ð��� ������ ȭ������� ��ǥ�� ������ ������ �ȵȰ�ó�� ���̰�(�ø�)
	{
		newPos = vec3(10000,10000,10000);
	}
	else // emit time�� ���� ���
	{
		// �ð��� ���� vertex�� position ��� ����
		//float t = u_Time;
		//float tt = u_Time*u_Time;
		//newPos = newPos + t * a_Velocity + 0.5 * c_Gravity * tt;

		// ��ƼŬ ���� �����ð� ���Ŷ� �Ʒ�ó�� ����
		float t = newTime;
		float tt = newTime*newTime;
		newPos = newPos + t * a_Velocity + 0.5 * c_Gravity * tt;
	}

	

	gl_Position = vec4(newPos, 1); // openGL ������ ��°�
}
