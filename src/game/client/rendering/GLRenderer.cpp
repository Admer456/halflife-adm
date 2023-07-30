
#include "hud.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

#include <fstream>

#include "GLRenderer.hpp"
#include "view.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

cvar_t* sv_zmax;

// Taken from FoxGLBox:
// https://github.com/Admer456/FoxGLBox/blob/master/renderer/src/Backends/OpenGL45/Renderer.cpp#L25
static const char* glTranslateError(GLenum error)
{
	// Originally, it was int glEnum, but GCC complains about narrowing conversions
	// This way is more correct anyway
	struct GLErrorInfo
	{
		GLenum glEnum;
		const char* glString;
	};

	constexpr GLErrorInfo errorEnumStrings[] =
		{
#define glErrorInfo(glEnum) {glEnum, #glEnum}
			glErrorInfo(GL_INVALID_ENUM),
			glErrorInfo(GL_INVALID_FRAMEBUFFER_OPERATION),
			glErrorInfo(GL_INVALID_INDEX),
			glErrorInfo(GL_INVALID_VALUE),
			glErrorInfo(GL_INVALID_OPERATION),
			glErrorInfo(GL_STACK_OVERFLOW),
			glErrorInfo(GL_STACK_UNDERFLOW),
			glErrorInfo(GL_OUT_OF_MEMORY)
#undef glErrorInfo
		};

	for (const auto& errorInfo : errorEnumStrings)
	{
		if (error == errorInfo.glEnum)
			return errorInfo.glString;
	}

	return (const char*)glewGetErrorString(error);
}

static bool GLError(const char* why = nullptr)
{
	Con_Printf("OpenGL: %s", why);

	auto error = glGetError();
	if (error != GL_NO_ERROR)
	{
		Con_Printf(", got an error:\n 0x%x | %i (%s)\n_________________________\n",
			error, error, glTranslateError(error));
		return true;
	}

	Con_Printf(", went OK\n");

	return false;
}

static glm::vec3 convert(const Vector& v)
{
	return {v.x, v.y, v.z};
}

void RenderQuad::SetupModelMatrix()
{
	const glm::vec3 position = convert(this->position);
	const glm::quat orientation = glm::angleAxis(glm::radians(angle), convert(normal));

	// Adding 180 degs here, otherwise the model is flipped in-game
	// Dunno the maths and I'm too lazy to fix
	const float pitch = glm::pitch(orientation) + glm::radians(180.0f);
	const float yaw = glm::yaw(orientation);
	const float roll = glm::roll(orientation);

	constexpr glm::vec3 forward{1.0f, 0.0f, 0.0f};
	constexpr glm::vec3 right{0.0f, -1.0f, 0.0f};
	constexpr glm::vec3 up{0.0f, 0.0f, 1.0f};

	modelMatrix = glm::identity<glm::mat4>();
	// Translation
	modelMatrix = glm::translate(modelMatrix, position);
	// Orientation
	// There is probably a simpler and more efficient way to do this,
	// e.g. AngleVectors into the matrix or something like that, but we'll roll with this
	modelMatrix = glm::rotate(modelMatrix, roll, forward);
	modelMatrix = glm::rotate(modelMatrix, pitch, right);
	modelMatrix = glm::rotate(modelMatrix, yaw, up);
	// Scale, multiplied by 128 here arbitrarily, might wanna let MossWorld do that instead
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scale * 128.0f));
}

static GLRenderer gGLRenderer;

GLRenderer* GLRenderer::GetInstance()
{
	return &gGLRenderer;
}

void GLRenderer::Init()
{
	if (initialised)
	{
		return;
	}

	initialised = true;

	Con_Printf("Initialising GLRenderer...\n");

	// It seems Valve weren't the brightest OpenGL users, so I
	// gotta catch errors coming from the engine's renderer
	GLError("Init: catching the engine's OpenGL errors");

	if (glewInit() != GLEW_OK)
	{
		return Shutdown("GLEW init failure");
	}

	if (!glCreateProgram)
	{
		return Shutdown("Your GPU does not suport OpenGL GLSL shaders");
	}

	if (!glBindVertexArray)
	{
		return Shutdown("Your GPU does not suport OpenGL vertex array objects");
	}

	PushAttributes();
	GLError("Init: pushed GL attributes");

	if (!LoadShader())
	{
		return Shutdown("Shader failure");
	}

	if (!LoadTexture())
	{
		return Shutdown("Texture load failure");
	}

	if (!LoadGeometry())
	{
		return Shutdown("Geometry load failure");
	}

	PopAttributes();
	GLError("Init: popped GL attributes");

	sv_zmax = CVAR_GET_POINTER("sv_zmax");
}

void GLRenderer::Shutdown(const char* why)
{
	if (!initialised)
	{
		return;
	}

	Con_Printf("GLRenderer::Shutdown: %s\n", why);

	initialised = false;
}

cvar_t sv_zmax_dummy{"sv_zmax", "4096", 0, 4096.0f, nullptr};

void GLRenderer::RenderFrame()
{
	if (sv_zmax == nullptr)
	{
		sv_zmax = CVAR_GET_POINTER("sv_zmax");
		if (sv_zmax == nullptr)
		{
			sv_zmax = &sv_zmax_dummy;
		}
	}

	PushAttributes();

	glEnable(GL_BLEND);
	// color = src * alpha + dest * (1 - alpha)
	// dest = scene
	// src = water
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glUseProgram(waterShaderHandle);
	glBindVertexArray(vertexArrayHandle);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	SetupMatrices();

	glUniformMatrix4fv(viewMatrixHandle, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projectionMatrixHandle, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform1f(timeHandle, gHUD.m_flTime);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTextureHandle);

	RenderQuad quad;
	quad.position = Vector(0.0f, 0.0f, 8.0f);
	quad.angle = 0.0f;
	quad.normal = Vector(0.0f, 0.0f, 1.0f);
	quad.scale = 4.0f; // 1024 units for now
	quad.SetupModelMatrix();

	RenderQuadInstance(quad);
	
	// Must make sure to "unuse" everything afterwards,
	// otherwise VGUI2 might get a lil buggy
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindVertexArray(0);

	glUseProgram(0);

	PopAttributes();
}

bool GLRenderer::ReloadShaders()
{
	if (waterShaderHandle)
	{
		glDeleteProgram(waterShaderHandle);
	}

	return LoadShader();
}

void GLRenderer::SetupMatrices()
{
	// GoldSRC uses horizontal FOV, must compensate for this
	const float aspectRatio = (float(ScreenWidth) / float(ScreenHeight));
	const float horizontalFov = glm::radians<float>(gHUD.m_iFOV);
	// Thanks coyo_t for enlightening me
	const float verticalFov = std::atanf(std::tanf(horizontalFov * 0.5f) * (1.0f / aspectRatio)) * 2.0f;

	// Near is 4 according to the Quake engine sources and an apitrace log of Half-Life on bounce.bsp
	// I should write an article about all that eventually
	// Also, because zFar can be set by mappers, we gotta link that to a CVar
	projectionMatrix = glm::perspective(verticalFov, aspectRatio, 4.0f, sv_zmax->value);

	// The rest is taking the view vectors and forming a view matrix
	Vector forward, right, up;
	AngleVectors(v_angles, forward, right, up);

	// It could be done much better, for example, we could store the forward, right and up
	// vectors into the matrix, and set the view origin into the last row
	// This is actually what lookAt does in the end, but I'm lazy, so
	const Vector eye = v_origin;
	const Vector center = eye + forward;
	viewMatrix = glm::lookAt(convert(eye), convert(center), convert(up));
}

void GLRenderer::RenderQuadInstance(const RenderQuad& quad)
{
	glUniformMatrix4fv(modelMatrixHandle, 1, GL_FALSE, glm::value_ptr(quad.modelMatrix));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

bool GLRenderer::LoadShader()
{
	const auto loadShaderFromFile = [](const char* filePath, std::string& str)
	{
		std::ifstream file(filePath);

		if (!file)
		{
			Con_Printf("CreateShaders: '%s' does not exist\n", filePath);
			return false;
		}

		std::string temp;
		while (std::getline(file, temp))
		{
			str += temp + "\n";
		}

		return true;
	};

	std::string vertexShaderCode;
	if (!loadShaderFromFile("admswater/shaders/water.vertex.glsl", vertexShaderCode))
	{
		return false;
	}

	std::string fragmentShaderCode;
	if (!loadShaderFromFile("admswater/shaders/water.pixel.glsl", fragmentShaderCode))
	{
		return false;
	}

	const char* vertexShaderString = vertexShaderCode.c_str();
	const char* fragmentShaderString = fragmentShaderCode.c_str();

	waterShaderHandle = glCreateProgram();
	GLuint vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShaderHandle, 1, &vertexShaderString, nullptr);
	glShaderSource(fragmentShaderHandle, 1, &fragmentShaderString, nullptr);

	// Compile le shadeurs
	glCompileShader(vertexShaderHandle);
	glCompileShader(fragmentShaderHandle);

	const char* errorMessage = GetShaderError(vertexShaderHandle, fragmentShaderHandle);
	if (nullptr != errorMessage)
	{
		Con_Printf("Error while compiling: %s\n", errorMessage);
		return false;
	}

	glAttachShader(waterShaderHandle, vertexShaderHandle);
	glAttachShader(waterShaderHandle, fragmentShaderHandle);
	glLinkProgram(waterShaderHandle);

	glDeleteShader(vertexShaderHandle);
	glDeleteShader(fragmentShaderHandle);

	glUseProgram(waterShaderHandle);

	GLuint diffuseMapHandle = glGetUniformLocation(waterShaderHandle, "diffuseMap");
	glUniform1i(diffuseMapHandle, 0);

	viewMatrixHandle = glGetUniformLocation(waterShaderHandle, "viewMatrix");
	projectionMatrixHandle = glGetUniformLocation(waterShaderHandle, "projectionMatrix");
	modelMatrixHandle = glGetUniformLocation(waterShaderHandle, "modelMatrix");
	timeHandle = glGetUniformLocation(waterShaderHandle, "gTime");

	glUseProgram(0);

	return true;
}

const char* GLRenderer::GetShaderError(GLuint vs, GLuint fs) const
{
	int success;
	static char infoLog[512];

	// Check the vertex shader
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vs, 512, nullptr, infoLog);
		return infoLog;
	}
	// Then the fragment shader
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fs, 512, nullptr, infoLog);
		return infoLog;
	}

	return nullptr;
}

bool GLRenderer::LoadTexture()
{
	int x, y, channels;
	auto* textureData = stbi_load("admswater/gfx/water_blue.png", &x, &y, &channels, 4);
	if (nullptr == textureData)
	{
		Con_Printf("Cannot load 'gfx/water_c2a5.png'\n");
		return false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &waterTextureHandle);
	GLError("CreateTexture: created texture object");

	glBindTexture(GL_TEXTURE_2D, waterTextureHandle);
	GLError("CreateTexture: bound texture object");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	if (GLError("CreateTexture: fed texture object"))
	{
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// TODO: react to gl_texturemode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

bool GLRenderer::LoadGeometry()
{
	static const float GeometryData[] =
		{
			// Bottom-left corner
			-1.0f, -1.0f, 0.0f,
			-1.0f, -1.0f,
			// Bottom-right corner
			1.0f, -1.0f, 0.0f,
			1.0f, -1.0f,
			// Top-right corner
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f,
			// Top-left corner
			-1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f};

	static const uint32_t GeometryIndices[] =
		{
			0, 1, 2,
			2, 3, 0};

	const auto VBOffset = [](const int& offset)
	{
		return reinterpret_cast<void*>(offset);
	};

	// Generate buffer, bind buffer, buffer the buffer
	glGenBuffers(1, &vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GeometryData), GeometryData, GL_STATIC_DRAW);
	GLError("CreateGeometry: Created vertex buffer");

	// Generate VAO, bind VAO, set up vertex attributes
	glGenVertexArrays(1, &vertexArrayHandle);
	glBindVertexArray(vertexArrayHandle);
	GLError("CreateGeometry: Created vertex array");

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), VBOffset(0));
	GLError("CreateGeometry: Set up vertex position attribute");

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), VBOffset(12));
	GLError("CreateGeometry: Set up texture coord attribute");

	// Generate EBO, bind EBO, buffer the EBO
	glGenBuffers(1, &indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GeometryIndices), GeometryIndices, GL_STATIC_DRAW);
	GLError("CreateGeometry: Set up index buffer");

	glBindVertexArray(0);

	return true;
}

void GLRenderer::PushAttributes()
{
	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
}

void GLRenderer::PopAttributes()
{
	glPopAttrib();
	glPopClientAttrib();
}
