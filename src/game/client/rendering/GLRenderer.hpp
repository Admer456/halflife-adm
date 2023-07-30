
#pragma once

// GLM by default produces projection matrices that have depth -1 to 1
// GoldSRC uses 0 to 1
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

// TEMPORARY
// A simple quad that uses a hardcoded texture and gets rendered
struct RenderQuad
{
	Vector position{0.0f, 0.0f, 0.0f};
	Vector normal{0.0f, 0.0f, 1.0f};
	float scale{1.0f};
	float angle{0.0f}; // rotation around the normal

	glm::mat4 modelMatrix;

	void SetupModelMatrix();
};

class GLRenderer final
{
public:
	static GLRenderer* GetInstance();

	void Init();
	void Shutdown(const char* why);
	bool Okay() const
	{
		return initialised;
	}

	void RenderFrame();
	bool ReloadShaders();

private:
	void RenderQuadInstance(const RenderQuad& quad);
	void SetupMatrices();

	bool LoadShader();
	const char* GetShaderError(GLuint vs, GLuint fs) const;
	bool LoadTexture();
	bool LoadGeometry();

	void PushAttributes();
	void PopAttributes();

	GLuint vertexArrayHandle{};
	GLuint vertexBufferHandle{};
	GLuint indexBufferHandle{};

	GLuint waterTextureHandle{};
	GLuint waterShaderHandle{};

	GLuint timeHandle{};
	GLuint viewMatrixHandle{};
	GLuint projectionMatrixHandle{};
	GLuint modelMatrixHandle{};
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	bool initialised{false};
};
