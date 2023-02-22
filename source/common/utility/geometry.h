#pragma once

/*
** geometry.h
** basic geometry math routines
**
**---------------------------------------------------------------------------
** Copyright 2005-2022 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
*/

#include "vectors.h"

inline DVector2 rotatepoint(const DVector2& pivot, const DVector2& point, DAngle angle)
{
	return (point - pivot).Rotated(angle) + pivot;
}

//==========================================================================
//
// 
//
//==========================================================================

inline float PointOnLineSide(float x, float y, float linex, float liney, float deltax, float deltay)
{
	return (x - linex) * deltay - (y - liney) * deltax;
}

//==========================================================================
//
// 
//
//==========================================================================

inline float SquareDist(float lx1, float ly1, float lx2, float ly2)
{
	float dx = lx2 - lx1;
	float dy = ly2 - ly1;
	return dx * dx + dy * dy;
}

// This is for cases where only the factor is needed, and pre-validation was performed.
inline float NearestPointOnLineFast(float px, float py, float lx1, float ly1, float lx2, float ly2)
{
	float wall_length = SquareDist(lx1, ly1, lx2, ly2);
	assert(wall_length > 0);
	return ((px - lx1) * (lx2 - lx1) + (py - ly1) * (ly2 - ly1)) / wall_length;
}


inline DVector2 NearestPointOnLine(float px, float py, float lx1, float ly1, float lx2, float ly2, bool clamp = true)
{
	float wall_length = SquareDist(lx1, ly1, lx2, ly2);

	if (wall_length == 0)
	{
		return { lx1, ly1 };
	}

	float t = ((px - lx1) * (lx2 - lx1) + (py - ly1) * (ly2 - ly1)) / wall_length;
	if (clamp)
	{
		if (t <= 0) return { lx1, ly1 };
		if (t >= 1) return { lx2, ly2 };
	}
	float xx = lx1 + t * (lx2 - lx1);
	float yy = ly1 + t * (ly2 - ly1);
	return { xx, yy };
}

//==========================================================================
//
// 
//
//==========================================================================

inline float SquareDistToLine(float px, float py, float lx1, float ly1, float lx2, float ly2)
{
	float wall_length = SquareDist(lx1, ly1, lx2, ly2);

	if (wall_length == 0) return SquareDist(px, py, lx1, ly1);

	float t = ((px - lx1) * (lx2 - lx1) + (py - ly1) * (ly2 - ly1)) / wall_length;
	t = clamp(t, 0.f, 1.f);
	float xx = lx1 + t * (lx2 - lx1);
	float yy = ly1 + t * (ly2 - ly1);
	return SquareDist(px, py, xx, yy);
}

//==========================================================================
//
// taken from GZDoom with the divline_t parameters removed
//
//==========================================================================

inline float InterceptVector(float v2x, float v2y, float v2dx, float v2dy, float v1x, float v1y, float v1dx, float v1dy)
{
	float den = v1dy * v2dx - v1dx * v2dy;

	if (den == 0)
		return 0;		// parallel

	float num = (v1x - v2x) * v1dy + (v2y - v1y) * v1dx;
	return num / den;
}

//==========================================================================
//
// Essentially two InterceptVector calls. We can reduce the calculations 
// because the denominators for both calculations only differ by their sign.
//
//==========================================================================

inline float InterceptLineSegments(float v2x, float v2y, float v2dx, float v2dy, float v1x, float v1y, float v1dx, float v1dy, float* pfactor1 = nullptr, bool forcansee = false)
{
	float den = v1dy * v2dx - v1dx * v2dy;

	if (den == 0)
		return -2 * (float)FLT_MAX;		// parallel (return a magic value different from everything else, just in case it needs to be handled)

	if (forcansee && den < 0)  // cansee does this added check here, aside from that its logic is virtually the same.
		return -1;		// hitting the backside

	// perform the division first for better parallelization.
	den = 1 / den;

	float factor1 = ((v2x - v1x) * v2dy + (v1y - v2y) * v2dx) * -den;
	if (factor1 < 0 || factor1 > 1) return -FLT_MAX; // no intersection
	if (pfactor1) *pfactor1 = factor1;

	return ((v1x - v2x) * v1dy + (v2y - v1y) * v1dx) * den; // this one's for the line segment where we want to get the intercept factor for so it needs to be last.
}

//==========================================================================
//
// calculates intersection between a plane and line in 3D
//
//==========================================================================

inline float LinePlaneIntersect(const DVector3& start, const DVector3& trace, const DVector3& ppoint, const DVector3& pvec1, const DVector3& pvec2)
{
	auto normal = pvec1 ^ pvec2; // we do not need a unit vector here.
	float dist = normal.dot(ppoint);
	float dotStart = normal.dot(start);
	float dotTrace = normal.dot(trace);
	if (dotTrace == 0) return -FLT_MAX;
	return (dist - dotStart) / dotTrace; // we are only interested in the factor
}

//==========================================================================
//
// BoxOnLineSide
//
// Based on Doom's, but rewritten to be standalone
// 
//==========================================================================

inline int BoxOnLineSide(const DVector2& boxtl, const DVector2& boxbr, const DVector2& start, const DVector2& delta)
{
	int p1;
	int p2;

	if (delta.X == 0)
	{
		p1 = boxbr.X < start.X;
		p2 = boxtl.X < start.X;
		if (delta.Y < 0)
		{
			p1 ^= 1;
			p2 ^= 1;
		}
	}
	else if (delta.Y == 0)
	{
		p1 = boxtl.Y > start.Y;
		p2 = boxbr.Y > start.Y;
		if (delta.X < 0)
		{
			p1 ^= 1;
			p2 ^= 1;
		}
	}
	else if (delta.X * delta.Y <= 0)
	{
		p1 = PointOnLineSide(boxtl.X, boxtl.Y, start.X, start.Y, delta.X, delta.Y) > 0;
		p2 = PointOnLineSide(boxbr.X, boxbr.Y, start.X, start.Y, delta.X, delta.Y) > 0;
	}
	else
	{
		p1 = PointOnLineSide(boxbr.X, boxtl.Y, start.X, start.Y, delta.X, delta.Y) > 0;
		p2 = PointOnLineSide(boxtl.X, boxbr.Y, start.X, start.Y, delta.X, delta.Y) > 0;
	}

	return (p1 == p2) ? p1 : -1;
}

//==========================================================================
//
// BoxInRange
//
//==========================================================================

inline bool BoxInRange(const DVector2& boxtl, const DVector2& boxbr, const DVector2& start, const DVector2& end)
{
	return boxtl.X < max(start.X, end.X) &&
		boxbr.X > min(start.X, end.X) &&
		boxtl.Y < max(start.Y, end.Y) &&
		boxbr.Y > min(start.Y, end.Y);
}
