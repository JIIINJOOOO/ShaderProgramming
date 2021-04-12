#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"
#include <Windows.h>
#include <cstdlib>
#include <cassert>

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	//default settings
	glClearDepth(1.f);

	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders -> 여기에 쉐이더프로그램 ID 들어감
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_FSSandboxShader = CompileShaders("./Shaders/FSSandbox.vs", "./Shaders/FSSandbox.fs");
	//Create VBOs
	CreateVertexBufferObjects();

	//Initialize camera settings
	m_v3Camera_Position = glm::vec3(0.f, 0.f, 1000.f);
	m_v3Camera_Lookat = glm::vec3(0.f, 0.f, 0.f);
	m_v3Camera_Up = glm::vec3(0.f, 1.f, 0.f);
	m_m4View = glm::lookAt(
		m_v3Camera_Position,
		m_v3Camera_Lookat,
		m_v3Camera_Up
	);

	//Initialize projection matrix
	m_m4OrthoProj = glm::ortho(
		-(float)windowSizeX / 2.f, (float)windowSizeX / 2.f,
		-(float)windowSizeY / 2.f, (float)windowSizeY / 2.f,
		0.0001f, 10000.f);
	m_m4PersProj = glm::perspectiveRH(45.f, 1.f, 1.f, 1000.f);

	//Initialize projection-view matrix
	m_m4ProjView = m_m4OrthoProj * m_m4View; //use ortho at this time
	//m_m4ProjView = m_m4PersProj * m_m4View;

	//Initialize model transform matrix :; used for rotating quad normal to parallel to camera direction
	m_m4Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));

	// Create test data - lecture 2
	float tempVertices[] = { 0.f,0.f,0.f,1.f,0.f,0.f,1.f,1.f,0.f }; // 9개
	glGenBuffers(1, &m_VBO); // 2라는 ID -> 앞으로는 2로 접근함
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices), tempVertices, GL_STATIC_DRAW); // GPU 메모리에 VBO ID에 해당하는 데이터가 GPU메모리에 할당됨


	float tempVertices1[] = { 0.f,0.f,0.f,-1.f,0.f,0.f,-1.f,1.f,0.f }; // 9개
	glGenBuffers(1, &m_VBO1); // 2라는 ID -> 앞으로는 2로 접근함
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices1), tempVertices1, GL_STATIC_DRAW); // GPU 메모리에 VBO ID에 해당하는 데이터가 GPU메모리에 할당됨

	// box 
	float sizeRect = 0.5f;
	float tempVertices2[] = { // 시계방향으로 넣어준다
		-sizeRect,-sizeRect,0.f, //1 왼쪽 아래
		-sizeRect,sizeRect,0.f, //2 왼쪽 위
		sizeRect,sizeRect,0.f, //3 오른쪽 위
		-sizeRect,-sizeRect,0.f, // 1 왼쪽 아래
		sizeRect,sizeRect,0.f, // 2 오른쪽 위
		sizeRect,-sizeRect,0.f // 3 오른쪽 아래
	}; // 사각형 그릴것이기 때문에 vertex 6개 필요
	glGenBuffers(1, &m_VBOFSSandBox); // 2라는 ID -> 앞으로는 2로 접근함
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFSSandBox);
	// tempVertices2는 동적할당한 것이 아니기 때문에 sizeof 해도 사이즈 나오지만
	// 동적할당 했을 경우에는 포인터 크기가 나오기 때문에 주의
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices2), tempVertices2, GL_STATIC_DRAW); // GPU 메모리에 VBO ID에 해당하는 데이터가 GPU메모리에 할당됨

	//Create Particle
	CreateParticle(10000);
}

void Renderer::CreateVertexBufferObjects()
{
	float rect[]
		=
	{
		-0.5, -0.5, 0.f, -0.5, 0.5, 0.f, 0.5, 0.5, 0.f, //Triangle1
		-0.5, -0.5, 0.f,  0.5, 0.5, 0.f, 0.5, -0.5, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = (GLint)strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!! -> 한번 attacb가 끝나면 ShaderObj는 없어져도 됨. 로컬변수인 이유.
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.\n";

	return ShaderProgram;
}
unsigned char * Renderer::loadBMPRaw(const char * imagepath, unsigned int& outWidth, unsigned int& outHeight)
{
	std::cout << "Loading bmp file " << imagepath << " ... " << std::endl;
	outWidth = -1;
	outHeight = -1;
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = NULL;
	fopen_s(&file, imagepath, "rb");
	if (!file)
	{
		std::cout << "Image could not be opened, " << imagepath << " is missing. " << std::endl;
		return NULL;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1E]) != 0)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1C]) != 24)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	outWidth = *(int*)&(header[0x12]);
	outHeight = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = outWidth * outHeight * 3;

	if (dataPos == 0)
		dataPos = 54;

	data = new unsigned char[imageSize];

	fread(data, 1, imageSize, file);

	fclose(file);

	std::cout << imagepath << " is succesfully loaded. " << std::endl;

	return data;
}

void Renderer::CreateParticle(int count)
{
	int floatCount = count * (3 + 3 + 1 + 1 + 1 + 1 + 1 + 4)/*vertex position float 3개, vertex 속도 float 3개, emitTime 1개, 랜덤 lifeTime1개, 주기1개, 폭1개, value(매개변수) 1개, color(rgba): float 4개
													->하나의 vertex가 가진 정보가 늘어날 뿐 vertex count는 안늘어남*/
		* 3 * 2;
	float* particleVertices = new float[floatCount];
	int vertexCount = count * 3 * 2; // drawarrays 인자: vertex count 

	int index = 0;
	float particleSize = 0.01f;

	for (int i = 0; i < count; ++i)
	{
		float randomValueX = 0.f;
		float randomValueY = 0.f;
		float randomValueZ = 0.f;
		// 속도
		float randomValueVX = 1.f; // 오른쪽으로 날아가게
		//float randomValueVX = 0.f;
		float randomValueVY = 0.f;
		float randomValueVZ = 0.f;
		// 파티클 생성 시작 시간
		float randomEmitTime = 0.f;
		float randomLifeTime = 2.f;
		//float randomLifeTime = 0.f;
		float randomPeriod = 1.f; // 한주기 = 2PI 기준을 1로 둠
		float randomAmp = 1.f; // 진폭
		float randValue = 0.f;
		float randR = 0.f;
		float randG = 0.f;
		float randB = 0.f;
		float randA = 0.f;


		//randomValueX = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; // -1~1
		//randomValueY = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; // -1~1
		//randomValueZ = 0.f;
		randomValueVX = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f; // -1~1
		randomValueVY = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f; // -1~1
		randomValueVZ = 0.f;
		randomEmitTime = ((float)rand() / (float)RAND_MAX) * 10.f; // 0~10초사이 왔다갔다 하도록
		randomLifeTime = ((float)rand() / (float)RAND_MAX) * 0.5f;
		randomPeriod = ((float)rand() / (float)RAND_MAX) * 10.f + 1.f;  // 1~11
		randomAmp = ((float)rand() / (float)RAND_MAX) * 0.02f - 0.01f; // 음수도 나오도록
		randValue = ((float)rand() / (float)RAND_MAX);
		randR = ((float)rand() / (float)RAND_MAX); // 컬러: 0~1 사이 랜덤한값
		randG = ((float)rand() / (float)RAND_MAX); // 컬러: 0~1 사이 랜덤한값
		randB = ((float)rand() / (float)RAND_MAX); // 컬러: 0~1 사이 랜덤한값
		randA = ((float)rand() / (float)RAND_MAX); // 컬러: 0~1 사이 랜덤한값


		// 삼각형 두개가 하나의 사각형을 이루도록
		//v0 (왼쪽 아래)
		// Position XYZ
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++; 
		// Velocity XYZ
		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// emit time
		particleVertices[index] = randomEmitTime;
		index++;
		// Life time
		particleVertices[index] = randomLifeTime;
		index++;
		// 주기 Period
		particleVertices[index] = randomPeriod;
		index++;
		// 폭 Amp
		particleVertices[index] = randomAmp;
		index++;
		// rand value
		particleVertices[index] = randValue;
		index++;
		// R
		particleVertices[index] = randR;
		index++;
		// G
		particleVertices[index] = randG;
		index++;
		// B
		particleVertices[index] = randB;
		index++;
		// A
		particleVertices[index] = randA;
		index++;


		//v1 (오른쪽 아래)
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// Velocity XYZ
		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// emit time
		particleVertices[index] = randomEmitTime;
		index++;
		// Life time
		particleVertices[index] = randomLifeTime;
		index++;
		// 주기 Period
		particleVertices[index] = randomPeriod;
		index++;
		// 폭 Amp
		particleVertices[index] = randomAmp;
		index++;
		// rand value
		particleVertices[index] = randValue;
		index++;
		// R
		particleVertices[index] = randR;
		index++;
		// G
		particleVertices[index] = randG;
		index++;
		// B
		particleVertices[index] = randB;
		index++;
		// A
		particleVertices[index] = randA;
		index++;

		//v2 (오른쪽 위)
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// Velocity XYZ
		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// emit time
		particleVertices[index] = randomEmitTime;
		index++;
		// Life time
		particleVertices[index] = randomLifeTime;
		index++;
		// 주기 Period
		particleVertices[index] = randomPeriod;
		index++;
		// 폭 Amp
		particleVertices[index] = randomAmp;
		index++;
		// rand value
		particleVertices[index] = randValue;
		index++;
		// R
		particleVertices[index] = randR;
		index++;
		// G
		particleVertices[index] = randG;
		index++;
		// B
		particleVertices[index] = randB;
		index++;
		// A
		particleVertices[index] = randA;
		index++;

		// v3 (왼쪽 아래)
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// Velocity XYZ
		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// emit time
		particleVertices[index] = randomEmitTime;
		index++;
		// Life time
		particleVertices[index] = randomLifeTime;
		index++;
		// 주기 Period
		particleVertices[index] = randomPeriod;
		index++;
		// 폭 Amp
		particleVertices[index] = randomAmp;
		index++;
		// rand value
		particleVertices[index] = randValue;
		index++;
		// R
		particleVertices[index] = randR;
		index++;
		// G
		particleVertices[index] = randG;
		index++;
		// B
		particleVertices[index] = randB;
		index++;
		// A
		particleVertices[index] = randA;
		index++;

		// v4(오른쪽 위)
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// Velocity XYZ
		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// emit time
		particleVertices[index] = randomEmitTime;
		index++;
		// Life time
		particleVertices[index] = randomLifeTime;
		index++;
		// 주기 Period
		particleVertices[index] = randomPeriod;
		index++;
		// 폭 Amp
		particleVertices[index] = randomAmp;
		index++;
		// rand value
		particleVertices[index] = randValue;
		index++;
		// R
		particleVertices[index] = randR;
		index++;
		// G
		particleVertices[index] = randG;
		index++;
		// B
		particleVertices[index] = randB;
		index++;
		// A
		particleVertices[index] = randA;
		index++;

		// v5(오른쪽 아래)
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// Velocity XYZ
		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		// emit time
		particleVertices[index] = randomEmitTime;
		index++;
		// Life time
		particleVertices[index] = randomLifeTime;
		index++;
		// 주기 Period
		particleVertices[index] = randomPeriod;
		index++;
		// 폭 Amp
		particleVertices[index] = randomAmp;
		index++;
		// rand value
		particleVertices[index] = randValue;
		index++;
		// R
		particleVertices[index] = randR;
		index++;
		// G
		particleVertices[index] = randG;
		index++;
		// B
		particleVertices[index] = randB;
		index++;
		// A
		particleVertices[index] = randA;
		index++;
	}
	glGenBuffers(1, &m_VBOManyParticle); // 파티클 버텍스를 담기위한 VBO 생성
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle); // array형태로 쓸것이다
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW); // VBO ID를 주고 올려라?
	m_VBOManyParticleCount = vertexCount;
}

GLuint Renderer::CreatePngTexture(char * filePath)
{
	//Load Pngs: Load file and decode image.
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filePath);
	if (error != 0)
	{
		lodepng_error_text(error);
		assert(error == 0);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	return temp;
}

GLuint Renderer::CreateBmpTexture(char * filePath)
{
	//Load Bmp: Load file and decode image.
	unsigned int width, height;
	unsigned char * bmp
		= loadBMPRaw(filePath, width, height);

	if (bmp == NULL)
	{
		std::cout << "Error while loading bmp file : " << filePath << std::endl;
		assert(bmp != NULL);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp);

	return temp;
}

void Renderer::Test() // GLSLBase-RenderScene에서 매프레임 호출되고 있다
{
	glUseProgram(m_SolidRectShader); // 기존 교수님이 넣어논 쉐이더

	//int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position"); // vs가보면 a_Position 있음 -> vertex shader 입력에 관련된 함수구나
	// 이 버텍스 쉐이더에 입력을 주려면 주기위한 로케이션을 아이디 형태로 받아온다
	GLint VBOLocation = glGetAttribLocation(m_SolidRectShader, "a_Position");

	glEnableVertexAttribArray(VBOLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	//glEnableVertexAttribArray(0); // 버텍스 쉐이더의 layout (location =0)의 0을 의미
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBO); // m_VBO란 놈을 어레이형태로 쓸거다라는걸 이미 알려줌
	glVertexAttribPointer(VBOLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0); // 얘도 버텍스 쉐이더의 layout (location =0)의 0과 맞춰줘야됨

	// 이렇게 세줄 세트
	// layout 1번에서 m_VBO1 하나더 바인드해서 포인터에 집어넣어줌
	GLint VBOLocation1 = glGetAttribLocation(m_SolidRectShader, "a_Position1");
	glEnableVertexAttribArray(VBOLocation1); // 버텍스 쉐이더의 layout (location =0)의 0을 의미
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glVertexAttribPointer(VBOLocation1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0); // 얘도 버텍스 쉐이더의 layout (location =0)의 0과 맞춰줘야됨


	static float gScale = 0.f; // 삼각형이 커졌다 작아졌다 하게 변수
	// uniform 변수 id(로케이션) set
	GLint ScaleUniform = glGetUniformLocation(m_SolidRectShader, "u_Scale");
	glUniform1f(ScaleUniform, gScale/*값을 넣어줌*/);
	
	// uniform은 반드시 glGetUniformLocation 함수로 받아와야한다
	GLint ColorUniform = glGetUniformLocation(m_SolidRectShader, "u_Color");
	glUniform4f(ColorUniform, 1, gScale, 1, 1); // vec4 -> 4개의 float
	GLint PositionUniform = glGetUniformLocation(m_SolidRectShader, "u_Position");
	glUniform3f(PositionUniform, gScale,gScale,0); // vec4 -> 4개의 float => 이 값을 vs에서 써줘야함


	// Start Rendering, primitive를 알려줌: 어떤 녀석으로 그려라
	// 실제로 파이프라인을 동작시키는 코드 -> 끝날때 까지 다른 입력 못받음
	// 삼각형 하나당 glDrawArrays 하나. (1개의 vertex -> vs -> 1개의 삼각형)
	// 드로우콜 줄이고 싶다면 6개의 vertex를 그려라. 단 로케이션을 따로 쓰면 결국 총 6개의 float 포인트가 1개의 버텍스로 넘어가는 상태. 이건 드로우콜로 줄이는게 아님.
	// glDrawArrays(0,6) 한번만 쓰고 드로우콜 줄이는 방법이 뭘까 숙제.
	glDrawArrays(GL_TRIANGLES, 0, 6/*버텍스 개수*/); // 실제로 렌더링 시작하는 부분 => 즉시 리턴

	gScale += 0.01f;
	if(gScale > 1.f)
		gScale = 0.f;

	//glDisableVertexAttribArray(attribPosition); // 안정화를 위한 백업코드 -> 바인드 풀어줌?
	glDisableVertexAttribArray(VBOLocation); // 얘도 0으로 맞춰줌
	glDisableVertexAttribArray(VBOLocation1); // 얘도 0으로 맞춰줌

}


float g_Time = 0.f;

void Renderer::Particle() // 파티클 렌더함수
{
	GLuint shader = m_SolidRectShader;
	glUseProgram(shader); // shader program 설정


	// attribute location 주기
	// position
	GLint VBOLocation = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(VBOLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOLocation, 3/*vertex당 값 3개씩들어있다*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15/*몇칸 단위로 읽을거냐(stride)*/, (GLvoid*)0/*어디서부터 읽을거냐*/); 
	// velocity
	GLint VBOVLocation = glGetAttribLocation(m_SolidRectShader, "a_Velocity");
	glEnableVertexAttribArray(VBOVLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle); // 버퍼는 같은 버퍼 쓰기때문에 바인드 또해줄 필요는 X
	glVertexAttribPointer(VBOVLocation, 3/*vertex당 값 3개씩들어있다*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 3));
	// emit time
	GLint VBOEmitLocation = glGetAttribLocation(m_SolidRectShader, "a_EmitTime");
	glEnableVertexAttribArray(VBOEmitLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glVertexAttribPointer(VBOEmitLocation, 1/*float 1개*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 6/*6번째 float부터 emit time 등장*/));
	// Life time
	GLint VBOLifeLocation = glGetAttribLocation(m_SolidRectShader, "a_LifeTime");
	glEnableVertexAttribArray(VBOLifeLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glVertexAttribPointer(VBOLifeLocation, 1/*float 1개*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 7/*7번째 float부터 life time 등장*/));
	// Period
	GLint VBOPLocation = glGetAttribLocation(m_SolidRectShader, "a_P");
	glEnableVertexAttribArray(VBOPLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glVertexAttribPointer(VBOPLocation, 1/*float 1개*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 8/*8번째 float부터 period 등장*/));
	// Amp
	GLint VBOALocation = glGetAttribLocation(m_SolidRectShader, "a_A");
	glEnableVertexAttribArray(VBOALocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glVertexAttribPointer(VBOALocation, 1/*float 1개*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 9/*9번째 float부터 amp 등장*/));

	// rand value
	GLint VBORLocation = glGetAttribLocation(m_SolidRectShader, "a_RandValue");
	glEnableVertexAttribArray(VBORLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glVertexAttribPointer(VBORLocation, 1/*float 1개*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 10/*10번째 float부터 등장*/));

	// rand color
	GLint VBOColorLocation = glGetAttribLocation(m_SolidRectShader, "a_Color");
	glEnableVertexAttribArray(VBOColorLocation); // 해당되는 아이디가 사용 될건지 안될건지 알려줌
	glVertexAttribPointer(VBOColorLocation, 4/*float 4개*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 11/*11번째 float부터 amp 등장*/));



	GLint UniformTime = glGetUniformLocation(shader, "u_Time");
	glUniform1f(UniformTime, g_Time);

	// uniform - 외력
	GLint UniformExForce = glGetUniformLocation(shader, "u_ExForce"); 
	glUniform3f(UniformExForce, sin(g_Time), cos(g_Time), 0); // 힘이 연속적으로 바뀌게

	glDrawArrays(GL_TRIANGLES, 0/*시작지점:첨부터 다그릴거니까 0*/, m_VBOManyParticleCount);
	g_Time += 0.016; // 매 프레임
}

void Renderer::FSSandbox()
{
	GLuint shader = m_FSSandboxShader;
	glUseProgram(shader); // shader program 설정

	GLuint attribPosLoc = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(attribPosLoc);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFSSandBox);
	glVertexAttribPointer(attribPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)(0)/*처음부터 읽어옴*/);

	glDrawArrays(GL_TRIANGLES, 0, 6); // 0번째 버텍스 부터 6개
}
