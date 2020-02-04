#include <stdio.h>
#include <string>
#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glad/glad.h>
#include <cstdint>
#include <windows.h>
#include <bitset>
#include <thread> 
#include <vector>
#include <time.h>
#include <map>
#include "skn.h"
#include "skl.h"
#include "anm.h"
#include "dds.h"

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

HDC hDC;
HWND hWnd;
bool touch[256] = {0};
bool active = true;
int nwidth, nheight;
float mousex = 0, mousey = 0;
int omx = 0, omy = 0, mx = 0, my = 0;
LARGE_INTEGER Frequencye, Starte;
GLuint shaderid;

double GetTimeSinceStart()
{
	LARGE_INTEGER t_End;
	QueryPerformanceCounter(&t_End);

	return static_cast<float>(t_End.QuadPart - Starte.QuadPart) / Frequencye.QuadPart;
}

GLuint loadDDS(const char* filename)
{
	FILE *fp;
	fopen_s(&fp, filename, "rb");

	uint32_t DDS = MAKEFOURCC('D', 'D', 'S', ' ');
	uint32_t DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
	uint32_t DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
	uint32_t DXT5 = MAKEFOURCC('D', 'X', 'T', '5');

	uint32_t Signature;
	fread(&Signature, sizeof(uint32_t), 1, fp);
	if (Signature != DDS)
	{
		return 0;
	}

	DDS_HEADER Header;
	fread(&Header, sizeof(DDS_HEADER), 1, fp);

	uint32_t Format;
	if (Header.ddspf.fourCC == DXT1)
	{
		Format = 0x83F1;
	}
	else if (Header.ddspf.fourCC == DXT3)
	{
		Format = 0x83F2;
	}
	else if (Header.ddspf.fourCC == DXT5)
	{
		Format = 0x83F3;
	}

	uint32_t mID;
	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int BlockSize = (Format == 0x83F1) ? 8 : 16;
	unsigned int Height = Header.height;
	unsigned int Width = Header.width;
	for (unsigned int i = 0; i < max(1u, Header.mipMapCount); i++)
	{
		unsigned int Size = max(1u, ((Width + 3) / 4) * ((Height + 3) / 4)) * BlockSize;
		uint8_t* Buffer = (uint8_t*)calloc(Size, sizeof(uint8_t));
		fread(Buffer, sizeof(uint8_t), Size, fp);

		glCompressedTexImage2D(GL_TEXTURE_2D, i, Format, Width, Height, 0, Size, Buffer);

		Width /= 2;
		Height /= 2;
	}

	return mID;
}

GLuint loadShader(GLenum type, const char* filename)
{
	FILE *fp;
	fopen_s(&fp, filename, "rb");
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *string = new char[fsize + 1];
	fread(string, fsize, 1, fp);
	fclose(fp);
	string[fsize] = 0;

	GLint success;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &string, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("shader(%s) failed: Cannot compile shader\n", filename);
		printf("%s", infoLog);
	}
	return shader;
}

void linkProgram(GLuint vertexshader, GLuint fragmentshader)
{
	GLint success;
	shaderid = glCreateProgram();
	glAttachShader(shaderid, vertexshader);
	glAttachShader(shaderid, fragmentshader);
	glLinkProgram(shaderid);
	glGetProgramiv(shaderid, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(shaderid, 512, NULL, infoLog);
		printf("shader() failed: Cannot link shader\n");
		printf("%s", infoLog);
	}
	glUseProgram(0);
}

void useshader(const char* vertexfile, const char* fragmentfile)
{
	GLuint vertexshader = 0;
	GLuint fragmentshader = 0;
	vertexshader = loadShader(GL_VERTEX_SHADER, vertexfile);
	fragmentshader = loadShader(GL_FRAGMENT_SHADER, fragmentfile);
	linkProgram(vertexshader, fragmentshader);
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
}

void *GetAnyGLFuncAddress(const char *name)
{
	void *p = (void*)wglGetProcAddress(name);
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	return p;
}

void computeMatricesFromInputs(glm::vec3 &position, float &yaw, float &pitch, glm::mat4 &projectionmatrix, glm::mat4 &viewmatrix) {

	static double lastTime = GetTimeSinceStart();

	double currentTime = GetTimeSinceStart();
	float deltaTime = float(currentTime - lastTime);

	POINT pos = { nwidth / 2, nheight / 2 };
	ClientToScreen(hWnd, &pos);
	SetCursorPos(pos.x, pos.y);

	yaw += -float(nwidth / 2 - mousex) * .1f;
	pitch += float(nheight / 2 - mousey) * .1f;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction(
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	);
	direction = glm::normalize(direction);
	glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 up = glm::normalize(cross(right, direction));

	float turbo = 200.f;
	if (touch[VK_SHIFT]) {
		turbo = 400.f;
	}

	if (touch['W']) {
		position += direction * deltaTime * turbo;
	}
	if (touch['S']) {
		position -= direction * deltaTime * turbo;
	}
	if (touch['D']) {
		position += right * deltaTime * turbo;
	}
	if (touch['A']) {
		position -= right * deltaTime * turbo;
	}

	projectionmatrix = glm::perspective(glm::radians(45.f), (float)nwidth / (float)nheight, 0.1f, 1000.0f);
	viewmatrix = glm::lookAt(position, position + direction, up);
	lastTime = currentTime;
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	nwidth = width; nheight = height;
}

LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		case WM_SIZE:
			reshape(LOWORD(lParam), HIWORD(lParam));
			PostMessage(hWnd, WM_PAINT, 0, 0);
			break;

		case WM_CLOSE:
			active = FALSE;
			break;

		case WM_KEYDOWN:
			touch[wParam] = TRUE;
			break;

		case WM_KEYUP:
			touch[wParam] = FALSE;
			break;

		case WM_MOUSEMOVE:
			omx = mx;
			omy = my;
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			int dx = mx - omx;
			int dy = my - omy;
			mousex += dx; mousey += dy;
			PostMessage(hWnd, WM_PAINT, 0, 0);
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int random(int min, int max) 
{
	return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int main()
{
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

	int pf;
	MSG msg;
	HGLRC hRC;
	WNDCLASS wc;
	PIXELFORMATDESCRIPTOR pfd;
	static HINSTANCE hInstance = 0;
	const char* title = "windows test";
	int width = 1024, height = 600;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "mindcorp";

	if (!RegisterClass(&wc)) {
		printf("RegisterClass() failed: Cannot register window class\n");
		return 1;
	}

	RECT rectScreen;
	HWND hwndScreen = GetDesktopWindow();
	GetWindowRect(hwndScreen, &rectScreen);
	int PosX = ((rectScreen.right - rectScreen.left) / 2 - width / 2);
	int PosY = ((rectScreen.bottom - rectScreen.top) / 2 - height / 2);

	hWnd = CreateWindow("mindcorp", title, WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		PosX, PosY, width, height, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) {
		printf("CreateWindow() failed: Cannot create a window\n");
		return 1;
	}

	ShowCursor(FALSE);

	hDC = GetDC(hWnd);

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cDepthBits = 24;
	pfd.cColorBits = 32;

	pf = ChoosePixelFormat(hDC, &pfd);
	if (pf == 0) {
		printf("ChoosePixelFormat() failed: Cannot find a suitable pixel format\n");
		return 1;
	}

	if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
		printf("SetPixelFormat() failed: Cannot set format specified\n");
		return 1;
	}

	DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	ReleaseDC(hWnd, hDC);

	hDC = GetDC(hWnd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);

	if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
		printf("gladLoadGLLoader() failed: Cannot load glad\n");
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glClearColor(.5f, .5f, .5f, 1.f);

	useshader("league_model.vert", "league_model.frag");

	uint32_t idone = loadDDS("twistedfate_base_2012_cm.dds");
	glUseProgram(shaderid);
	glUniform1i(glGetUniformLocation(shaderid, "Diffuse"), idone);

	Skin tfskn;
	Skeleton tfskl;
	Animation tfanm;
	openskn(&tfskn, "twistedfate2012.skn");
	openskl(&tfskl, "twistedfate2012.skl");
	openanm(&tfanm, "twistedfate_2012_laugh.anm");

	uint32_t vertexBuffer;     
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * tfskn.Meshes[0].VertexCount, tfskn.Meshes[0].Positions, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t uvBuffer;
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * tfskn.Meshes[0].VertexCount, tfskn.Meshes[0].UVs, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t boneindexBuffer;
	glGenBuffers(1, &boneindexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, boneindexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * tfskn.Meshes[0].VertexCount, tfskn.Meshes[0].BoneIndices, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t boneweightsBuffer;
	glGenBuffers(1, &boneweightsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, boneweightsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * tfskn.Meshes[0].VertexCount, tfskn.Meshes[0].Weights, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * tfskn.Meshes[0].IndexCount, tfskn.Meshes[0].Indices, GL_DYNAMIC_DRAW);

	float yaw = -90.f; float pitch = 0.f;
	glm::vec3 camposition(0.f, 100.f, 200.f);
	glm::mat4 viewmatrix(0.f);
	glm::mat4 projectionmatrix(0.f);

	std::vector<glm::mat4> BoneTransforms;
	BoneTransforms.resize(tfskl.Bones.size());
	for (unsigned int i = 0; i < BoneTransforms.size(); i++)
		BoneTransforms.at(i) = glm::mat4(1.f);

	float Time = 0;
	double Lastedtime = 0;
	ShowWindow(hWnd, TRUE);
	SetFocus(hWnd);
	while (active)
	{
		if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				active = FALSE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			float Deltatime = float(GetTimeSinceStart() - Lastedtime);
			Lastedtime = GetTimeSinceStart();
			Time += Deltatime;

			char tmp[64];
			sprintf_s(tmp, "MindCorpLowUltraGameEngine - FPS: %1.0f", 1 / Deltatime);
			SetWindowText(hWnd, tmp);

			if (Time > tfanm.Duration)
			{
				Time = 0;
			}

			if (touch[VK_ESCAPE])
				active = FALSE;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 mvp;
			computeMatricesFromInputs(camposition, yaw, pitch, projectionmatrix, viewmatrix);
			mvp = projectionmatrix * viewmatrix;

			SetupAnimation(BoneTransforms, Time, tfanm, tfskl);

			glUseProgram(shaderid);

			glActiveTexture(GL_TEXTURE0 + idone);
			glBindTexture(GL_TEXTURE_2D, idone);

			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, boneindexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, boneweightsBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glUniformMatrix4fv(glGetUniformLocation(shaderid, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix4fv(glGetUniformLocation(shaderid, "Bones"), BoneTransforms.size(), GL_FALSE, (float*)&BoneTransforms[0]);
			glDrawElements(GL_TRIANGLES, tfskn.Meshes[0].IndexCount, GL_UNSIGNED_SHORT, 0);

			SwapBuffers(hDC);

			glUseProgram(0);
			glActiveTexture(0);		
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	hRC = NULL;
	ReleaseDC(hWnd, hDC);
	hDC = NULL;
	DestroyWindow(hWnd);
	hWnd = NULL;
	UnregisterClass("mindcorp", hInstance);
	hInstance = NULL;

	return (int)msg.wParam;
}