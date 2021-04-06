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

	//Load shaders -> ���⿡ ���̴����α׷� ID ��
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	
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
	float tempVertices[] = { 0.f,0.f,0.f,1.f,0.f,0.f,1.f,1.f,0.f }; // 9��
	glGenBuffers(1, &m_VBO); // 2��� ID -> �����δ� 2�� ������
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices), tempVertices, GL_STATIC_DRAW); // GPU �޸𸮿� VBO ID�� �ش��ϴ� �����Ͱ� GPU�޸𸮿� �Ҵ��


	float tempVertices1[] = { 0.f,0.f,0.f,-1.f,0.f,0.f,-1.f,1.f,0.f }; // 9��
	glGenBuffers(1, &m_VBO1); // 2��� ID -> �����δ� 2�� ������
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices1), tempVertices1, GL_STATIC_DRAW); // GPU �޸𸮿� VBO ID�� �ش��ϴ� �����Ͱ� GPU�޸𸮿� �Ҵ��

	//Create Particle
	CreateParticle(1000);
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
	//���̴� ������Ʈ ����
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = (GLint)strlen(pShaderText);
	//���̴� �ڵ带 ���̴� ������Ʈ�� �Ҵ�
	glShaderSource(ShaderObj, 1, p, Lengths);

	//�Ҵ�� ���̴� �ڵ带 ������
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj �� ���������� ������ �Ǿ����� Ȯ��
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL �� shader log �����͸� ������
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram �� attach!! -> �ѹ� attacb�� ������ ShaderObj�� �������� ��. ���ú����� ����.
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
	GLuint ShaderProgram = glCreateProgram(); //�� ���̴� ���α׷� ����

	if (ShaderProgram == 0) { //���̴� ���α׷��� ����������� Ȯ��
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs �� vs ������ �ε���
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs �� fs ������ �ε���
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram �� vs.c_str() ���ؽ� ���̴��� �������� ����� attach��
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram �� fs.c_str() �����׸�Ʈ ���̴��� �������� ����� attach��
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach �Ϸ�� shaderProgram �� ��ŷ��
	glLinkProgram(ShaderProgram);

	//��ũ�� �����ߴ��� Ȯ��
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program �α׸� �޾ƿ�
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
	int floatCount = count * (3 + 3 + 1 + 1 + 1 + 1)/*vertex position float 3��, vertex �ӵ� float 3��, emitTime 1��, ���� lifeTime1��, �ֱ�1��, ��1��
													->�ϳ��� vertex�� ���� ������ �þ �� vertex count�� �ȴþ*/
		* 3 * 2;
	float* particleVertices = new float[floatCount];
	int vertexCount = count * 3 * 2; // drawarrays ����: vertex count 

	int index = 0;
	float particleSize = 0.01f;

	for (int i = 0; i < count; ++i)
	{
		float randomValueX = 0.f;
		float randomValueY = 0.f;
		float randomValueZ = 0.f;
		// �ӵ�
		float randomValueVX = 1.f; // ���������� ���ư���
		//float randomValueVX = 0.f;
		float randomValueVY = 0.f;
		float randomValueVZ = 0.f;
		// ��ƼŬ ���� ���� �ð�
		float randomEmitTime = 0.f;
		float randomLifeTime = 0.f;
		//float randomLifeTime = 0.f;
		float randomPeriod = 1.f; // ���ֱ� = 2PI ������ 1�� ��
		float randomAmp = 1.f; // ����


		//randomValueX = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; // -1~1
		//randomValueY = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; // -1~1
		//randomValueZ = 0.f;
		//randomValueVX = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; // -1~1
		//randomValueVY = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; // -1~1
		//randomValueVZ = 0.f;
		randomEmitTime = ((float)rand() / (float)RAND_MAX) * 10.f; // 0~10�ʻ��� �Դٰ��� �ϵ���
		randomLifeTime = ((float)rand() / (float)RAND_MAX) * 2.f;
		randomPeriod = ((float)rand() / (float)RAND_MAX) * 10.f + 1;  // 1~11
		randomAmp = ((float)rand() / (float)RAND_MAX) * 0.4f - 0.2f; // ������ ��������



		// �ﰢ�� �ΰ��� �ϳ��� �簢���� �̷絵��
		//v0 (���� �Ʒ�)
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
		// �ֱ� Period
		particleVertices[index] = randomPeriod;
		index++;
		// �� Amp
		particleVertices[index] = randomAmp;
		index++;


		//v1 (������ �Ʒ�)
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
		// �ֱ� Period
		particleVertices[index] = randomPeriod;
		index++;
		// �� Amp
		particleVertices[index] = randomAmp;
		index++;

		//v2 (������ ��)
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
		// �ֱ� Period
		particleVertices[index] = randomPeriod;
		index++;
		// �� Amp
		particleVertices[index] = randomAmp;
		index++;

		// v3 (���� �Ʒ�)
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
		// �ֱ� Period
		particleVertices[index] = randomPeriod;
		index++;
		// �� Amp
		particleVertices[index] = randomAmp;
		index++;

		// v4(������ ��)
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
		// �ֱ� Period
		particleVertices[index] = randomPeriod;
		index++;
		// �� Amp
		particleVertices[index] = randomAmp;
		index++;

		// v5(������ �Ʒ�)
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
		// �ֱ� Period
		particleVertices[index] = randomPeriod;
		index++;
		// �� Amp
		particleVertices[index] = randomAmp;
		index++;
	}
	glGenBuffers(1, &m_VBOManyParticle); // ��ƼŬ ���ؽ��� ������� VBO ����
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle); // array���·� �����̴�
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW); // VBO ID�� �ְ� �÷���?
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

void Renderer::Test() // GLSLBase-RenderScene���� �������� ȣ��ǰ� �ִ�
{
	glUseProgram(m_SolidRectShader); // ���� �������� �־�� ���̴�

	//int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position"); // vs������ a_Position ���� -> vertex shader �Է¿� ���õ� �Լ�����
	// �� ���ؽ� ���̴��� �Է��� �ַ��� �ֱ����� �����̼��� ���̵� ���·� �޾ƿ´�
	GLint VBOLocation = glGetAttribLocation(m_SolidRectShader, "a_Position");

	glEnableVertexAttribArray(VBOLocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	//glEnableVertexAttribArray(0); // ���ؽ� ���̴��� layout (location =0)�� 0�� �ǹ�
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBO); // m_VBO�� ���� ������·� ���Ŵٶ�°� �̹� �˷���
	glVertexAttribPointer(VBOLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0); // �굵 ���ؽ� ���̴��� layout (location =0)�� 0�� ������ߵ�

	// �̷��� ���� ��Ʈ
	// layout 1������ m_VBO1 �ϳ��� ���ε��ؼ� �����Ϳ� ����־���
	GLint VBOLocation1 = glGetAttribLocation(m_SolidRectShader, "a_Position1");
	glEnableVertexAttribArray(VBOLocation1); // ���ؽ� ���̴��� layout (location =0)�� 0�� �ǹ�
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glVertexAttribPointer(VBOLocation1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0); // �굵 ���ؽ� ���̴��� layout (location =0)�� 0�� ������ߵ�


	static float gScale = 0.f; // �ﰢ���� Ŀ���� �۾����� �ϰ� ����
	// uniform ���� id(�����̼�) set
	GLint ScaleUniform = glGetUniformLocation(m_SolidRectShader, "u_Scale");
	glUniform1f(ScaleUniform, gScale/*���� �־���*/);
	
	// uniform�� �ݵ�� glGetUniformLocation �Լ��� �޾ƿ;��Ѵ�
	GLint ColorUniform = glGetUniformLocation(m_SolidRectShader, "u_Color");
	glUniform4f(ColorUniform, 1, gScale, 1, 1); // vec4 -> 4���� float
	GLint PositionUniform = glGetUniformLocation(m_SolidRectShader, "u_Position");
	glUniform3f(PositionUniform, gScale,gScale,0); // vec4 -> 4���� float => �� ���� vs���� �������


	// Start Rendering, primitive�� �˷���: � �༮���� �׷���
	// ������ ������������ ���۽�Ű�� �ڵ� -> ������ ���� �ٸ� �Է� ������
	// �ﰢ�� �ϳ��� glDrawArrays �ϳ�. (1���� vertex -> vs -> 1���� �ﰢ��)
	// ��ο��� ���̰� �ʹٸ� 6���� vertex�� �׷���. �� �����̼��� ���� ���� �ᱹ �� 6���� float ����Ʈ�� 1���� ���ؽ��� �Ѿ�� ����. �̰� ��ο��ݷ� ���̴°� �ƴ�.
	// glDrawArrays(0,6) �ѹ��� ���� ��ο��� ���̴� ����� ���� ����.
	glDrawArrays(GL_TRIANGLES, 0, 6/*���ؽ� ����*/); // ������ ������ �����ϴ� �κ� => ��� ����

	gScale += 0.01f;
	if(gScale > 1.f)
		gScale = 0.f;

	//glDisableVertexAttribArray(attribPosition); // ����ȭ�� ���� ����ڵ� -> ���ε� Ǯ����?
	glDisableVertexAttribArray(VBOLocation); // �굵 0���� ������
	glDisableVertexAttribArray(VBOLocation1); // �굵 0���� ������

}


float g_Time = 0.f;

void Renderer::Particle() // ��ƼŬ �����Լ�
{
	GLuint shader = m_SolidRectShader;
	glUseProgram(shader); // shader program ����


	// attribute location �ֱ�
	// position
	GLint VBOLocation = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(VBOLocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOLocation, 3/*vertex�� �� 3��������ִ�*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 10/*��ĭ ������ �����ų�(stride)*/, (GLvoid*)0/*��𼭺��� �����ų�*/); 
	// velocity
	GLint VBOVLocation = glGetAttribLocation(m_SolidRectShader, "a_Velocity");
	glEnableVertexAttribArray(VBOVLocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle); // ���۴� ���� ���� ���⶧���� ���ε� ������ �ʿ�� X
	glVertexAttribPointer(VBOVLocation, 3/*vertex�� �� 3��������ִ�*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 10, (GLvoid*)(sizeof(float) * 3));
	// emit time
	GLint VBOEmitLocation = glGetAttribLocation(m_SolidRectShader, "a_EmitTime");
	glEnableVertexAttribArray(VBOEmitLocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	glVertexAttribPointer(VBOEmitLocation, 1/*float 1��*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 10, (GLvoid*)(sizeof(float) * 6/*6��° float���� emit time ����*/));
	// Life time
	GLint VBOLifeLocation = glGetAttribLocation(m_SolidRectShader, "a_LifeTime");
	glEnableVertexAttribArray(VBOLifeLocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	glVertexAttribPointer(VBOLifeLocation, 1/*float 1��*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 10, (GLvoid*)(sizeof(float) * 7/*7��° float���� life time ����*/));
	// Period
	GLint VBOPLocation = glGetAttribLocation(m_SolidRectShader, "a_P");
	glEnableVertexAttribArray(VBOPLocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	glVertexAttribPointer(VBOPLocation, 1/*float 1��*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 10, (GLvoid*)(sizeof(float) * 8/*8��° float���� period ����*/));
	// Amp
	GLint VBOALocation = glGetAttribLocation(m_SolidRectShader, "a_A");
	glEnableVertexAttribArray(VBOALocation); // �ش�Ǵ� ���̵� ��� �ɰ��� �ȵɰ��� �˷���
	glVertexAttribPointer(VBOALocation, 1/*float 1��*/, GL_FLOAT,
		GL_FALSE, sizeof(float) * 10, (GLvoid*)(sizeof(float) * 9/*9��° float���� amp ����*/));



	GLint UniformTime = glGetUniformLocation(shader, "u_Time");
	glUniform1f(UniformTime, g_Time);

	glDrawArrays(GL_TRIANGLES, 0/*��������:÷���� �ٱ׸��Ŵϱ� 0*/, m_VBOManyParticleCount);
	g_Time += 0.016; // �� ������
}
