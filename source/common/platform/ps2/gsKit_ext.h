#pragma once

#include <gsKit.h>
#include <gsInline.h>

void gsKit_prim_triangle_fan_gouraud_texture_stq_3d(GSGLOBAL* gsGlobal, GSTEXTURE* Texture, int count, const void* vertices);

static inline gs_xyz2 vertex_to_XYZ_clip(const GSGLOBAL *gsGlobal, float fx, float fy, int iz, bool clip)
{
	gs_xyz2 res;

	res.xyz.x = gsKit_float_to_int_x(gsGlobal, fx);
	res.xyz.y = gsKit_float_to_int_y(gsGlobal, fy);
	res.xyz.z = iz;
	res.tag = clip ? GS_XYZ3 : GS_XYZ2;

	return res;
}
