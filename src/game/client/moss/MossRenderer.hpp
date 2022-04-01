
#pragma once

// GLM by default produces projection matrices that have depth -1 to 1
// GoldSRC uses 0 to 1
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

class MossRenderer final : public IMossRenderer
{
public:
	static IMossRenderer* GetInstance();

	void Init() override;
	void Shutdown(const char* why) override;
	bool Okay() const
	{
		return initialised;
	}

	void RenderFrame(const MossBlobVector& renderData) override;
	bool ReloadShaders() override;

private:
	void SetupMatrices();
	void RenderMossBlob(const MossBlob& blob);

	bool LoadShader();
	const char* GetShaderError(GLuint vs, GLuint fs) const;
	bool LoadTexture();
	bool LoadGeometry();

	void PushAttributes();
	void PopAttributes();

	GLuint vertexArrayHandle{0};
	GLuint vertexBufferHandle{0};
	GLuint indexBufferHandle{0};

	GLuint mossTextureHandle{0};

	GLuint gpuProgramHandle{0};

	GLuint viewMatrixHandle;
	GLuint projectionMatrixHandle;
	GLuint modelMatrixHandle;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	bool initialised{false};
};
