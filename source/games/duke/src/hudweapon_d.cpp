//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2000, 2003 - Matt Saettler (EDuke Enhancements)
Copyright (C) 2020 - Christoph Oelckers

This file is part of Enhanced Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms

EDuke enhancements integrated: 04/13/2003 - Matt Saettler

Note: EDuke source was in transition.  Changes are in-progress in the
source as it is released.

*/
//-------------------------------------------------------------------------

#include "ns.h"
#include "global.h"
#include "names_d.h"
#include "dukeactor.h"
#include "buildtiles.h"

BEGIN_DUKE_NS 

inline static float getavel(int snum)
{
	return PlayerInputAngVel(snum) * (2048. / 360.);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

inline static void hud_drawpal(float x, float y, int tilenum, int shade, int orientation, int p, DAngle angle)
{
	hud_drawsprite(x, y, 65536, angle.Degrees(), tilenum, shade, p, 2 | orientation);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

static void displayloogie(player_struct* p, float const interpfrac)
{
	if (p->loogcnt == 0) return;

	const float loogi = interpolatedvalue<float>(p->oloogcnt, p->loogcnt, interpfrac);
	const float y = loogi * 4.;

	for (int i = 0; i < p->numloogs; i++)
	{
		const float a = fabs(BobVal((loogi + i) * 32.f) * 90);
		const float z = 4096. + ((loogi + i) * 512.);
		const float x = -getavel(p->GetPlayerNum()) + BobVal((loogi + i) * 64.f) * 16;

		hud_drawsprite((p->loogie[i].X + x), (200 + p->loogie[i].Y - y), z - (i << 8), a - 22.5, DTILE_LOOGIE, 0, 0, 2);
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

static bool animatefist(int gs, player_struct* p, float xoffset, float yoffset, int fistpal, float const interpfrac)
{
	const float fisti = min(interpolatedvalue<float>(p->ofist_incs, p->fist_incs, interpfrac), 32.f);
	if (fisti <= 0) return false;

	hud_drawsprite(
		(-fisti + 222 + xoffset),
		(yoffset + 194 + BobVal((6 + fisti) * 128.f) * 32),
		clamp(65536.f - 65536.f * BobVal(512 + fisti * 64.f), 40920.f, 90612.f), 0, DTILE_FIST, gs, fistpal, 2);

	return true;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

static bool animateknee(int gs, player_struct* p, float xoffset, float yoffset, int pal, float const interpfrac, DAngle angle)
{
	if (p->knee_incs > 11 || p->knee_incs == 0 || p->GetActor()->spr.extra <= 0) return false;

	static const int8_t knee_y[] = { 0,-8,-16,-32,-64,-84,-108,-108,-108,-72,-32,-8 };
	const float kneei = interpolatedvalue<float>(knee_y[p->oknee_incs], knee_y[p->knee_incs], interpfrac);

	hud_drawpal(105 + (kneei * 0.25) + xoffset, 280 + kneei + yoffset, DTILE_KNEE, gs, 4, pal, angle);

	return true;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

static bool animateknuckles(int gs, player_struct* p, float xoffset, float yoffset, int pal, DAngle angle)
{
	if (isWW2GI() || p->over_shoulder_on != 0 || p->knuckle_incs == 0 || p->GetActor()->spr.extra <= 0) return false;

	static const uint8_t knuckle_frames[] = { 0,1,2,2,3,3,3,2,2,1,0 };

	hud_drawpal(160 + xoffset, 180 + yoffset, DTILE_CRACKKNUCKLES + knuckle_frames[p->knuckle_incs >> 1], gs, 4, pal, angle);

	return true;
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void displaymasks_d(int snum, int p, float interpfrac)
{
	if (ps[snum].scuba_on)
	{
		int y = 200 - tileHeight(DTILE_SCUBAMASK);
		hud_drawsprite(44, y, 65536, 0, DTILE_SCUBAMASK, 0, p, 2 + 16);
		hud_drawsprite((320 - 43), y, 65536, 0, DTILE_SCUBAMASK, 0, p, 2 + 4 + 16);
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

static bool animatetip(int gs, player_struct* p, float xoffset, float yoffset, int pal, float const interpfrac, DAngle angle)
{
	if (p->tipincs == 0) return false;

	static const int8_t tip_y[] = { 0,-8,-16,-32,-64,-84,-108,-108,-108,-108,-108,-108,-108,-108,-108,-108,-96,-72,-64,-32,-16 };
	const float tipi = interpolatedvalue<float>(tip_y[p->otipincs], tip_y[p->tipincs], interpfrac) * 0.5;

	hud_drawpal(170 + xoffset, 240 + tipi + yoffset, DTILE_TIP + ((26 - p->tipincs) >> 4), gs, 0, pal, angle);

	return true;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

static bool animateaccess(int gs, player_struct* p, float xoffset, float yoffset, float const interpfrac, DAngle angle)
{
	if (p->access_incs == 0 || p->GetActor()->spr.extra <= 0) return false;

	static const int8_t access_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-108,-108,-108,-108,-108,-108,-108,-96,-72,-64,-32,-16};
	const float accessi = interpolatedvalue<float>(access_y[p->oaccess_incs], access_y[p->access_incs], interpfrac);

	const int pal = p->access_spritenum != nullptr ? p->access_spritenum->spr.pal : 0;

	if ((p->access_incs-3) > 0 && (p->access_incs-3)>>3)
		hud_drawpal(170 + (accessi * 0.25) + xoffset, 266 + accessi + yoffset, DTILE_HANDHOLDINGLASER + (p->access_incs >> 3), gs, 0, pal, angle);
	else
		hud_drawpal(170 + (accessi * 0.25) + xoffset, 266 + accessi + yoffset, DTILE_HANDHOLDINGACCESS, gs, 4, pal, angle);

	return true;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void displayweapon_d(int snum, float interpfrac)
{
	int pal, pal2;
	player_struct* p = &ps[snum];

	if (p->newOwner != nullptr || ud.cameraactor != nullptr || p->over_shoulder_on > 0 || (p->GetActor()->spr.pal != 1 && p->GetActor()->spr.extra <= 0))
		return;

	float weapon_sway, gun_pos, kickback_pic, random_club_frame, hard_landing;
	auto kb = &p->kickback_pic;
	int o = 0;

	if (cl_hudinterpolation)
	{
		weapon_sway = interpolatedvalue<float>(p->oweapon_sway, p->weapon_sway, interpfrac);
		kickback_pic = interpolatedvalue<float>(p->okickback_pic, p->kickback_pic, interpfrac);
		random_club_frame = interpolatedvalue<float>(p->orandom_club_frame, p->random_club_frame, interpfrac);
		hard_landing = interpolatedvalue<float>(p->ohard_landing, p->hard_landing, interpfrac);
		gun_pos = 80 - interpolatedvalue<float>(p->oweapon_pos * p->oweapon_pos, p->weapon_pos * p->weapon_pos, interpfrac);
	}
	else
	{
		weapon_sway = p->weapon_sway;
		kickback_pic = p->kickback_pic;
		random_club_frame = p->random_club_frame;
		hard_landing = p->hard_landing;
		gun_pos = 80 - (p->weapon_pos * p->weapon_pos);
	}

	hard_landing *= 8.;
	gun_pos -= fabs(p->GetActor()->spr.scale.X < 0.5 ? BobVal(weapon_sway * 4.f) * 32 : BobVal(weapon_sway * 0.5f) * 16) + hard_landing;

	auto offpair = p->Angles.getWeaponOffsets(interpfrac);
	auto offsets = offpair.first;
	auto pitchoffset = 16. * (p->Angles.getRenderAngles(interpfrac).Pitch / DAngle90);
	auto yawinput = getavel(snum) * (1. / 16.);
	auto angle = offpair.second;
	auto weapon_xoffset = 160 - 90 - (BobVal(512 + weapon_sway * 0.5f) * (16384.f / 1536.f)) - 58 - p->weapon_ang;
	auto shade = min(p->GetActor()->spr.shade, (int8_t)24);

	pal2 = pal = !p->insector() ? 0 : p->GetActor()->spr.pal == 1 ? 1 : p->cursector->floorpal;
	if (pal2 == 0) pal2 = p->palookup;

	auto animoffs = offsets + DVector2(yawinput, -hard_landing + pitchoffset);

	if (animatefist(shade, p, yawinput, offsets.Y, pal, interpfrac))
		return;
	if (animateknuckles(shade, p, animoffs.X, animoffs.Y, pal, angle))
		return;
	if (animatetip(shade, p, animoffs.X, animoffs.Y, pal, interpfrac, angle))
		return;
	if (animateaccess(shade, p, animoffs.X, animoffs.Y, interpfrac, angle))
		return;

	animateknee(shade, p, animoffs.X, animoffs.Y, pal2, interpfrac, angle);

	offsets.X += weapon_xoffset;
	offsets.Y -= gun_pos;

	int cw = p->last_weapon >= 0 ? p->last_weapon : p->curr_weapon;
	if (isWW2GI()) cw = aplWeaponWorksLike(cw, snum);

	// onevent should go here..
	// rest of code should be moved to CON..
	int quick_kick = 14 - p->quick_kick;

	if (quick_kick != 14 || p->last_quick_kick)
	{
		if (quick_kick < 5 || quick_kick > 9)
		{
			hud_drawpal(80 + offsets.X, 250 + offsets.Y, DTILE_KNEE, shade, o | 4, pal2, angle);
		}
		else
		{
			hud_drawpal(160 - 16 + offsets.X, 214 + offsets.Y, DTILE_KNEE + 1, shade, o | 4, pal2, angle);
		}
	}

	if (p->GetActor()->spr.scale.X < 0.625)
	{
		//shrunken..
		animateshrunken(p, offsets.X, offsets.Y + gun_pos, DTILE_FIST, shade, o, interpfrac);
	}
	else
	{
		auto weapTotalTime = aplWeaponTotalTime(p->curr_weapon, snum);
		auto weapFireDelay = aplWeaponFireDelay(p->curr_weapon, snum);
		auto weapReload = aplWeaponReload(p->curr_weapon, snum);

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayknee = [&]()
		{
			if (*kb > 0)
			{
				if (*kb < 5 || *kb > 9)
				{
					hud_drawpal(220 + offsets.X, 250 + offsets.Y, DTILE_KNEE, shade, o, pal2, angle);
				}
				else
				{
					hud_drawpal(160 + offsets.X, 214 + offsets.Y, DTILE_KNEE + 1, shade, o, pal2, angle);
				}
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaytripbomb = [&]()
		{
			offsets.X += 8;
			offsets.Y -= 10;

			if (*kb > 6)
				offsets.Y += kickback_pic * 8.;
			else if (*kb < 4)
				hud_drawpal(142 + offsets.X, 234 + offsets.Y, DTILE_HANDHOLDINGLASER + 3, shade, o, pal, angle);

			hud_drawpal(130 + offsets.X, 249 + offsets.Y, DTILE_HANDHOLDINGLASER + (*kb >> 2), shade, o, pal, angle);
			hud_drawpal(152 + offsets.X, 249 + offsets.Y, DTILE_HANDHOLDINGLASER + (*kb >> 2), shade, o | 4, pal, angle);
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayrpg = [&]()
		{
			const int pin = ((gs.displayflags & DUKE3D_NO_WIDESCREEN_PINNING)) ? 0 : RS_ALIGN_R;

			offsets -= BobVal(512 + (min(kickback_pic, 16.f) * 128.f)) * 8;

			if (*kb > 0)
			{
				if (*kb < (isWW2GI() ? weapTotalTime : 8))
				{
					hud_drawpal(164 + offsets.X, 176 + offsets.Y, DTILE_RPGGUN + (*kb >> 1), shade, o | pin, pal, angle);
				}
				else if (isWW2GI())
				{
					// else we are in 'reload time'
					if (*kb < ((weapReload - weapTotalTime) / 2 + weapTotalTime))
					{
						// down 
						offsets.Y += 10 * (kickback_pic - weapTotalTime); //D
					}
					else
					{
						// up and left
						offsets.Y += 10 * (weapReload - kickback_pic); //U
					}
				}
			}

			hud_drawpal(164 + offsets.X, 176 + offsets.Y, DTILE_RPGGUN, shade, o | pin, pal, angle);
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayshotgun_ww = [&]()
		{
			offsets.X -= 8;

			if (*kb > 0)
				offsets.Y += BobVal(kickback_pic * 128.f) * 4;

			if (*kb > 0 && p->GetActor()->spr.pal != 1)
				offsets.X += 1 - (rand() & 3);

			int pic = DTILE_SHOTGUN;

			if (*kb == 0)
			{
				// Just fall through here.
			}
			else if (*kb <= weapTotalTime)
			{
				pic += 1;
			}
			// else we are in 'reload time'
			else if (*kb < ((weapReload - weapTotalTime) / 2 + weapTotalTime))
			{
				// down 
				offsets.Y += 10 * (kickback_pic - weapTotalTime); //D
			}
			else
			{
				// up and left
				offsets.Y += 10 * (weapReload - kickback_pic); //U
			}

			hud_drawpal(146 + offsets.X, 202 + offsets.Y, pic, shade, o, pal, angle);
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayshotgun = [&]()
		{
			offsets.X -= 8;

			switch(*kb)
			{
				case 1:
				case 2:
					hud_drawpal(168 + offsets.X, 201 + offsets.Y, DTILE_SHOTGUN + 2, -128, o, pal, angle);
					[[fallthrough]];
				case 0:
				case 6:
				case 7:
				case 8:
					hud_drawpal(146 + offsets.X, 202 + offsets.Y, DTILE_SHOTGUN, shade, o, pal, angle);
					break;
				case 3:
				case 4:
				case 5:
				case 9:
				case 10:
				case 11:
				case 12:
					if (*kb > 1 && *kb < 5)
					{
						offsets.Y += 40;
						offsets.X += 20;

						hud_drawpal(178 + offsets.X, 194 + offsets.Y, DTILE_SHOTGUN + 1 + ((*(kb)-1) >> 1), -128, o, pal, angle);
					}
					hud_drawpal(158 + offsets.X, 220 + offsets.Y, DTILE_SHOTGUN + 3, shade, o, pal, angle);
					break;
				case 13:
				case 14:
				case 15:
					hud_drawpal(198 + offsets.X, 210 + offsets.Y, DTILE_SHOTGUN + 4, shade, o, pal, angle);
					break;
				case 16:
				case 17:
				case 18:
				case 19:
					hud_drawpal(234 + offsets.X, 196 + offsets.Y, DTILE_SHOTGUN + 5, shade, o, pal, angle);
					break;
				case 20:
				case 21:
				case 22:
				case 23:
					hud_drawpal(240 + offsets.X, 196 + offsets.Y, DTILE_SHOTGUN + 6, shade, o, pal, angle);
					break;
				case 24:
				case 25:
				case 26:
				case 27:
					hud_drawpal(234 + offsets.X, 196 + offsets.Y, DTILE_SHOTGUN + 5, shade, o, pal, angle);
					break;
				case 28:
				case 29:
				case 30:
					hud_drawpal(188 + offsets.X, 206 + offsets.Y, DTILE_SHOTGUN + 4, shade, o, pal, angle);
					break;
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaychaingun_ww = [&]()
		{
			if (*kb > 0)
				offsets.Y += BobVal(kickback_pic * 128.f) * 4;

			if (*kb > 0 && p->GetActor()->spr.pal != 1)
				offsets.X += 1 - (rand() & 3);

			if (*kb == 0)
			{
				hud_drawpal(178 + offsets.X, 233 + offsets.Y, DTILE_CHAINGUN + 1, shade, o, pal, angle);
			}
			else if (*kb <= weapTotalTime)
			{
				hud_drawpal(188 + offsets.X, 243 + offsets.Y, DTILE_CHAINGUN + 2, shade, o, pal, angle);
			}
			else
			{
				// else we are in 'reload time', divide reload time into fifths.
				// 1) move weapon up/right, hand on clip (2519)
				// 2) move weapon up/right, hand removing clip (2518)
				// 3) hold weapon up/right, hand removed clip (2517)
				// 4) hold weapon up/right, hand inserting clip (2518)
				// 5) move weapon down/left, clip inserted (2519)

				float adj;
				int pic;
				const int iFifths = max((weapReload - weapTotalTime) / 5, 1);

				if (*kb < (iFifths + weapTotalTime))
				{
					// first segment
					pic = 2519;
					adj = 80 - (10 * (weapTotalTime + iFifths - kickback_pic));
				}
				else if (*kb < (iFifths * 2 + weapTotalTime))
				{
					// second segment (down)
					pic = 2518;
					adj = 80;
				}
				else if (*kb < (iFifths * 3 + weapTotalTime))
				{
					// third segment (up)
					pic = 2517;
					adj = 80;
				}
				else if (*kb < (iFifths * 4 + weapTotalTime))
				{
					// fourth segment (down)
					pic = 2518;
					adj = 80;
				}
				else
				{
					// up and left
					pic = 2519;
					adj = 10 * (weapReload - kickback_pic);
				}

				hud_drawpal(168 + offsets.X + adj, 260 + offsets.Y - adj, pic, shade, o, pal, angle);
			}

		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaychaingun = [&]
		{
			if (*kb > 0)
				offsets.Y += BobVal(kickback_pic * 128.f) * 4;

			if (*kb > 0 && p->GetActor()->spr.pal != 1)
				offsets.X += 1 - (rand() & 3);

			hud_drawpal(168 + offsets.X, 260 + offsets.Y, DTILE_CHAINGUN, shade, o, pal, angle);

			switch(*kb)
			{
				case 0:
					hud_drawpal(178 + offsets.X, 233 + offsets.Y, DTILE_CHAINGUN + 1, shade, o, pal, angle);
					break;
				default:
					if (*kb > 4 && *kb < 12)
					{
						auto rnd = p->GetActor()->spr.pal != 1 ? rand() & 7 : 0;
						hud_drawpal(136 + offsets.X + rnd, 208 + offsets.Y + rnd - (kickback_pic * 0.5), DTILE_CHAINGUN + 5 + ((*kb - 4) / 5), shade, o, pal, angle);

						if (p->GetActor()->spr.pal != 1) rnd = rand() & 7;
						hud_drawpal(180 + offsets.X + rnd, 208 + offsets.Y + rnd - (kickback_pic * 0.5), DTILE_CHAINGUN + 5 + ((*kb - 4) / 5), shade, o, pal, angle);
					}

					if (*kb < 8)
					{
						auto rnd = rand() & 7;
						hud_drawpal(158 + offsets.X + rnd, 208 + offsets.Y + rnd - (kickback_pic * 0.5), DTILE_CHAINGUN + 5 + ((*kb - 2) / 5), shade, o, pal, angle);
						hud_drawpal(178 + offsets.X, 233 + offsets.Y, DTILE_CHAINGUN + 1 + (*kb >> 1), shade, o, pal, angle);
					}
					else
					{
						hud_drawpal(178 + offsets.X, 233 + offsets.Y, DTILE_CHAINGUN + 1, shade, o, pal, angle);
					}
					break;
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaypistol = [&]()
		{
			if (*kb < 5)
			{
				static constexpr uint8_t kb_frames[] = { 0,1,2,0,0 };
				hud_drawpal((195 - 12 - (*kb == 2) * 3) + offsets.X, 244 + offsets.Y, DTILE_FIRSTGUN + kb_frames[*kb], shade, 2, pal, angle);
			}
			else
			{
				const int pin = (isWW2GI() || (gs.displayflags & DUKE3D_NO_WIDESCREEN_PINNING)) ? 0 : RS_ALIGN_R;
				const int pic_5 = DTILE_FIRSTGUN+5;
				const int WEAPON2_RELOAD_TIME = 50;
				const int reload_time = isWW2GI() ? weapReload : WEAPON2_RELOAD_TIME;

				offsets.X -= weapon_xoffset;

				if (*kb < 10)
				{
					hud_drawpal(194 + offsets.X, 230 + offsets.Y, DTILE_FIRSTGUN + 4, shade, o | pin, pal, angle);
				}
				else if (*kb < 15)
				{
					hud_drawpal(244 + offsets.X - (kickback_pic * 8.), 130 + offsets.Y + (kickback_pic * 16.), DTILE_FIRSTGUN + 6, shade, o | pin, pal, angle);
					hud_drawpal(224 + offsets.X, 220 + offsets.Y, pic_5, shade, o | pin, pal, angle);
				}
				else if (*kb < 20)
				{
					hud_drawpal(124 + offsets.X + (kickback_pic * 2.), 430 + offsets.Y - (kickback_pic * 8.), DTILE_FIRSTGUN + 6, shade, o | pin, pal, angle);
					hud_drawpal(224 + offsets.X, 220 + offsets.Y, pic_5, shade, o | pin, pal, angle);
				}
				else if (*kb < (isNamWW2GI()? (reload_time - 12) : 23))
				{
					hud_drawpal(184 + offsets.X, 235 + offsets.Y, DTILE_FIRSTGUN + 8, shade, o | pin, pal, angle);
					hud_drawpal(224 + offsets.X, 210 + offsets.Y, pic_5, shade, o | pin, pal, angle);
				}
				else if (*kb < (isNamWW2GI()? (reload_time - 6) : 25))
				{
					hud_drawpal(164 + offsets.X, 245 + offsets.Y, DTILE_FIRSTGUN + 8, shade, o | pin, pal, angle);
					hud_drawpal(224 + offsets.X, 220 + offsets.Y, pic_5, shade, o | pin, pal, angle);
				}
				else if (*kb < (isNamWW2GI()? reload_time : 27))
				{
					hud_drawpal(194 + offsets.X, 235 + offsets.Y, pic_5, shade, o, pal, angle);
				}
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayhandbomb = [&]()
		{
			int pic = DTILE_HANDTHROW;

			if (*kb)
			{
				static constexpr uint8_t throw_frames[] = { 0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2 };

				if (isWW2GI())
				{
					if (*kb <= weapFireDelay)
					{
						// it holds here
						offsets.Y += 5 * kickback_pic; //D
					}
					else if (*kb < ((weapTotalTime - weapFireDelay) / 2 + weapFireDelay))
					{
						// up and left
						offsets.Y -= 10 * (kickback_pic - weapFireDelay); //U
						offsets.X += 80 * (kickback_pic - weapFireDelay);
					}
					else if (*kb < weapTotalTime)
					{
						// move left
						offsets.Y -= 240; // start high
						offsets.Y += 12 * (kickback_pic - weapFireDelay); //D
						weapon_xoffset += 90 - (5 * (weapTotalTime - kickback_pic));
					}
				}
				else
				{
					if (*kb < 7)
						offsets.Y += 10 * kickback_pic;        //D
					else if (*kb < 12)
						offsets.Y -= 20 * (kickback_pic - 10); //U
					else if (*kb < 20)
						offsets.Y += 9 * (kickback_pic - 14);  //D
				}

				pic += throw_frames[*kb];
				offsets.Y -= 10;
			}

			hud_drawpal(190 + offsets.X, 260 + offsets.Y, pic, shade, o, pal, angle);
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayhandremote = [&]()
		{
			static constexpr uint8_t remote_frames[] = { 0,1,1,2,1,1,0,0,0,0,0 };
			hud_drawpal(102 + offsets.X, 258 + offsets.Y, DTILE_HANDREMOTE + (*kb ? remote_frames[*kb] : 0), shade, o, pal, angle);
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaydevastator_ww = [&]
		{
			if (*kb)
			{
				if (*kb < weapTotalTime)
				{
					const int i = Sgn(*kb >> 2);

					if (p->ammo_amount[p->curr_weapon] & 1)
					{
						hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR, shade, o | 4, pal, angle);
						hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR + i, -32, o, pal, angle);
					}
					else
					{
						hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR + i, -32, o | 4, pal, angle);
						hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR, shade, o, pal, angle);
					}
				}
				// else we are in 'reload time'
				else if (*kb < ((weapReload - weapTotalTime) / 2 + weapTotalTime))
				{
					// down 
					offsets.Y += 10 * (kickback_pic - weapTotalTime); //D
					// offsets.X += 80 * (*kb - aplWeaponTotalTime[cw][snum]);
					hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR, shade, o, pal, angle);
					hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR, shade, o | 4, pal, angle);
				}
				else
				{
					// up and left
					offsets.Y += 10 * (weapReload - kickback_pic); //U
					// offsets.X += 80 * (*kb - aplWeaponTotalTime[cw][snum]);
					hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR, shade, o, pal, angle);
					hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR, shade, o | 4, pal, angle);
				}
			}
			else
			{
				hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR, shade, o, pal, angle);
				hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR, shade, o | 4, pal, angle);
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaydevastator = [&]
		{
			if (*kb)
			{
				static constexpr uint8_t cycloidy[] = { 0,4,12,24,12,4,0 };
				const int i = Sgn(*kb >> 2);

				if (p->hbomb_hold_delay)
				{
					hud_drawpal(268 + offsets.X + (cycloidy[*kb] >> 1), 238 + offsets.Y + cycloidy[*kb], DTILE_DEVISTATOR + i, -32, o, pal, angle);
					hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR, shade, o | 4, pal, angle);
				}
				else
				{
					hud_drawpal(30 + offsets.X - (cycloidy[*kb] >> 1), 240 + offsets.Y + cycloidy[*kb], DTILE_DEVISTATOR + i, -32, o | 4, pal, angle);
					hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR, shade, o, pal, angle);
				}
			}
			else
			{
				hud_drawpal(268 + offsets.X, 238 + offsets.Y, DTILE_DEVISTATOR, shade, o, pal, angle);
				hud_drawpal(30 + offsets.X, 240 + offsets.Y, DTILE_DEVISTATOR, shade, o | 4, pal, angle);
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayfreezer = [&]
		{
			const int pin = (isWW2GI() || (gs.displayflags & DUKE3D_NO_WIDESCREEN_PINNING)) ? 0 : RS_ALIGN_R;

			if (*kb)
			{
				static constexpr uint8_t cat_frames[] = { 0,0,1,1,2,2 };

				if (p->GetActor()->spr.pal != 1)
				{
					offsets.X += rand() & 3;
					offsets.Y += rand() & 3;
				}

				offsets.Y += 16;

				hud_drawpal(210 + offsets.X, 261 + offsets.Y, DTILE_FREEZE + 2, -32, o | pin, pal, angle);
				hud_drawpal(210 + offsets.X, 235 + offsets.Y, DTILE_FREEZE + 3 + cat_frames[*kb % 6], -32, o | pin, pal, angle);
			}
			else
			{
				hud_drawpal(210 + offsets.X, 261 + offsets.Y, DTILE_FREEZE, shade, o | pin, pal, angle);
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayshrinker_ww = [&]
		{
			offsets.X += 28;
			offsets.Y += 18;

			if (*kb == 0)
			{
				// the 'at rest' display
				if (p->ammo_amount[cw] <= 0) //p->last_weapon >= 0)
				{
					hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 3 + (*kb & 3), -32, o, 0, angle);
					hud_drawpal(188 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 1, shade, o, pal, angle);
				}
				else
				{
					hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 2, 16 - int(BobVal(random_club_frame) * 16), o, 0, angle);
					hud_drawpal(188 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER, shade, o, pal, angle);
				}
			}
			else
			{
				// the 'active' display.
				if (p->GetActor()->spr.pal != 1)
				{
					offsets.X += rand() & 3;
					offsets.Y -= rand() & 3;
				}


				if (*kb < weapTotalTime)
				{
					if (!(*kb < weapFireDelay))
					{
						// lower weapon to reload cartridge (not clip)
						offsets.Y += 10 * (weapTotalTime - kickback_pic);
					}
				}
				// else we are in 'reload time'
				else if (*kb < ((weapReload - weapTotalTime) / 2 + weapTotalTime))
				{
					// down 
					offsets.Y += 10 * (kickback_pic - weapTotalTime); //D
				}
				else
				{
					// up
					offsets.Y += 10 * (weapReload - kickback_pic); //U
				}

				// draw weapon
				hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 3 + (*kb & 3), -32, o, 0, angle);
				hud_drawpal(188 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 1, shade, o, pal, angle);
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displaygrower_ww = [&]
		{
			offsets.X += 28;
			offsets.Y += 18;

			if (*kb == 0)
			{
				hud_drawpal(188 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER - 2, shade, o, pal, angle);
			}
			else
			{
				if (p->GetActor()->spr.pal != 1)
				{
					offsets.X += rand() & 3;
					offsets.Y -= rand() & 3;
				}

				if (*kb < weapTotalTime)
				{
					if (!(*kb < weapFireDelay))
					{
						// lower weapon to reload cartridge (not clip)
						offsets.Y += 15 * (weapTotalTime - kickback_pic);
					}
				}
				// else we are in 'reload time'
				else if (*kb < ((weapReload - weapTotalTime) / 2 + weapTotalTime))
				{
					// down 
					offsets.Y += 5 * (kickback_pic - weapTotalTime);
				}
				else
				{
					// up
					offsets.Y += 10 * (weapReload - kickback_pic);
				}

				// display weapon
				hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 3 + (*kb & 3), -32, o, 2, angle);
				hud_drawpal(188 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER - 1, shade, o, pal, angle);
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayshrinker = [&]
		{
			auto shrinker = /*isWorldTour() ? DTILE_SHRINKERWIDE :*/ DTILE_SHRINKER;
			offsets.X += 28;
			offsets.Y += 18;

			if (*kb == 0)
			{
				if (cw == GROW_WEAPON)
				{
					hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 2, 16 - int(BobVal(random_club_frame) * 16), o, 2, angle);
					hud_drawpal(188 + offsets.X, 240 + offsets.Y, shrinker - 2, shade, o, pal, angle);
				}
				else
				{
					hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 2, 16 - int(BobVal(random_club_frame) * 16), o, 0, angle);
					hud_drawpal(188 + offsets.X, 240 + offsets.Y, shrinker, shade, o, pal, angle);
				}
			}
			else
			{
				if (p->GetActor()->spr.pal != 1)
				{
					offsets.X += rand() & 3;
					offsets.Y -= rand() & 3;
				}

				if (cw == GROW_WEAPON)
				{
					hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 3 + (*kb & 3), -32, o, 2, angle);
					hud_drawpal(188 + offsets.X, 240 + offsets.Y, shrinker - 1, shade, o, pal, angle);
				}
				else
				{
					hud_drawpal(184 + offsets.X, 240 + offsets.Y, DTILE_SHRINKER + 3 + (*kb & 3), -32, o, 0, angle);
					hud_drawpal(188 + offsets.X, 240 + offsets.Y, shrinker + 1, shade, o, pal, angle);
				}
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		auto displayflamethrower = [&]()
		{
			if (*kb < 1 || p->cursector->lotag == 2)
			{
				hud_drawpal(210 + offsets.X, 261 + offsets.Y, DTILE_FLAMETHROWER, shade, o, pal, angle);
				hud_drawpal(210 + offsets.X, 261 + offsets.Y, DTILE_FLAMETHROWERPILOT, shade, o, pal, angle);
			}
			else
			{
				static constexpr uint8_t cat_frames[] = { 0, 0, 1, 1, 2, 2 };

				if (p->GetActor()->spr.pal != 1)
				{
					offsets.X += krand() & 1;
					offsets.Y += krand() & 1;
				}

				offsets.Y += 16;

				hud_drawpal(210 + offsets.X, 261 + offsets.Y, DTILE_FLAMETHROWER + 1, -32, o, pal, angle);
				hud_drawpal(210 + offsets.X, 235 + offsets.Y, DTILE_FLAMETHROWER + 2 + cat_frames[*kb % 6], -32, o, pal, angle);
			}
		};

		//---------------------------------------------------------------------------
		//
		//
		//
		//---------------------------------------------------------------------------

		switch (cw)
		{
		case KNEE_WEAPON:
			displayknee();
			break;

		case TRIPBOMB_WEAPON:
			displaytripbomb();
			break;

		case RPG_WEAPON:
			displayrpg();
			break;

		case SHOTGUN_WEAPON:
			if (isWW2GI()) displayshotgun_ww();
			else displayshotgun();
			break;

		case CHAINGUN_WEAPON:
			if (isWW2GI()) displaychaingun_ww();
			else displaychaingun();
			break;

		case PISTOL_WEAPON:
			displaypistol();
			break;

		case HANDBOMB_WEAPON:
			displayhandbomb();
			break;

		case HANDREMOTE_WEAPON:
			displayhandremote();
			break;

		case DEVISTATOR_WEAPON:
			if (isWW2GI()) displaydevastator_ww();
			else displaydevastator();
			break;

		case FREEZE_WEAPON:
			displayfreezer();
			break;

		case SHRINKER_WEAPON:
			if (isWW2GI()) displayshrinker_ww();
			else displayshrinker();
			break;

		case GROW_WEAPON:
			if (isWW2GI()) displaygrower_ww();
			else displayshrinker();
			break;

		case FLAMETHROWER_WEAPON:
			displayflamethrower();
			break;
		}
	}

	displayloogie(p, interpfrac);
}

END_DUKE_NS
