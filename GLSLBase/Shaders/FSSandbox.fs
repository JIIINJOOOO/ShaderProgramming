#version 450

layout(location=0) out vec4 FragColor; // ����ڰ� ������ ��°�

varying vec4 v_Color; // VS�� ��Ʈ �����ֱ�

const vec3 Circle = vec3(0.5, 0.5, 0.0); // ���� -> �ؽ��� ��ǥ�� �ѱ涧 0~1���̸� �Ѱ�� ������
const float PI = 3.141592;

uniform vec3 u_Point;
uniform vec3 u_Points[10]; // ���� ��ǥ�� �޾Ƽ� �� �׸���
uniform float u_Time; // �ð��� ��� �����ϱ� ������ ������

// �����δ� �÷���ü ������ִ°� �Լ��� ���� ���� ��
vec4 CenteredCircle() // ���ɿ� �Լ�
{
	float d = length(v_Color.rgb - Circle); // �̹� distance�� �ִ�
	float count = 3; // ���ɿ��� ����
	float rad = d * 2.0 * 2.0 * PI * count; // d*2.0 = 0.5�� 1�� �ٲ� ��
	float greyScale = sin(rad);
	float width = 200; // *��Ʈ����Ʈ ���길 �ϸ� �����Ƿ� ������ ���� �ø��ٰ� ���ɿ� ������ ���� �ʴ´�.
	greyScale = pow(greyScale, width); // ���� �β��� ���̱� ���� -> sin�Լ��� �� �ֱ⿡�� 1���� ���� ���� �����Ҽ��� �۾���.
	return vec4(greyScale);
}

vec4 IndicatePoint() // ���ΰ� ä���� ��
{
	vec4 returnColor = vec4(0);
	float d = length(v_Color.rg/*x,y���̶� b���� �ǹ̰� ����*/ - u_Point.xy); // u_Point�� �ؽ�����ǥ�� �׷��� ���� ���͸� �޾���
	if(d<u_Point.z) // point.z = �β�
	{
		returnColor = vec4(1); // ���θ� ä���
	} // �ƴҰ�쿡�� returnColor �״�� �θ� �Ǳ⿡ else ����
	return returnColor; 
}

vec4 IndicatePoints() // ���ΰ� ä���� ��
{
	vec4 returnColor = vec4(0);
	for(int i=0;i<10;++i)
	{
		float d = length(v_Color.rg/*x,y���̶� b���� �ǹ̰� ����*/ - u_Points[i].xy); // ������ ����Ʈ�� �ε���
		if(d<u_Points[i].z) // ������ ����Ʈ ���� ������ �ִ� ���� ũ��
		{
			returnColor = vec4(1); 
		} 
	}
	return returnColor; 
}

// ���蹮�� - Ư�� ������ ���� ������ ������ ������ �׶��̼� �ǵ���
vec4 IndicateGradatedPoints() // ���ΰ� ä���� ��
{
	vec4 returnColor = vec4(0);
	for(int i=0;i<10;++i)
	{
		float d = length(v_Color.rg/*x,y���̶� b���� �ǹ̰� ����*/ - u_Points[i].xy); // ������ ����Ʈ�� �ε���
		if(d<u_Points[i].z) // ������ ����Ʈ ���� ������ �ִ� ũ��
		{
		// ��� �Ѵ� renderer.cpp���� �ϳ��� �� ������ Ű����� Ƽ������
		// ���1.
			//float rad = d * 2.0 * 2.0 * PI;
			//float greyScale = sin(-rad+1);
			//returnColor = vec4(greyScale); 
		// ���2.
			returnColor = vec4(1-10*d,1-10*d,1-10*d,1);
		} 
	}
	return returnColor; 
}

vec4 Radar()
{
	// ���� ���ؽ����� �Ѿ���� �ִ� ���Ͱ��� -0.5���� 0.5�� ������ 0,0
	float d = length(v_Color.rg - vec2(0,0)); //distance -> ����� ������ 0�� ������ ������ ������ ���� Ŀ����
	vec4 returnColor = vec4(0);
	// 4. glClear�� ����� alpha���� �ָ�
	// ���� �������� �������°� �ƴϰ� �ܻ��� ���µ��� ȿ��
	// 0.2 alpha���� 0,0,0(������)�� �����ؼ� ������ �������µ��� ȿ��
	// �̰� �����ؼ� �󸶳� ���������� ���� ����
	returnColor.a = 0.4;
	float ringRadius = mod(u_Time,0.7/*�ִ� ������ 0.7���� ����*/); // 0~0.7 ���� �Դٰ���
	float radarWidth = 0.015; //���̴� �� �β�

	if(d>ringRadius && d<ringRadius + radarWidth) // 1.�����ϰ� �� ����� 
	// 2. �����̴� �� ����� ���ؼ� �ð��� ���� �������� �ϴ� ���� Ŀ������ -> ��� Ư�� ������ �Ǹ� �ٽ� 0���� �����ϵ���
	{
		returnColor = vec4(0.5);
		// 6. ���̴��� �ɸ��� �����͸� ������
		for(int i=0;i<10;++i)
		{
			// 3. ���̴� ���� �������� ���� Ư�� ����Ʈ�� �ִٸ� �ش� ����Ʈ�� ���̶���Ʈ
			// �Էµ� ����Ʈ�κ����� distance
			float pointDistance = length(u_Points[i].xy - v_Color.rg/*���� �� �����׸�Ʈ�� ��ǥ��ġ*/);
			if(pointDistance < 0.05)
			{
			// 5. �����̷� ������ ��ο�������
				pointDistance = 0.05 - pointDistance; // 0~0.05�� �ƴ� 0.05���� 0 ->  ����Ʈ�� ����������� Ŀ��
				pointDistance *= 20; // 0.05�� 1�� �ǵ��� -> ����� 0.05���ٰ� 0���� �ٲ� 1���� 0���� �ٲ�� ������ ��
				returnColor += vec4(pointDistance); // point�� ������ ������ ���� �ε巴�� ����
			}
		}
		//returnColor = vec4(1);
		// ���蹮�� - ���̴��� ���� �׶��̼� �ǵ���
		// ��� 1.
		float ringCenter = ringRadius + radarWidth/2;
		if(d<ringCenter)
		{
		 //d - ringCenter ���� ���� Ŀ������ ��� ����
			d = -(d - ringCenter) * -20; 
		}
		else if (d>ringCenter)
		{
		 //ringCenter -d ���� ������ ���� �۾���
			d = -(ringCenter - d) * -20;
		}

		// ��� 2.
		//float lineDistance = d - ringRadius - (radarWidth/2);
		//float lineDistanceDouble = lineDistance * lineDistance;
		//d = lineDistanceDouble * -1000;
		//returnColor += vec4(d);
	}
	return returnColor;
}

vec4 wave()
{
	vec4 returnColor = vec4(0);

	for(int i=0;i<10;++i)
	{
		vec2 ori = u_Points[i].xy; // �ܺ��Է����� ����
		vec2 pos = v_Color.rg; // ���� �����׸�Ʈ�� �Ѿ�� ������
		float d = length(ori-pos); // �Էµ� ����Ʈ�� �������� ���� �� �����׸�Ʈ�� �Ÿ��� ���ؼ� d�� �������
		float preq = 8; // �ֱ� 
		returnColor += 0.5 * vec4(sin(d* 2 * PI * preq - u_Time)); // 0.5 * 2PI => ���ֱ�
		// u_Time : ������. ���� ������ �ִ� dist���� u_Time��ŭ ���� �־����� �� ó��
	}
	//returnColor = normalize(returnColor); // 1�� �Ѿ�� �߸��� ���� -> ��ֶ������ 1�� �ٲ���
	return returnColor;
}

void main()
{
	/*vec4 color = vec4(0);

	// length() : ������ ũ�⸦ �������ִ� �Լ�
	float tempLength = length(v_Color.rgb - Circle); // �����׸�Ʈ�� ��ǥ�� v_Color�� �Ѿ��
	if ( tempLength > 0.49 && tempLength < 0.5 )
	{
		color = vec4(1,1,1,1);
	}
	else
	{
		color = vec4(0,0,0,0);
	}*/
	//FragColor = CenteredCircle();
	//FragColor = IndicatePoint();
	//FragColor = IndicateGradatedPoints();
	//FragColor = Radar();
	FragColor = wave();

}
