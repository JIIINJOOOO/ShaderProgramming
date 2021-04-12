#version 450

// ����ڰ� ������ �Է� ��
// location ���� => ���������� �� �����̼� �˾Ƽ� �Ҵ��
// in: VS ��ü�� �Է�
//in vec3 a_Position1;
in vec3 a_Position;  // float position 3��
in vec3 a_Velocity;	 // float velocity 3��
in float a_EmitTime; // float emittime 1��
in float a_LifeTime; // float lifetime 1��
in float a_P;		 // float period 1��
in float a_A;		 // float amp 1��
in float a_RandValue;	
in vec4 a_Color; 

uniform float u_Time; // �����ð�
uniform vec3 u_ExForce; // �ܷ�

const vec3 c_Gravity = vec3(0, -0.8, 0);  // �߷� ���ӵ�
const mat3 c_NV = mat3(0, -1, 0, 1, 0, 0, 0, 0, 0); // �� �������� ������ ��

// uniform: ���������� ������ �Է� -> glDrawArrays�� ȣ��ǰ� ���ؽ��� �� ó���� �� ������ ����
//uniform float u_Scale; 
// Ư���� ��ǥ�� �ָ� �� ���� �������� �ﰢ���� �׷�������
//uniform vec3 u_Position; // ��� vertex�� ������ ���� ����ɶ� ���. �� �ٸ����� ����Ǿ�� �Ҷ� attribute������ ���� �����. 

// ��������� �����̼� �ִ� �� => ���� ����
//layout (location = 0) in vec3 a_Position; 
//layout (location = 1) in vec3 a_Position1; // ppt�� ���� -> �������� �Է��� �ϳ��� ���ؽ� ���·� ����
// => �� ���ؽ��� ������ ����: �� float 3�� + �Ʒ� float 3�� = �Է�: �� 6�� floats
// �Է��� ������ �޴� ���: �����̼��� �ٸ��� �༭ �������� �ڷᱸ���� �ϳ��� ���ؽ��� ������� �Է�


out vec4 v_Color; // VS�� ���

void main()
{
	//vec3 temp = a_Position; // ���� �ԷµǴ� position -> in == const => �ٲܼ� ����
	//temp = temp + u_Position;

	float newTime = u_Time - a_EmitTime; // �����ð��� ����ؼ�
	

	//vec3 newPos = a_Position;

	// �Ű����� ���������� ������ġ ������: ���� ������
	// ���� ��ġ�� �츮�� ���Ƿ� ���س��� randvalue�� ��������
	vec3 newPos;
	newPos.x = a_Position.x + cos(a_RandValue * 2 * 3.14); // randvalue= 0~1���� �� -> ���� 360��(2PI)���� ������
	newPos.y = a_Position.y + sin(a_RandValue * 2 * 3.14);
	

	newPos.x = a_Position.x + (16*pow(sin(a_RandValue * 2 * 3.14),3)) * 0.03; // randvalue= 0~1���� �� -> ���� 360��(2PI)���� ������
	newPos.y = a_Position.y + (13*cos(a_RandValue * 2 * 3.14) - 5*cos(2*a_RandValue* 2 * 3.14) 
				- 2*cos(3*a_RandValue* 2 * 3.14) 
				- cos(4*a_RandValue* 2 * 3.14))*0.03;

	vec4 color = vec4(0);
	

	// emit�� �� ���� ���
	if (newTime < 0.0) // �����ð��� ������ ȭ������� ��ǥ�� ������ ������ �ȵȰ�ó�� ���̰�(�ø�)
	{
		newPos = vec3(10000,10000,10000);
	}
	else // emit time�� ���� ���
	{
		/*newTime = mod(newTime, a_LifeTime); // lifetime���� �ݺ��ǵ��� -> ������ �Ǹ� �̻��ϰ� �� ���ɼ� �����Ƿ� else�� �־���
		newPos = newPos + vec3(newTime, 0, 0); // x�� newTime�� ���� ������
		newPos.y = newPos.y + (a_A * newTime) * sin(newTime * 3.14 * 2 * a_P); // ���� sin�Լ��� �ٱ�,�ֱ�(a_P)�� sin�Լ��� ���ڷ� ���������*/
		// a_A * newTime : �ð��� ������ ���� ���� �о����� -> ������ ȿ��
		
		newTime = mod(newTime, a_LifeTime);
		// �ð��� ���� vertex�� position ��� ����
		float t = newTime;
		float tt = newTime*newTime;
		vec3 newAcc = c_Gravity + u_ExForce;
		vec3 curVel = a_Velocity + t * newAcc/*���ӵ��� ��*/; // �ش� �ð����� �ӵ� ���ϱ�
		vec3 normalV = normalize(curVel * c_NV); //���� �ӵ������� ���� ���� -> 2�����̱� ������ ȭ���� ���� �ִ� ��� �������� ���� �븻����(3�����̸� ������ �븻������ �ϳ�)
		newPos = newPos + t * a_Velocity + 0.5 * newAcc * tt;
		newPos = newPos + normalV * a_A * sin(newTime*2*3.14*a_P); // ���� �������� ��鸮�鼭 ���ư�

		float intensity = 1.0 - t/a_LifeTime; // lifeTime�� Ŀ������ 0�� ��������� intensity��
		// ��¦�Ÿ��鼭 ������ ������� �ϴ� ���Ǽ� �ڵ�
		/*float flickering = cos(t/40000);
		if(flickering == 1 || flickering == 0)
		{
			color = a_Color * flickering;
		}
		
		color *= intensity;*/
		color = a_Color * intensity; // ó���� 1�� �������ٰ� ���� 0�� ����� ���� ������
		

		// ��ƼŬ ���� �����ð� ���Ŷ� �Ʒ�ó�� ����
		//float t = newTime;
		//float tt = newTime*newTime;
		//newPos = newPos + t * a_Velocity + 0.5 * c_Gravity * tt;
	}

	

	gl_Position = vec4(newPos, 1); // openGL ������ ��°�
	v_Color = color; // VS�� ��� -> ������ �� -> FS�� �Է����� ��
}
