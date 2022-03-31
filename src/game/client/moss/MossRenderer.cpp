
#include "hud.h"

#include <glm.hpp>
#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

#include <fstream>

#include "MossWorld.hpp"
#include "MossRenderer.hpp"

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
#define glErrorInfo(glEnum) {glEnum, #glEnum},
		glErrorInfo(GL_INVALID_ENUM)
		glErrorInfo(GL_INVALID_FRAMEBUFFER_OPERATION)
		glErrorInfo(GL_INVALID_INDEX)
		glErrorInfo(GL_INVALID_VALUE)
		glErrorInfo(GL_INVALID_OPERATION)
		glErrorInfo(GL_STACK_OVERFLOW)
		glErrorInfo(GL_STACK_UNDERFLOW)
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

IMossRenderer* MossRenderer::GetInstance()
{
	static MossRenderer instance;
	return &instance;
}

void MossRenderer::Init()
{
	if (initialised)
	{
		return;
	}

	initialised = true;

	Con_Printf("Initialising MossRenderer...\n");

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
}

void MossRenderer::Shutdown(const char* why)
{
	if (!initialised)
	{
		return;
	}

	Con_Printf("MossRenderer::Shutdown: %s\n", why);

	initialised = false;
}

void MossRenderer::RenderFrame(const MossBlobVector& renderData)
{
	PushAttributes();

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);

	glDisable(GL_CULL_FACE);

	glUseProgram(gpuProgramHandle);
	glBindVertexArray(vertexArrayHandle);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mossTextureHandle);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	
	//for (const auto& blob : renderData)
	//{
	//	RenderMossBlob(blob);
	//}

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glEnable(GL_CULL_FACE);
	//glDisable(GL_ALPHA_TEST);

	PopAttributes();
}

void MossRenderer::RenderMossBlob(const MossBlob& blob)
{
	glDrawElements(GL_TRIANGLES, 4, GL_UNSIGNED_INT, nullptr);
}

bool MossRenderer::LoadShader()
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
	if (!loadShaderFromFile("admoss/shaders/mossVertexShader.glsl", vertexShaderCode))
	{
		return false;
	}

	std::string fragmentShaderCode;
	if (!loadShaderFromFile("admoss/shaders/mossPixelShader.glsl", fragmentShaderCode))
	{
		return false;
	}

	const char* vertexShaderString = vertexShaderCode.c_str();
	const char* fragmentShaderString = fragmentShaderCode.c_str();

	gpuProgramHandle = glCreateProgram();
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

	glAttachShader(gpuProgramHandle, vertexShaderHandle);
	glAttachShader(gpuProgramHandle, fragmentShaderHandle);
	glLinkProgram(gpuProgramHandle);

	glDeleteShader(vertexShaderHandle);
	glDeleteShader(fragmentShaderHandle);

	glUseProgram(gpuProgramHandle);

	GLuint diffuseMapHandle = glGetUniformLocation(gpuProgramHandle, "diffuseMap");
	glUniform1i(diffuseMapHandle, 0);
	
	glUseProgram(0);

	return true;
}

const char* MossRenderer::GetShaderError(GLuint vs, GLuint fs) const
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

bool MossRenderer::LoadTexture()
{
	int x, y, channels;
	auto* textureData = stbi_load("admoss/gfx/moss.png", &x, &y, &channels, 4);
	if (nullptr == textureData)
	{
		Con_Printf("Cannot load 'gfx/moss.png'\n");
		return false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &mossTextureHandle);
	GLError("CreateTexture: created texture object");

	glBindTexture(GL_TEXTURE_2D, mossTextureHandle);
	GLError("CreateTexture: bound texture object");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	if (GLError("CreateTexture: fed texture object"))
	{
		return false;
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	return true;
}

bool MossRenderer::LoadGeometry()
{
	static const float GeometryData[] =
	{
		// Bottom-left corner
		-1.0f, -1.0f, 0.0f,
		0.0f, 0.0f,
		// Bottom-right corner
		1.0f, -1.0f, 0.0f,
		1.0f, 0.0f,
		// Top-right corner
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f,
		// Top-left corner
		-1.0f, 1.0f, 0.0f,
		0.0f, 1.0f
	};

	static const uint32_t GeometryIndices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

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

void MossRenderer::PushAttributes()
{
	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
}

void MossRenderer::PopAttributes()
{
	glPopAttrib();
	glPopClientAttrib();
}
