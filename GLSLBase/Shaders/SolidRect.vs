#version 450

// 사용자가 정의한 입력 값
// location 생략 => 내부적으로 빈 로케이션 알아서 할당됨
// in: VS 자체의 입력
//in vec3 a_Position1;
in vec3 a_Position;  // float position 3개
in vec3 a_Velocity;	 // float velocity 3개
in float a_EmitTime; // float emittime 1개
in float a_LifeTime; // float lifetime 1개
in float a_P;		 // float period 1개
in float a_A;		 // float amp 1개
in float a_RandValue;	
in vec4 a_Color; 

uniform float u_Time; // 누적시간
uniform vec3 u_ExForce; // 외력

const vec3 c_Gravity = vec3(0, -0.8, 0);  // 중력 가속도
const mat3 c_NV = mat3(0, -1, 0, 1, 0, 0, 0, 0, 0); // 열 내려가는 순서로 씀

// uniform: 파이프라인 단위의 입력 -> glDrawArrays가 호출되고 버텍스가 다 처리될 때 까지의 단위
//uniform float u_Scale; 
// 특정한 좌표를 주면 그 점을 기준으로 삼각형이 그려지도록
//uniform vec3 u_Position; // 모든 vertex에 동일한 값이 적용될때 사용. 다 다른값이 적용되어야 할땐 attribute값으로 따로 줘야함. 

// 명시적으로 로케이션 주는 법 => 생략 가능
//layout (location = 0) in vec3 a_Position; 
//layout (location = 1) in vec3 a_Position1; // ppt에 있음 -> 여러개의 입력을 하나의 버텍스 형태로 받음
// => 이 버텍스가 가지는 정보: 위 float 3개 + 아래 float 3개 = 입력: 총 6개 floats
// 입력을 여러개 받는 방법: 로케이션을 다르게 줘서 여러개의 자료구조가 하나의 버텍스로 모아져서 입력


out vec4 v_Color; // VS의 출력

void main()
{
	//vec3 temp = a_Position; // 원래 입력되는 position -> in == const => 바꿀수 없다
	//temp = temp + u_Position;

	float newTime = u_Time - a_EmitTime; // 누적시간을 계산해서
	

	//vec3 newPos = a_Position;

	// 매개변수 방정식으로 시작위치 정해줌: 원의 방정식
	// 시작 위치가 우리가 임의로 정해놓은 randvalue로 정해진다
	vec3 newPos;
	newPos.x = a_Position.x + cos(a_RandValue * 2 * 3.14); // randvalue= 0~1사이 값 -> 원은 360도(2PI)까지 가능함
	newPos.y = a_Position.y + sin(a_RandValue * 2 * 3.14);
	

	newPos.x = a_Position.x + (16*pow(sin(a_RandValue * 2 * 3.14),3)) * 0.03; // randvalue= 0~1사이 값 -> 원은 360도(2PI)까지 가능함
	newPos.y = a_Position.y + (13*cos(a_RandValue * 2 * 3.14) - 5*cos(2*a_RandValue* 2 * 3.14) 
				- 2*cos(3*a_RandValue* 2 * 3.14) 
				- cos(4*a_RandValue* 2 * 3.14))*0.03;

	vec4 color = vec4(0);
	

	// emit이 안 됐을 경우
	if (newTime < 0.0) // 누적시간이 음수면 화면밖으로 좌표를 날려서 생성이 안된것처럼 보이게(컬링)
	{
		newPos = vec3(10000,10000,10000);
	}
	else // emit time이 됐을 경우
	{
		/*newTime = mod(newTime, a_LifeTime); // lifetime마다 반복되도록 -> 음수가 되면 이상하게 될 가능성 있으므로 else에 넣어줌
		newPos = newPos + vec3(newTime, 0, 0); // x는 newTime에 따라 움직임
		newPos.y = newPos.y + (a_A * newTime) * sin(newTime * 3.14 * 2 * a_P); // 폭은 sin함수의 바깥,주기(a_P)는 sin함수의 인자로 곱해줘야함*/
		// a_A * newTime : 시간이 갈수록 폭이 점점 넓어지게 -> 퍼지는 효과
		
		newTime = mod(newTime, a_LifeTime);
		// 시간에 따른 vertex의 position 계산 가능
		float t = newTime;
		float tt = newTime*newTime;
		vec3 newAcc = c_Gravity + u_ExForce;
		vec3 curVel = a_Velocity + t * newAcc/*가속도의 합*/; // 해당 시간대의 속도 구하기
		vec3 normalV = normalize(curVel * c_NV); //현재 속도벡터의 수직 방향 -> 2차원이기 때문에 화면이 갖고 있는 평면 기준으로 만든 노말벡터(3차원이면 수많은 노말벡터중 하나)
		newPos = newPos + t * a_Velocity + 0.5 * newAcc * tt;
		newPos = newPos + normalV * a_A * sin(newTime*2*3.14*a_P); // 수직 방향으로 흔들리면서 날아감

		float intensity = 1.0 - t/a_LifeTime; // lifeTime이 커질수록 0에 가까워지는 intensity값
		// 반짝거리면서 서서히 사라지게 하는 뇌피셜 코드
		/*float flickering = cos(t/40000);
		if(flickering == 1 || flickering == 0)
		{
			color = a_Color * flickering;
		}
		
		color *= intensity;*/
		color = a_Color * intensity; // 처음엔 1이 곱해졌다가 점점 0에 가까운 값이 곱해짐
		

		// 파티클 별로 누적시간 쓸거라 아래처럼 수정
		//float t = newTime;
		//float tt = newTime*newTime;
		//newPos = newPos + t * a_Velocity + 0.5 * c_Gravity * tt;
	}

	

	gl_Position = vec4(newPos, 1); // openGL 고유의 출력값
	v_Color = color; // VS의 출력 -> 보간된 후 -> FS의 입력으로 들어감
}
