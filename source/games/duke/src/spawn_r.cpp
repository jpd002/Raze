//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2017-2019 Nuke.YKT
Copyright (C) 2020 - Christoph Oelckers

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <utility>
#include "ns.h"
#include "global.h"
#include "sounds.h"
#include "names_r.h"
#include "dukeactor.h"

BEGIN_DUKE_NS

DDukeActor* spawninit_r(DDukeActor* actj, DDukeActor* act, TArray<DDukeActor*>* actors)
{
	if (actorflag(act, SFLAG2_TRIGGERRESPAWN))
	{
		act->spr.yint = act->spr.hitag;
		act->spr.hitag = -1;
	}

	if (iseffector(act))
	{
		spawneffector(act, actors);
		return act;
	}

	if (act->GetClass() != RUNTIME_CLASS(DDukeActor))
	{
		if (!badguy(act) || commonEnemySetup(act, actj))
			CallInitialize(act);
		return act;
	}
	auto sectp = act->sector();

	switch (act->spr.picnum)
	{
	default:
	default_case:
		if (!badguy(act) || commonEnemySetup(act, actj))
			CallInitialize(act);
		break;
	case RTILE_RRTILE7936:
		if (!isRRRA()) goto default_case;
		act->spr.scale = DVector2(0, 0);
		fogactive = 1;
		break;
	case RTILE_RRTILE6144:
		if (!isRRRA()) goto default_case;
		act->spr.scale = DVector2(0, 0);
		ps[screenpeek].sea_sick_stat = 1;
		break;
	case RTILE_RRTILE8193:
		if (!isRRRA()) goto default_case;
		act->spr.scale = DVector2(0, 0);
		pistonsound = 1;
		break;

	case RTILE_TRANSPORTERSTAR:
	case RTILE_TRANSPORTERBEAM:
		spawntransporter(actj, act, act->spr.picnum == RTILE_TRANSPORTERBEAM);
		break;

	case RTILE_BLOOD:
		act->spr.scale = DVector2(0.0625, 0.0625);
		act->spr.pos.Z -= 26;
		ChangeActorStat(act, STAT_MISC);
		break;
	case RTILE_GRATE1:
		act->clipdist = 8;
		act->spr.cstat |= CSTAT_SPRITE_BLOCK_ALL;
		ChangeActorStat(act, 0);
		break;
	case RTILE_FEM10:
	case RTILE_NAKED1:
	case RTILE_STATUE:
	case RTILE_TOUGHGAL:
		act->spr.cstat |= CSTAT_SPRITE_BLOCK_ALL;
		act->clipdist = 8;
		ChangeActorStat(act, STAT_ZOMBIEACTOR);
		break;
	case RTILE_DUKELYINGDEAD:
		if (actj && actj->isPlayer())
		{
			act->spr.scale = actj->spr.scale;
			act->spr.shade = actj->spr.shade;
			act->spr.pal = ps[actj->PlayerIndex()].palookup;
		}
		act->spr.cstat = 0;
		act->spr.extra = 1;
		act->vel.X = 292 / 16.;
		act->vel.Z = 360 / 256.;
		act->spr.cstat |= CSTAT_SPRITE_BLOCK_ALL;
		act->clipdist = 32;
		[[fallthrough]];
	case RTILE_MIKE:
		if (act->spr.picnum == RTILE_MIKE)
			act->spr.yint = act->spr.hitag;
		ChangeActorStat(act, STAT_ACTOR);
		break;

	case RTILE_EXPLOSION2:
	case RTILE_EXPLOSION3:
	case RTILE_BURNING:
	case RTILE_SMALLSMOKE:
		if (actj)
		{
			act->spr.Angles.Yaw = actj->spr.Angles.Yaw;
			act->spr.shade = -64;
			act->spr.cstat = CSTAT_SPRITE_YCENTER | randomXFlip();
		}

		if (act->spr.picnum == RTILE_EXPLOSION2)
		{
			act->spr.scale = DVector2(0.75, 0.75);
			act->spr.shade = -127;
			act->spr.cstat |= CSTAT_SPRITE_YCENTER;
		}
		else if (act->spr.picnum == RTILE_EXPLOSION3)
		{
			act->spr.scale = DVector2(2, 2);
			act->spr.shade = -127;
			act->spr.cstat |= CSTAT_SPRITE_YCENTER;
		}
		else if (act->spr.picnum == RTILE_SMALLSMOKE)
		{
			act->spr.scale = DVector2(0.1875, 0.1875);
		}
		else if (act->spr.picnum == RTILE_BURNING)
		{
			act->spr.scale = DVector2(0.0625, 0.0625);
		}

		if (actj)
		{
			float x = getflorzofslopeptr(act->sector(), act->spr.pos);
			if (act->spr.pos.Z > x - 12)
				act->spr.pos.Z = x - 12;
		}

		ChangeActorStat(act, STAT_MISC);

		break;

	case RTILE_PLAYERONWATER:
		if (actj)
		{
			act->spr.scale = actj->spr.scale;
			act->vel.Z = 0.5;
			if (act->sector()->lotag != 2)
				act->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
		}
		ChangeActorStat(act, STAT_DUMMYPLAYER);
		break;

	case RTILE_APLAYER:
	{
		act->spr.scale = DVector2(0, 0);
		int j = ud.coop;
		if (j == 2) j = 0;

		if (ud.multimode < 2 || (ud.multimode > 1 && j != act->spr.lotag))
			ChangeActorStat(act, STAT_MISC);
		else
			ChangeActorStat(act, STAT_PLAYER);
		break;
	}
	case RTILE_WATERBUBBLE:
		if (actj && actj->isPlayer())
			act->spr.pos.Z -= 16;
		if (act->spr.picnum == RTILE_WATERBUBBLE)
		{
			if (actj)
				act->spr.Angles.Yaw = actj->spr.Angles.Yaw;
			float s = REPEAT_SCALE + (krand() & 7) * REPEAT_SCALE;
			act->spr.scale = DVector2(s, s);
		}
		else
			act->spr.scale = DVector2(0.5, 0.5);
		ChangeActorStat(act, STAT_MISC);
		break;

	case RTILE_WATERBUBBLEMAKER:
		act->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
		ChangeActorStat(act, STAT_STANDABLE);
		break;

		// this is not really nice...
	case RTILE_BIKERB:
	case RTILE_BIKERBV2:
	case RTILE_BIKER:
	case RTILE_MAKEOUT:
	case RTILE_CHEERB:
	case RTILE_CHEER:
	case RTILE_COOTPLAY:
	case RTILE_BILLYPLAY:
	case RTILE_MINIONBOAT:
	case RTILE_HULKBOAT:
	case RTILE_CHEERBOAT:
	case RTILE_RABBIT:
	case RTILE_ROCK:
	case RTILE_ROCK2:
	case RTILE_MAMACLOUD:
	case RTILE_MAMA:
		if (isRRRA()) goto rrra_badguy2;
		else goto default_case;

	case RTILE_SBSWIPE:
	case RTILE_CHEERSTAYPUT:
		if (isRRRA()) goto rrra_stayput;
		else goto default_case;
	case RTILE_SBMOVE:
		if (isRRRA()) goto default_case;
		[[fallthrough]];

	case RTILE_BILLYRAYSTAYPUT:
	case RTILE_BRAYSNIPER:
	case RTILE_BUBBASTAND:
	case RTILE_HULKSTAYPUT:
	case RTILE_HENSTAYPUT:
	case RTILE_PIGSTAYPUT:
	case RTILE_MINIONSTAYPUT:
	case RTILE_COOTSTAYPUT:
	rrra_stayput:
		act->actorstayput = act->sector();
		[[fallthrough]];
	case RTILE_BOULDER:
	case RTILE_BOULDER1:
	case RTILE_TORNADO:
	case RTILE_BILLYCOCK:
	case RTILE_BILLYRAY:
	case RTILE_DOGRUN:
	case RTILE_LTH:
	case RTILE_HULK:
	case RTILE_HEN:
	case RTILE_DRONE:
	case RTILE_PIG:
	case RTILE_MINION:
	case RTILE_COW:
	case RTILE_COOT:
	case RTILE_VIXEN:
	rrra_badguy2:
		act->spr.scale = DVector2(0.625, 0.625);
		// Note: All inappropriate tiles have already been weeded out by the outer switch block so this does not need game type checks anymore.
		switch (act->spr.picnum)
		{
		case RTILE_VIXEN:
			if (act->spr.pal == 34)
			{
				act->spr.scale = DVector2(0.34375, 0.328125);
			}
			else
			{
				act->spr.scale = DVector2(0.34375, 0.3125);
			}
			act->setClipDistFromTile();
			break;
		case RTILE_HULKHANG:
		case RTILE_HULKHANGDEAD:
		case RTILE_HULKJUMP:
		case RTILE_HULK:
		case RTILE_HULKSTAYPUT:
			act->spr.scale = DVector2(0.5, 0.5);
			act->setClipDistFromTile();
			break;
		case RTILE_COOTPLAY:
		case RTILE_COOT:
		case RTILE_COOTSTAYPUT:
			act->spr.scale = DVector2(0.375, 0.28125);
			act->setClipDistFromTile();
			act->clipdist *= 4;
			break;
		case RTILE_DRONE:
			act->spr.scale = DVector2(0.21875, 0.109375);
			act->clipdist = 32;
			break;
		case RTILE_SBSWIPE:
		case RTILE_BILLYPLAY:
		case RTILE_BILLYCOCK:
		case RTILE_BILLYRAY:
		case RTILE_BILLYRAYSTAYPUT:
		case RTILE_BRAYSNIPER:
		case RTILE_BUBBASTAND:
			act->spr.scale = DVector2(0.390625, 0.328125);
			act->setClipDistFromTile();
			break;
		case RTILE_COW:
			act->spr.scale = DVector2(0.5, 0.5);
			act->setClipDistFromTile();
			break;
		case RTILE_HEN:
		case RTILE_HENSTAYPUT:
			if (act->spr.pal == 35)
			{
				act->spr.scale = DVector2(0.65625, 0.46875);
				act->setClipDistFromTile();
			}
			else
			{
				act->spr.scale = DVector2(0.328125, 0.234375);
				act->clipdist = 16;
			}
			break;
		case RTILE_MINION:
		case RTILE_MINIONSTAYPUT:
			act->spr.scale = DVector2(0.25, 0.25);
			act->setClipDistFromTile();
			if (isRRRA() && ud.ufospawnsminion)
				act->spr.pal = 8;
			break;
		case RTILE_DOGRUN:
		case RTILE_PIG:
			act->spr.scale = DVector2(0.25, 0.25);
			act->setClipDistFromTile();
			break;
		case RTILE_RABBIT:
			act->spr.scale = DVector2(0.28125, 0.28125);
			act->setClipDistFromTile();
			break;
		case RTILE_MAMACLOUD:
			act->spr.scale = DVector2(1, 1);
			act->spr.cstat = CSTAT_SPRITE_TRANSLUCENT;
			act->spr.cstat |= CSTAT_SPRITE_TRANS_FLIP;
			act->spr.pos.X += krandf(128) - 64;
			act->spr.pos.Y += krandf(128) - 64;
			act->spr.pos.Z += krandf(8) - 4;
			break;
		case RTILE_MAMA:
			if (actj && isrespawncontroller(actj))
				act->spr.pal = actj->spr.pal;

			if (act->spr.pal == 30)
			{
				act->spr.scale = DVector2(0.40625, 0.40625);
				act->clipdist = 18.75;
			}
			else if (act->spr.pal == 31)
			{
				act->spr.scale = DVector2(0.5625, 0.5625);
				act->clipdist = 25;
			}
			else if (act->spr.pal == 32)
			{
				act->spr.scale = DVector2(0.78125, 0.78125);
				act->clipdist = 25;
			}
			else
			{
				act->spr.scale = DVector2(0.78125, 0.78125);
				act->clipdist = 25;
			}
			break;
		case RTILE_BIKERB:
			act->spr.scale = DVector2(0.4375, 0.34375);
			act->clipdist = 18;
			break;
		case RTILE_BIKERBV2:
			act->spr.scale = DVector2(0.4375, 0.34375);
			act->clipdist = 18;
			break;
		case RTILE_BIKER:
			act->spr.scale = DVector2(0.4375, 0.34375);
			act->setClipDistFromTile();
			break;
		case RTILE_CHEERB:
			act->spr.scale = DVector2(0.4375, 0.34375);
			act->clipdist = 18;
			break;
		case RTILE_CHEER:
		case RTILE_CHEERSTAYPUT:
			act->spr.scale = DVector2(0.34375, 0.3125);
			act->setClipDistFromTile();
			break;
		case RTILE_MAKEOUT:
			act->spr.scale = DVector2(0.40625, 0.40625);
			act->setClipDistFromTile();
			break;
		case RTILE_MINIONBOAT:
			act->spr.scale = DVector2(0.25, 0.25);
			act->setClipDistFromTile();
			break;
		case RTILE_HULKBOAT:
			act->spr.scale = DVector2(0.75, 0.75);
			act->setClipDistFromTile();
			break;
		case RTILE_CHEERBOAT:
			act->spr.scale = DVector2(0.5, 0.5);
			act->setClipDistFromTile();
			break;

		case RTILE_TORNADO:
			act->spr.scale = DVector2(1, 2);
			act->setClipDistFromTile();
			act->clipdist *= 0.25;
			act->spr.cstat = CSTAT_SPRITE_TRANSLUCENT;
			break;
		case RTILE_LTH:
			act->spr.scale = DVector2(0.375, 0.34375);
			act->setClipDistFromTile();
			break;
		case RTILE_ROCK:
		case RTILE_ROCK2:
			act->spr.scale = DVector2(1, 1);
			act->setClipDistFromTile();
			break;

		case RTILE_SBMOVE:
			act->spr.scale = DVector2(0.75, 0.75);
			act->setClipDistFromTile();
			break;

		default:
			break;
		}

		if (actj) act->spr.lotag = 0;

		if ((act->spr.lotag > ud.player_skill) || ud.monsters_off == 1)
		{
			act->spr.scale = DVector2(0, 0);
			ChangeActorStat(act, STAT_MISC);
			break;
		}
		else
		{
			makeitfall(act);

			act->spr.cstat |= CSTAT_SPRITE_BLOCK_ALL;

			if (act->spr.picnum != 5501)
				if (actorfella(act))
					ps[myconnectindex].max_actors_killed++;

			if (actj)
			{
				act->timetosleep = 0;
				check_fta_sounds_r(act);
				ChangeActorStat(act, STAT_ACTOR);
				act->spr.shade = actj->spr.shade;
			}
			else ChangeActorStat(act, STAT_ZOMBIEACTOR);

		}

		break;

	case RTILE_RPG2SPRITE:
	case RTILE_MOTOAMMO:
	case RTILE_BOATAMMO:
		if (!isRRRA()) goto default_case;
		[[fallthrough]];

	case RTILE_ATOMICHEALTH:
	case RTILE_STEROIDS:
	case RTILE_HEATSENSOR:
	case RTILE_SHIELD:
	case RTILE_AIRTANK:
	case RTILE_POWDERKEG:
	case RTILE_COWPIE:
	case RTILE_HOLODUKE:

	case RTILE_FIRSTGUNSPRITE:
	case RTILE_RIFLEGUNSPRITE:
	case RTILE_SHOTGUNSPRITE:
	case RTILE_CROSSBOWSPRITE:
	case RTILE_RIPSAWSPRITE:
	case RTILE_TITSPRITE:
	case RTILE_ALIENBLASTERSPRITE:

	case RTILE_SHOTGUNAMMO:
	case RTILE_FREEZEAMMO:
	case RTILE_HBOMBAMMO:
	case RTILE_CRYSTALAMMO:
	case RTILE_GROWAMMO:
	case RTILE_BATTERYAMMO:
	case RTILE_DEVISTATORAMMO:
	case RTILE_RPGAMMO:
	case RTILE_BOOTS:
	case RTILE_AMMO:
	case RTILE_AMMOLOTS:
	case RTILE_BEER:
	case RTILE_FIRSTAID:
	case RTILE_SIXPAK:

	case RTILE_SAWAMMO:
	case RTILE_BOWLINGBALLSPRITE:
		if (actj)
		{
			act->spr.lotag = 0;
			if (act->spr.picnum != RTILE_BOWLINGBALLSPRITE)
			{
				act->spr.pos.Z -= 32;
				act->vel.Z = -4;
			}
			else
			{
				act->vel.Z = 0;
			}
			ssp(act, CLIPMASK0);
			act->spr.cstat = randomXFlip();
		}
		else
		{
			act->SetOwner(act);
			act->spr.cstat = 0;
		}

		if ((ud.multimode < 2 && act->spr.pal != 0) || (act->spr.lotag > ud.player_skill))
		{
			act->spr.scale = DVector2(0, 0);
			ChangeActorStat(act, STAT_MISC);
			break;
		}

		act->spr.pal = 0;
		[[fallthrough]];

	case RTILE_ACCESSCARD:

		if (act->spr.picnum == RTILE_ATOMICHEALTH)
			act->spr.cstat |= CSTAT_SPRITE_YCENTER;

		if (ud.multimode > 1 && ud.coop != 1 && act->spr.picnum == RTILE_ACCESSCARD)
		{
			act->spr.scale = DVector2(0, 0);
			ChangeActorStat(act, STAT_MISC);
			break;
		}
		else
		{
			if (act->spr.picnum == RTILE_AMMO)
				act->spr.scale = DVector2(0.25, 0.25);
			else act->spr.scale = DVector2(0.5, 0.5);
		}

		act->spr.shade = -17;

		if (actj) ChangeActorStat(act, STAT_ACTOR);
		else
		{
			ChangeActorStat(act, STAT_ZOMBIEACTOR);
			makeitfall(act);
		}
		switch (act->spr.picnum)
		{
		case RTILE_FIRSTGUNSPRITE:
			act->spr.scale = DVector2(0.25, 0.25);
			break;
		case RTILE_SHOTGUNAMMO:
			act->spr.scale = DVector2(0.28125, 0.265625);
			if (isRRRA()) act->spr.cstat = CSTAT_SPRITE_BLOCK_HITSCAN;
			break;
		case RTILE_SIXPAK:
			act->spr.scale = DVector2(0.203125, 0.140625);
			if (isRRRA()) act->spr.cstat = CSTAT_SPRITE_BLOCK_HITSCAN;
			break;
		case RTILE_FIRSTAID:
			act->spr.scale = DVector2(0.125, 0.125);
			break;
		case RTILE_BEER:
			act->spr.scale = DVector2(0.078125, 0.0625);
			break;
		case RTILE_AMMO:
			act->spr.scale = DVector2(0.140625, 0.140625);
			break;
		case RTILE_MOTOAMMO:
			if (!isRRRA()) goto default_case;
			act->spr.scale = DVector2(0.359375, 0.359375);
			break;
		case RTILE_BOATAMMO:
			if (!isRRRA()) goto default_case;
			act->spr.scale = DVector2(0.25, 0.25);
			break;
		case RTILE_COWPIE:
			act->spr.scale = DVector2(0.125, 0.09375);
			break;
		case RTILE_STEROIDS:
			act->spr.scale = DVector2(0.203125, 0.140625);
			break;
		case RTILE_ACCESSCARD:
			act->spr.scale = DVector2(0.171875, 0.1875);
			break;
		case RTILE_HEATSENSOR:
			act->spr.scale = DVector2(0.09375, 0.0625);
			break;
		case RTILE_AIRTANK:
			act->spr.scale = DVector2(0.296875, 0.25);
			break;
		case RTILE_BATTERYAMMO:
			act->spr.scale = DVector2(0.234375, 0.234375);
			break;
		case RTILE_BOWLINGBALLSPRITE:
			act->spr.scale = DVector2(0.171875, 0.171875);
			break;
		case RTILE_POWDERKEG:
			act->spr.scale = DVector2(0.171875, 0.171875);
			act->spr.yint = 4;
			act->vel.X = 2;
			break;
		case RTILE_CROSSBOWSPRITE:
			act->spr.scale = DVector2(0.25, 0.21875);
			break;
		case RTILE_RPG2SPRITE:
			if (!isRRRA()) goto default_case;
			act->spr.scale = DVector2(0.34375, 0.3125);
			break;
		case RTILE_RIPSAWSPRITE:
			act->spr.scale = DVector2(0.34375, 0.203125);
			break;
		case RTILE_ALIENBLASTERSPRITE:
			act->spr.scale = DVector2(0.28125, 0.265625);
			break;
		case RTILE_SAWAMMO:
			act->spr.scale = DVector2(0.1875, 0.109375);
			break;
		case RTILE_GROWSPRITEICON:
			act->spr.scale = DVector2(0.15625, 0.140625);
			break;
		case RTILE_DEVISTATORAMMO:
			act->spr.scale = DVector2(0.15625, 0.140625);
			break;
		case RTILE_ATOMICHEALTH:
			act->spr.scale = DVector2(0.125, 0.125);
			break;
		case RTILE_TITSPRITE:
			act->spr.scale = DVector2(0.265625, 0.25);
			break;
		}
		act->spr.shade = act->sector()->floorshade;
		break;
	case RTILE_STEAM:
		if (actj)
		{
			act->spr.Angles.Yaw = actj->spr.Angles.Yaw;
			act->spr.cstat = CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_YCENTER | CSTAT_SPRITE_TRANSLUCENT;
			act->spr.scale = DVector2(REPEAT_SCALE, REPEAT_SCALE);
			act->vel.X = -0.5;
			ssp(act, CLIPMASK0);
		}
		[[fallthrough]];
	case RTILE_CEILINGSTEAM:
		ChangeActorStat(act, STAT_STANDABLE);
		break;

	case RTILE_RUBBERCAN:
		act->spr.extra = 0;
		[[fallthrough]];
	case RTILE_EXPLODINGBARREL:
	case RTILE_HORSEONSIDE:
	case RTILE_FIREBARREL:
	case RTILE_NUKEBARREL:
	case RTILE_FIREVASE:
	case RTILE_NUKEBARRELDENTED:
	case RTILE_NUKEBARRELLEAKED:
	case RTILE_WOODENHORSE:

		if (actj)
			act->spr.scale = DVector2(0.5, 0.5);
		act->clipdist = 18;
		makeitfall(act);
		if (actj) act->SetOwner(actj);
		else act->SetOwner(act);
		[[fallthrough]];

	case RTILE_EGG:
		if (ud.monsters_off == 1 && act->spr.picnum == RTILE_EGG)
		{
			act->spr.scale = DVector2(0, 0);
			ChangeActorStat(act, STAT_MISC);
		}
		else
		{
			if (act->spr.picnum == RTILE_EGG)
				act->clipdist = 6;
			act->spr.cstat = CSTAT_SPRITE_BLOCK_ALL | randomXFlip();
			ChangeActorStat(act, STAT_ZOMBIEACTOR);
		}
		break;
	case RTILE_TOILETWATER:
		act->spr.shade = -16;
		ChangeActorStat(act, STAT_STANDABLE);
		break;
	case RTILE_RRTILE63:
		act->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
		act->spr.scale = DVector2(REPEAT_SCALE, REPEAT_SCALE);
		act->clipdist = 0.25;
		ChangeActorStat(act, 100);
		break;
	}
	return act;
}

END_DUKE_NS
