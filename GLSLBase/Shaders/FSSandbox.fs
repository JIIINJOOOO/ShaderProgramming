#version 450

layout(location=0) out vec4 FragColor; // 사용자가 정의한 출력값

varying vec4 v_Color; // VS랑 핀트 맞춰주기

const vec3 Circle = vec3(0.5, 0.5, 0.0); // 원점 -> 텍스쳐 좌표를 넘길때 0~1사이를 넘겼기 때문에
const float PI = 3.141592;

uniform vec3 u_Point;
uniform vec3 u_Points[10]; // 여러 좌표를 받아서 원 그리기
uniform float u_Time; // 시간은 모두 동일하기 때문에 유니폼

// 앞으로는 컬러자체 계산해주는걸 함수로 빼서 만들 것
vec4 CenteredCircle() // 동심원 함수
{
	float d = length(v_Color.rgb - Circle); // 이미 distance는 있다
	float count = 3; // 동심원의 개수
	float rad = d * 2.0 * 2.0 * PI * count; // d*2.0 = 0.5를 1로 바꾼 것
	float greyScale = sin(rad);
	float width = 200; // *비트쉬프트 연산만 하면 끝나므로 제곱의 수를 늘린다고 성능에 문제가 가진 않는다.
	greyScale = pow(greyScale, width); // 원의 두께를 줄이기 위함 -> sin함수의 한 주기에서 1보다 작은 값은 제곱할수록 작아짐.
	return vec4(greyScale);
}

vec4 IndicatePoint() // 내부가 채워진 원
{
	vec4 returnColor = vec4(0);
	float d = length(v_Color.rg/*x,y축이라 b값은 의미가 없다*/ - u_Point.xy); // u_Point로 텍스쳐좌표에 그려질 원의 센터를 받았음
	if(d<u_Point.z) // point.z = 두께
	{
		returnColor = vec4(1); // 내부를 채운다
	} // 아닐경우에는 returnColor 그대로 두면 되기에 else 생략
	return returnColor; 
}

vec4 IndicatePoints() // 내부가 채워진 원
{
	vec4 returnColor = vec4(0);
	for(int i=0;i<10;++i)
	{
		float d = length(v_Color.rg/*x,y축이라 b값은 의미가 없다*/ - u_Points[i].xy); // 각각의 포인트별 인덱스
		if(d<u_Points[i].z) // 각각의 포인트 별로 가지고 있는 원의 크기
		{
			returnColor = vec4(1); 
		} 
	}
	return returnColor; 
}

// 시험문제 - 특정 지점의 원을 점마다 가새로 갈수록 그라데이션 되도록
vec4 IndicateGradatedPoints() // 내부가 채워진 원
{
	vec4 returnColor = vec4(0);
	for(int i=0;i<10;++i)
	{
		float d = length(v_Color.rg/*x,y축이라 b값은 의미가 없다*/ - u_Points[i].xy); // 각각의 포인트별 인덱스
		if(d<u_Points[i].z) // 각각의 포인트 별로 가지고 있는 크기
		{
		// 방법 둘다 renderer.cpp에서 하나의 점 반지름 키워줘야 티가난다
		// 방법1.
			//float rad = d * 2.0 * 2.0 * PI;
			//float greyScale = sin(-rad+1);
			//returnColor = vec4(greyScale); 
		// 방법2.
			returnColor = vec4(1-10*d,1-10*d,1-10*d,1);
		} 
	}
	return returnColor; 
}

vec4 Radar()
{
	// 현재 버텍스에서 넘어오고 있는 센터값은 -0.5에서 0.5의 센터인 0,0
	float d = length(v_Color.rg - vec2(0,0)); //distance -> 가운데로 갈수록 0에 가깝고 겉으로 갈수록 점점 커진다
	vec4 returnColor = vec4(0);
	// 4. glClear를 지우고 alpha값을 주면
	// 원이 매프레임 지워지는게 아니고 잔상이 남는듯한 효과
	// 0.2 alpha값이 0,0,0(검정색)을 블렌딩해서 서서히 지워지는듯한 효과
	// 이걸 조절해서 얼마나 지워지는지 결정 가능
	returnColor.a = 0.4;
	float ringRadius = mod(u_Time,0.7/*최대 반지름 0.7정도 예상*/); // 0~0.7 사이 왔다갔다
	float radarWidth = 0.015; //레이더 원 두께

	if(d>ringRadius && d<ringRadius + radarWidth) // 1.간단하게 링 만들기 
	// 2. 움직이는 원 만들기 위해선 시간에 따라서 기준으로 하는 값이 커져야함 -> 어느 특정 지점이 되면 다시 0에서 시작하도록
	{
		returnColor = vec4(0.5);
		// 6. 레이더에 걸리는 포인터를 여러개
		for(int i=0;i<10;++i)
		{
			// 3. 레이더 원이 지나가는 곳에 특정 포인트가 있다면 해당 포인트만 하이라이트
			// 입력된 포인트로부터의 distance
			float pointDistance = length(u_Points[i].xy - v_Color.rg/*현재 내 프래그먼트의 좌표위치*/);
			if(pointDistance < 0.05)
			{
			// 5. 가생이로 갈수록 어두워지도록
				pointDistance = 0.05 - pointDistance; // 0~0.05가 아닌 0.05에서 0 ->  포인트에 가까워질수록 커짐
				pointDistance *= 20; // 0.05가 1이 되도록 -> 가운데가 0.05였다가 0으로 바뀔때 1에서 0으로 바뀌는 것으로 됨
				returnColor += vec4(pointDistance); // point가 겉으로 갈수록 점점 부드럽게 보임
			}
		}
		//returnColor = vec4(1);
		// 시험문제 - 레이더의 링이 그라데이션 되도록
		// 방법 1.
		float ringCenter = ringRadius + radarWidth/2;
		if(d<ringCenter)
		{
		 //d - ringCenter 값이 점점 커지지만 계속 음수
			d = -(d - ringCenter) * -20; 
		}
		else if (d>ringCenter)
		{
		 //ringCenter -d 값이 음수로 점점 작아짐
			d = -(ringCenter - d) * -20;
		}

		// 방법 2.
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
		vec2 ori = u_Points[i].xy; // 외부입력으로 받음
		vec2 pos = v_Color.rg; // 현재 프래그먼트에 넘어온 포지션
		float d = length(ori-pos); // 입력된 포인트를 기준으로 현재 내 프래그먼트의 거리를 구해서 d에 집어넣음
		float preq = 8; // 주기 
		returnColor += 0.5 * vec4(sin(d* 2 * PI * preq - u_Time)); // 0.5 * 2PI => 한주기
		// u_Time : 움직임. 원래 가지고 있는 dist에서 u_Time만큼 점점 멀어지는 것 처럼
	}
	//returnColor = normalize(returnColor); // 1이 넘어가면 잘리는 느낌 -> 노멀라이즈로 1로 바꿔줌
	return returnColor;
}

void main()
{
	/*vec4 color = vec4(0);

	// length() : 벡터의 크기를 리턴해주는 함수
	float tempLength = length(v_Color.rgb - Circle); // 프래그먼트의 좌표는 v_Color로 넘어옴
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
