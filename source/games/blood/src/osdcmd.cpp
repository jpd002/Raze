//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) 2020 Raze developers and contributors

This file was part of NBlood.

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
#include "mapinfo.h"
#include "gamestate.h"

BEGIN_BLD_NS

void GameInterface::WarpToCoords(float x, float y, float z, DAngle ang)
{
	PLAYER* pPlayer = &gPlayer[myconnectindex];

	pPlayer->actor->spr.pos = { x, y, z };
	playerResetInertia(pPlayer);

	if (ang != DAngle::fromDeg(INT_MIN))
	{
		pPlayer->actor->spr.Angles.Yaw = ang;
		pPlayer->actor->backupang();
	}
}

void GameInterface::ToggleThirdPerson()
{
	if (gamestate != GS_LEVEL) return;
	if (gViewPos > VIEWPOS_0)
	{
		gViewPos = VIEWPOS_0;
	}
	else
	{
		gViewPos = VIEWPOS_1;
		cameradist = 0;
		cameraclock = INT_MIN;
	}
}

void GameInterface::SwitchCoopView()
{
	if (gamestate != GS_LEVEL) return;
	if (gGameOptions.nGameType == 1)
	{
		gViewIndex = connectpoint2[gViewIndex];
		if (gViewIndex == -1)
			gViewIndex = connecthead;
	}
	else if (gGameOptions.nGameType == 3)
	{
		int oldViewIndex = gViewIndex;
		do
		{
			gViewIndex = connectpoint2[gViewIndex];
			if (gViewIndex == -1)
				gViewIndex = connecthead;
			if (oldViewIndex == gViewIndex || gPlayer[myconnectindex].teamId == gPlayer[gViewIndex].teamId)
				break;
		} while (oldViewIndex != gViewIndex);
	}
}

void GameInterface::ToggleShowWeapon()
{
	if (gamestate != GS_LEVEL) return;
	cl_showweapon = (cl_showweapon + 1) & 3;
}

END_BLD_NS
