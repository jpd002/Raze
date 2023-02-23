//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#pragma once

#include "m_fixed.h"
#include "filesystem.h"
#include "texinfo.h"

#include "buildtiles.h"

BEGIN_BLD_NS

class DBloodActor;
using HitInfo = THitInfo<DBloodActor>;
using Collision = TCollision<DBloodActor>;

void playlogos();
unsigned int qrand(void);
int wrand(void);
void wsrand(int);

void FireInit(void);
void FireProcess(void);
void UpdateNetworkMenus(void);
void InitMirrors(void);
void setPortalFlags(int mode);
void processSpritesOnOtherSideOfPortal(int x, int y, int interpolation);
void DrawMirrors(int x, int y, int z, fixed_t a, fixed_t horiz, int smooth, int viewPlayer);
int qanimateoffs(int a1, int a2);

struct PLAYER;

bool checkLitSprayOrTNT(PLAYER* pPlayer);
void WeaponInit(void);
void WeaponDraw(PLAYER* pPlayer, int shade, float xpos, float ypos, int palnum, DAngle angle);
void WeaponRaise(PLAYER* pPlayer);
void WeaponLower(PLAYER* pPlayer);
int WeaponUpgrade(PLAYER* pPlayer, int newWeapon);
void WeaponProcess(PLAYER* pPlayer);
void WeaponUpdateState(PLAYER* pPlayer);
void teslaHit(DBloodActor* pMissile, int a2);
void WeaponPrecache();

struct ZONE {
	DVector3 pos;
	sectortype* sector;
	DAngle angle;
};
extern ZONE gStartZone[8];

void warpInit(TArray<DBloodActor*>& actors);
int CheckLink(DBloodActor* pSprite);
int CheckLink(DVector3& cPos, sectortype** pSector);

#include "m_fixed.h"

enum SurfaceType {
	kSurfNone = 0,
	kSurfStone,
	kSurfMetal,
	kSurfWood,
	kSurfFlesh,
	kSurfWater,
	kSurfDirt,
	kSurfClay,
	kSurfSnow,
	kSurfIce,
	kSurfLeaves,
	kSurfCloth,
	kSurfPlant,
	kSurfGoo,
	kSurfLava,
	kSurfMax
};

extern int nPrecacheCount;
inline FTextureID mirrortile;

void tilePrecacheTile(int nTile, int nType, int palette);
void tilePrecacheTile(FTextureID nTile, int nType, int palette);

int tileGetSurfType(CollisionBase& hit);

END_BLD_NS
