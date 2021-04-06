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

uniform float u_Time; // 누적시간

//const vec3 c_Gravity = vec3(0, -2.8, 0); // 중력 가속도
const vec3 c_Gravity = vec3(0, 0, 0); 


// uniform: 파이프라인 단위의 입력 -> glDrawArrays가 호출되고 버텍스가 다 처리될 때 까지의 단위
//uniform float u_Scale; 
// 특정한 좌표를 주면 그 점을 기준으로 삼각형이 그려지도록
//uniform vec3 u_Position; // 모든 vertex에 동일한 값이 적용될때 사용. 다 다른값이 적용되어야 할땐 attribute값으로 따로 줘야함. 

// 명시적으로 로케이션 주는 법 => 생략 가능
//layout (location = 0) in vec3 a_Position; 
//layout (location = 1) in vec3 a_Position1; // ppt에 있음 -> 여러개의 입력을 하나의 버텍스 형태로 받음
// => 이 버텍스가 가지는 정보: 위 float 3개 + 아래 float 3개 = 입력: 총 6개 floats
// 입력을 여러개 받는 방법: 로케이션을 다르게 줘서 여러개의 자료구조가 하나의 버텍스로 모아져서 입력

void main()
{
	//vec3 temp = a_Position; // 원래 입력되는 position -> in == const => 바꿀수 없다
	//temp = temp + u_Position;

	float newTime = u_Time - a_EmitTime; // 누적시간을 계산해서
	

	vec3 newPos = a_Position;
	

	// emit이 안 됐을 경우
	if (newTime < 0.0) // 누적시간이 음수면 화면밖으로 좌표를 날려서 생성이 안된것처럼 보이게(컬링)
	{
		newPos = vec3(10000,10000,10000);
	}
	else // emit time이 됐을 경우
	{
		newTime = mod(newTime, a_LifeTime); // lifetime마다 반복되도록 -> 음수가 되면 이상하게 될 가능성 있으므로 else에 넣어줌
		newPos = newPos + vec3(newTime, 0, 0); // x는 newTime에 따라 움직임
		newPos.y = newPos.y + (a_A * newTime) * sin(newTime * 3.14 * 2 * a_P); // 폭은 sin함수의 바깥,주기(a_P)는 sin함수의 인자로 곱해줘야함
		// a_A * newTime : 시간이 갈수록 폭이 점점 넓어지게 -> 퍼지는 효과
		// 시간에 따른 vertex의 position 계산 가능
		//float t = u_Time;
		//float tt = u_Time*u_Time;
		//newPos = newPos + t * a_Velocity + 0.5 * c_Gravity * tt;

		// 파티클 별로 누적시간 쓸거라 아래처럼 수정
		//float t = newTime;
		//float tt = newTime*newTime;
		//newPos = newPos + t * a_Velocity + 0.5 * c_Gravity * tt;
	}

	

	gl_Position = vec4(newPos, 1); // openGL 고유의 출력값
}
