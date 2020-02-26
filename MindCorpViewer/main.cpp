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
#include <algorithm>
#include <windows.h>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include "skn.h"
#include "skl.h"
#include "anm.h"

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

//HDC hDC;
//HWND hWnd;
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

std::vector<std::string> ListDirectoryContents(const char *sDir, bool anm)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];
	std::vector<std::string> paths;

	if (anm)
	{
		sprintf_s(sPath, 2048, "%s\\*.anm", sDir);
		if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
		{
			printf("Path not found: [%s]\n", sPath);
			return paths;
		}
	}
	else
	{
		sprintf_s(sPath, 2048, "*.dds");
		if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
		{
			printf("File not found: [%s]\n", sPath);
			return paths;
		}
	}

	do
	{
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			if (!(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (anm)
				{
					sprintf_s(sPath, 2048, "%s\\%s", sDir, fdFile.cFileName);
					paths.push_back(sPath);
				}
				else
				{
					sprintf_s(sPath, 2048, "%s", fdFile.cFileName);
					paths.push_back(sPath);
				}
			}
		}
	} while (FindNextFile(hFind, &fdFile));    

	FindClose(hFind);     
	return paths;
}

static auto vector_getter = [](void* vec, int idx, const char** out_text)
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

glm::mat4 computeMatricesFromInputs(glm::vec3 &trans, float &yaw, float &pitch, glm::vec3 center)
{
	static float lastx = mousex;
	static float lasty = mousey;

	if (state == 1)
	{
		if (mx > 0 && mx < nwidth && my > 0 && my < nheight && !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyWindowFocused())
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
		if (mx > 0 && mx < nwidth && my > 0 && my < nheight && !ImGui::IsAnyWindowHovered())
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

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	int value;
	switch (uMsg) 
	{
		case WM_SIZE:
			glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
			nwidth = LOWORD(lParam); nheight = HIWORD(lParam);
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
				zoom -= value > 100 ? 100 : value < -100 ? -100 : value;
				zoom = zoom > 3000.f ? 3000.f : zoom < 100.f ? 100.f : zoom;
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

	int nowanm = 0;
	bool showskl = true;
	bool playanm = false;
	bool jumpnext = false;
	bool gotostart = true;
	char* anmf = new char[256];
	char* sknf = new char[256];
	char* sklf = new char[256];
	std::map<std::string, std::pair<int, bool>> nowshowddsv;

	FILE *fp;
	fopen_s(&fp, "config.txt", "rb+");
	char line[256];
	int linenum = 0;
	int lineptrdds = 0;
	int lineptrconf = 0;
	int lineptrtemp = 0;
	bool ddsinit = false;
	bool confinit = false;
	bool fileinit = false;
	while (fgets(line, 256, fp) != NULL)
	{
		linenum++;		
		lineptrtemp += strlen(line);
		if (line[0] == '#' || line[0] == '\r' && line[1] == '\n')
			continue;
		if (strcmp(line, "-file-init-\r\n") == 0)
		{
			fileinit = true;
			ddsinit = false;
			confinit = false;
			continue;
		}
		else if (strcmp(line, "-dds-init-\r\n") == 0)
		{
			ddsinit = true;
			fileinit = false;
			confinit = false;
			lineptrdds = lineptrtemp;
			continue;
		}
		else if (strcmp(line, "-config-init-\r\n") == 0)
		{
			confinit = true;
			ddsinit = false;
			fileinit = false;
			lineptrconf = lineptrtemp;
			continue;
		}
		if (fileinit)
		{
			char* type = new char[6];
			char* value = new char[256];
			if (sscanf_s(line, "%s = %s", type, 6, value, 255) != 2)
			{
				fprintf(stderr, "Syntax error, line %d\n", linenum);
				continue;
			}
			if (strcmp(type, "anm") == 0)
				anmf = value;
			else if (strcmp(type, "skn") == 0)
				sknf = value;
			else if (strcmp(type, "skl") == 0)
				sklf = value;
		}
		else if (ddsinit)
		{
			int value = 0;
			bool show = true;
			char* type = new char[256];
			if (sscanf_s(line, "%s = %d %d", type, 255, &value, &show) != 3)
			{
				fprintf(stderr, "Syntax error, line %d\n", linenum);
				continue;
			}
			std::pair<int, bool> paird(value, show);
			nowshowddsv[type] = paird;
		}
		else if (confinit)
		{
			int value = 0;
			char* type = new char[256];
			if (sscanf_s(line, "%s = %d", type, 255, &value) != 2)
			{
				fprintf(stderr, "Syntax error, line %d\n", linenum);
				continue;
			}
			if (strcmp(type, "anmlist") == 0)
				nowanm = value;
			else if (strcmp(type, "showskl") == 0)
				showskl = value;
			else if (strcmp(type, "playanm") == 0)
				playanm = value;
			else if (strcmp(type, "jumpnext") == 0)
				jumpnext = value;
			else if (strcmp(type, "gotostart") == 0)
				gotostart = value;
		}
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
		printf("Failed to register window.");
		return 1;
	}

	int width = 1024, height = 600;
	RECT rectScreen;
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
		printf("Failed to create window.");
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
		printf("Failed to register dummy OpenGL window.");
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
		printf("Failed to create dummy OpenGL window.");
		return 1;
	}

	HDC dummy_dc = GetDC(dummy_window);

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.cDepthBits = 24;

	int pixel_formate = ChoosePixelFormat(dummy_dc, &pfd);
	if (!pixel_formate) {
		printf("Failed to find a suitable pixel format.");
		return 1;
	}
	if (!SetPixelFormat(dummy_dc, pixel_formate, &pfd)) {
		printf("Failed to set the pixel format.");
		return 1;
	}

	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context) {
		printf("Failed to create a dummy OpenGL rendering context.");
		return 1;
	}

	if (!wglMakeCurrent(dummy_dc, dummy_context)) {
		printf("Failed to activate dummy OpenGL rendering context.");
		return 1;
	}

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
		"wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
		"wglChoosePixelFormatARB");

	wglMakeCurrent(dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC(dummy_window, dummy_dc);
	DestroyWindow(dummy_window);

	HDC real_dc = GetDC(window);

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
	  WGL_SAMPLES_ARB, 2,
	  0
	};

	int pixel_format;
	UINT num_formats;
	wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	if (!num_formats) {
		printf("Failed to set the OpenGL 3.3 pixel format.");
		return 1;
	}

	PIXELFORMATDESCRIPTOR pfde;
	DescribePixelFormat(real_dc, pixel_format, sizeof(pfde), &pfde);
	if (!SetPixelFormat(real_dc, pixel_format, &pfde)) {
		printf("Failed to set the OpenGL 3.3 pixel format.");
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
		printf("Failed to create OpenGL 3.3 context.");
		return 1;
	}

	if (!wglMakeCurrent(real_dc, gl33_context)) {
		printf("Failed to activate OpenGL 3.3 rendering context.");
		return 1;
	}

	if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
		printf("gladLoadGLLoader() failed: Cannot load glad\n");
		return 1;
	}

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(.5f, .5f, .5f, 1.f);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = "";
	ImGui_ImplWin32_Init(window);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();

	GLuint shaderidmap = 0, shaderidline = 0, shaderidmodel = 0;
	useshader(shaderidmap, "glsl/map.vert", "glsl/map.frag");
	useshader(shaderidline, "glsl/line.vert", "glsl/line.frag");
	useshader(shaderidmodel, "glsl/model.vert", "glsl/model.frag");

	GLuint idone = loadDDS("glsl/map.dds");
	glActiveTexture(GL_TEXTURE0 + idone);
	glBindTexture(GL_TEXTURE_2D, idone);

	glUseProgram(shaderidmap);
	GLuint mvprefe = glGetUniformLocation(shaderidmap, "MVP");
	GLuint texrefe = glGetUniformLocation(shaderidmap, "Diffuse");

	glUseProgram(shaderidline);
	GLuint mvprefel = glGetUniformLocation(shaderidline, "MVP");
	GLuint colorrefe = glGetUniformLocation(shaderidline, "Color");

	glUseProgram(shaderidmodel);
	GLuint mvprefet = glGetUniformLocation(shaderidmodel, "MVP");
	GLuint bonerefet = glGetUniformLocation(shaderidmodel, "Bones");
	GLuint texrefet = glGetUniformLocation(shaderidmodel, "Diffuse");

	Skin myskn;
	Skeleton myskl;
	openskn(&myskn, sknf);
	openskl(&myskl, sklf);
	fixbone(&myskn, &myskl);

	std::vector<Animation> myanm;
	std::vector<std::string> pathsanm = ListDirectoryContents(anmf, true);
	for (uint32_t i = 0; i < pathsanm.size(); i++)
	{
		Animation temp;
		openanm(&temp, pathsanm[i].c_str());
		myanm.push_back(temp);
	}
	if (nowanm > pathsanm.size())
		nowanm = 0;

	std::vector<GLuint> mydds;
	std::vector<std::string> pathsdds = ListDirectoryContents("", false);
	for (uint32_t i = 0; i < pathsdds.size(); i++)
	{
		mydds.push_back(loadDDS(pathsdds[i].c_str()));
	}

	for (uint32_t i = 0; i < mydds.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + mydds[i]);
		glBindTexture(GL_TEXTURE_2D, mydds[i]);
	}

	uint32_t vertexarrayBuffer;
	glGenVertexArrays(1, &vertexarrayBuffer);
	glBindVertexArray(vertexarrayBuffer);

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

	glBindVertexArray(0);

	float planebufvertex[] = {
		400.f, 0.f, 400.f, 1.f,1.f,
		400.f, 0.f,-400.f, 1.f,0.f,
	   -400.f, 0.f,-400.f, 0.f,0.f,
	   -400.f, 0.f, 400.f, 0.f,1.f
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

	std::vector<glm::mat4> BoneTransforms;
	BoneTransforms.resize(myskl.Bones.size());
	for (uint32_t i = 0; i < BoneTransforms.size(); i++)
		BoneTransforms[i] = glm::identity<glm::mat4>();

	std::vector<glm::vec4> lines;
	lines.resize(myskl.Bones.size() * 2);
	for (uint32_t i = 0; i < lines.size(); i++)
		lines[i] = glm::vec4(1.f);

	std::vector<glm::vec4> joints;
	joints.resize(myskl.Bones.size());
	for (uint32_t i = 0; i < joints.size(); i++)
		joints[i] = glm::vec4(1.f);

	uint32_t vertexarraylineBuffer;
	glGenVertexArrays(1, &vertexarraylineBuffer);
	glBindVertexArray(vertexarraylineBuffer);

	uint32_t vertexlineBuffer;
	glGenBuffers(1, &vertexlineBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexlineBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * lines.size(), lines.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	uint32_t vertexarrayjointBuffer;
	glGenVertexArrays(1, &vertexarrayjointBuffer);
	glBindVertexArray(vertexarrayjointBuffer);

	uint32_t vertexjointBuffer;
	glGenBuffers(1, &vertexjointBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexjointBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * joints.size(), joints.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	glm::vec3 trans(1.f);
	double Lastedtime = 0;
	float yaw = 90.f, pitch = 70.f;

	float Time = 0.f;
	float speedanm = 1.f;
	int* nowdds = (int*)calloc(myskn.Meshes.size(), sizeof(int));
	bool* showmesh = (bool*)calloc(myskn.Meshes.size(), sizeof(bool));
	memset(showmesh, 1, myskn.Meshes.size() * sizeof(bool));

	for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
	{
		auto it = nowshowddsv.find(myskn.Meshes[i].Name);
		if (it != nowshowddsv.end())
		{
			nowdds[i] = it->second.first;
			showmesh[i] = it->second.second;
		}
	}

	MSG msg;
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

		float Deltatime = float(GetTimeSinceStart() - Lastedtime);
		Lastedtime = GetTimeSinceStart();

		char tmp[64];
		sprintf_s(tmp, "MindCorpLowUltraGameEngine - FPS: %1.0f", 1 / Deltatime);
		SetWindowText(window, tmp);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(0, nheight/2));
		ImGui::Begin("Main", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Skin");
		ImGui::Checkbox("Show Skeleton", &showskl);
		for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
		{
			ImGui::Text(myskn.Meshes[i].Name.c_str());
			ImGui::PushID(i);
			ListBox("", &nowdds[i], pathsdds);
			myskn.Meshes[i].texid = mydds[nowdds[i]];
			ImGui::PopID();
		}
		for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
		{
			ImGui::Checkbox(myskn.Meshes[i].Name.c_str(), &showmesh[i]);
		}
		ImGui::Text("Animation");
		ImGui::Checkbox("Play/Stop", &playanm);
		ImGui::Checkbox("Go To Start", &gotostart);
		ImGui::Checkbox("Jump To Next", &jumpnext);
		ImGui::SliderFloat("Speed", &speedanm, 0.001f, 5.f);
		ImGui::SliderFloat("Time", &Time, 0.001f, myanm[nowanm].Duration);
		ListBox("List", &nowanm, pathsanm);
		ImGui::End();

		fseek(fp, lineptrdds, 0);
		for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
		{
			auto it = nowshowddsv.find(myskn.Meshes[i].Name);
			if (it != nowshowddsv.end())
			{
				fprintf_s(fp, "%s = %d %d\r\n", myskn.Meshes[i].Name.c_str(), nowdds[i], showmesh[i]);
			}
		}
		fseek(fp, lineptrconf, 0);
		fprintf_s(fp, "anmlist = %d\r\n", nowanm);
		fprintf_s(fp, "showskl = %d\r\n", showskl);
		fprintf_s(fp, "playanm = %d\r\n", playanm);
		fprintf_s(fp, "jumpnext = %d\r\n", jumpnext);
		fprintf_s(fp, "gotostart = %d", gotostart);

		bool dur = Time > myanm[nowanm].Duration;
		if (playanm && !dur)
			Time += Deltatime * speedanm;
		if (dur)
		{
			if(gotostart)
				Time = 0;
			if(jumpnext)
				nowanm += 1;
			if (nowanm == myanm.size())
				nowanm = 0;
		}

		SetupAnimation(&BoneTransforms, Time, &myanm[nowanm], &myskl);

		glm::mat4 mvp = computeMatricesFromInputs(trans, yaw, pitch, myskn.center);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderidmap);
		glUniform1i(texrefe, idone - 1);
		glBindVertexArray(vertexarrayplaneBuffer);
		glUniformMatrix4fv(mvprefe, 1, GL_FALSE, (float*)&mvp);
		glDrawElements(GL_TRIANGLES, sizeof(planebufindex) / sizeof(planebufindex[0]), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(shaderidmodel);
		glBindVertexArray(vertexarrayBuffer);
		glUniformMatrix4fv(mvprefet, 1, GL_FALSE, (float*)&mvp);
		glUniformMatrix4fv(bonerefet, BoneTransforms.size(), GL_FALSE, (float*)&BoneTransforms[0]);
		for (uint32_t i = 0; i < myskn.Meshes.size(); i++)
		{
			if (showmesh[i])
			{
				glUniform1i(texrefet, myskn.Meshes[i].texid);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[i]);
				glDrawElements(GL_TRIANGLES, myskn.Meshes[i].IndexCount, GL_UNSIGNED_SHORT, 0);
			}
		}

		if (showskl)
		{
			uint32_t k = 0;
			for (size_t i = 0; i < myskl.Bones.size(); i++)
			{
				int16_t parentid = myskl.Bones[i].ParentID;
				if (parentid != -1)
				{
					lines[k++] = BoneTransforms[i] * myskl.Bones[i].GlobalMatrix * glm::vec4(1.f);
					lines[k++] = BoneTransforms[parentid] * myskl.Bones[parentid].GlobalMatrix * glm::vec4(1.f);
				}
			}

			for (uint32_t i = 0; i < BoneTransforms.size(); i++)
				joints[i] = BoneTransforms[i] * myskl.Bones[i].GlobalMatrix * glm::vec4(1.f);

			glDisable(GL_DEPTH_TEST);
			glUseProgram(shaderidline);
			glUniform1i(colorrefe, 0);
			glUniformMatrix4fv(mvprefel, 1, GL_FALSE, (float*)&mvp);

			glBindVertexArray(vertexarraylineBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexlineBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * lines.size(), lines.data());
			glDrawArrays(GL_LINES, 0, lines.size());
			glBindVertexArray(0);

			glPointSize(3.f);
			glUniform1i(colorrefe, 1);
			glBindVertexArray(vertexarrayjointBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexjointBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * joints.size(), joints.data());
			glDrawArrays(GL_POINTS, 0, joints.size());
			glBindVertexArray(0);
			glEnable(GL_DEPTH_TEST);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SwapBuffers(gldc);
	}

	fclose(fp);

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