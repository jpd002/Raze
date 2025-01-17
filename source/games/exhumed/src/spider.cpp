//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
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
#include "ns.h"
#include "aistuff.h"
#include "exhumed.h"
#include "engine.h"
#include "sequence.h"
#include "sound.h"
#include <assert.h>

BEGIN_PS_NS

static actionSeq SpiderSeq[] = {
    {16, 0},
    {8,  0},
    {32, 0},
    {24, 0},
    {0,  0},
    {40, 1},
    {41, 1}
};


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

DExhumedActor* BuildSpider(DExhumedActor* spp, const DVector3& pos, sectortype* pSector, DAngle nAngle)
{
    if (spp == nullptr)
    {
        spp = insertActor(pSector, 99);
		spp->spr.pos = pos;
    }
    else
    {
        ChangeActorStat(spp, 99);

        spp->spr.pos.Z = spp->sector()->floorz;
        nAngle = spp->spr.Angles.Yaw;
    }

    spp->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;
    spp->spr.shade = -12;
	spp->clipdist = 3.75;
    spp->vel.X = 0;
    spp->vel.Y = 0;
    spp->vel.Z = 0;
    spp->spr.scale = DVector2(0.625, 0.625);
    spp->spr.pal = spp->sector()->ceilingpal;
    spp->spr.xoffset = 0;
    spp->spr.yoffset = 0;
    spp->spr.Angles.Yaw = nAngle;
    spp->spr.picnum = 1;
    spp->spr.hitag = 0;
    spp->spr.lotag = runlist_HeadRun() + 1;
    spp->spr.extra = -1;

    //	GrabTimeSlot(3);

    spp->nAction = 0;
    spp->nFrame = 0;
    spp->pTarget = nullptr;
    spp->nHealth = 160;
    spp->nPhase = Counters[kCountSpider]++;

    spp->spr.intowner = runlist_AddRunRec(spp->spr.lotag - 1, spp, 0xC0000);

    spp->nRun = runlist_AddRunRec(NewRun, spp, 0xC0000);

    nCreaturesTotal++;

    return spp;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AISpider::Tick(RunListEvent* ev)
{
    auto spp = ev->pObjActor;
    if (!spp) return;

    int nAction = spp->nAction;

    double nVel = 0.25;

    if (spp->nHealth)
    {
        if (spp->spr.cstat & CSTAT_SPRITE_YFLIP)
        {
            spp->spr.pos.Z = spp->sector()->ceilingz + GetActorHeight(spp);
        }
        else
        {
            Gravity(spp);
        }
    }

    int nSeq = SeqOffsets[kSeqSpider] + SpiderSeq[nAction].a;

    spp->spr.picnum = seq_GetSeqPicnum2(nSeq, spp->nFrame);

    seq_MoveSequence(spp, nSeq, spp->nFrame);

    int nFrameFlag = FrameFlag[SeqBase[nSeq] + spp->nFrame];

    spp->nFrame++;
    if (spp->nFrame >= SeqSize[nSeq]) {
        spp->nFrame = 0;
    }

    DExhumedActor* pTarget = spp->pTarget;

    if (pTarget == nullptr || pTarget->spr.cstat & CSTAT_SPRITE_BLOCK_ALL)
    {
        switch (nAction)
        {
        default:
            return;

        case 0:
        {
            if ((spp->nPhase & 0x1F) == (totalmoves & 0x1F))
            {
                if (pTarget == nullptr) {
                    pTarget = FindPlayer(spp, 100);
                }

                if (pTarget)
                {
                    spp->nAction = 1;
                    spp->nFrame = 0;
                    spp->pTarget = pTarget;

					spp->VelFromAngle();
                    return;
                }
            }

            break;
        }
        case 1:
        {
            if (pTarget) {
                nVel *= 2;
            }
            goto case_3;
            break;
        }
        case 4:
        {
            if (!spp->nFrame)
            {
                spp->nFrame = 0;
                spp->nAction = 1;
            }
            [[fallthrough]];
        }
        case 3:
        {
        case_3:
            auto pSector =spp->sector();

            if (spp->spr.cstat & CSTAT_SPRITE_YFLIP)
            {
                spp->vel.Z = 0;
                auto tex = TexMan.GetGameTexture(spp->spr.spritetexture());
                spp->spr.pos.Z = pSector->ceilingz + (tex->GetDisplayHeight() / 8.); // was << 5 in Build coordinates

                if (pSector->ceilingstat & CSTAT_SECTOR_SKY)
                {
                    spp->spr.cstat ^= CSTAT_SPRITE_YFLIP;
                    spp->vel.Z = 1./256.;

                    spp->nAction = 3;
                    spp->nFrame = 0;
                }
            }

            if ((totalmoves & 0x1F) == (spp->nPhase & 0x1F))
            {
                PlotCourseToSprite(spp, pTarget);

                if (RandomSize(3))
                {
					spp->VelFromAngle();
                }
                else
                {
                    spp->vel.X = 0;
                    spp->vel.Y = 0;
                }

                if (spp->nAction == 1 && RandomBit())
                {
                    if (spp->spr.cstat & CSTAT_SPRITE_YFLIP)
                    {
                        spp->spr.cstat ^= CSTAT_SPRITE_YFLIP;
						spp->vel.Z = 1./256.;
						spp->spr.pos.Z = spp->sector()->ceilingz+ GetActorHeight(spp);
                    }
                    else
                    {
						spp->vel.Z = -20;
                    }

                    spp->nAction = 3;
                    spp->nFrame = 0;

                    if (!RandomSize(3)) {
                        D3PlayFX(StaticSound[kSound29], spp);
                    }
                }
            }
            break;
        }
        case 5:
        {
            if (!spp->nFrame)
            {
                runlist_DoSubRunRec(spp->spr.intowner);
                runlist_FreeRun(spp->spr.lotag - 1);
                runlist_SubRunRec(spp->nRun);
                spp->spr.cstat = CSTAT_SPRITE_INVISIBLE;
                DeleteActor(spp);
            }
            return;
        }
        case 2:
        {
            if (pTarget)
            {
                if (nFrameFlag & 0x80)
                {
                    runlist_DamageEnemy(pTarget, spp, 3);
                    D3PlayFX(StaticSound[kSound38], spp);
                }

                if (PlotCourseToSprite(spp, pTarget) < 64) {
                    return;
                }

                spp->nAction = 1;
            }
            else
            {
                spp->nAction = 0;
                spp->vel.X = 0;
                spp->vel.Y = 0;
            }

            spp->nFrame = 0;
            break;
        }
        }
    }
    else
    {
        spp->pTarget = nullptr;
        spp->nAction = 0;
        spp->nFrame = 0;

        spp->vel.X = 0;
        spp->vel.Y = 0;
    }

    auto nMov = movespritevel(spp, spp->vel, nVel, -5, CLIPMASK0);

    if (nMov.type == kHitNone && nMov.exbits == 0)
        return;

    if (nMov.exbits & kHitAux1
        && spp->vel.Z < 0
        && hiHit.type != kHitSprite
        && !((spp->sector()->ceilingstat) & CSTAT_SECTOR_SKY))
    {
        spp->spr.cstat |= CSTAT_SPRITE_YFLIP;
		spp->spr.pos.Z = spp->sector()->ceilingz + GetActorHeight(spp);
        spp->vel.Z = 0;

        spp->nAction = 1;
        spp->nFrame = 0;
        return;
    }
    else
    {
        switch (nMov.type)
        {
        case kHitWall:
        {
            spp->spr.Angles.Yaw += DAngle45;
			spp->VelFromAngle();
            return;
        }
        case kHitSprite:
        {
            if (nMov.actor() == pTarget)
            {
                auto nAngDiff = absangle(spp->spr.Angles.Yaw, (pTarget->spr.pos - spp->spr.pos).Angle());
                if (nAngDiff < DAngle22_5 / 2)
                {
                    spp->nAction = 2;
                    spp->nFrame = 0;
                }
            }
            return;
        }
        default:
            break;
        }

        if (spp->nAction == 3)
        {
            spp->nAction = 1;
            spp->nFrame = 0;
        }
        return;
    }

    return;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AISpider::Draw(RunListEvent* ev)
{
    auto spp = ev->pObjActor;
    if (!spp) return;

    int nAction = spp->nAction;

    seq_PlotSequence(ev->nParam, SeqOffsets[kSeqSpider] + SpiderSeq[nAction].a, spp->nFrame, SpiderSeq[nAction].b);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AISpider::RadialDamage(RunListEvent* ev)
{
    auto spp = ev->pObjActor;
    if (!spp) return;

    if (spp->nHealth <= 0)
        return;

    ev->nDamage = runlist_CheckRadialDamage(spp);
    Damage(ev);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AISpider::Damage(RunListEvent* ev)
{
    auto spp = ev->pObjActor;
    if (!spp) return;

    if (!ev->nDamage)
        return;

    DExhumedActor* pTarget = ev->pOtherActor;

    spp->nHealth -= dmgAdjust(ev->nDamage);
    if (spp->nHealth > 0)
    {
        /*
        NOTE:
            nTarget check was added, but should we return if it's invalid instead
            or should code below (action set, b set) happen?
            Other AI doesn't show consistency in this regard (see Scorpion code)
        */
        if (pTarget && pTarget->spr.statnum == 100)
        {
            spp->pTarget = pTarget;
        }

        spp->nAction = 4;
        spp->nFrame = 0;
    }
    else
    {
        // creature is dead, make some chunks
        spp->nHealth = 0;
        spp->nAction = 5;
        spp->nFrame = 0;

        spp->spr.cstat &= ~CSTAT_SPRITE_BLOCK_ALL;

        nCreaturesKilled++;

        for (int i = 0; i < 7; i++)
        {
            BuildCreatureChunk(spp, seq_GetSeqPicnum(kSeqSpider, i + 41, 0));
        }
    }
}

END_PS_NS
