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
#include <vector>
#include <time.h>
#include <map>
#include "skn.h"
#include "skl.h"
#include "anm.h"
#include "dds.h"

HDC hDC;
HWND hWnd;
bool touch[256];
float zoom = 700;
bool active = true;
int nwidth, nheight;
float mousex = 0, mousey = 0;
int omx = 0, omy = 0, mx = 0, my = 0, state;
LARGE_INTEGER Frequencye, Starte;

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

void linkProgram(GLuint &shaderid, GLuint vertexshader, GLuint fragmentshader)
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

void useshader(GLuint &shaderid, const char* vertexfile, const char* fragmentfile)
{
	GLuint vertexshader = 0;
	GLuint fragmentshader = 0;
	vertexshader = loadShader(GL_VERTEX_SHADER, vertexfile);
	fragmentshader = loadShader(GL_FRAGMENT_SHADER, fragmentfile);
	linkProgram(shaderid, vertexshader, fragmentshader);
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

std::vector<std::string> ListDirectoryContents(const char *sDir)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];
	std::vector<std::string> paths;

	sprintf_s(sPath, 2048, "%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", sDir);
		return paths;
	}

	do
	{
		if (strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			sprintf_s(sPath, 2048, "%s\\%s", sDir, fdFile.cFileName);

			if (!(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY))
			{
				paths.push_back(sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile));    

	FindClose(hFind);     
	return paths;
}

glm::mat4 computeMatricesFromInputs(glm::vec3 &trans, float &yaw, float &pitch, glm::vec3 center)
{
	static float lastx = mousex;
	static float lasty = mousey;

	if (state == 1)
	{
		if (mx > 0 && mx < nwidth && my > 0 && my < nheight)
		{
			if (mousex != lastx)
				yaw += mousex * .6f;
			if (mousey != lasty)
				pitch -= mousey * .6f;
		}

		pitch = pitch > 179.f ? 179.f : pitch < 1.f ? pitch = 1.f : pitch;
		yaw = yaw > 360.f ? yaw - 360.f : yaw < -360.f ? yaw += 360.f : yaw;
	}

	glm::vec3 position = glm::vec3(
		sin(glm::radians(pitch)) * cos(glm::radians(yaw)),
		cos(glm::radians(pitch)),
		sin(glm::radians(pitch)) * sin(glm::radians(yaw))
	);

	glm::vec3 right = glm::normalize(glm::cross(position, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 up = glm::normalize(glm::cross(right, position));

	if (state == 2)
	{
		if (mx > 0 && mx < nwidth && my > 0 && my < nheight)
		{
			if (mousex != lastx)
			{
				trans.x -= right.x * mousex;
				trans.z -= right.z * mousex;
			}
			if (mousey != lasty)
				trans.y -= mousey;
		}
	}

	lastx = mousex;
	lasty = mousey;

	glm::mat4 viewmatrix = glm::lookAt(position * zoom, center, up) * glm::translate(trans);
	return glm::perspective(glm::radians(45.f), (float)nwidth / (float)nheight, 0.1f, 10000.0f) * viewmatrix;
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	nwidth = width; nheight = height;
}

LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int value;
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

		case WM_MOUSEWHEEL:
			value = (int)(short)HIWORD(wParam);
			zoom -= value > 100 ? 100 : value < -100 ? -100 : value;
			zoom = zoom > 3000.f ? 3000.f : zoom < 100.f ? 100.f : zoom;
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			if (uMsg == WM_LBUTTONDOWN)
				state = 1;
			if (uMsg == WM_RBUTTONDOWN)
				state = 2;
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture();
			state = 0;
			break;

		case WM_MOUSEMOVE:
			omx = mx;
			omy = my;
			mx = (int)(short)LOWORD(lParam);
			my = (int)(short)HIWORD(lParam);
			mousex = mx - omx;
			mousey = my - omy;
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int main()
{
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

	MSG msg;
	WNDCLASS wc;
	PIXELFORMATDESCRIPTOR pfd;
	int width = 800, height = 600;

	static HINSTANCE hInstance = GetModuleHandle(NULL);
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

	hWnd = CreateWindow("mindcorp", "", WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		PosX, PosY, width, height, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) {
		printf("CreateWindow() failed: Cannot create a window\n");
		return 1;
	}

	hDC = GetDC(hWnd);

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cDepthBits = 24;
	pfd.cColorBits = 32;

	int pf = ChoosePixelFormat(hDC, &pfd);
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
	HGLRC hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);

	if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
		printf("gladLoadGLLoader() failed: Cannot load glad\n");
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(.5f, .5f, .5f, 1.f);

	GLuint shaderidone = 0, shaderidtwo = 0;
	useshader(shaderidone, "map.vert", "map.frag");
	useshader(shaderidtwo, "model.vert", "model.frag");

	uint32_t idone = loadDDS("map.dds");
	uint32_t idtwo = loadDDS("yasuo_base_tx_cm.dds");

	glActiveTexture(GL_TEXTURE0 + idone);
	glBindTexture(GL_TEXTURE_2D, idone);

	glActiveTexture(GL_TEXTURE0 + idtwo);
	glBindTexture(GL_TEXTURE_2D, idtwo);

	glUseProgram(shaderidone);
	GLuint mvprefe = glGetUniformLocation(shaderidone, "MVP");
	glUniform1i(glGetUniformLocation(shaderidone, "Diffuse"), idone);

	glUseProgram(shaderidtwo);
	GLuint mvprefet = glGetUniformLocation(shaderidtwo, "MVP");
	GLuint bonerefet = glGetUniformLocation(shaderidtwo, "Bones");
	glUniform1i(glGetUniformLocation(shaderidtwo, "Diffuse"), idtwo);

	Skin myskn;
	Skeleton myskl;
	openskn(&myskn, "yasuo.skn");
	openskl(&myskl, "yasuo.skl");
	fixallthings(&myskn, &myskl);

	std::vector<Animation> myanm;
	std::vector<std::string> paths = ListDirectoryContents("animations");
	for (int i = 0; i < paths.size(); i++)
	{
		Animation temp;
		openanm(&temp, paths[i].c_str());
		myanm.push_back(temp);
	}

	uint32_t vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * myskn.Positions.size(), myskn.Positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t uvBuffer;
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * myskn.UVs.size(), myskn.UVs.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t boneindexBuffer;
	glGenBuffers(1, &boneindexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, boneindexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * myskn.BoneIndices.size(), myskn.BoneIndices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);

	uint32_t boneweightsBuffer;
	glGenBuffers(1, &boneweightsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, boneweightsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * myskn.Weights.size(), myskn.Weights.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);

	std::vector<uint32_t> indexBuffer;
	indexBuffer.resize(myskn.Meshes.size());
	for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
	{
		glGenBuffers(1, &indexBuffer[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * myskn.Meshes[i].IndexCount, myskn.Meshes[i].Indices, GL_STATIC_DRAW);
	}

	float planebufvertex[] = {
	    400.f, 0.f, 400.f, 1.f,1.f,
		400.f, 0.f,-400.f, 1.f,0.f,
	   -400.f, 0.f,-400.f, 0.f,0.f,
	   -400.f, 0.f, 400.f, 0.f,1.f
	};

	unsigned int planebufindex[] = {
		0, 1, 3,
		1, 2, 3
	};

	uint32_t vertexarrayplaneBuffer, vertexplaneBuffer, indexplaneBuffer;
	glGenVertexArrays(1, &vertexarrayplaneBuffer);
	glGenBuffers(1, &vertexplaneBuffer);
	glGenBuffers(1, &indexplaneBuffer);
	glBindVertexArray(vertexarrayplaneBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexplaneBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planebufvertex), planebufvertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexplaneBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planebufindex), planebufindex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	std::vector<glm::mat4> BoneTransforms;
	BoneTransforms.resize(myskl.Bones.size());
	for (unsigned int i = 0; i < BoneTransforms.size(); i++)
		BoneTransforms[i] = glm::identity<glm::mat4>();

	int nowanm = 0;
	glm::vec3 trans(1.f);
	float yaw = 90.f, pitch = 70.f;
	float Time = 0;
	double Lastedtime = 0;
	ShowWindow(hWnd, TRUE);
	SetFocus(hWnd);
	while (active)
	{
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		float Deltatime = float(GetTimeSinceStart() - Lastedtime);
		Lastedtime = GetTimeSinceStart();

		char tmp[64];
		sprintf_s(tmp, "MindCorpLowUltraGameEngine - FPS: %1.0f", 1 / Deltatime);
		SetWindowText(hWnd, tmp);

		Time += Deltatime;
		if (Time > myanm[nowanm].Duration)
		{
			Time = 0;
			nowanm += 1;
			if (nowanm == myanm.size())
				nowanm = 0;
		}

		if (touch[VK_ESCAPE])
			active = FALSE;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 mvp = computeMatricesFromInputs(trans, yaw, pitch, myskn.center);

		SetupAnimation(&BoneTransforms, Time, &myanm[nowanm], &myskl);

		glUseProgram(shaderidone);
		glBindVertexArray(vertexarrayplaneBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexplaneBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexplaneBuffer);
		glUniformMatrix4fv(mvprefe, 1, GL_FALSE, (float*)&mvp);
		glDrawElements(GL_TRIANGLES, sizeof(planebufindex) / sizeof(planebufindex[0]), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(shaderidtwo);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, boneindexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, boneweightsBuffer);
		glUniformMatrix4fv(mvprefet, 1, GL_FALSE, (float*)&mvp);
		glUniformMatrix4fv(bonerefet, BoneTransforms.size(), GL_FALSE, (float*)&BoneTransforms[0]);
		for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[i]);
			glDrawElements(GL_TRIANGLES, myskn.Meshes[i].IndexCount, GL_UNSIGNED_SHORT, 0);
		}

		SwapBuffers(hDC);
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