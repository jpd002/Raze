#include "gs_framebuffer.h"
#include "v_draw.h"
#include "gs_buffers.h"
#include "flatvertices.h"
#include "hw_skydome.h"
#include "hw_viewpointbuffer.h"
#include "hw_lightbuffer.h"
#include "hw_bonebuffer.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 448

GsFrameBuffer::GsFrameBuffer()
	: DFrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT)
	, m_gsContext(gsKit_init_global_custom(GS_RENDER_QUEUE_OS_POOLSIZE, GS_RENDER_QUEUE_PER_POOLSIZE))
	, m_renderState(m_gsContext)
{
}

void GsFrameBuffer::InitializeState()
{
	dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_GIF);

	gsKit_mode_switch(m_gsContext, GS_ONESHOT);
	gsKit_init_screen(m_gsContext);

	SetViewportRects(nullptr);

	mVertexData = new FFlatVertexBuffer(GetWidth(), GetHeight(), screen->mPipelineNbr);
	mSkyData = new FSkyVertexBuffer;
	mViewpoints = new HWViewpointBuffer(screen->mPipelineNbr);
	mLights = new FLightBuffer(screen->mPipelineNbr);
	mBones = new BoneBuffer(screen->mPipelineNbr);
}

bool GsFrameBuffer::IsFullscreen()
{
	return true;
}

int GsFrameBuffer::GetClientWidth()
{
	return SCREEN_WIDTH;
}

int GsFrameBuffer::GetClientHeight()
{
	return SCREEN_HEIGHT;
}

IVertexBuffer* GsFrameBuffer::CreateVertexBuffer()
{
	return new GsVertexBuffer();
}

IIndexBuffer* GsFrameBuffer::CreateIndexBuffer()
{
	return new GsIndexBuffer();
}

IDataBuffer* GsFrameBuffer::CreateDataBuffer(int bindingpoint, bool ssbo, bool needsresize)
{
	return new GsDataBuffer();
}

void GsFrameBuffer::Update()
{
	gsKit_clear(m_gsContext, GS_SETREG_RGBAQ(0, 0, 0, 0, 0));

	::Draw2D(twod, m_renderState);

	gsKit_queue_exec(m_gsContext);
	FPSLimit();
	gsKit_sync_flip(m_gsContext);
	Super::Update();
}
