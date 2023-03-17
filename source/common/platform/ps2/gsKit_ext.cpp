#include "gskit_ext.h"
#include <gsInline.h>

static inline u32 lzw(u32 val)
{
	u32 res;
	__asm__ __volatile__ ("   plzcw   %0, %1    " : "=r" (res) : "r" (val));
	return(res);
}

static inline void gsKit_set_tw_th(const GSTEXTURE *Texture, int *tw, int *th)
{
	*tw = 31 - (lzw(Texture->Width) + 1);
	if(Texture->Width > (1<<*tw))
		(*tw)++;

	*th = 31 - (lzw(Texture->Height) + 1);
	if(Texture->Height > (1<<*th))
		(*th)++;
}

void gsKit_prim_triangle_fan_gouraud_texture_stq_3d(GSGLOBAL *gsGlobal, GSTEXTURE *Texture, int count, const void *vertices)
{
	u64* p_data;
	u64* p_store;
	int tw, th;

	int qsize = (count*3) + 4;
	int bytes = count * sizeof(GSPRIMSTQPOINT);

	gsKit_set_texfilter(gsGlobal, Texture->Filter);
	gsKit_set_tw_th(Texture, &tw, &th);

	p_store = p_data = reinterpret_cast<u64*>(gsKit_heap_alloc(gsGlobal, qsize, (qsize*16), GIF_AD));

	*p_data++ = GIF_TAG_AD(qsize);
	*p_data++ = GIF_AD;

	if(p_store == gsGlobal->CurQueue->last_tag)
	{
		*p_data++ = GIF_TAG_TRIANGLE_GORAUD_TEXTURED(count - 1);
		*p_data++ = GIF_TAG_TRIANGLE_GORAUD_TEXTURED_STQ_REGS(gsGlobal->PrimContext);
	}

	if(Texture->VramClut == 0)
	{
		*p_data++ = GS_SETREG_TEX0(Texture->Vram/256, Texture->TBW, Texture->PSM,
			tw, th, gsGlobal->PrimAlphaEnable, 0,
			0, 0, 0, 0, GS_CLUT_STOREMODE_NOLOAD);
	}
	else
	{
		*p_data++ = GS_SETREG_TEX0(Texture->Vram/256, Texture->TBW, Texture->PSM,
			tw, th, gsGlobal->PrimAlphaEnable, 0,
			Texture->VramClut/256, Texture->ClutPSM, 0, 0, GS_CLUT_STOREMODE_LOAD);
	}
	*p_data++ = GS_TEX0_1 + gsGlobal->PrimContext;

	*p_data++ = GS_SETREG_PRIM( GS_PRIM_PRIM_TRIFAN, 1, 1, gsGlobal->PrimFogEnable,
				gsGlobal->PrimAlphaEnable, gsGlobal->PrimAAEnable,
				0, gsGlobal->PrimContext, 0);
	
	*p_data++ = GS_PRIM;

	memcpy(p_data, vertices, bytes);
}
