
#pragma once

class IMossRenderer
{
public:
	virtual void Init() = 0;
	virtual void Shutdown(const char* why) = 0;

	virtual void SetRenderData() = 0;
	virtual void RenderFrame() = 0;
};
