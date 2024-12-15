#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>
#include <string>

using namespace std;

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> XYdis(-1, 1);
uniform_real_distribution<float> xz_dis(-10.f, 10.f);
uniform_real_distribution<double> dis(0.0, 1.0);

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 GetTransform()
	{
		glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 S = glm::scale(glm::mat4(1.0), scale);
		glm::mat4 RX = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
		glm::mat4 RY = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
		return T * RX * RY * RZ * S;
	}
};

typedef struct {
	float x, y, z;
} Vertex;

typedef struct {
	unsigned int v1, v2, v3;
} Face;

typedef struct {
	Vertex* vertices;
	size_t vertex_count;
	Face* faces;
	size_t face_count;
} Model;

typedef struct {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
} OpenGLModelBuffers;

typedef struct {
	float x, y, z;
	float dx, dy, dz;
	bool active;
} Bullet;

OpenGLModelBuffers gunBuffers;
Model gunModel;
std::vector<Bullet> bullets;
int ammo = 30;
int maxAmmo = 30;
float bulletSpeed = 0.1f;
int score = 0;
void* font = GLUT_BITMAP_HELVETICA_18;

struct OBJECT {
	GLuint vao, vbo[3];
	Transform worldmatrix;
	Transform modelmatrix;
	OBJECT* parent{ nullptr };

	glm::vec3* vertex;
	glm::vec3* face;
	glm::vec3* vertexdata;
	glm::vec3* normaldata;
	glm::vec3* colordata;

	int v_count = 0;
	int f_count = 0;
	int vertex_count = f_count * 3;

	void ReadObj(string fileName)
	{
		ifstream in{ fileName };

		string s;

		while (in >> s)
		{
			if (s == "v") v_count++;
			else if (s == "f") ++f_count;
		}
		in.close();
		in.open(fileName);

		vertex_count = f_count * 3;

		vertex = new glm::vec3[v_count];
		face = new glm::vec3[f_count];
		vertexdata = new glm::vec3[vertex_count];
		normaldata = new glm::vec3[vertex_count];
		colordata = new glm::vec3[vertex_count];

		int v_incount = 0;
		int f_incount = 0;
		while (in >> s)
		{
			if (s == "v") {
				in >> vertex[v_incount].x >> vertex[v_incount].y >> vertex[v_incount].z;
				v_incount++;
			}
			else if (s == "f") {
				in >> face[f_incount].x >> face[f_incount].y >> face[f_incount].z;
				vertexdata[f_incount * 3 + 0] = vertex[static_cast<int>(face[f_incount].x - 1)];
				vertexdata[f_incount * 3 + 1] = vertex[static_cast<int>(face[f_incount].y - 1)];
				vertexdata[f_incount * 3 + 2] = vertex[static_cast<int>(face[f_incount].z - 1)];
				f_incount++;
			}
		}

		for (int i = 0; i < f_count; i++)
		{
			glm::vec3 normal = glm::cross(vertexdata[i * 3 + 1] - vertexdata[i * 3 + 0], vertexdata[i * 3 + 2] - vertexdata[i * 3 + 0]);
			//glm::vec3 normal = glm::vec3(0.0, 1.0, 0.0);
			normaldata[i * 3 + 0] = normal;
			normaldata[i * 3 + 1] = normal;
			normaldata[i * 3 + 2] = normal;
		}
	}

	glm::mat4 GetTransform()
	{
		if (parent)
			return parent->GetTransform() * worldmatrix.GetTransform();
		return worldmatrix.GetTransform();
	}

	glm::mat4 GetmodelTransform()
	{
		return modelmatrix.GetTransform();
	}

	void Move(float distance)
	{
		worldmatrix.position.x += distance;
	}
};

struct CUBE :OBJECT
{
	void Init()
	{
		for (int i = 0; i < vertex_count; i++)
		{
			// double random_color = dis(gen);

			colordata[i].x = 0.3f;
			colordata[i].y = 0.3f;
			colordata[i].z = 0.3f;
		}
		for (int i = 0; i < vertex_count; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.5, 0.5);
		}

		glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
		glBindVertexArray(vao); //--- VAO를 바인드하기
		glGenBuffers(3, vbo); //--- 3개의 VBO를 지정하고 할당하기

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
	}

	void draw(int shaderID)
	{
		unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}

	void resize()
	{

	}

	void update()
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}
};
CUBE cube;
CUBE minicube;
CUBE player;
constexpr int MONSTER_COUNT = 10;
CUBE monster[MONSTER_COUNT];

class Monster : CUBE {
	void Init() {
		for (int i = 0; i < vertex_count; i++)
		{
			// double random_color = dis(gen);

			colordata[i].x = 0.7f;
			colordata[i].y = 0.7f;
			colordata[i].z = 0.7f;
		}
	}
};

GLfloat lineShape[10][2][3] = {};	//--- 선분 위치 값

glm::vec3 colors[12][3] = {};

GLfloat XYZShape[3][2][3] = {
	{{-1.0,0.0,0.0},{1.0,0.0,0.0}},
	{{0.0,-1.0,0.0},{0.0,1.0,0.0}},
	{{0.0,0.0,-1.0},{0.0,0.0,1.0}} };

GLfloat XYZcolors[6][3] = { //--- 축 색상
	{ 1.0, 0.0, 0.0 },	   	{ 1.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0 },	   	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },	   	{ 0.0, 0.0, 1.0 }
};

// cameraPos를 플레이어가 움직이는 만큼 움직여주기
glm::vec3 cameraPos = glm::vec3(0.0f, 30.0f, 0.0f); //--- 카메라 위치
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

glm::mat4 view = glm::lookAt(
	cameraPos,                // 카메라 위치
	cameraDirection,             // 카메라가 바라보는 방향
	cameraUp                  // 카메라의 위쪽 방향
);

// 여기 총 부분 코드
void read_obj_file(const char* filename, Model* model);
OpenGLModelBuffers create_opengl_buffers(Model* model);
void fireBullet();
void updateBullets();
void checkCollisions();
void renderText(float x, float y, const char* text);


//


GLuint vao, vbo[3];
GLuint TriPosVbo, TriColorVbo;

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

int windowWidth = 800;
int windowHeight = 800;

float openGLX, openGLY;
int movingRectangle = -1;

float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 0.15;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;

bool start = true;
bool monsterActive[MONSTER_COUNT] = {true,}; // monster가 활성 상태인지 여부

bool NSelection = true;	//true : cube, false : pyramid
bool YSelection = false;
int RSelection = 0;
int RSelectionCnt = 0;
int CCnt = 0;
float CX, CY, CZ;

void make_shaderProgram();
void make_vertexShaders();
void make_fragmentShaders();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void InitBuffer();
char* filetobuf(const char*);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid WindowToOpenGL(int mouseX, int mouseY, float& x, float& y);
GLvoid Motion(int x, int y);
GLvoid TimerFunction(int value);
GLvoid SpecialKeys(int key, int x, int y);
void ReadObj(FILE* path);
GLvoid mouseWheel(int button, int dir, int x, int y);

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{

	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Example1");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n";
	}
	cube.ReadObj("map.obj");
	minicube.ReadObj("map.obj");
	player.ReadObj("cube.obj");
	for (int i = 0; i < MONSTER_COUNT; ++i) 
	{
		monster[i].ReadObj("cube.obj");
	}
	std::cout << "Objects initialized.\n";

	read_obj_file("gun.obj", &gunModel);
	gunBuffers = create_opengl_buffers(&gunModel);
	CX = 1.0, CY = 1.0, CZ = 1.0;

	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_shaderProgram(); //--- 세이더 프로그램 만들기
	InitBuffer();
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE); //--- 상태 설정은 필요한 곳에서 하면 된다.
	//glDisable(GL_DEPTH_TEST | GL_CULL_FACE);	//해제

	glutTimerFunc(10, TimerFunction, 1);
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys); // 방향키 콜백 함수 등록
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMouseWheelFunc(mouseWheel);

	glutMainLoop();
}

GLvoid drawScene()
{
	glUseProgram(shaderProgramID);
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //--- 깊이 버퍼를 클리어한다.

	glBindVertexArray(vao);

	// 색상 바꾸기
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), XYZcolors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	unsigned int cameraLocation = glGetUniformLocation(shaderProgramID, "camera_position");
	glUniform3f(cameraLocation, cameraPos.x, cameraPos.y, cameraPos.z);

	projection = glm::mat4(1.0f);
	projection = glm::scale(projection, glm::vec3(wheel_scale, wheel_scale, wheel_scale));
	projection = glm::rotate(projection, (float)glm::radians(x_angle), glm::vec3(1.0, 0.0, 0.0));
	projection = glm::rotate(projection, (float)glm::radians(y_angle), glm::vec3(0.0, 1.0, 0.0));	

	int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 perspect = glm::mat4(1.0f);
	perspect = glm::perspective(glm::radians(fovy), (float)windowWidth / (float)windowHeight, near_1, far_1);
	perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

	glm::mat4 lightmatrix = cube.GetTransform();
	glm::vec3 lightposition = glm::vec3(0, 10, 0);

	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3f(lightPosLocation, lightposition.x, lightposition.y, lightposition.z);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	glUniform3f(lightColorLocation, CX, CY, CZ);
	unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 1.0, 0.5, 0.3);

	// player를 그린다
	player.draw(shaderProgramID);

	// monster가 활성 상태일 때만 그린다
	for (int i = 0; i < MONSTER_COUNT; ++i) {
		if (monsterActive[i])
		{
			monster[i].draw(shaderProgramID);
		}
	}
	cube.draw(shaderProgramID);

	glBindVertexArray(gunBuffers.vao);
	glDrawElements(GL_TRIANGLES, gunModel.face_count * 3, GL_UNSIGNED_INT, 0);

	for (const auto& bullet : bullets) {
		if (bullet.active) {
			glPushMatrix();
			glTranslatef(bullet.x, bullet.y, bullet.z);
			glutSolidSphere(0.05f, 10, 10);
			glPopMatrix();
		}
	}

	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);
	char hud[128];
	sprintf_s(hud, "Ammo: %d / %d", ammo, maxAmmo);
	renderText(-0.9f, 0.9f, hud);
	sprintf_s(hud, "Score: %d", score);
	renderText(-0.9f, 0.8f, hud);
	glEnable(GL_DEPTH_TEST);

	glutSwapBuffers();
}
//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(vao); //--- VAO를 바인드하기
	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기

	// 초기화
	cube.Init();
	for (int i = 0; i < MONSTER_COUNT; ++i) {
		monster[i].Init();
		monsterActive[i] = true;
	}
	player.Init();

	// 플레이어와 몬스터 초기 위치 설정
	player.worldmatrix.position = glm::vec3(0.0f, -30.f, 0.0f); // 플레이어 중심
	for (int i = 0; i < MONSTER_COUNT; ++i) {
		float random_x = xz_dis(gen);
		float random_z = xz_dis(gen);
		monster[i].worldmatrix.position = glm::vec3(random_x, -30.0f, random_z); // 몬스터 초기 위치
	}
	for (int i = 0; i < MONSTER_COUNT; ++i) {
		monster[i].parent = &cube; // 몬스터의 부모 설정 (필요 시)
	}
}

void make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}

void make_vertexShaders()
{
	vertexSource = filetobuf("vertex3.glsl");
	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment3.glsl");
	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cout << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}

bool CheckCollision(const CUBE& player, const CUBE& monster)
{
	float distance = glm::length(player.worldmatrix.position - monster.worldmatrix.position);
	float collisionDistance = 0.5f; // 충돌 거리 기준 (플레이어와 몬스터의 중심 거리)

	return distance < collisionDistance; // 두 객체가 충돌했으면 true 반환
}


void MoveMonsterTowardsPlayer(float speed)
{
	for (int i = 0; i < MONSTER_COUNT; ++i) {

		if (!monsterActive[i]) continue; // 몬스터가 비활성화되면 움직이지 않음

		glm::vec3 direction = player.worldmatrix.position - monster[i].worldmatrix.position;

		if (glm::length(direction) > 0.01f) // 너무 가까우면 움직임 중지
		{
			direction = glm::normalize(direction);
			monster[i].worldmatrix.position += direction * speed;
		}

		// 충돌 감지
		if (CheckCollision(player, monster[i]))
		{
			monsterActive[i] = false; // 충돌하면 monster 비활성화
		}
	}
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	// w a s d를 입력했을때 플레이어 위치가 변경되도록 하기 + 카메라 위치도
	if (key == 'r' || key == 'R') {
		reload();
	}

	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid SpecialKeys(int key, int x, int y)
{

	glutPostRedisplay(); // 화면 갱신
}

int movingMouse = -1;
float beforeX, beforeY;

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		ox = x;
		oy = y;
		left_button = true;
	}
	else
	{
		ox = 0;
		oy = 0;
		pre_x_angle = x_angle;
		pre_y_angle = y_angle;
		left_button = false;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		fireBullet();
	}
}

GLvoid Motion(int x, int y)
{
	if (left_button)
	{
		y_angle = x - ox;
		x_angle = y - oy;
		x_angle += pre_x_angle;
		y_angle += pre_y_angle;

		y_angle /= 2;
		x_angle /= 2;
	}
	glutPostRedisplay();
}

GLvoid mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		wheel_scale += dir * 0.1;
	}
	else if (dir < 0)
	{
		wheel_scale += dir * 0.1;
		if (wheel_scale < 0.1)
		{
			wheel_scale = 0.1;
		}
	}
	glutPostRedisplay();
}

GLvoid WindowToOpenGL(int mouseX, int mouseY, float& x, float& y)
{
	x = (2.0f * mouseX) / windowWidth - 1.0f;
	y = 1.0f - (2.0f * mouseY) / windowHeight;
}

GLvoid TimerFunction(int value)
{
	switch (value)
	{
	case 1:
		// 몬스터가 플레이어를 향해 이동
		MoveMonsterTowardsPlayer(0.005f); // 속도 0.05
		break;
	}
	updateBullets();
	checkCollisions();
	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);

	
}


// 총


void read_obj_file(const char* filename, Model* model) {
	FILE* file;
	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening OBJ file");
		exit(EXIT_FAILURE);
	}

	char line[128];
	model->vertex_count = 0;
	model->face_count = 0;

	while (fgets(line, sizeof(line), file)) {
		if (line[0] == 'v' && line[1] == ' ')
			model->vertex_count++;
		else if (line[0] == 'f' && line[1] == ' ')
			model->face_count++;
	}

	fseek(file, 0, SEEK_SET);
	model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex));
	model->faces = (Face*)malloc(model->face_count * sizeof(Face));

	size_t vertex_index = 0, face_index = 0;
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == 'v' && line[1] == ' ') {
			sscanf_s(line + 2, "%f %f %f", &model->vertices[vertex_index].x,
				&model->vertices[vertex_index].y,
				&model->vertices[vertex_index].z);
			vertex_index++;
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			unsigned int v1, v2, v3;
			sscanf_s(line + 2, "%u %u %u", &v1, &v2, &v3);
			model->faces[face_index].v1 = v1 - 1;
			model->faces[face_index].v2 = v2 - 1;
			model->faces[face_index].v3 = v3 - 1;
			face_index++;
		}
	}
	fclose(file);
}

void fireBullet() {
	if (ammo > 0) {
		Bullet newBullet;
		newBullet.x = 0.0f;
		newBullet.y = -0.5f;
		newBullet.z = 0.0f;
		newBullet.dx = 0.0f;
		newBullet.dy = 0.0f;
		newBullet.dz = 1.0f;
		newBullet.active = true;
		bullets.push_back(newBullet);
		ammo--;
	}
	else {
		std::cout << "Out of ammo! Press 'R' to reload." << std::endl;
	}
}

void reload() {
	if (ammo < maxAmmo) {
		ammo = maxAmmo;
		std::cout << "Reloaded! Ammo: " << ammo << std::endl;
	}
}

OpenGLModelBuffers create_opengl_buffers(Model* model) {
	OpenGLModelBuffers buffers;

	glGenVertexArrays(1, &buffers.vao);
	glBindVertexArray(buffers.vao);

	glGenBuffers(1, &buffers.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, buffers.vbo);
	glBufferData(GL_ARRAY_BUFFER, model->vertex_count * sizeof(Vertex), model->vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &buffers.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->face_count * sizeof(Face), model->faces, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	return buffers;
}


void updateBullets() {
	for (auto& bullet : bullets) {
		if (bullet.active) {
			bullet.x += bullet.dx * bulletSpeed;
			bullet.y += bullet.dy * bulletSpeed;
			bullet.z += bullet.dz * bulletSpeed;

			if (bullet.z > 10.0f) {
				bullet.active = false;
			}
		}
	}
}

void checkCollisions() {
	for (auto& bullet : bullets) {
		if (!bullet.active) continue;

		if (std::abs(bullet.x - 0.0f) < 0.5f &&
			std::abs(bullet.y - 0.0f) < 0.5f &&
			std::abs(bullet.z - 5.0f) < 0.5f) {
			bullet.active = false;
			score++;
			std::cout << "Hit! Score: " << score << std::endl;
		}
	}
}

void renderText(float x, float y, const char* text) {
	glRasterPos2f(x, y);
	for (const char* c = text; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}



//update() : 아예 데이터를 바꾸고 싶을때 쓴다.

// player
