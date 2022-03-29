
#pragma once

class MossRenderer final : public IMossRenderer
{
public:
	void Init() override;
	void Shutdown(const char* why) override;

	void RenderFrame() override;

private:
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

	bool initialised{false};
};
