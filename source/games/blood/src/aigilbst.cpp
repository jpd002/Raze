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

#include "ns.h"	// Must come before everything else!

#include "build.h"

#include "blood.h"

BEGIN_BLD_NS

static void gillThinkSearch(DBloodActor*);
static void gillThinkGoto(DBloodActor*);
static void gillThinkChase(DBloodActor*);
static void gillThinkSwimGoto(DBloodActor*);
static void gillThinkSwimChase(DBloodActor*);
static void sub_6CB00(DBloodActor*);
static void sub_6CD74(DBloodActor*);
static void sub_6D03C(DBloodActor*);


AISTATE gillBeastIdle = { kAiStateIdle, 0, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE gillBeastChase = { kAiStateChase, 9, -1, 0, NULL, aiMoveForward, gillThinkChase, NULL };
AISTATE gillBeastDodge = { kAiStateMove, 9, -1, 90, NULL, aiMoveDodge, NULL, &gillBeastChase };
AISTATE gillBeastGoto = { kAiStateMove, 9, -1, 600, NULL, aiMoveForward, gillThinkGoto, &gillBeastIdle };
AISTATE gillBeastBite = { kAiStateChase, 6, nGillBiteClient, 120, NULL, NULL, NULL, &gillBeastChase };
AISTATE gillBeastSearch = { kAiStateMove, 9, -1, 120, NULL, aiMoveForward, gillThinkSearch, &gillBeastIdle };
AISTATE gillBeastRecoil = { kAiStateRecoil, 5, -1, 0, NULL, NULL, NULL, &gillBeastDodge };
AISTATE gillBeastSwimIdle = { kAiStateIdle, 10, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE gillBeastSwimChase = { kAiStateChase, 10, -1, 0, NULL, sub_6CB00, gillThinkSwimChase, NULL };
AISTATE gillBeastSwimDodge = { kAiStateMove, 10, -1, 90, NULL, aiMoveDodge, NULL, &gillBeastSwimChase };
AISTATE gillBeastSwimGoto = { kAiStateMove, 10, -1, 600, NULL, aiMoveForward, gillThinkSwimGoto, &gillBeastSwimIdle };
AISTATE gillBeastSwimSearch = { kAiStateSearch, 10, -1, 120, NULL, aiMoveForward, gillThinkSearch, &gillBeastSwimIdle };
AISTATE gillBeastSwimBite = { kAiStateChase, 7, nGillBiteClient, 0, NULL, NULL, gillThinkSwimChase, &gillBeastSwimChase };
AISTATE gillBeastSwimRecoil = { kAiStateRecoil, 5, -1, 0, NULL, NULL, NULL, &gillBeastSwimDodge };
AISTATE gillBeast13A138 = { kAiStateOther, 10, -1, 120, NULL, sub_6CD74, gillThinkSwimChase, &gillBeastSwimChase };
AISTATE gillBeast13A154 = { kAiStateOther, 10, -1, 0, NULL, sub_6D03C, gillThinkSwimChase, &gillBeastSwimChase };
AISTATE gillBeast13A170 = { kAiStateOther, 10, -1, 120, NULL, NULL, aiMoveTurn, &gillBeastSwimChase };

void GillBiteSeqCallback(int, DBloodActor* actor)
{
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();
	DVector3 vec(actor->spr.Angles.Yaw.ToVector() * 64, actor->spr.pos.Z - target->spr.pos.Z);
	vec.X += Random3F(2000, 8);
	vec.Y += Random3F(2000, 8);
	actFireVector(actor, 0, 0, vec, kVectorGillBite);
	actFireVector(actor, 0, 0, vec, kVectorGillBite);
	actFireVector(actor, 0, 0, vec, kVectorGillBite);
}

static void gillThinkSearch(DBloodActor* actor)
{
	aiChooseDirection(actor, actor->xspr.goalAng);
	aiThinkTarget(actor);
}

static void gillThinkGoto(DBloodActor* actor)
{
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);

	auto pSector = actor->sector();
	auto pXSector = pSector->hasX() ? &pSector->xs() : nullptr;

	auto dvec = actor->xspr.TargetPos.XY() - actor->spr.pos.XY();
	DAngle nAngle = dvec.Angle();
	float nDist = dvec.Length();
	aiChooseDirection(actor, nAngle);
	if (nDist < 32 && absangle(actor->spr.Angles.Yaw, nAngle) < pDudeInfo->Periphery())
	{
		if (pXSector && pXSector->Underwater)
			aiNewState(actor, &gillBeastSwimSearch);
		else
			aiNewState(actor, &gillBeastSearch);
	}
	aiThinkTarget(actor);
}

static void gillThinkChase(DBloodActor* actor)
{
	auto pSector = actor->sector();
	auto pXSector = pSector->hasX() ? &pSector->xs() : nullptr;

	if (actor->GetTarget() == nullptr)
	{
		if (pXSector && pXSector->Underwater)
			aiNewState(actor, &gillBeastSwimSearch);
		else
			aiNewState(actor, &gillBeastSearch);
		return;
	}
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();
	auto dv = target->spr.pos.XY() - actor->spr.pos.XY();
	DAngle nAngle = dv.Angle();
	aiChooseDirection(actor, nAngle);
	if (actor->xspr.health == 0)
	{
		if (pXSector && pXSector->Underwater)
			aiNewState(actor, &gillBeastSwimSearch);
		else
			aiNewState(actor, &gillBeastSearch);
		return;
	}
	if (target->IsPlayerActor() && powerupCheck(&gPlayer[target->spr.type - kDudePlayer1], kPwUpShadowCloak) > 0)
	{
		if (pXSector && pXSector->Underwater)
			aiNewState(actor, &gillBeastSwimSearch);
		else
			aiNewState(actor, &gillBeastSearch);
		return;
	}
	float nDist = dv.Length();
	if (nDist <= pDudeInfo->SeeDist())
	{
		DAngle nDeltaAngle = absangle(actor->spr.Angles.Yaw, nAngle);
		float height = (pDudeInfo->eyeHeight * actor->spr.scale.Y);
		if (cansee(target->spr.pos, target->sector(), actor->spr.pos.plusZ(-height), actor->sector()))
		{
			if (nDist < pDudeInfo->SeeDist() && nDeltaAngle <= pDudeInfo->Periphery())
			{
				aiSetTarget(actor, actor->GetTarget());
				actor->dudeSlope = nDist == 0 ? 0 : (target->spr.pos.Z - actor->spr.pos.Z) / nDist;
				if (nDist < 57.5625 && abs(nDeltaAngle) < DAngle1 * 5)
				{
					int hit = HitScan(actor, actor->spr.pos.Z, DVector3(dv, 0), CLIPMASK1, 0);
					switch (hit)
					{
					case -1:
						if (pXSector && pXSector->Underwater)
							aiNewState(actor, &gillBeastSwimBite);
						else
							aiNewState(actor, &gillBeastBite);
						break;
					case 3:
						if (actor->spr.type != gHitInfo.actor()->spr.type)
						{
							if (pXSector && pXSector->Underwater)
								aiNewState(actor, &gillBeastSwimBite);
							else
								aiNewState(actor, &gillBeastBite);
						}
						else
						{
							if (pXSector && pXSector->Underwater)
								aiNewState(actor, &gillBeastSwimDodge);
							else
								aiNewState(actor, &gillBeastDodge);
						}
						break;
					default:
						if (pXSector && pXSector->Underwater)
							aiNewState(actor, &gillBeastSwimBite);
						else
							aiNewState(actor, &gillBeastBite);
						break;
					}
				}
			}
			return;
		}
	}

	if (pXSector && pXSector->Underwater)
		aiNewState(actor, &gillBeastSwimGoto);
	else
		aiNewState(actor, &gillBeastGoto);
	sfxPlay3DSound(actor, 1701, -1, 0);
	actor->SetTarget(nullptr);
}

static void gillThinkSwimGoto(DBloodActor* actor)
{
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	auto dvec = actor->xspr.TargetPos.XY() - actor->spr.pos.XY();
	DAngle nAngle = dvec.Angle();
	float nDist = dvec.Length();
	aiChooseDirection(actor, nAngle);
	if (nDist < 32 && absangle(actor->spr.Angles.Yaw, nAngle) < pDudeInfo->Periphery())
		aiNewState(actor, &gillBeastSwimSearch);
	aiThinkTarget(actor);
}

static void gillThinkSwimChase(DBloodActor* actor)
{
	if (actor->GetTarget() == nullptr)
	{
		aiNewState(actor, &gillBeastSwimSearch);
		return;
	}
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();
	auto dvec = target->spr.pos.XY() - actor->spr.pos.XY();
	DAngle nAngle = dvec.Angle();
	float nDist = dvec.Length();
	aiChooseDirection(actor, nAngle);
	
	if (actor->xspr.health == 0)
	{
		aiNewState(actor, &gillBeastSwimSearch);
		return;
	}
	if (target->IsPlayerActor() && powerupCheck(&gPlayer[target->spr.type - kDudePlayer1], kPwUpShadowCloak) > 0)
	{
		aiNewState(actor, &gillBeastSwimSearch);
		return;
	}

	if (nDist <= pDudeInfo->SeeDist())
	{
		DAngle nDeltaAngle = absangle(actor->spr.Angles.Yaw, nAngle);
		float height = (pDudeInfo->eyeHeight * actor->spr.scale.Y);
		float top, bottom;
		GetActorExtents(actor, &top, &bottom);
		if (cansee(target->spr.pos, target->sector(), actor->spr.pos.plusZ(-height), actor->sector()))
		{
			if (nDist < pDudeInfo->SeeDist() && nDeltaAngle <= pDudeInfo->Periphery())
			{
				aiSetTarget(actor, actor->GetTarget());
				if (nDist < 0x40 && nDeltaAngle < DAngle15)
					aiNewState(actor, &gillBeastSwimBite);
				else
				{
					aiPlay3DSound(actor, 1700, AI_SFX_PRIORITY_1, -1);
					aiNewState(actor, &gillBeast13A154);
				}
			}
		}
		else
			aiNewState(actor, &gillBeast13A154);
		return;
	}
	aiNewState(actor, &gillBeastSwimGoto);
	actor->SetTarget(nullptr);
}

static void sub_6CB00(DBloodActor* actor)
{
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	auto nAng = deltaangle(actor->spr.Angles.Yaw, actor->xspr.goalAng);
	auto nTurnRange = pDudeInfo->TurnRange();
	actor->spr.Angles.Yaw += clamp(nAng, -nTurnRange, nTurnRange);
	float nAccel = FixedToFloat((pDudeInfo->frontSpeed - (((4 - gGameOptions.nDifficulty) << 27) / 120) / 120) << 2);
	if (abs(nAng) > DAngle60)
		return;
	if (actor->GetTarget() == nullptr)
		actor->spr.Angles.Yaw += DAngle45;
	auto dvec = actor->xspr.TargetPos.XY() - actor->spr.pos.XY();
	float nDist = dvec.Length();
	if (Random(64) < 32 && nDist <= 0x40)
		return;
	AdjustVelocity(actor, ADJUSTER{
		if (actor->GetTarget() == nullptr)
			t1 += nAccel;
		else
			t1 += nAccel * 0.25;
	});

}

static void sub_6CD74(DBloodActor* actor)
{
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();

	auto nAng = deltaangle(actor->spr.Angles.Yaw, actor->xspr.goalAng);
	auto nTurnRange = pDudeInfo->TurnRange();
	actor->spr.Angles.Yaw += clamp(nAng, -nTurnRange, nTurnRange);
	float nAccel = FixedToFloat((pDudeInfo->frontSpeed - (((4 - gGameOptions.nDifficulty) << 27) / 120) / 120) << 2);
	if (abs(nAng) > DAngle60)
	{
		actor->xspr.goalAng += DAngle90;
		return;
	}
	auto dvec = actor->xspr.TargetPos.XY() - actor->spr.pos.XY();
	float nDist = dvec.Length();
	if (Chance(0x600) && nDist <= 0x40)
		return;
	AdjustVelocity(actor, ADJUSTER{
		t1 += nAccel;
	});

	actor->vel.Z = -(target->spr.pos.Z - actor->spr.pos.Z) / 256.;
}

static void sub_6D03C(DBloodActor* actor)
{
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();

	auto nAng = deltaangle(actor->spr.Angles.Yaw, actor->xspr.goalAng);
	auto nTurnRange = pDudeInfo->TurnRange();
	actor->spr.Angles.Yaw += clamp(nAng, -nTurnRange, nTurnRange);
	float nAccel = FixedToFloat((pDudeInfo->frontSpeed - (((4 - gGameOptions.nDifficulty) << 27) / 120) / 120) << 2);
	if (abs(nAng) > DAngle60)
	{
		actor->spr.Angles.Yaw += DAngle90;
		return;
	}
	auto dvec = actor->xspr.TargetPos.XY() - actor->spr.pos.XY();
	float nDist = dvec.Length();
	if (Chance(0x4000) && nDist <= 0x40)
		return;
	AdjustVelocity(actor, ADJUSTER{
		t1 += nAccel * 0.5;
	});

	actor->vel.Z = (target->spr.pos.Z - actor->spr.pos.Z) / 32.;
}

END_BLD_NS
