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

BEGIN_BLD_NS

enum DAMAGE_TYPE {
	kDamageFall = 0,
	kDamageBurn,
	kDamageBullet,
	kDamageExplode,
	kDamageDrown,
	kDamageSpirit,
	kDamageTesla,
	kDamageMax = 7,
};

enum VECTOR_TYPE {
	kVectorTine = 0,
	kVectorShell,
	kVectorBullet,
	kVectorTommyAP,
	kVectorShellAP,
	kVectorTommyRegular,
	kVectorBatBite,
	kVectorBoneelBite,
	kVectorGillBite,
	kVectorBeastSlash,
	kVectorAxe,
	kVectorCleaver,
	kVectorGhost,
	kVectorGargSlash,
	kVectorCerberusHack,
	kVectorHoundBite,
	kVectorRatBite,
	kVectorSpiderBite,
	VECTOR_TYPE_18,
	VECTOR_TYPE_19,
	kVectorTchernobogBurn,
	kVectorVoodoo10,
	#ifdef NOONE_EXTENSIONS
	kVectorGenDudePunch,
	#endif
	kVectorMax,
};

struct THINGINFO
{
	int16_t startHealth;
	int16_t mass;
	uint8_t clipdist;
	int16_t flags;
	int32_t elastic;
	int32_t dmgResist;
	ESpriteFlags cstat;
	int16_t picnum;
	int8_t shade;
	uint8_t pal;
	uint8_t xrepeat;
	uint8_t yrepeat;
	int dmgControl[kDamageMax]; // damage
	
	float fClipdist() const { return clipdist * 0.25; }
};

struct AMMOITEMDATA
{
	int16_t cstat;
	int16_t picnum;
	int8_t shade;
	uint8_t pal;
	uint8_t xrepeat;
	uint8_t yrepeat;
	int16_t count;
	uint8_t type;
	uint8_t weaponType;
};

struct WEAPONITEMDATA
{
	int16_t cstat;
	int16_t picnum;
	int8_t shade;
	uint8_t pal;
	uint8_t xrepeat;
	uint8_t yrepeat;
	int16_t type;
	int16_t ammoType;
	int16_t count;
};

struct ITEMDATA
{
	int16_t cstat;
	int16_t picnum;
	int8_t shade;
	uint8_t pal;
	uint8_t xrepeat;
	uint8_t yrepeat;
	int16_t packSlot;
};

struct MissileType
{
	int16_t picnum;
	int velocity;
	int angleOfs;
	uint8_t xrepeat;
	uint8_t yrepeat;
	int8_t shade;
	uint8_t clipDist;

	float fClipDist() const
	{
		return clipDist * 0.25;
	}
	float fVelocity() const
	{
		return FixedToFloat(velocity);
	}
};

struct EXPLOSION
{
	uint8_t repeat;
	uint8_t dmg;
	uint8_t dmgRng;
	int radius;
	int dmgType;
	int burnTime;
	int ticks;
	int quakeEffect;
	int flashEffect;
};

struct SURFHIT {
	FX_ID fx1;
	FX_ID fx2;
	FX_ID fx3;
	int fxSnd;
};

struct VECTORDATA {
	DAMAGE_TYPE dmgType;
	int dmg; // damage
	int impulse;
	int maxDist;
	int fxChance;
	int burnTime; // burn
	int bloodSplats; // blood splats
	int splatChance; // blood splat chance
	SURFHIT surfHit[15];

	float fMaxDist() const { return maxDist * maptoworld; }
};

extern const AMMOITEMDATA gAmmoItemData[];
extern const WEAPONITEMDATA gWeaponItemData[];
extern const ITEMDATA gItemData[];
extern const MissileType missileInfo[];
extern const EXPLOSION explodeInfo[];
extern const THINGINFO thingInfo[];
extern const VECTORDATA gVectorData[];

const int gDudeDrag = 0x2a00;

template<typename T> bool IsPlayerSprite(T const * const pSprite)
{
	return pSprite->type >= kDudePlayer1 && pSprite->type <= kDudePlayer8;
}

template<typename T> bool IsDudeSprite(T const * const pSprite)
{
	return pSprite->type >= kDudeBase && pSprite->type < kDudeMax;
}

template<typename T> bool IsThingSprite(T const* const pSprite)
{
	return pSprite->type >= kThingBase && pSprite->type < kThingMax;
}

template<typename T> bool IsItemSprite(T const * const pSprite)
{
	return pSprite->type >= kItemBase && pSprite->type < kItemMax;
}

template<typename T> bool IsWeaponSprite(T const * const pSprite)
{
	return pSprite->type >= kItemWeaponBase && pSprite->type < kItemWeaponMax;
}

template<typename T> bool IsAmmoSprite(T const * const pSprite)
{
	return pSprite->type >= kItemAmmoBase && pSprite->type < kItemAmmoMax;
}


#ifdef POLYMER
void actAddGameLight(int lightRadius, int spriteNum, int zOffset, int lightRange, int lightColor, int lightPrio);
void actDoLight(int spriteNum);
#endif

void FireballSeqCallback(int, int);
void sub_38938(int, int);
void NapalmSeqCallback(int, int);
void sub_3888C(int, int);
void TreeToGibCallback(int, int);

bool IsUnderwaterSector(sectortype* pSector);
void actInit(TArray<DBloodActor*>& actors);
void actWallBounceVector(DBloodActor* actor, walltype* pWall, float factor);
DVector4 actFloorBounceVector(DBloodActor* actor, float oldz, sectortype* pSector, float factor);
void actRadiusDamage(DBloodActor* source, const DVector3& pos, sectortype* pSector, int nDist, int a7, int a8, DAMAGE_TYPE a9, int a10, int a11);
DBloodActor *actDropObject(DBloodActor *pSprite, int nType);
bool actHealDude(DBloodActor* pXDude, int a2, int a3);
void actKillDude(DBloodActor* a1, DBloodActor* pSprite, DAMAGE_TYPE a3, int a4);
int actDamageSprite(DBloodActor* pSource, DBloodActor* pTarget, DAMAGE_TYPE damageType, int damage);
void actHitcodeToData(int a1, HitInfo *pHitInfo, DBloodActor **actor, walltype **a7 = nullptr);
void actAirDrag(DBloodActor *pSprite, fixed_t drag);
void actExplodeSprite(DBloodActor *pSprite);
void actActivateGibObject(DBloodActor *actor);
void actProcessSprites(void);
DBloodActor* actSpawnSprite(sectortype* pSector, const DVector3& pos, int nStat, bool a6);
DBloodActor* actSpawnDude(DBloodActor* pSource, int nType, float dist);
DBloodActor * actSpawnSprite(DBloodActor *pSource, int nStat);
DBloodActor* actSpawnThing(sectortype* pSector, const DVector3& pos, int nThingType);

DBloodActor* actFireThing(DBloodActor* actor, float xyoff, float zoff, float zvel, int thingType, float nSpeed);
DBloodActor* actFireMissile(DBloodActor* actor, float xyoff, float zoff, DVector3 dc, int nType);

void actBurnSprite(DBloodActor* pSource, DBloodActor* pTarget, int nTime);

int actGetRespawnTime(DBloodActor *pSprite);
bool actCheckRespawn(DBloodActor *pSprite);

void actFireVector(DBloodActor* shooter, float offset, float zoffset, DVector3 dv, VECTOR_TYPE vectorType, float nRange = -1);
void actPostSprite(DBloodActor* actor, int status);
void actPostProcess(void);
void MakeSplash(DBloodActor *actor);
void actBuildMissile(DBloodActor* spawned, DBloodActor* actor);

extern const int16_t DudeDifficulty[];


END_BLD_NS
