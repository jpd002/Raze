#pragma once

#include <gsKit.h>
#include "hw_renderstate.h"

class GsRenderState : public FRenderState
{
public:
	GsRenderState(GSGLOBAL*);
	virtual ~GsRenderState() = default;

	void ClearScreen() override;
	void Draw(int dt, int index, int count, bool apply = true) override;
	void DrawIndexed(int dt, int index, int count, bool apply = true) override;

	bool SetDepthClamp(bool on) override;
	void SetDepthMask(bool on) override;
	void SetDepthFunc(int func) override;
	void SetDepthRange(float min, float max) override;
	void SetColorMask(bool r, bool g, bool b, bool a) override;
	void SetStencil(int offs, int op, int flags=-1) override;
	void SetCulling(int mode) override;
	void EnableClipDistance(int num, bool state) override;
	void Clear(int targets) override;
	void EnableStencil(bool on) override;
	void SetScissor(int x, int y, int w, int h) override;
	void SetViewport(int x, int y, int w, int h) override;
	void EnableDepthTest(bool on) override;
	void EnableMultisampling(bool on) override;
	void EnableLineSmooth(bool on) override;
	void EnableDrawBuffers(int count, bool apply = false) override;

	void BindUniformBuffer(int bindingPoint, uint8_t* buffer, size_t bufferSize);

private:
	GSGLOBAL* m_gsContext = nullptr;
	uint8_t* m_viewpointBuffer = nullptr;
	size_t m_viewpointBufferSize = 0;
};
