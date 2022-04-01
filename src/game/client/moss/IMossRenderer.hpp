
#pragma once

class IMossRenderer
{
public:
	virtual void Init() = 0;
	virtual void Shutdown(const char* why) = 0;
	virtual bool Okay() const = 0;

	virtual void RenderFrame(const MossBlobVector& renderData) = 0;
	virtual bool ReloadShaders() = 0;
};
