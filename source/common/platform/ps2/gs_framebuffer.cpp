#include "gs_framebuffer.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 448

GsFrameBuffer::GsFrameBuffer()
	: DFrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT)
{
}

void GsFrameBuffer::InitializeState()
{
	m_gsContext = gsKit_init_global();
	
	dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_GIF);

	gsKit_mode_switch(m_gsContext, GS_ONESHOT);
	gsKit_init_screen(m_gsContext);

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
void GsFrameBuffer::Update()
{
	printf("Updating.\r\n");

	gsKit_clear(m_gsContext, GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x00,0x00));

	//::Draw2D(twod, m_renderState);

	gsKit_queue_exec(m_gsContext);
	gsKit_sync_flip(m_gsContext);
}
