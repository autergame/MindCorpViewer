#define GLM_FORCE_XYZW_ONLY
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glfw3")

#include <windows.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include <unordered_map>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "MemReader.h"

#include "skn.h"
#include "skl.h"
#include "anm.h"
#include "cJSON.h"

float zoom = 700, mousex = 0, mousey = 0;
int omx = 0, omy = 0, mx = 0, my = 0, state, width, height;
LARGE_INTEGER Frequencye, Starte;

double GetTimeSinceStart()
{
	LARGE_INTEGER t_End;
	QueryPerformanceCounter(&t_End);
	return (float)(t_End.QuadPart - Starte.QuadPart) / Frequencye.QuadPart;
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

uint8_t* loadDDS(const char* filename, uint32_t& format, uint32_t& mipmapCount, uint32_t& width, uint32_t& height)
{
	FILE* file;
	errno_t err = fopen_s(&file, filename, "rb");
	if (err)
	{
		char errMsg[255] = { "\0" };
		strerror_s(errMsg, 255, errno);
		printf("ERROR: Cannot read file %s %s\n", filename, errMsg);
		return 0;
	}

	printf("Reading file: %s\n", filename);
	fseek(file, 0, SEEK_END);
	size_t fsize = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	std::string fp(fsize + 1, '\0');
	myassert(fread(fp.data(), 1, fsize, file) != fsize)
	fclose(file);
	printf("Finised reading file\n");

	CharMemVector input;
	input.MemWrite(fp.data(), fsize);

	uint32_t DDS = MAKEFOURCC('D', 'D', 'S', ' ');
	uint32_t DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
	uint32_t DXT5 = MAKEFOURCC('D', 'X', 'T', '5');

	uint32_t Signature = input.MemRead<uint32_t>();
	if (Signature != DDS)
	{
		printf("dds has no valid signature\n");
		scanf_s("press enter to exit.");
		return 0;
	}

	DDS_HEADER Header;
	input.MemRead(&Header, sizeof(DDS_HEADER));

	format = 0x83F1;
	if (Header.ddspf.fourCC == DXT3)
	{
		format = 0x83F2;
	}
	else if (Header.ddspf.fourCC == DXT5)
	{
		format = 0x83F3;
	}

	height = Header.height;
	width = Header.width;
	mipmapCount = Header.mipMapCount;

	uint32_t imageSize = fsize - 128;
	uint8_t* buffer = (uint8_t*)calloc(1, imageSize);
	input.MemRead(buffer, imageSize);

	return buffer;
}

GLuint loadDDSToOpengl(const char* filename)
{
	uint32_t format, width, height, mipMapCount;
	uint8_t* buffer = loadDDS(filename, format, mipMapCount, width, height);

	uint32_t mID;
	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, std::max(1u, mipMapCount - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uint32_t offset = 0;
	uint32_t blockSize = (format == 0x83F1) ? 8 : 16;
	for (uint32_t i = 0; i < std::max(1u, mipMapCount); i++)
	{
		uint32_t size = std::max(1u, ((width + 3) / 4) * ((height + 3) / 4)) * blockSize;

		glCompressedTexImage2D(GL_TEXTURE_2D, i, format, width, height, 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;
	}

	if (std::max(1u, mipMapCount) == 1)
		glGenerateMipmap(GL_TEXTURE_2D);

	free(buffer);
	return mID;
}

GLuint loadCubemap(const char* faces[6])
{
	GLuint cubemap;
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (GLuint i = 0; i < 6; i++)
	{
		uint32_t format, width, height, mipMapCount;
		uint8_t* image = loadDDS(faces[i], format, mipMapCount, width, height);

		uint32_t offset = 0;
		uint32_t blockSize = (format == 0x83F1) ? 8 : 16;
		for (uint32_t level = 0; level < std::max(1u, mipMapCount); level++)
		{
			uint32_t size = std::max(1u, ((width + 3) / 4) * ((height + 3) / 4)) * blockSize;

			glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level, format, width, height, 0, size, image + offset);

			offset += size;
			width /= 2;
			height /= 2;
		}

		free(image);
	}

	return cubemap;
}

GLuint loadShader(GLenum type, const char* filename)
{
	FILE* file;
	errno_t err = fopen_s(&file, filename, "rb");
	if (err)
	{
		char errMsg[255] = { "\0" };
		strerror_s(errMsg, 255, errno);
		printf("ERROR: Cannot read file %s %s\n", filename, errMsg);
		return 0;
	}

	printf("Reading file: %s\n", filename);
	fseek(file, 0, SEEK_END);
	size_t fsize = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	std::string fp(fsize + 1, '\0');
	myassert(fread(fp.data(), 1, fsize, file) != fsize)
	fclose(file);
	printf("Finised reading file\n");

	const char* c_str = fp.c_str();

	GLint success;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &c_str, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		printf("shader(%s) failed: Cannot compile shader\n", filename);
		printf("%s", infoLog);
		scanf_s("press enter to exit.");
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
		glGetProgramInfoLog(shaderid, 512, nullptr, infoLog);
		printf("shader() failed: Cannot link shader\n");
		printf("%s", infoLog);
		scanf_s("press enter to exit.");
	}
	glUseProgram(0);
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	return shaderid;
}

std::vector<std::string> ListDirectoryContents(const char* sDir, const char* ext)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = nullptr;

	char sPath[2048];
	std::vector<std::string> paths;

	sprintf_s(sPath, 2048, "%s\\*.%s", sDir, ext);
	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", sPath);
		scanf_s("press enter to exit.");
		return paths;
	}

	do
	{
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			if (!(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				sprintf_s(sPath, 2048, "%s\\%s", sDir, fdFile.cFileName);
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

glm::mat4 computeMatricesFromInputs(glm::vec3& trans, float& yaw, float& pitch)
{
	static float lastx = mousex;
	static float lasty = mousey;

	if (state == 1)
	{
		if (mx > 0 && mx < width && my > 0 && my < height && 
			!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
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
	position = glm::normalize(position);

	glm::vec3 right = glm::normalize(glm::cross(position, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 up = glm::normalize(glm::cross(right, position));

	if (state == 2)
	{
		if (mx > 0 && mx < width && my > 0 && my < height &&
			!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
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

	glm::mat4 viewmatrix = glm::lookAt(position * zoom, glm::vec3(0.f), up);
	viewmatrix = viewmatrix * glm::translate(trans) * glm::scale(glm::vec3(-1.f, 1.f, 1.f));

	return viewmatrix;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	{
		zoom -= ((float)yoffset * 60) * .5f;
		zoom = zoom < 1.f ? 1.f : zoom;
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	omx = mx;
	omy = my;
	mx = (int)xpos;
	my = (int)ypos;
	mousex = float(mx - omx);
	mousey = float(my - omy);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mx = (int)xpos; my = (int)ypos;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		state = 1;
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		state = 2;
		return;
	}
	if (action == GLFW_RELEASE)
	{
		state = 0;
		return;
	}
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	scanf_s("press enter to exit.");
	exit(1);
}

std::string ReadConfig()
{
	FILE* file;
	errno_t err = fopen_s(&file, "config.json", "rb");
	if (err)
	{
		char errMsg[255] = { "\0" };
		strerror_s(errMsg, 255, errno);
		printf("ERROR: Cannot read file config.json %s\n", errMsg);
		return std::string();
	}

	printf("Reading file: config.json\n");
	fseek(file, 0, SEEK_END);
	size_t fsize = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	std::string fp(fsize + 1, '\0');
	myassert(fread(fp.data(), 1, fsize, file) != fsize)
	fclose(file);
	printf("Finised reading file\n");
	return fp;
}

int main()
{
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

	std::string configStr = ReadConfig();

	cJSON* json = cJSON_ParseWithLength(configStr.data(), configStr.size());
	cJSON* paths = cJSON_GetObjectItem(json, "PATHS");
	cJSON* config = cJSON_GetObjectItem(json, "CONFIG");
	cJSON* textures = cJSON_GetObjectItem(json, "TEXTURES");
	size_t pathsize = cJSON_GetArraySize(paths);

	char** name = (char**)calloc(pathsize, sizeof(char*));
	char** ddsf = (char**)calloc(pathsize, sizeof(char*));
	char** anmf = (char**)calloc(pathsize, sizeof(char*));
	char** sknf = (char**)calloc(pathsize, sizeof(char*));
	char** sklf = (char**)calloc(pathsize, sizeof(char*));

	cJSON* jobj;
	size_t oh = 0;
	for (oh = 0, jobj = paths->child; jobj != nullptr; jobj = jobj->next, oh++)
	{
		name[oh] = cJSON_GetObjectItem(jobj, "name")->valuestring;
		ddsf[oh] = cJSON_GetObjectItem(jobj, "dds")->valuestring;
		anmf[oh] = cJSON_GetObjectItem(jobj, "anm")->valuestring;
		sknf[oh] = cJSON_GetObjectItem(jobj, "skn")->valuestring;
		sklf[oh] = cJSON_GetObjectItem(jobj, "skl")->valuestring;
	}

	std::pair<int, bool> paird;
	std::unordered_map<std::string, std::pair<size_t, bool>> nowshowddsv;
	for (jobj = textures->child; jobj != nullptr; jobj = jobj->next)
	{
		paird = {
			cJSON_GetObjectItem(jobj, "texture")->valueint,
			cJSON_GetObjectItem(jobj, "show")->valueint
		};
		nowshowddsv[cJSON_GetObjectItem(jobj, "name")->valuestring] = paird;
	}

	size_t oe = 0;
	bool* setupanm = (bool*)calloc(pathsize, 1);
	int* nowanm = (int*)calloc(pathsize, 4);
	bool* playanm = (bool*)calloc(pathsize, 1);
	bool* jumpnext = (bool*)calloc(pathsize, 1);
	bool* gotostart = (bool*)calloc(pathsize, 1);
	bool* wireframe = (bool*)calloc(pathsize, 1);
	bool* showskeleton = (bool*)calloc(pathsize, 1);
	bool vsync = cJSON_GetObjectItem(json, "vsync")->valueint;
	bool showground = cJSON_GetObjectItem(json, "showground")->valueint;
	bool showskybox = cJSON_GetObjectItem(json, "showskybox")->valueint;
	bool synchronizedtime = cJSON_GetObjectItem(json, "synchronizedtime")->valueint;
	for (oe = 0, jobj = config->child; jobj != nullptr; jobj = jobj->next, oe++)
	{
		setupanm[oe] = cJSON_GetObjectItem(jobj, "setupanm")->valueint;
		nowanm[oe] = cJSON_GetObjectItem(jobj, "anmlist")->valueint;
		playanm[oe] = cJSON_GetObjectItem(jobj, "playanm")->valueint;
		jumpnext[oe] = cJSON_GetObjectItem(jobj, "jumpnext")->valueint;
		gotostart[oe] = cJSON_GetObjectItem(jobj, "gotostart")->valueint;
		wireframe[oe] = cJSON_GetObjectItem(jobj, "wireframe")->valueint;
		showskeleton[oe] = cJSON_GetObjectItem(jobj, "showskeleton")->valueint;
	}

	RECT rectScreen;
	width = 1024, height = 576;
	HWND hwndScreen = GetDesktopWindow();
	GetWindowRect(hwndScreen, &rectScreen);
	int PosX = ((rectScreen.right - rectScreen.left) / 2 - width / 2);
	int PosY = ((rectScreen.bottom - rectScreen.top) / 2 - height / 2);

	glfwSetErrorCallback(glfw_error_callback);
	myassert(glfwInit() == GLFW_FALSE)

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "BinReaderGUI", nullptr, nullptr);
	myassert(window == nullptr)

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetWindowPos(window, PosX, PosY);
	myassert(gladLoadGL() == 0)

	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwSwapInterval(vsync);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(.5f, .5f, .5f, 1.f);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = nullptr;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark();
	GImGui->Style.GrabRounding = 4.f;
	GImGui->Style.FrameRounding = 4.f;
	GImGui->Style.WindowRounding = 6.f;
	GImGui->Style.FrameBorderSize = 1.f;
	GImGui->Style.WindowBorderSize = 1.f;
	GImGui->Style.IndentSpacing = GImGui->Style.FramePadding.x * 3.0f - 2.0f;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 13);
	int WINDOW_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;

	GLuint idone = loadDDSToOpengl("glsl/map.dds");
	glBindTexture(GL_TEXTURE_2D, idone);
	glActiveTexture(GL_TEXTURE0 + idone);

	const char* cubeFaces[] = {
		"glsl/cube/right.dds",
		"glsl/cube/left.dds",
		"glsl/cube/top.dds",
		"glsl/cube/bottom.dds",
		"glsl/cube/front.dds",
		"glsl/cube/back.dds"
	};

	GLuint idtwo = loadCubemap(cubeFaces);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idtwo);
	glActiveTexture(GL_TEXTURE0 + idtwo);

	GLuint shaderidmap = useshader("glsl/map.vert", "glsl/model.frag");
	GLuint shaderidline = useshader("glsl/line.vert", "glsl/line.frag");
	GLuint shaderidmodel = useshader("glsl/model.vert", "glsl/model.frag");
	GLuint shaderidcube = useshader("glsl/skybox.vert", "glsl/skybox.frag");

	glUseProgram(shaderidmap);
	GLuint mvprefmap = glGetUniformLocation(shaderidmap, "MVP");
	glUniform1i(glGetUniformLocation(shaderidmap, "Diffuse"), idone - 1);

	glUseProgram(shaderidline);
	GLuint mvprefline = glGetUniformLocation(shaderidline, "MVP");
	GLuint colorrefline = glGetUniformLocation(shaderidline, "Color");

	glUseProgram(shaderidmodel);
	GLuint mvprefmodel = glGetUniformLocation(shaderidmodel, "MVP");
	GLuint bonerefmodel = glGetUniformLocation(shaderidmodel, "Bones");
	GLuint texrefmodel = glGetUniformLocation(shaderidmodel, "Diffuse");

	glUseProgram(shaderidcube);
	GLuint mvprefcube = glGetUniformLocation(shaderidcube, "MVP");
	glUniform1i(glGetUniformLocation(shaderidcube, "Skybox"), idtwo - 1);

	float planebufvertex[] = {
		 750.f, 0.f, 750.f, 0.f,1.f,
		 750.f, 0.f,-750.f, 0.f,0.f,
		-750.f, 0.f,-750.f, 1.f,0.f,
		-750.f, 0.f, 750.f, 1.f,1.f
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

	float skyboxbufvertex[] =
	{
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f
	};

	uint32_t skyboxbufindex[] =
	{
		1, 2, 6,
		6, 5, 1,
		0, 4, 7,
		7, 3, 0,
		4, 5, 6,
		6, 7, 4,
		0, 3, 2,
		2, 1, 0,
		0, 1, 5,
		5, 4, 0,
		3, 7, 6,
		6, 2, 3
	};

	uint32_t vertexarrayskyboxBuffer;
	glGenVertexArrays(1, &vertexarrayskyboxBuffer);
	glBindVertexArray(vertexarrayskyboxBuffer);

	uint32_t vertexskyboxBuffer;
	glGenBuffers(1, &vertexskyboxBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexskyboxBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxbufvertex), skyboxbufvertex, GL_STATIC_DRAW);

	uint32_t indexskyboxBuffer;
	glGenBuffers(1, &indexskyboxBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexskyboxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxbufindex), skyboxbufindex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

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

	for (size_t k = 0; k < pathsize; k++)
	{
		speedanm[k] = 1.f;

		openskn(&myskn[k], sknf[k]);
		openskl(&myskl[k], sklf[k]);
		fixbone(&myskn[k], &myskl[k]);

		pathsanm[k] = ListDirectoryContents(anmf[k], "anm");
		for (size_t i = 0; i < pathsanm[k].size(); i++)
		{
			Animation temp;
			openanm(&temp, pathsanm[k][i].c_str());
			myanm[k].emplace_back(temp);
		}
		if (nowanm[k] > (int)pathsanm[k].size())
			nowanm[k] = 0;

		pathsdds[k] = ListDirectoryContents(ddsf[k], "dds");
		for (size_t i = 0; i < pathsdds[k].size(); i++)
		{
			mydds[k].emplace_back(loadDDSToOpengl(pathsdds[k][i].c_str()));
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
		for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
		{
			glGenBuffers(1, &indexBuffer[k][i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[k][i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * myskn[k].Meshes[i].IndexCount, myskn[k].Meshes[i].Indices, GL_STATIC_DRAW);
		}

		glBindVertexArray(0);

		BoneTransforms[k].resize(myskl[k].Bones.size());
		for (size_t i = 0; i < BoneTransforms[k].size(); i++)
			BoneTransforms[k][i] = glm::identity<glm::mat4>();

		lines[k].resize(myskl[k].Bones.size() * 2);
		for (size_t i = 0; i < lines[k].size(); i++)
			lines[k][i] = glm::vec4(1.f);

		joints[k].resize(myskl[k].Bones.size());
		for (size_t i = 0; i < joints[k].size(); i++)
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

		for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
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

	for (size_t k = 0; k < pathsize; k++)
	{
		for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
			myskn[k].Meshes[i].texid = mydds[k][nowdds[k][i]];
		for (size_t i = 0; i < pathsdds[k].size(); i++)
		{
			char* path = (char*)pathsdds[k][i].c_str();
			char* pfile = path + strlen(path);
			for (; pfile > path; pfile--)
			{
				if ((*pfile == '\\') || (*pfile == '/'))
				{
					pfile++;
					break;
				}
			}
			pathsdds[k][i] = pfile;
		}
	}

	glm::vec3 trans(1.f);
	float yaw = 90.f, pitch = 70.f;
	float Deltatime = 0, Lastedtime = 0;

	char windowTitle[64] = { '\0' };
	while (!glfwWindowShouldClose(window))
	{
		Deltatime = float(GetTimeSinceStart() - Lastedtime);
		Lastedtime = (float)GetTimeSinceStart();

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myassert(sprintf_s(windowTitle, 64,
			"MindCorpViewer - Fps: %1.0f / Ms: %1.3f", GImGui->IO.Framerate, 1000.0f / GImGui->IO.Framerate) <= 0)
			glfwSetWindowTitle(window, windowTitle);

		glm::mat4 viewmatrix = computeMatricesFromInputs(trans, yaw, pitch);
		glm::mat4 projectionmatrix = glm::perspective(glm::radians(45.f), (float)width / (float)height, 0.1f, 10000.0f);
		glm::mat4 projviewmatrix = projectionmatrix * viewmatrix;

		if (showground)
		{
			glUseProgram(shaderidmap);
			glBindVertexArray(vertexarrayplaneBuffer);
			glUniformMatrix4fv(mvprefmap, 1, GL_FALSE, (float*)&projviewmatrix);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(0, (float)height / 2.f));
		ImGui::Begin("Main", 0, WINDOW_FLAGS);
		if (ImGui::Checkbox("Enable Vsync", &vsync))
			glfwSwapInterval(vsync);
		ImGui::Checkbox("Show Ground", &showground);
		ImGui::Checkbox("Show Sky Box", &showskybox);
		ImGui::Checkbox("Synchronized Time", &synchronizedtime);
		for (size_t k = 0; k < pathsize; k++)
		{
			if (ImGui::TreeNodeEx(name[k], ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				ImGui::PushID(k * 2);
				ImGui::Checkbox("Wireframe", &wireframe[k]);
				ImGui::Checkbox("Show Skeleton", &showskeleton[k]);
				for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
				{
					ImGui::PushID(i * 3);
					ImGui::Text(myskn[k].Meshes[i].Name);
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
				ImGui::Text("CTRL+Click Change To Input");
				ImGui::SliderFloat("Speed", &speedanm[k], 0.00001f, 10.f, "%.5f");
				ImGui::SliderFloat("Time", &Time[k], 0.00001f, myanm[k][nowanm[k]].Duration, "%.5f");
				ListBox("List", &nowanm[k], pathsanm[k]);
				ImGui::PopID();
				ImGui::TreePop();
			}

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
				for (size_t i = 0; i < pathsize; i++)
					Time[i] = Time[0];

			if (setupanm[k]) {
				SetupAnimation(&BoneTransforms[k], Time[k], &myanm[k][nowanm[k]], &myskl[k]);
			} else {
				for (size_t i = 0; i < BoneTransforms[k].size(); i++)
					BoneTransforms[k][i] = glm::identity<glm::mat4>();
			}

			glUseProgram(shaderidmodel);
			glBindVertexArray(vertexarrayBuffer[k]);
			glUniformMatrix4fv(mvprefmodel, 1, GL_FALSE, (float*)&projviewmatrix);
			glUniformMatrix4fv(bonerefmodel, BoneTransforms[k].size(), GL_FALSE, (float*)&BoneTransforms[k][0]);
			if (wireframe[k])
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
			{
				if (showmesh[k][i])
				{
					glUniform1i(texrefmodel, myskn[k].Meshes[i].texid);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[k][i]);
					glDrawElements(GL_TRIANGLES, myskn[k].Meshes[i].IndexCount, GL_UNSIGNED_SHORT, 0);
				}
			}
			if (wireframe[k])
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (showskeleton[k])
			{
				uint32_t sklk = 0;
				for (size_t i = 0; i < myskl[k].Bones.size(); i++)
				{
					int16_t parentid = myskl[k].Bones[i].ParentID;
					if (parentid != -1)
					{
						lines[k][sklk++] = BoneTransforms[k][i] * myskl[k].Bones[i].GlobalMatrix * glm::vec4(1.f);
						lines[k][sklk++] = BoneTransforms[k][parentid] * myskl[k].Bones[parentid].GlobalMatrix * glm::vec4(1.f);
					}
				}

				for (size_t i = 0; i < BoneTransforms[k].size(); i++)
					joints[k][i] = BoneTransforms[k][i] * myskl[k].Bones[i].GlobalMatrix * glm::vec4(1.f);

				glDisable(GL_DEPTH_TEST);
				glUseProgram(shaderidline);
				glUniform1i(colorrefline, 0);
				glUniformMatrix4fv(mvprefline, 1, GL_FALSE, (float*)&projviewmatrix);
				glBindVertexArray(vertexarraylineBuffer[k]);
				glBindBuffer(GL_ARRAY_BUFFER, vertexlineBuffer[k]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * lines[k].size(), lines[k].data());
				glDrawArrays(GL_LINES, 0, lines[k].size());
				glBindVertexArray(0);

				glPointSize(3.f);
				glUniform1i(colorrefline, 1);
				glBindVertexArray(vertexarrayjointBuffer[k]);
				glBindBuffer(GL_ARRAY_BUFFER, vertexjointBuffer[k]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * joints[k].size(), joints[k].data());
				glDrawArrays(GL_POINTS, 0, joints[k].size());
				glBindVertexArray(0);
				glEnable(GL_DEPTH_TEST);
			}
		}
		if (ImGui::Button("Save Configuration"))
		{
			cJSON* jsons = cJSON_CreateObject();
			cJSON_AddItemToObject(jsons, "vsync", cJSON_CreateBool(vsync));
			cJSON_AddItemToObject(jsons, "showground", cJSON_CreateBool(showground));
			cJSON_AddItemToObject(jsons, "showskybox", cJSON_CreateBool(showskybox));
			cJSON_AddItemToObject(jsons, "synchronizedtime", cJSON_CreateBool(synchronizedtime));

			cJSON* pathss = cJSON_CreateArray();
			cJSON* configs = cJSON_CreateArray();
			cJSON* texturess = cJSON_CreateArray();
			cJSON_AddItemToObject(jsons, "PATHS", pathss);
			cJSON_AddItemToObject(jsons, "CONFIG", configs);
			cJSON_AddItemToObject(jsons, "TEXTURES", texturess);

			for (size_t k = 0; k < pathsize; k++)
			{
				jobj = cJSON_CreateObject();
				cJSON_AddItemToObject(jobj, "name", cJSON_CreateString(name[k]));
				cJSON_AddItemToObject(jobj, "dds", cJSON_CreateString(ddsf[k]));
				cJSON_AddItemToObject(jobj, "anm", cJSON_CreateString(anmf[k]));
				cJSON_AddItemToObject(jobj, "skn", cJSON_CreateString(sknf[k]));
				cJSON_AddItemToObject(jobj, "skl", cJSON_CreateString(sklf[k]));
				cJSON_AddItemToArray(pathss, jobj);

				jobj = cJSON_CreateObject();
				cJSON_AddItemToObject(jobj, "setupanm", cJSON_CreateBool(setupanm[k]));
				cJSON_AddItemToObject(jobj, "anmlist", cJSON_CreateNumber(nowanm[k]));
				cJSON_AddItemToObject(jobj, "playanm", cJSON_CreateBool(playanm[k]));
				cJSON_AddItemToObject(jobj, "jumpnext", cJSON_CreateBool(jumpnext[k]));
				cJSON_AddItemToObject(jobj, "gotostart", cJSON_CreateBool(gotostart[k]));
				cJSON_AddItemToObject(jobj, "wireframe", cJSON_CreateBool(wireframe[k]));
				cJSON_AddItemToObject(jobj, "showskeleton", cJSON_CreateBool(showskeleton[k]));
				cJSON_AddItemToArray(configs, jobj);

				for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
				{
					auto it = nowshowddsv.find(myskn[k].Meshes[i].Name);
					if (it != nowshowddsv.end())
					{
						jobj = cJSON_CreateObject();
						cJSON_AddItemToObject(jobj, "name", cJSON_CreateString(myskn[k].Meshes[i].Name));
						cJSON_AddItemToObject(jobj, "texture", cJSON_CreateNumber(nowdds[k][i]));
						cJSON_AddItemToObject(jobj, "show", cJSON_CreateBool(showmesh[k][i]));
						cJSON_AddItemToArray(texturess, jobj);
					}
				}
			}

			FILE* file;
			errno_t err = fopen_s(&file, "config.json", "wb");
			if (err)
			{
				char errMsg[255] = { '\0' };
				strerror_s(errMsg, 255, err);
				printf("ERROR: Cannot write file config.json %s\n", errMsg);
				return 0;
			}
			fprintf(file, "%s", cJSON_Print(jsons));
			fclose(file);
		}
		if (ImGui::Button("Export Models"))
		{
			for (size_t k = 0; k < pathsize; k++)
			{
				int indexp = 1, size = 0, strsize = strlen(name[k]) + 16;
				char* nameobj = (char*)calloc(strsize, 1);
				sprintf_s(nameobj, strsize, "export/%s.obj", name[k]);

				FILE* fout;
				errno_t err = fopen_s(&fout, nameobj, "wb");
				if (err)
				{
					char errMsg[255] = { '\0' };
					strerror_s(errMsg, 255, err);
					printf("ERROR: Cannot write file %s %s\n", nameobj, errMsg);
					return 0;
				}
				fprintf(fout, "mtllib %s.mtl\n", name[k]);

				for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
				{
					if (showmesh[k][i])
					{
						size += myskn[k].Meshes[i].IndexCount;
						fprintf(fout, "usemtl %s%d\n", name[k], nowdds[k][i]);
						fprintf(fout, "o %s%s\n", name[k], myskn[k].Meshes[i].Name);
						for (uint32_t o = 0; o < myskn[k].Meshes[i].IndexCount; o++)
						{
							uint16_t index = myskn[k].Meshes[i].Indices[o];
							glm::vec3 Positions = myskn[k].Positions[index];
							glm::vec4 BoneIndices = myskn[k].BoneIndices[index];
							glm::vec4 BoneWeights = myskn[k].Weights[index];

							glm::mat4 BoneTransform = BoneTransforms[k][int(BoneIndices[0])] * BoneWeights[0];
							BoneTransform += BoneTransforms[k][int(BoneIndices[1])] * BoneWeights[1];
							BoneTransform += BoneTransforms[k][int(BoneIndices[2])] * BoneWeights[2];
							BoneTransform += BoneTransforms[k][int(BoneIndices[3])] * BoneWeights[3];

							glm::vec3 glposition(BoneTransform * glm::vec4(Positions, 1.0));
							fprintf(fout, "v %.6f %.6f %.6f\n", -glposition.x, glposition.y, glposition.z);
						}
						for (uint32_t o = 0; o < myskn[k].Meshes[i].IndexCount; o++)
						{
							uint16_t index = myskn[k].Meshes[i].Indices[o];
							fprintf(fout, "vt %.6f %.6f\n", myskn[k].UVs[index].x, 1.f - myskn[k].UVs[index].y);
						}
						for (uint32_t o = 0; o < myskn[k].Meshes[i].IndexCount; o++)
						{
							uint16_t index = myskn[k].Meshes[i].Indices[o];
							fprintf(fout, "vn %.6f %.6f %.6f\n",
								myskn[k].Normals[index].x, myskn[k].Normals[index].y, myskn[k].Normals[index].z);
						}
						fprintf(fout, "s 1\n");
						while (indexp < size)
						{
							fprintf(fout, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
								indexp, indexp, indexp, indexp+1, indexp+1, indexp+1, indexp+2, indexp+2, indexp+2);
							indexp += 3;
						}
					}
				}		
				fclose(fout);
				nameobj = (char*)calloc(strsize, 1);
				sprintf_s(nameobj, strsize, "export/%s.mtl", name[k]);

				err = fopen_s(&fout, nameobj, "wb");
				if (err)
				{
					char errMsg[255] = { '\0' };
					strerror_s(errMsg, 255, err);
					printf("ERROR: Cannot write file %s %s\n", nameobj, errMsg);
					return 0;
				}

				std::vector<int> dupli;
				for (size_t i = 0; i < myskn[k].Meshes.size(); i++)
				{
					if (showmesh[k][i])
					{ 
						if (std::find(dupli.begin(), dupli.end(), nowdds[k][i]) == dupli.end())
						{
							fprintf(fout, "newmtl %s%d\n", name[k], nowdds[k][i]);
							fprintf(fout, "map_Kd %s\n\n", pathsdds[k][nowdds[k][i]].c_str());
							dupli.emplace_back(nowdds[k][i]);
						}
					}
				}
				fclose(fout);
			}
		}
		ImGui::End();

		#ifdef _DEBUG
			ImGui::ShowMetricsWindow();
			ImGui::Begin("Dear ImGui Style Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::ShowStyleEditor();
			ImGui::End();
			ImGui::SetWindowCollapsed("Dear ImGui Style Editor", true, ImGuiCond_Once);
			ImGui::SetWindowCollapsed("Dear ImGui Metrics/Debugger", true, ImGuiCond_Once);
			ImGui::SetWindowPos("Dear ImGui Style Editor", ImVec2(width / 1.75f, 73), ImGuiCond_Once);
			ImGui::SetWindowPos("Dear ImGui Metrics/Debugger", ImVec2(width / 1.75f, 50), ImGuiCond_Once);
		#endif

		if (showskybox)
		{
			viewmatrix = glm::mat4(glm::mat3(viewmatrix));
			projviewmatrix = projectionmatrix * viewmatrix;

			glDepthFunc(GL_LEQUAL);
			glUseProgram(shaderidcube);
			glBindVertexArray(vertexarrayskyboxBuffer);
			glUniformMatrix4fv(mvprefcube, 1, GL_FALSE, (float*)&projviewmatrix);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS);
		}

		glDisable(GL_MULTISAMPLE);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glEnable(GL_MULTISAMPLE);

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}