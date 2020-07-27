//author https://github.com/autergame
#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <unordered_map>
#include <algorithm>
#include <windows.h>
#include <bitset>
#include <string>
#include <vector>
#include <stdio.h>
#include "skn.h"
#include "skl.h"
#include "anm.h"
#include "cJSON.h"

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
	const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
	const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_SAMPLE_BUFFERS_ARB                    0x2041
#define WGL_SAMPLES_ARB                           0x2042

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

bool touch[256];
float zoom = 700;
int width, height;
bool active = true;
size_t i = 0, k = 0;
float mousex = 0, mousey = 0;
int omx = 0, omy = 0, mx = 0, my = 0, state;
LARGE_INTEGER Frequencye, Starte;

double GetTimeSinceStart()
{
	LARGE_INTEGER t_End;
	QueryPerformanceCounter(&t_End);

	return static_cast<float>(t_End.QuadPart - Starte.QuadPart) / Frequencye.QuadPart;
}

struct DDS_PIXELFORMAT
{
	uint32_t    size;
	uint32_t    flags;
	uint32_t    fourCC;
	uint32_t    RGBBitCount;
	uint32_t    RBitMask;
	uint32_t    GBitMask;
	uint32_t    BBitMask;
	uint32_t    ABitMask;
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth;
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

GLuint loadDDS(const char* filename)
{
	FILE *fp = fopen(filename, "rb");

	uint32_t DDS = MAKEFOURCC('D', 'D', 'S', ' ');
	uint32_t DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
	uint32_t DXT5 = MAKEFOURCC('D', 'X', 'T', '5');

	uint32_t Signature;
	fread(&Signature, sizeof(uint32_t), 1, fp);
	if (Signature != DDS)
	{
		printf("dds has no valid signature\n");
		scanf("press enter to exit.");
		return 0;
	}

	DDS_HEADER Header;
	fread(&Header, sizeof(DDS_HEADER), 1, fp);

	uint32_t Format = 0x83F1;
	if (Header.ddspf.fourCC == DXT3)
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

	uint32_t BlockSize = (Format == 0x83F1) ? 8 : 16;
	uint32_t Height = Header.height;
	uint32_t Width = Header.width;
	for (uint32_t i = 0; i < std::max(1u, Header.mipMapCount); i++)
	{
		uint32_t Size = std::max(1u, ((Width + 3) / 4) * ((Height + 3) / 4)) * BlockSize;
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
	FILE *fp = fopen(filename, "rb");
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
		scanf("press enter to exit.");
	}
	return shader;
}

GLuint useshader(const char* vertexfile, const char* fragmentfile)
{
	GLint success = 0;
	GLuint vertexshader = 0;
	GLuint fragmentshader = 0;
	vertexshader = loadShader(GL_VERTEX_SHADER, vertexfile);
	fragmentshader = loadShader(GL_FRAGMENT_SHADER, fragmentfile);
	GLuint shaderid = glCreateProgram();
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
		scanf("press enter to exit.");
	}
	glUseProgram(0);
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	return shaderid;
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

std::vector<std::string> ListDirectoryContents(const char *sDir, char* ext)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];
	std::vector<std::string> paths;

	sprintf(sPath, "%s\\*.%s", sDir, ext);
	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", sPath);
		scanf("press enter to exit.");
		return paths;
	}

	do
	{
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			if (!(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);
				paths.emplace_back(sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile));    

	FindClose(hFind);     
	return paths;
}

static auto vector_getter = [](void* vec, int idx, const char** out_text)->bool
{
	auto& vector = *static_cast<std::vector<std::string>*>(vec);
	if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
	*out_text = vector.at(idx).c_str();
	return true;
};

bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
{
	if (values.empty()) { return false; }
	return ImGui::Combo(label, currIndex, vector_getter, static_cast<void*>(&values), values.size());
}

glm::mat4 computeMatricesFromInputs(glm::vec3 &trans, float &yaw, float &pitch)
{
	static float lastx = mousex;
	static float lasty = mousey;

	if (state == 1)
	{
		if (mx > 0 && mx < width && my > 0 && my < height && !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyWindowFocused())
		{
			if (mousex != lastx)
				yaw += mousex * .5f;
			if (mousey != lasty)
				pitch -= mousey * .5f;
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
		if (mx > 0 && mx < width && my > 0 && my < height && !ImGui::IsAnyWindowHovered())
		{
			if (mousex != lastx)
			{
				trans.x -= right.x * (mousex * .5f);
				trans.z -= right.z * (mousex * .5f);
			}
			if (mousey != lasty)
				trans.y -= mousey * .5f;
		}
	}

	lastx = mousex;
	lasty = mousey;

	glm::mat4 viewmatrix = glm::lookAt(position * zoom, glm::vec3(0.f, 0.f, 0.f), up);
	viewmatrix = viewmatrix * glm::translate(trans) * glm::scale(glm::vec3(-1.f, 1.f, 1.f));
	return glm::perspective(glm::radians(45.f), (float)width / (float)height, 0.1f, 10000.0f) * viewmatrix;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	int value;
	switch (uMsg) 
	{
		case WM_SIZE:
			width = LOWORD(lParam); 
			height = HIWORD(lParam);
			glViewport(0, 0, width, height);	
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
			if (!ImGui::IsAnyWindowHovered())
			{
				zoom -= value * .5f;
				zoom = zoom < 1.f ? 1.f : zoom;
			}
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
			mousex = float(mx - omx);
			mousey = float(my - omy);
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int main()
{
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

	FILE* file = fopen("config.json", "rb");
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* string = (char*)malloc(fsize + 1);
	fread(string, fsize, 1, file);
	fclose(file);

	cJSON* json = cJSON_ParseWithLength(string, fsize);
	cJSON* paths = cJSON_GetObjectItemCaseSensitive(json, "PATHS");
	cJSON* config = cJSON_GetObjectItemCaseSensitive(json, "CONFIG");
	cJSON* textures = cJSON_GetObjectItemCaseSensitive(json, "TEXTURES");

	size_t pathsize = cJSON_GetArraySize(paths);

	char** ddsf = (char**)calloc(pathsize, 1);
	char** anmf = (char**)calloc(pathsize, 1);
	char** sknf = (char**)calloc(pathsize, 1);
	char** sklf = (char**)calloc(pathsize, 1);

	cJSON* obj;
	for (i = 0, obj = paths->child; obj != NULL; obj = obj->next, i++)
	{
		ddsf[i] = cJSON_GetObjectItemCaseSensitive(obj, "dds")->valuestring;
		anmf[i] = cJSON_GetObjectItemCaseSensitive(obj, "anm")->valuestring;
		sknf[i] = cJSON_GetObjectItemCaseSensitive(obj, "skn")->valuestring;
		sklf[i] = cJSON_GetObjectItemCaseSensitive(obj, "skl")->valuestring;
	}

	std::pair<int, bool> paird;
	std::unordered_map<std::string, std::pair<size_t, bool>> nowshowddsv;
	for (obj = textures->child; obj != NULL; obj = obj->next)
	{
		paird = { 
			cJSON_GetObjectItemCaseSensitive(obj, "texture")->valueint,
			cJSON_GetObjectItemCaseSensitive(obj, "show")->valueint 
		};
		nowshowddsv[cJSON_GetObjectItemCaseSensitive(obj, "name")->valuestring] = paird;
	}

	bool* setupanm = (bool*)calloc(pathsize, 1);
	int* nowanm = (int*)calloc(pathsize, 4);
	bool* playanm = (bool*)calloc(pathsize, 1);
	bool* jumpnext = (bool*)calloc(pathsize, 1);
	bool* gotostart = (bool*)calloc(pathsize, 1);
	bool* wireframe = (bool*)calloc(pathsize, 1);
	bool* showskeleton = (bool*)calloc(pathsize, 1);
	bool showground = cJSON_GetObjectItemCaseSensitive(json, "showground")->valueint;
	bool synchronizedtime = cJSON_GetObjectItemCaseSensitive(json, "synchronizedtime")->valueint;
	for (i = 0, obj = config->child; obj != NULL; obj = obj->next, i++)
	{
		setupanm[i] = cJSON_GetObjectItemCaseSensitive(obj, "setupanm")->valueint;
		nowanm[i] = cJSON_GetObjectItemCaseSensitive(obj, "anmlist")->valueint;
		playanm[i] = cJSON_GetObjectItemCaseSensitive(obj, "playanm")->valueint;
		jumpnext[i] = cJSON_GetObjectItemCaseSensitive(obj, "jumpnext")->valueint;
		gotostart[i] = cJSON_GetObjectItemCaseSensitive(obj, "gotostart")->valueint;
		wireframe[i] = cJSON_GetObjectItemCaseSensitive(obj, "wireframe")->valueint;
		showskeleton[i] = cJSON_GetObjectItemCaseSensitive(obj, "showskeleton")->valueint;
	}

	WNDCLASS window_class;
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = (WNDPROC)WindowProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = GetModuleHandle(0);
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = 0;
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "mindcorpviewer";

	if (!RegisterClassA(&window_class)) {
		printf("Failed to register window.\n");
		scanf("press enter to exit.");
		return 1;
	}

	RECT rectScreen;
	int width = 1024, height = 600;
	HWND hwndScreen = GetDesktopWindow();
	GetWindowRect(hwndScreen, &rectScreen);
	int PosX = ((rectScreen.right - rectScreen.left) / 2 - width / 2);
	int PosY = ((rectScreen.bottom - rectScreen.top) / 2 - height / 2);

	HWND window = CreateWindowExA(
		0,
		window_class.lpszClassName,
		"OpenGL Window",
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		PosX,
		PosY,
		width,
		height,
		0,
		0,
		window_class.hInstance,
		0);

	if (!window) {
		printf("Failed to create window.\n");
		scanf("press enter to exit.");
		return 1;
	}

	WNDCLASSA window_classe;
	window_classe.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_classe.lpfnWndProc = DefWindowProcA;
	window_classe.cbClsExtra = 0;
	window_classe.cbWndExtra = 0;
	window_classe.hInstance = GetModuleHandle(0);
	window_classe.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_classe.hCursor = LoadCursor(0, IDC_ARROW);
	window_classe.hbrBackground = 0;
	window_classe.hbrBackground = NULL;
	window_classe.lpszMenuName = NULL;
	window_classe.lpszClassName = "Dummy_mindcorpviewer";

	if (!RegisterClassA(&window_classe)) {
		printf("Failed to register dummy OpenGL window.\n");
		scanf("press enter to exit.");
		return 1;
	}

	HWND dummy_window = CreateWindowExA(
		0,
		window_classe.lpszClassName,
		"Dummy OpenGL Window",
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		window_classe.hInstance,
		0);

	if (!dummy_window) {
		printf("Failed to create dummy OpenGL window.\n");
		scanf("press enter to exit.");
		return 1;
	}

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.cDepthBits = 24;

	HDC dummy_dc = GetDC(dummy_window);
	int pixel_formate = ChoosePixelFormat(dummy_dc, &pfd);
	if (!pixel_formate) {
		printf("Failed to find a suitable pixel format.\n");
		scanf("press enter to exit.");
		return 1;
	}
	if (!SetPixelFormat(dummy_dc, pixel_formate, &pfd)) {
		printf("Failed to set the pixel format.\n");
		scanf("press enter to exit.");
		return 1;
	}

	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context) {
		printf("Failed to create a dummy OpenGL rendering context.\n");
		scanf("press enter to exit.");
		return 1;
	}

	if (!wglMakeCurrent(dummy_dc, dummy_context)) {
		printf("Failed to activate dummy OpenGL rendering context.\n");
		scanf("press enter to exit.");
		return 1;
	}

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
		"wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
		"wglChoosePixelFormatARB");

	int pixel_format_attribs[] = {
	  WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
	  WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
	  WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
	  WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
	  WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
	  WGL_COLOR_BITS_ARB,         32,
	  WGL_DEPTH_BITS_ARB,         24,
	  WGL_STENCIL_BITS_ARB,       0,
	  WGL_SAMPLE_BUFFERS_ARB,     GL_TRUE,
	  WGL_SAMPLES_ARB,			  4,
	  0
	};

	int pixel_format;
	UINT num_formats;
	HDC real_dc = GetDC(window);
	wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	if (!num_formats) {
		printf("Failed to set the OpenGL 3.3 pixel format.\n");
		scanf("press enter to exit.");
		return 1;
	}

	PIXELFORMATDESCRIPTOR pfde;
	DescribePixelFormat(real_dc, pixel_format, sizeof(pfde), &pfde);
	if (!SetPixelFormat(real_dc, pixel_format, &pfde)) {
		printf("Failed to set the OpenGL 3.3 pixel format.\n");
		scanf("press enter to exit.");
		return 1;
	}

	int gl33_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};

	HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
	if (!gl33_context) {
		printf("Failed to create OpenGL 3.3 context.\n");
		scanf("press enter to exit.");
		return 1;
	}

	if (!wglMakeCurrent(real_dc, gl33_context)) {
		printf("Failed to activate OpenGL 3.3 rendering context.\n");
		scanf("press enter to exit.");
		return 1;
	}

	if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
		printf("gladLoadGLLoader() failed: Cannot load glad\n");
		return 1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(.5f, .5f, .5f, 1.f);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = NULL;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark(); 

	GLuint idone = loadDDS("glsl/map.dds");
	glActiveTexture(GL_TEXTURE0 + idone);

	GLuint shaderidmap = useshader("glsl/map.vert", "glsl/model.frag");
	GLuint shaderidline = useshader("glsl/line.vert", "glsl/line.frag");
	GLuint shaderidmodel = useshader("glsl/model.vert", "glsl/model.frag");

	glUseProgram(shaderidmap);
	GLuint mvprefe = glGetUniformLocation(shaderidmap, "MVP");
	glUniform1i(glGetUniformLocation(shaderidmap, "Diffuse"), idone - 1);

	glUseProgram(shaderidline);
	GLuint mvprefel = glGetUniformLocation(shaderidline, "MVP");
	GLuint colorrefe = glGetUniformLocation(shaderidline, "Color");

	glUseProgram(shaderidmodel);
	GLuint mvprefet = glGetUniformLocation(shaderidmodel, "MVP");
	GLuint bonerefet = glGetUniformLocation(shaderidmodel, "Bones");
	GLuint texrefet = glGetUniformLocation(shaderidmodel, "Diffuse");

	float planebufvertex[] = {
	500.f, 0.f, 500.f, 0.f,1.f,
	500.f, 0.f,-500.f, 0.f,0.f,
   -500.f, 0.f,-500.f, 1.f,0.f,
   -500.f, 0.f, 500.f, 1.f,1.f
	};

	uint32_t planebufindex[] = {
		0, 1, 3,
		1, 2, 3
	};

	uint32_t vertexarrayplaneBuffer;
	glGenVertexArrays(1, &vertexarrayplaneBuffer);
	glBindVertexArray(vertexarrayplaneBuffer);

	uint32_t vertexplaneBuffer;
	glGenBuffers(1, &vertexplaneBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexplaneBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planebufvertex), planebufvertex, GL_STATIC_DRAW);

	uint32_t indexplaneBuffer;
	glGenBuffers(1, &indexplaneBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexplaneBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planebufindex), planebufindex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	std::vector<Skin> myskn(pathsize);
	std::vector<Skeleton> myskl(pathsize);

	std::vector<std::vector<Animation>> myanm(pathsize);
	std::vector<std::vector<std::string>> pathsanm(pathsize);

	std::vector<std::vector<GLuint>> mydds(pathsize);
	std::vector<std::vector<std::string>> pathsdds(pathsize);

	std::vector<uint32_t> vertexarrayBuffer(pathsize);
	std::vector<uint32_t> vertexBuffer(pathsize);
	std::vector<uint32_t> uvBuffer(pathsize);
	std::vector<uint32_t> boneindexBuffer(pathsize);
	std::vector<uint32_t> boneweightsBuffer(pathsize);
	std::vector<std::vector<uint32_t>> indexBuffer(pathsize);

	std::vector<std::vector<glm::mat4>> BoneTransforms(pathsize);
	std::vector<std::vector<glm::vec4>> lines(pathsize);
	std::vector<std::vector<glm::vec4>> joints(pathsize);

	std::vector<uint32_t> vertexarraylineBuffer(pathsize);
	std::vector<uint32_t> vertexlineBuffer(pathsize);
	std::vector<uint32_t> vertexarrayjointBuffer(pathsize);
	std::vector<uint32_t> vertexjointBuffer(pathsize);

	std::vector<float> Time(pathsize);
	std::vector<float> speedanm(pathsize);
	std::vector<std::vector<int>> nowdds(pathsize);
	std::vector<std::vector<int>> showmesh(pathsize);

	for (k = 0; k < pathsize; k++)
	{
		speedanm[k] = 1.f;

		openskn(&myskn[k], sknf[k]);
		openskl(&myskl[k], sklf[k]);
		fixbone(&myskn[k], &myskl[k]);

		pathsanm[k] = ListDirectoryContents(anmf[k], "anm");
		for (i = 0; i < pathsanm[k].size(); i++)
		{
			Animation temp;
			openanm(&temp, pathsanm[k][i].c_str());
			myanm[k].emplace_back(temp);
		}
		if (nowanm[k] > (int)pathsanm[k].size())
			nowanm[k] = 0;

		pathsdds[k] = ListDirectoryContents(ddsf[k], "dds");
		for (i = 0; i < pathsdds[k].size(); i++)
			mydds[k].emplace_back(loadDDS(pathsdds[k][i].c_str()));

		for (i = 0; i < mydds[k].size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, mydds[k][i]);
			glActiveTexture(GL_TEXTURE0 + mydds[k][i]);
			mydds[k][i] = mydds[k][i] - 1;
		}

		glGenVertexArrays(1, &vertexarrayBuffer[k]);
		glBindVertexArray(vertexarrayBuffer[k]);

		glGenBuffers(1, &vertexBuffer[k]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[k]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * myskn[k].Positions.size(), myskn[k].Positions.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &uvBuffer[k]);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[k]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * myskn[k].UVs.size(), myskn[k].UVs.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &boneindexBuffer[k]);
		glBindBuffer(GL_ARRAY_BUFFER, boneindexBuffer[k]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * myskn[k].BoneIndices.size(), myskn[k].BoneIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &boneweightsBuffer[k]);
		glBindBuffer(GL_ARRAY_BUFFER, boneweightsBuffer[k]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * myskn[k].Weights.size(), myskn[k].Weights.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);

		indexBuffer[k].resize(myskn[k].Meshes.size());
		for (i = 0; i < myskn[k].Meshes.size(); i++)
		{
			glGenBuffers(1, &indexBuffer[k][i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[k][i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * myskn[k].Meshes[i].IndexCount, myskn[k].Meshes[i].Indices, GL_STATIC_DRAW);
		}

		glBindVertexArray(0);

		BoneTransforms[k].resize(myskl[k].Bones.size());
		for (i = 0; i < BoneTransforms[k].size(); i++)
			BoneTransforms[k][i] = glm::identity<glm::mat4>();

		lines[k].resize(myskl[k].Bones.size() * 2);
		for (i = 0; i < lines[k].size(); i++)
			lines[k][i] = glm::vec4(1.f);

		joints[k].resize(myskl[k].Bones.size());
		for (i = 0; i < joints[k].size(); i++)
			joints[k][i] = glm::vec4(1.f);

		glGenVertexArrays(1, &vertexarraylineBuffer[k]);
		glBindVertexArray(vertexarraylineBuffer[k]);

		glGenBuffers(1, &vertexlineBuffer[k]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexlineBuffer[k]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * lines[k].size(), lines[k].data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);

		glGenVertexArrays(1, &vertexarrayjointBuffer[k]);
		glBindVertexArray(vertexarrayjointBuffer[k]);

		glGenBuffers(1, &vertexjointBuffer[k]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexjointBuffer[k]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * joints[k].size(), joints[k].data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);

		for (i = 0; i < myskn[k].Meshes.size(); i++)
		{
			auto it = nowshowddsv.find(myskn[k].Meshes[i].Name);
			if (it != nowshowddsv.end())
			{
				if (it->second.first < mydds[k][mydds[k].size() - 1])
					nowdds[k].emplace_back(it->second.first);
				else
					nowdds[k].emplace_back(0);
				showmesh[k].emplace_back(it->second.second);
			}
			else
			{
				nowdds[k].emplace_back(0);
				showmesh[k].emplace_back(0);
				nowshowddsv[myskn[k].Meshes[i].Name] = std::pair<int, bool>(0, 0);
			}
		}
	}

	glm::vec3 trans(1.f);
	float yaw = 90.f, pitch = 70.f;
	float Deltatime = 0, Lastedtime = 0;

	MSG msg{0};
	char tmp[64];
	uint32_t sklk = 0;
	ShowWindow(window, TRUE);
	UpdateWindow(window);
	HDC gldc = GetDC(window);
	while (active)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		if (touch[VK_ESCAPE])
			active = FALSE;

		Deltatime = float(GetTimeSinceStart() - Lastedtime);
		Lastedtime = (float)GetTimeSinceStart();

		sprintf_s(tmp, "MindCorpLowUltraGameEngine - FPS: %1.0f", 1 / Deltatime);
		SetWindowText(window, tmp);

		glm::mat4 mvp = computeMatricesFromInputs(trans, yaw, pitch);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (showground)
		{
			glUseProgram(shaderidmap);
			glBindVertexArray(vertexarrayplaneBuffer);
			glUniformMatrix4fv(mvprefe, 1, GL_FALSE, (float*)&mvp);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(0, (float)height/2.f));
		ImGui::Begin("Main", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Checkbox("Show Ground", &showground);
		ImGui::Checkbox("Synchronized Time", &synchronizedtime);
		for (k = 0; k < pathsize; k++)
		{
			ImGui::PushID(k);
			ImGui::Text("Skin");
			ImGui::Checkbox("Wireframe", &wireframe[k]);
			ImGui::Checkbox("Show Skeleton", &showskeleton[k]);
			for (i = 0; i < myskn[k].Meshes.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::Text(myskn[k].Meshes[i].Name.c_str());
				ImGui::Checkbox("Show model", (bool*)&showmesh[k][i]);
				if (showmesh[k][i])
				{
					ListBox("", &nowdds[k][i], pathsdds[k]);
					myskn[k].Meshes[i].texid = mydds[k][nowdds[k][i]];
					ImGui::Image((void*)(myskn[k].Meshes[i].texid + 1), ImVec2(64, 64));
				}
				ImGui::PopID();
			}
			ImGui::Text("Animation");
			ImGui::Checkbox("Use Animation", &setupanm[k]);
			ImGui::Checkbox("Play / Stop", &playanm[k]);
			ImGui::Checkbox("Go To Start", &gotostart[k]);
			ImGui::Checkbox("Jump To Next", &jumpnext[k]);
			ImGui::SliderFloat("Speed", &speedanm[k], 0.001f, 5.f);
			ImGui::SliderFloat("Time", &Time[k], 0.001f, myanm[k][nowanm[k]].Duration);
			ListBox("List", &nowanm[k], pathsanm[k]);
			ImGui::PopID();
		}
		if (ImGui::Button("Save Configuration"))
		{
			cJSON* jsons = cJSON_CreateObject();
			cJSON_AddItemToObject(jsons, "showground", cJSON_CreateBool(showground));
			cJSON_AddItemToObject(jsons, "synchronizedtime", cJSON_CreateBool(synchronizedtime));

			cJSON* pathss = cJSON_CreateArray();
			cJSON* configs = cJSON_CreateArray();
			cJSON* texturess = cJSON_CreateArray();
			cJSON_AddItemToObject(jsons, "PATHS", pathss);
			cJSON_AddItemToObject(jsons, "CONFIG", configs);
			cJSON_AddItemToObject(jsons, "TEXTURES", texturess);

			for (k = 0; k < pathsize; k++)
			{
				obj = cJSON_CreateObject();
				cJSON_AddItemToObject(obj, "dds", cJSON_CreateString(ddsf[k]));
				cJSON_AddItemToObject(obj, "anm", cJSON_CreateString(anmf[k]));
				cJSON_AddItemToObject(obj, "skn", cJSON_CreateString(sknf[k]));
				cJSON_AddItemToObject(obj, "skl", cJSON_CreateString(sklf[k]));
				cJSON_AddItemToArray(pathss, obj);

				obj = cJSON_CreateObject();
				cJSON_AddItemToObject(obj, "setupanm", cJSON_CreateBool(setupanm[k]));
				cJSON_AddItemToObject(obj, "anmlist", cJSON_CreateNumber(nowanm[k]));
				cJSON_AddItemToObject(obj, "playanm", cJSON_CreateBool(playanm[k]));
				cJSON_AddItemToObject(obj, "jumpnext", cJSON_CreateBool(jumpnext[k]));
				cJSON_AddItemToObject(obj, "gotostart", cJSON_CreateBool(gotostart[k]));
				cJSON_AddItemToObject(obj, "wireframe", cJSON_CreateBool(wireframe[k]));
				cJSON_AddItemToObject(obj, "showskeleton", cJSON_CreateBool(showskeleton[k]));
				cJSON_AddItemToArray(configs, obj);

				for (i = 0; i < myskn[k].Meshes.size(); i++)
				{
					auto it = nowshowddsv.find(myskn[k].Meshes[i].Name);
					if (it != nowshowddsv.end())
					{
						obj = cJSON_CreateObject();
						cJSON_AddItemToObject(obj, "name", cJSON_CreateString(myskn[k].Meshes[i].Name.c_str()));
						cJSON_AddItemToObject(obj, "texture", cJSON_CreateNumber(nowdds[k][i]));
						cJSON_AddItemToObject(obj, "show", cJSON_CreateBool(showmesh[k][i]));
						cJSON_AddItemToArray(texturess, obj);
					}
				}
			}

			file = fopen("config.json", "wb");
			fprintf(file, cJSON_Print(jsons));
			fclose(file);
		}
		ImGui::End();	
		for (k = 0; k < pathsize; k++)
		{
			bool dur = Time[k] > myanm[k][nowanm[k]].Duration;
			if (playanm[k] && !dur)
				Time[k] += Deltatime * speedanm[k];
			if (dur)
			{
				if (gotostart[k])
					Time[k] = 0;
				if (jumpnext[k])
					nowanm[k] += 1;
				if (nowanm[k] == myanm[k].size())
					nowanm[k] = 0;
			}

			if (synchronizedtime)
				for (size_t o = 0; o < pathsize; o++)
						Time[o] = Time[0];

			if (setupanm[k])
				SetupAnimation(&BoneTransforms[k], Time[k], &myanm[k][nowanm[k]], &myskl[k]);
			else
			{
				for (i = 0; i < BoneTransforms[k].size(); i++)
					BoneTransforms[k][i] = glm::identity<glm::mat4>();
			}

			glUseProgram(shaderidmodel);
			glBindVertexArray(vertexarrayBuffer[k]);
			glUniformMatrix4fv(mvprefet, 1, GL_FALSE, (float*)&mvp);
			glUniformMatrix4fv(bonerefet, BoneTransforms[k].size(), GL_FALSE, (float*)&BoneTransforms[k][0]);
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			if(wireframe[k])
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			for (i = 0; i < myskn[k].Meshes.size(); i++)
			{
				if (showmesh[k][i])
				{
					glUniform1i(texrefet, myskn[k].Meshes[i].texid);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[k][i]);
					glDrawElements(GL_TRIANGLES, myskn[k].Meshes[i].IndexCount, GL_UNSIGNED_SHORT, 0);
				}
			}
			if (wireframe[k])
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

			if (showskeleton[k])
			{
				sklk = 0;
				for (i = 0; i < myskl[k].Bones.size(); i++)
				{
					int16_t parentid = myskl[k].Bones[i].ParentID;
					if (parentid != -1)
					{
						lines[k][sklk++] = BoneTransforms[k][i] * myskl[k].Bones[i].GlobalMatrix * glm::vec4(1.f);
						lines[k][sklk++] = BoneTransforms[k][parentid] * myskl[k].Bones[parentid].GlobalMatrix * glm::vec4(1.f);
					}
				}

				for (i = 0; i < BoneTransforms[k].size(); i++)
					joints[k][i] = BoneTransforms[k][i] * myskl[k].Bones[i].GlobalMatrix * glm::vec4(1.f);

				glDisable(GL_DEPTH_TEST);
				glUseProgram(shaderidline);
				glUniform1i(colorrefe, 0);
				glUniformMatrix4fv(mvprefel, 1, GL_FALSE, (float*)&mvp);
				glBindVertexArray(vertexarraylineBuffer[k]);
				glBindBuffer(GL_ARRAY_BUFFER, vertexlineBuffer[k]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * lines[k].size(), lines[k].data());
				glDrawArrays(GL_LINES, 0, lines[k].size());
				glBindVertexArray(0);

				glPointSize(3.f);
				glUniform1i(colorrefe, 1);
				glBindVertexArray(vertexarrayjointBuffer[k]);
				glBindBuffer(GL_ARRAY_BUFFER, vertexjointBuffer[k]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * joints[k].size(), joints[k].data());
				glDrawArrays(GL_POINTS, 0, joints[k].size());
				glBindVertexArray(0);
				glEnable(GL_DEPTH_TEST);
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SwapBuffers(gldc);
	}

	ImGui_ImplOpenGL3_Shutdown();
	wglDeleteContext(gl33_context);
	ImGui::DestroyContext();
	ImGui_ImplWin32_Shutdown();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gl33_context);
	ReleaseDC(window, gldc);
	DestroyWindow(window);
	UnregisterClass("WGL_fdjhsklf", window_class.hInstance);

	return (int)msg.wParam;
}