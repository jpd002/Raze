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

#include "ns.h"
#include "global.h"
#include "names_r.h"
#include "mapinfo.h"
#include "dukeactor.h"

BEGIN_DUKE_NS

void dojaildoor();
void moveminecart();

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void check_fta_sounds_r(DDukeActor* actor)
{
	if (actor->spr.extra > 0) switch (actor->spr.picnum)
	{
	case RTILE_COOT: // LIZTROOP
		if (!isRRRA() && (krand() & 3) == 2)
			S_PlayActorSound(PRED_RECOG, actor);
		break;
	case RTILE_BILLYCOCK:
	case RTILE_BILLYRAY:
	case RTILE_BRAYSNIPER: // PIGCOP
		S_PlayActorSound(PIG_RECOG, actor);
		break;
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void addweapon_r(player_struct* p, int weapon, bool wswitch)
{
	int cw = p->curr_weapon;
	if (p->OnMotorcycle || p->OnBoat)
	{
		p->gotweapon[weapon] = true;;
		if (weapon == THROWSAW_WEAPON)
		{
			p->gotweapon[BUZZSAW_WEAPON] = true;
			p->ammo_amount[BUZZSAW_WEAPON] = 1;
		}
		else if (weapon == CROSSBOW_WEAPON)
		{
			p->gotweapon[CHICKEN_WEAPON] = true;
			p->gotweapon[DYNAMITE_WEAPON] = true;
		}
		else if (weapon == SLINGBLADE_WEAPON)
		{
			p->ammo_amount[SLINGBLADE_WEAPON] = 1;
		}
		return;
	}

	if (p->gotweapon[weapon] == 0)
	{
		p->gotweapon[weapon] = true;;
		if (weapon == THROWSAW_WEAPON)
		{
			p->gotweapon[BUZZSAW_WEAPON] = true;
			p->ammo_amount[BUZZSAW_WEAPON] = 1;
		}
		if (isRRRA())
		{
			if (weapon == CROSSBOW_WEAPON)
			{
				p->gotweapon[CHICKEN_WEAPON] = true;
			}
			if (weapon == SLINGBLADE_WEAPON)
			{
				p->ammo_amount[SLINGBLADE_WEAPON] = 50;
			}
		}
		if (weapon == CROSSBOW_WEAPON)
		{
			p->gotweapon[DYNAMITE_WEAPON] = true;
		}

		if (weapon != DYNAMITE_WEAPON)
			cw = weapon;
	}
	else
		cw = weapon;

	if (!wswitch) return;

	if (weapon == DYNAMITE_WEAPON)
		p->last_weapon = -1;

	p->random_club_frame = 0;

	if (p->holster_weapon == 0)
	{
		p->weapon_pos = -1;
		p->last_weapon = p->curr_weapon;
	}
	else
	{
		p->weapon_pos = 10;
		p->holster_weapon = 0;
		p->last_weapon = -1;
	}

	p->okickback_pic = p->kickback_pic = 0;
	p->curr_weapon = cw;
	p->wantweaponfire = -1;

	switch (weapon)
	{
	case SLINGBLADE_WEAPON:
		if (!isRRRA()) break;
	case KNEE_WEAPON:
	case DYNAMITE_WEAPON:	 
	case TRIPBOMB_WEAPON:
	case THROWINGDYNAMITE_WEAPON:
		break;
	case SHOTGUN_WEAPON:	  
		S_PlayActorSound(SHOTGUN_COCK, p->GetActor()); 
		break;
	case PISTOL_WEAPON:	   
		S_PlayActorSound(INSERT_CLIP, p->GetActor());
		break;
	default:	  
		S_PlayActorSound(EJECT_CLIP, p->GetActor());
		break;
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void hitradius_r(DDukeActor* actor, int  r, int  hp1, int  hp2, int  hp3, int  hp4)
{
	float radius = r * inttoworld;
	static const uint8_t statlist[] = { STAT_DEFAULT, STAT_ACTOR, STAT_STANDABLE, STAT_PLAYER, STAT_FALLER, STAT_ZOMBIEACTOR, STAT_MISC };

	if (actor->spr.scale.X >= 0.17675 || !(actor->spr.picnum == RTILE_RPG || ((isRRRA()) && actor->spr.picnum == RTILE_RPG2)))
	{
		BFSSectorSearch search(actor->sector());

		while (auto dasectp = search.GetNext())
		{
			if ((dasectp->ceilingz- actor->spr.pos.Z) < radius * 16) // what value range is this supposed to be? The check that was here did not multiply correctly
			{
				auto wal = dasectp->walls.Data();
				float d = (wal->pos - actor->spr.pos.XY()).Sum();
				if (d < radius)
					checkhitceiling(dasectp);
				else
				{
					auto thirdpoint = wal->point2Wall()->point2Wall();
					d = (thirdpoint->pos - actor->spr.pos.XY()).Sum();
					if (d < radius)
						checkhitceiling(dasectp);
				}
			}

			for (auto& wal : dasectp->walls)
			{
				if ((wal.pos - actor->spr.pos.XY()).Sum() < radius)
				{
					if (wal.twoSided())
					{
						search.Add(wal.nextSector());
					}
					DVector3 w1(((wal.pos + wal.point2Wall()->pos) * 0.5 + actor->spr.pos) * 0.5, actor->spr.pos.Z); // half way between the actor and the wall's center.
					sectortype* sect = wal.sectorp();
					updatesector(w1, &sect);

					if (sect && cansee(w1, sect, actor->spr.pos, actor->sector()))
						checkhitwall(actor, &wal, DVector3(wal.pos, actor->spr.pos.Z));
				}
			}
		}
	}

	float q = zrand(32) - 24;

	auto Owner = actor->GetOwner();
	for (int x = 0; x < 7; x++)
	{
		DukeStatIterator it1(statlist[x]);
		while (auto act2 = it1.Next())
		{
			if (x == 0 || x >= 5 || actorflag(act2, SFLAG_HITRADIUS_FLAG1))
			{
				if (act2->spr.cstat & CSTAT_SPRITE_BLOCK_ALL)
					if ((actor->spr.pos - act2->spr.pos).Length() < radius)
					{
						if (badguy(act2) && !cansee(act2->spr.pos.plusZ(q), act2->sector(), actor->spr.pos.plusZ(q), actor->sector()))
							continue;

						fi.checkhitsprite(act2, actor);
					}
			}
			else if (act2->spr.extra >= 0 && act2 != actor && (actorflag(act2, SFLAG_HITRADIUS_FLAG2) || badguy(act2) || (act2->spr.cstat & CSTAT_SPRITE_BLOCK_ALL)))
			{
				if (actor->spr.picnum == RTILE_MORTER && act2 == Owner)
				{
					continue;
				}
				if ((isRRRA()) && actor->spr.picnum == RTILE_CHEERBOMB && act2 == Owner)
				{
					continue;
				}

				float dist = (act2->getPosWithOffsetZ() - actor->spr.pos).Length();

				if (dist < radius && cansee(act2->spr.pos.plusZ(-8), act2->sector(), actor->spr.pos.plusZ(-12), actor->sector()))
				{
					if ((isRRRA()) && act2->spr.picnum == RTILE_MINION && act2->spr.pal == 19)
					{
						continue;
					}

					act2->hitang = (act2->spr.pos - actor->spr.pos).Angle();

					if (actor->spr.picnum == RTILE_RPG && act2->spr.extra > 0)
						act2->attackertype = RTILE_RPG;
					else if ((isRRRA()) && actor->spr.picnum == RTILE_RPG2 && act2->spr.extra > 0)
						act2->attackertype = RTILE_RPG;
					else
						act2->attackertype = RTILE_RADIUSEXPLOSION;

					if (dist < radius / 3)
					{
						if (hp4 == hp3) hp4++;
						act2->hitextra = hp3 + (krand() % (hp4 - hp3));
					}
					else if (dist < 2 * radius / 3)
					{
						if (hp3 == hp2) hp3++;
						act2->hitextra = hp2 + (krand() % (hp3 - hp2));
					}
					else if (dist < radius)
					{
						if (hp2 == hp1) hp2++;
						act2->hitextra = hp1 + (krand() % (hp2 - hp1));
					}

					if (!actorflag(act2, SFLAG2_NORADIUSPUSH) && !bossguy(act2))
					{
						if (act2->vel.X < 0) act2->vel.X = 0;
						act2->vel.X += ((actor->spr.extra / 4.f));
					}

					if (actorflag(act2, SFLAG_HITRADIUSCHECK))
						fi.checkhitsprite(act2, actor);

					if (act2->spr.picnum != RTILE_RADIUSEXPLOSION &&
						Owner && Owner->spr.statnum < MAXSTATUS)
					{
						if (act2->isPlayer())
						{
							int p = act2->PlayerIndex();
							if (ps[p].newOwner != nullptr)
							{
								clearcamera(&ps[p]);
							}
						}
						act2->SetHitOwner(actor->GetOwner());
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int movesprite_ex_r(DDukeActor* actor, const DVector3& change, unsigned int cliptype, Collision &result)
{
	int bg = badguy(actor);

	if (actor->spr.statnum == 5 || (bg && actor->spr.scale.X < 0.0625))
	{
		actor->spr.pos += change;
		if (bg)
			SetActor(actor, actor->spr.pos);
		return result.setNone();
	}

	auto dasectp = actor->sector();

	auto ppos = actor->spr.pos;
	auto tex = TexMan.GetGameTexture(actor->spr.spritetexture());
	ppos.Z -= tex->GetDisplayHeight() * actor->spr.scale.Y * 0.5f;

	if (bg)
	{
		if (actor->spr.scale.X > 0.9375 )
			clipmove(ppos, &dasectp, change * 0.5, 64., 4., 4., cliptype, result);
		else
		{
			clipmove(ppos, &dasectp, change * 0.5, 12., 4., 4., cliptype, result);
		}

		if (dasectp == nullptr || (dasectp != nullptr && actor->actorstayput != nullptr && actor->actorstayput != dasectp))
		{
			if (dasectp && dasectp->lotag == ST_1_ABOVE_WATER)
				actor->spr.Angles.Yaw = randomAngle();
			else if ((actor->temp_data[0] & 3) == 1)
				actor->spr.Angles.Yaw = randomAngle();
			SetActor(actor, actor->spr.pos);
			if (dasectp == nullptr) dasectp = &sector[0];
			return result.setSector(dasectp);
		}
		if ((result.type == kHitWall || result.type == kHitSprite) && (actor->cgg == 0)) actor->spr.Angles.Yaw += DAngle45 + DAngle90;
	}
	else
	{
		if (actor->spr.statnum == STAT_PROJECTILE)
			clipmove(ppos, &dasectp, change * 0.5, 0.5, 4., 4., cliptype, result);
		else
			clipmove(ppos, &dasectp, change * 0.5, actor->clipdist, 4., 4., cliptype, result);
	}
	actor->spr.pos.XY() = ppos.XY();

	if (dasectp)
		if ((dasectp != actor->sector()))
			ChangeActorSect(actor, dasectp);
	float daz = actor->spr.pos.Z + change.Z * 0.5f;
	if (daz > actor->ceilingz && daz <= actor->floorz)
		actor->spr.pos.Z = daz;
	else if (result.type == kHitNone)
		return result.setSector(dasectp);

	return result.type;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void lotsoffeathers_r(DDukeActor *actor, int n)
{
	lotsofstuff(actor, n, RTILE_FEATHER);
}


//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int ifhitbyweapon_r(DDukeActor *actor)
{
	int p;
	auto hitowner = actor->GetHitOwner();

	if (actor->hitextra >= 0)
	{
		if (actor->spr.extra >= 0)
		{
			if (actor->isPlayer())
			{
				if (ud.god) return -1;

				p = actor->PlayerIndex();

				if (hitowner &&
					hitowner->isPlayer() &&
					ud.coop == 1 &&
					ud.ffire == 0)
					return -1;

				actor->spr.extra -= actor->hitextra;

				if (hitowner)
				{
					if (actor->spr.extra <= 0 && actor->attackertype != RTILE_FREEZEBLAST)
					{
						actor->spr.extra = 0;

						ps[p].wackedbyactor = hitowner;

						if (hitowner->isPlayer() && p != hitowner->PlayerIndex())
						{
							ps[p].frag_ps = hitowner->PlayerIndex();
						}
						actor->SetHitOwner(ps[p].GetActor());
					}
				}

				if (attackerflag(actor, SFLAG2_DOUBLEDMGTHRUST))
				{
					ps[p].vel.XY() += actor->hitang.ToVector() * actor->hitextra * 0.25;
				}
				else
				{
					ps[p].vel.XY() += actor->hitang.ToVector() * actor->hitextra * 0.125;
				}
			}
			else
			{
				if (actor->hitextra == 0)
					if (actor->spr.scale.X < 0.375)
						return -1;

				actor->spr.extra -= actor->hitextra;
				auto Owner = actor->GetOwner();
				if (!actorflag(actor, SFLAG2_IGNOREHITOWNER) && Owner && Owner->spr.statnum < MAXSTATUS)
					actor->SetOwner(hitowner);
			}

			actor->hitextra = -1;
			return actor->attackertype;
		}
	}

	actor->hitextra = -1;
	return -1;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void movefallers_r(void)
{
	DukeStatIterator it(STAT_FALLER);
	while (auto act = it.Next())
	{
		auto sectp = act->sector();

		if (act->temp_data[0] == 0)
		{
			act->spr.pos.Z -= 16;
			DAngle saved_angle = act->spr.Angles.Yaw;
			int x = act->spr.extra;
			int j = fi.ifhitbyweapon(act);
			if (j >= 0)
			{
				if (gs.actorinfo[j].flags2 & SFLAG2_EXPLOSIVE)
				{
					if (act->spr.extra <= 0)
					{
						act->temp_data[0] = 1;
						DukeStatIterator itr(STAT_FALLER);
						while (auto ac2 = itr.Next())
						{
							if (ac2->spr.hitag == act->spr.hitag)
							{
								ac2->temp_data[0] = 1;
								ac2->spr.cstat &= ~CSTAT_SPRITE_ONE_SIDE;
								if (ac2->spr.picnum == RTILE_CEILINGSTEAM || ac2->spr.picnum == RTILE_STEAM)
									ac2->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
							}
						}
					}
				}
				else
				{
					act->hitextra = 0;
					act->spr.extra = x;
				}
			}
			act->spr.Angles.Yaw = saved_angle;
			act->spr.pos.Z += 16;
		}
		else if (act->temp_data[0] == 1)
		{
			if (act->spr.lotag > 0)
			{
				act->spr.lotag -= 3;
				act->vel.X = 4 + krandf(8);
				act->vel.Z = -4 + krandf(4);
			}
			else
			{
				if (act->vel.X > 0)
				{
					act->vel.X -= 1/8.;
					ssp(act, CLIPMASK0);
				}

				float grav;
				if (floorspace(act->sector())) grav = 0;
				else
				{
					if (ceilingspace(act->sector()))
						grav = gs.gravity / 6;
					else
						grav = gs.gravity;
				}

				if (act->spr.pos.Z < sectp->floorz - 1)
				{
					act->vel.Z += grav;
					if (act->vel.Z > 24)
						act->vel.Z = 24;
					act->spr.pos.Z += act->vel.Z;
				}
				if ((sectp->floorz - act->spr.pos.Z) < 16)
				{
					int j = 1 + (krand() & 7);
					for (int x = 0; x < j; x++) RANDOMSCRAP(act);
					act->Destroy();
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void movetransports_r(void)
{
	uint8_t warpdir = 0, warpspriteto;
	int k, p, sectlotag;
	int onfloorz;
	float ll, ll2 = 0;
	Collision coll;

	 //Transporters

	DukeStatIterator iti(STAT_TRANSPORT);
	while (auto act = iti.Next())
	{
		auto sectp = act->sector();
		sectlotag = sectp->lotag;

		auto Owner = act->GetOwner();
		if (Owner == act || Owner == nullptr)
		{
			continue;
		}

		onfloorz = act->temp_data[4];

		if (act->temp_data[0] > 0) act->temp_data[0]--;

		DukeSectIterator itj(act->sector());
		while (auto act2 = itj.Next())
		{
			switch (act2->spr.statnum)
			{
			case STAT_PLAYER:	// Player

				if (act2->GetOwner())
				{
					p = act2->PlayerIndex();

					ps[p].on_warping_sector = 1;

					if (ps[p].transporter_hold == 0 && ps[p].jumping_counter == 0)
					{
						if (ps[p].on_ground && sectlotag == 0 && onfloorz && ps[p].jetpack_on == 0)
						{
							spawn(act,  RTILE_TRANSPORTERBEAM);
							S_PlayActorSound(TELEPORTER, act);

							for (k = connecthead; k >= 0; k = connectpoint2[k])
								if (ps[k].cursector == Owner->sector())
								{
									ps[k].frag_ps = p;
									ps[k].GetActor()->spr.extra = 0;
								}

							ps[p].GetActor()->PrevAngles.Yaw = ps[p].GetActor()->spr.Angles.Yaw = Owner->spr.Angles.Yaw;

							if (Owner->GetOwner() != Owner)
							{
								act->temp_data[0] = 13;
								Owner->temp_data[0] = 13;
								ps[p].transporter_hold = 13;
							}

							ps[p].GetActor()->spr.pos = Owner->spr.pos.plusZ(4);
							ps[p].GetActor()->backuppos();
							ps[p].setbobpos();

							ChangeActorSect(act2, Owner->sector());
							ps[p].setCursector(act2->sector());

							auto beam = spawn(Owner, RTILE_TRANSPORTERBEAM);
							if (beam) S_PlayActorSound(TELEPORTER, beam);

							break;
						}
					}
					else break;

					if (onfloorz == 0 && fabs(act->spr.pos.Z - ps[p].GetActor()->getOffsetZ()) < 24)
						if ((ps[p].jetpack_on == 0) || (ps[p].jetpack_on && PlayerInput(p, SB_JUMP)) ||
							(ps[p].jetpack_on && PlayerInput(p, SB_CROUCH)))
						{
							ps[p].GetActor()->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
							ps[p].GetActor()->backupvec2();

							if (ps[p].jetpack_on && (PlayerInput(p, SB_JUMP) || ps[p].jetpack_on < 11))
								ps[p].GetActor()->spr.pos.Z = Owner->spr.pos.Z - 24 + gs.playerheight;
							else ps[p].GetActor()->spr.pos.Z = Owner->spr.pos.Z + 24 + gs.playerheight;
							ps[p].GetActor()->backupz();

							ChangeActorSect(act2, Owner->sector());
							ps[p].setCursector(Owner->sector());

							break;
						}

					k = 0;

					if (isRRRA())
					{
						if (onfloorz && sectlotag == ST_160_FLOOR_TELEPORT && ps[p].GetActor()->getOffsetZ() > sectp->floorz - 48)
						{
							k = 2;
							ps[p].GetActor()->spr.pos.Z = Owner->sector()->ceilingz + 7 + gs.playerheight;
							ps[p].GetActor()->backupz();
						}

						if (onfloorz && sectlotag == ST_161_CEILING_TELEPORT && ps[p].GetActor()->getOffsetZ() < sectp->ceilingz + 6)
						{
							k = 2;
							if (ps[p].GetActor()->spr.extra <= 0) break;
							ps[p].GetActor()->spr.pos.Z = Owner->sector()->floorz - 49 + gs.playerheight;
							ps[p].GetActor()->backupz();
						}
					}

					if ((onfloorz && sectlotag == ST_1_ABOVE_WATER && ps[p].GetActor()->getOffsetZ() > sectp->floorz - 6) ||
						(onfloorz && sectlotag == ST_1_ABOVE_WATER && ps[p].OnMotorcycle))
					{
						if (ps[p].OnBoat) break;
						k = 1;
						if (screenpeek == p)
						{
							FX_StopAllSounds();
						}
						S_PlayActorSound(DUKE_UNDERWATER, ps[p].GetActor());
						ps[p].GetActor()->spr.pos.Z = Owner->sector()->ceilingz + 7 + gs.playerheight;
						ps[p].GetActor()->backupz();
						if (ps[p].OnMotorcycle)
							ps[p].moto_underwater = 1;
					}

					if (onfloorz && sectlotag == ST_2_UNDERWATER && ps[p].GetActor()->getOffsetZ() < sectp->ceilingz + 6)
					{
						k = 1;
						if (ps[p].GetActor()->spr.extra <= 0) break;
						if (screenpeek == p)
						{
							FX_StopAllSounds();
						}
						S_PlayActorSound(DUKE_GASP, ps[p].GetActor());

						ps[p].GetActor()->spr.pos.Z = Owner->sector()->floorz - 7 + gs.playerheight;
						ps[p].GetActor()->backupz();
					}

					if (k == 1)
					{
						ps[p].GetActor()->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
						ps[p].GetActor()->backupvec2();

						if (Owner->GetOwner() != Owner)
							ps[p].transporter_hold = -2;
						ps[p].setCursector(Owner->sector());

						ChangeActorSect(act2, Owner->sector());

						if ((krand() & 255) < 32)
							spawn(ps[p].GetActor(), RTILE_WATERSPLASH2);
					}
					else if (isRRRA() && k == 2)
					{
						ps[p].GetActor()->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
						ps[p].GetActor()->backupvec2();

						if (Owner->GetOwner() != Owner)
							ps[p].transporter_hold = -2;
						ps[p].setCursector(Owner->sector());

						ChangeActorSect(act2, Owner->sector());
					}
				}
				break;

			case STAT_ACTOR:
			case STAT_PROJECTILE:
			case STAT_MISC:
			case STAT_DUMMYPLAYER:
				if (actorflag(act, SFLAG2_DONTDIVE)) continue;

				ll = abs(act2->vel.Z);
				if (isRRRA())
				{
					if (act2->vel.Z >= 0)
						warpdir = 2;
					else
						warpdir = 1;
				}

				{
					warpspriteto = 0;
					if (ll && sectlotag == ST_2_UNDERWATER && act2->spr.pos.Z < (sectp->ceilingz + ll))
						warpspriteto = 1;

					if (ll && sectlotag == ST_1_ABOVE_WATER && act2->spr.pos.Z > (sectp->floorz - ll))
						warpspriteto = 1;

					if (isRRRA())
					{
						if (ll && sectlotag == ST_161_CEILING_TELEPORT && act2->spr.pos.Z < (sectp->ceilingz + ll) && warpdir == 1)
						{
							warpspriteto = 1;
							ll2 = ll - abs(act2->spr.pos.Z - sectp->ceilingz);
						}
						else if (sectlotag == ST_161_CEILING_TELEPORT && act2->spr.pos.Z < (sectp->ceilingz + 3.90625) && warpdir == 1)
						{
							warpspriteto = 1;
							ll2 = zmaptoworld;
						}
						if (ll && sectlotag == ST_160_FLOOR_TELEPORT && act2->spr.pos.Z > (sectp->floorz - ll) && warpdir == 2)
						{
							warpspriteto = 1;
							ll2 = ll - abs(sectp->floorz - act2->spr.pos.Z);
						}
						else if (sectlotag == ST_160_FLOOR_TELEPORT && act2->spr.pos.Z > (sectp->floorz - 3.90625) && warpdir == 2)
						{
							warpspriteto = 1;
							ll2 = zmaptoworld;
						}
					}

					if (sectlotag == 0 && (onfloorz || abs(act2->spr.pos.Z - act->spr.pos.Z) < 16))
					{
						if (Owner->GetOwner() != Owner && onfloorz && act->temp_data[0] > 0 && act2->spr.statnum != 5)
						{
							act->temp_data[0]++;
							continue;
						}
						warpspriteto = 1;
					}

					if (warpspriteto)
					{
						if (actorflag(act2, SFLAG_NOTELEPORT)) continue;
						switch (act2->spr.picnum)
						{
						case RTILE_PLAYERONWATER:
							if (sectlotag == ST_2_UNDERWATER)
							{
								act2->spr.cstat &= ~CSTAT_SPRITE_INVISIBLE;
								break;
							}
							[[fallthrough]];
						default:
							if (act2->spr.statnum == 5 && !(sectlotag == ST_1_ABOVE_WATER || sectlotag == ST_2_UNDERWATER || (isRRRA() && (sectlotag == ST_160_FLOOR_TELEPORT || sectlotag == ST_161_CEILING_TELEPORT))))
								break;
							[[fallthrough]];

						case RTILE_WATERBUBBLE:
							if (rnd(192) && act2->spr.picnum == RTILE_WATERBUBBLE)
								break;

							if (sectlotag > 0)
							{
								auto spawned = spawn(act2, RTILE_WATERSPLASH2);
								if (spawned && sectlotag == 1 && act2->spr.statnum == 4)
								{
									spawned->vel.X = act2->vel.X * 0.5f;
									spawned->spr.Angles.Yaw = act2->spr.Angles.Yaw;
									ssp(spawned, CLIPMASK0);
								}
							}

							switch (sectlotag)
							{
							case ST_0_NO_EFFECT:
								if (onfloorz)
								{
									if (checkcursectnums(act->sector()) == -1 && checkcursectnums(Owner->sector()) == -1)
									{
										act2->spr.pos += (Owner->spr.pos - act->spr.pos.XY()).plusZ(-Owner->sector()->floorz);
										act2->spr.Angles.Yaw = Owner->spr.Angles.Yaw;

										act2->backupang();

										auto beam = spawn(act, RTILE_TRANSPORTERBEAM);
										if (beam) S_PlayActorSound(TELEPORTER, beam);

										beam = spawn(Owner, RTILE_TRANSPORTERBEAM);
										if (beam) S_PlayActorSound(TELEPORTER, beam);

										if (Owner->GetOwner() != Owner)
										{
											act->temp_data[0] = 13;
											Owner->temp_data[0] = 13;
										}

										ChangeActorSect(act2, Owner->sector());
									}
								}
								else
								{
									act2->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
									act2->spr.pos.Z = Owner->spr.pos.Z + 16;
									act2->backupz();

									ChangeActorSect(act2, Owner->sector());
								}
								break;
							case ST_1_ABOVE_WATER:
								act2->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
								act2->spr.pos.Z = Owner->sector()->ceilingz + ll;
								act2->backupz();

								ChangeActorSect(act2, Owner->sector());

								break;
							case ST_2_UNDERWATER:
								act2->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
								act2->spr.pos.Z = Owner->sector()->ceilingz - ll;
								act2->backupz();

								ChangeActorSect(act2, Owner->sector());

								break;

							case 160:
								if (!isRRRA()) break;
								act2->spr.pos.XY() += Owner->spr.pos.XY() - act->spr.pos.XY();
								act2->spr.pos.Z = Owner->sector()->ceilingz + ll2;
								act2->backupz();

								ChangeActorSect(act2, Owner->sector());

								movesprite_ex(act2, DVector3(act2->spr.Angles.Yaw.ToVector() * act2->vel.X, 0), CLIPMASK1, coll);

								break;
							case 161:
								if (!isRRRA()) break;
								act2->spr.pos += Owner->spr.pos.XY() - act->spr.pos.XY();
								act2->spr.pos.Z = Owner->sector()->floorz - ll;
								act2->backupz();

								ChangeActorSect(act2, Owner->sector());

								movesprite_ex(act2, DVector3(act2->spr.Angles.Yaw.ToVector() * act2->vel.X, 0), CLIPMASK1, coll);

								break;
							}

							break;
						}
					}
				}
				break;

			}
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void rrra_specialstats()
{
	Collision coll;
	DukeStatIterator it(STAT_BOBBING);
	while (auto act = it.Next())
	{
		if (act->spr.hitag > 1)
			act->spr.hitag = 0;
		if (act->spr.hitag == 0)
		{
			act->spr.extra++;
			if (act->spr.extra >= 20)
				act->spr.hitag = 1;
		}
		else if (act->spr.hitag == 1)
		{
			act->spr.extra--;
			if (act->spr.extra <= -20)
				act->spr.hitag = 0;
		}
		movesprite_ex(act, DVector3(0, 0, act->spr.extra / 256.f), CLIPMASK0, coll);
	}

	if (ps[screenpeek].MamaEnd > 0)
	{
		ps[screenpeek].MamaEnd--;
		if (ps[screenpeek].MamaEnd == 0)
		{
			CompleteLevel(nullptr);
		}
	}

	if (enemysizecheat > 0)
	{
		DukeSpriteIterator itr;
		while (auto act = itr.Next())
		{
			switch (act->spr.picnum)
			{
				//case 4049:
				//case 4050:
			case RTILE_BILLYCOCK:
			case RTILE_BILLYRAY:
			case RTILE_BILLYRAYSTAYPUT:
			case RTILE_BRAYSNIPER:
			case RTILE_DOGRUN:
			case RTILE_LTH:
			case RTILE_HULKJUMP:
			case RTILE_HULK:
			case RTILE_HULKSTAYPUT:
			case RTILE_HEN:
			case RTILE_DRONE:
			case RTILE_PIG:
			case RTILE_MINION:
			case RTILE_MINIONSTAYPUT:
			case RTILE_UFO1_RRRA:
			case RTILE_UFO2:
			case RTILE_UFO3:
			case RTILE_UFO4:
			case RTILE_UFO5:
			case RTILE_COOT:
			case RTILE_COOTSTAYPUT:
			case RTILE_VIXEN:
			case RTILE_BIKERB:
			case RTILE_BIKERBV2:
			case RTILE_BIKER:
			case RTILE_MAKEOUT:
			case RTILE_CHEERB:
			case RTILE_CHEER:
			case RTILE_CHEERSTAYPUT:
			case RTILE_COOTPLAY:
			case RTILE_BILLYPLAY:
			case RTILE_MINIONBOAT:
			case RTILE_HULKBOAT:
			case RTILE_CHEERBOAT:
			case RTILE_RABBIT:
			case RTILE_MAMA:
				if (enemysizecheat == 3)
				{
					act->spr.scale *= 2;
					act->setClipDistFromTile();
				}
				else if (enemysizecheat == 2)
				{
					act->spr.scale *= 0.5;
					auto tex = TexMan.GetGameTexture(act->spr.spritetexture());
					act->clipdist = act->spr.scale.X, tex->GetDisplayHeight() * 0.125;
				}
				break;
			}

		}
		enemysizecheat = 0;
	}

	tickstat(STAT_RABBITSPAWN);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void moveactors_r(void)
{
	dojaildoor();
	moveminecart();

	if (isRRRA())
	{
		rrra_specialstats();
	}
	tickstat(STAT_LUMBERMILL);
	if (ud.chickenplant) tickstat(STAT_CHICKENPLANT);
	tickstat(STAT_BOWLING);
	tickstat(STAT_TELEPORT);
	tickstat(STAT_ACTOR);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void handle_se06_r(DDukeActor *actor)
{
	auto sc = actor->sector();
	int sh = actor->spr.hitag;

	int k = sc->extra;

	if (actor->temp_data[4] > 0)
	{
		actor->temp_data[4]--;
		if (actor->temp_data[4] >= (k - (k >> 3)))
			actor->vel.X -= (k >> 5) / 16.f;
		if (actor->temp_data[4] > ((k >> 1) - 1) && actor->temp_data[4] < (k - (k >> 3)))
			actor->vel.X = 0;
		if (actor->temp_data[4] < (k >> 1))
			actor->vel.X += (k >> 5) / 16.f;
		if (actor->temp_data[4] < ((k >> 1) - (k >> 3)))
		{
			actor->temp_data[4] = 0;
			actor->vel.X = k / 16.f;
			if ((!isRRRA() || lastlevel) && hulkspawn)
			{
				hulkspawn--;
				auto ns = spawn(actor, RTILE_HULK);
				if (ns)
				{
					ns->spr.pos.Z = ns->sector()->ceilingz;
					ns->spr.pal = 33;
				}
				if (!hulkspawn)
				{
					ns = CreateActor(actor->sector(), DVector3(actor->spr.pos.XY(), actor->sector()->ceilingz + 466.5f), RTILE_UFOLIGHT, -8, DVector2(0.25f, 0.25f), nullAngle, 0., 0., actor, 5);
					if (ns)
					{
						ns->spr.cstat = CSTAT_SPRITE_TRANS_FLIP | CSTAT_SPRITE_TRANSLUCENT;
						ns->spr.pal = 7;
						ns->spr.scale = DVector2(1.25, 3.984375);
					}
					ns = spawn(actor, 296);
					if (ns)
					{
						ns->spr.cstat = 0;
						ns->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
						ns->spr.pos.Z = actor->sector()->floorz - 24;
					}
					actor->Destroy();
					return;
				}
			}
		}
	}
	else
	{
		actor->vel.X = k / 16.f;
		DukeSectIterator it(actor->sector());
		while (auto a2 = it.Next())
		{
			if (a2->spr.picnum == RTILE_UFOBEAM && ufospawn && ++ufocnt == 64)
			{
				ufocnt = 0;
				ufospawn--;
				const char* pn;
				if (!isRRRA())
				{
					switch (krand() & 3)
					{
					default:
					case 0:
						pn = "RedneckUfo1";
						break;
					case 1:
						pn = "RedneckUfo2";
						break;
					case 2:
						pn = "RedneckUfo3";
						break;
					case 3:
						pn = "RedneckUfo4";
						break;
					}
				}
				else pn = "RedneckUfoRRRA";
				auto ns = spawn(actor, PClass::FindActor(pn));
				if (ns) ns->spr.pos.Z = ns->sector()->ceilingz;
			}
		}
	}

	DukeStatIterator it(STAT_EFFECTOR);
	while (auto act2 = it.Next())
	{
		if ((act2->spr.lotag == SE_14_SUBWAY_CAR) && (sh == act2->spr.hitag) && (act2->temp_data[0] == actor->temp_data[0]))
		{
			act2->vel.X = actor->vel.X;
			//if( actor->temp_data[4] == 1 )
			{
				if (act2->temp_pos.X == 0)
					act2->temp_pos.X = (act2->spr.pos - actor->spr.pos).LengthSquared();
				int x = Sgn((act2->spr.pos - actor->spr.pos).LengthSquared() - act2->temp_pos.X);
				if (act2->spr.extra) x = -x;
				actor->vel.X += x / 16.f;
			}
			act2->temp_data[4] = actor->temp_data[4];
		}
	}
	handle_se14(actor, false, RTILE_RPG, RTILE_JIBS6);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void moveeffectors_r(void)   //STATNUM 3
{
	clearfriction();

	DukeStatIterator it(STAT_EFFECTOR);
	while (auto act = it.Next())
	{
		auto sc = act->sector();
		int st = act->spr.lotag;

		switch (st)
		{
		case SE_0_ROTATING_SECTOR:
			handle_se00(act);
			break;

		case SE_1_PIVOT: //Nothing for now used as the pivot
			handle_se01(act);
			break;

		case SE_6_SUBWAY:
			handle_se06_r(act);
			break;

		case SE_14_SUBWAY_CAR:
			handle_se14(act, false, RTILE_RPG, RTILE_JIBS6);
			break;

		case SE_30_TWO_WAY_TRAIN:
			handle_se30(act, RTILE_JIBS6);
			break;


		case SE_2_EARTHQUAKE:
			handle_se02(act);
			break;

			//Flashing sector lights after reactor RTILE_EXPLOSION2
		case SE_3_RANDOM_LIGHTS_AFTER_SHOT_OUT:
			handle_se03(act);
			break;

		case SE_4_RANDOM_LIGHTS:
			handle_se04(act);
			break;

			//BOSS
		case SE_5_BOSS:
			handle_se05(act);
			break;

		case SE_8_UP_OPEN_DOOR_LIGHTS:
		case SE_9_DOWN_OPEN_DOOR_LIGHTS:
			handle_se08(act, true);
			break;

		case SE_10_DOOR_AUTO_CLOSE:
			handle_se10(act, nullptr);
			break;

		case SE_11_SWINGING_DOOR:
			handle_se11(act);
			break;

		case SE_12_LIGHT_SWITCH:
			handle_se12(act);
			break;

		case SE_47_LIGHT_SWITCH:
			if (isRRRA()) handle_se12(act, 1);
			break;

		case SE_48_LIGHT_SWITCH:
			if (isRRRA()) handle_se12(act, 2);
			break;


		case SE_13_EXPLOSIVE:
			handle_se13(act);
			break;

		case SE_15_SLIDING_DOOR:
			handle_se15(act);
			break;

		case SE_16_REACTOR:
			handle_se16(act);
			break;

		case SE_17_WARP_ELEVATOR:
			handle_se17(act);
			break;

		case SE_18_INCREMENTAL_SECTOR_RISE_FALL:
			handle_se18(act, true);
			break;

		case SE_19_EXPLOSION_LOWERS_CEILING:
			handle_se19(act);
			break;

		case SE_20_STRETCH_BRIDGE:
			handle_se20(act);
			break;

		case SE_21_DROP_FLOOR:
			handle_se21(act);
			break;

		case SE_22_TEETH_DOOR:
			handle_se22(act);
			break;

		case SE_156_CONVEYOR_NOSCROLL:
			if (!isRRRA()) break;
			[[fallthrough]];
		case SE_24_CONVEYOR:
		case SE_34:
		{
			handle_se24(act, st != SE_156_CONVEYOR_NOSCROLL, 0.5);
			break;
		}

		case SE_35:
			handle_se35(act, RTILE_SMALLSMOKE, RTILE_EXPLOSION2);
			break;

		case SE_25_PISTON: //PISTONS
			if (act->temp_data[4] == 0) break;
			handle_se25(act, isRRRA() ? 371 : -1, isRRRA() ? 167 : -1);
			break;

		case SE_26:
			handle_se26(act);
			break;

		case SE_27_DEMO_CAM:
			handle_se27(act);
			break;

		case SE_29_WAVES:
			handle_se29(act);
			break;

		case SE_31_FLOOR_RISE_FALL: // True Drop Floor
			handle_se31(act, false);
			break;

		case SE_32_CEILING_RISE_FALL: // True Drop Ceiling
			handle_se32(act);
			break;

		case SE_33_QUAKE_DEBRIS:
			if (ud.earthquaketime > 0 && (krand() & 7) == 0)
				RANDOMSCRAP(act);
			break;
		case SE_36_PROJ_SHOOTER:

			if (act->temp_data[0])
			{
				if (act->temp_data[0] == 1)
					fi.shoot(act, sc->extra, nullptr);
				else if (act->temp_data[0] == 26 * 5)
					act->temp_data[0] = 0;
				act->temp_data[0]++;
			}
			break;

		case SE_128_GLASS_BREAKING:
			handle_se128(act);
			break;

		case SE_130:
			handle_se130(act, 80, RTILE_EXPLOSION2);
			break;
		case SE_131:
			handle_se130(act, 40, RTILE_EXPLOSION2);
			break;
		}
	}

	//Sloped sin-wave floors!
	it.Reset(STAT_EFFECTOR);
	while (auto act = it.Next())
	{
		if (act->spr.lotag != SE_29_WAVES) continue;
		auto sc = act->sector();
		if (sc->walls.Size() != 4) continue;
		auto wal = &sc->walls[2];
		if (wal->nextSector()) alignflorslope(act->sector(), DVector3(wal->pos, wal->nextSector()->floorz));
	}
}


//---------------------------------------------------------------------------
//
// game specific part of makeitfall.
//
//---------------------------------------------------------------------------

float adjustfall(DDukeActor *actor, float c)
{
	if ((actor->spr.picnum == RTILE_BIKERB || actor->spr.picnum == RTILE_CHEERB) && c == gs.gravity)
		c = gs.gravity * 0.25;
	else if (actor->spr.picnum == RTILE_BIKERBV2 && c == gs.gravity)
		c = gs.gravity * 0.125;
	return c;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void move_r(DDukeActor *actor, int pnum, int xvel)
{
	DAngle goalang, angdif;
	float daxvel;

	int a = actor->spr.hitag;

	if (a == -1) a = 0;

	actor->temp_data[0]++;

	if (a & face_player)
	{
		if (ps[pnum].newOwner != nullptr)
			goalang = (ps[pnum].GetActor()->opos.XY() - actor->spr.pos.XY()).Angle();
		else goalang = (ps[pnum].GetActor()->spr.pos.XY() - actor->spr.pos.XY()).Angle();
		angdif = deltaangle(actor->spr.Angles.Yaw, goalang) * 0.25;
		if (angdif > -DAngle22_5 / 16 && angdif < nullAngle) angdif = nullAngle;
		actor->spr.Angles.Yaw += angdif;
	}

	if (a & spin)
		actor->spr.Angles.Yaw += DAngle45 * BobVal(actor->temp_data[0] << 3);

	if (a & face_player_slow)
	{
		if (ps[pnum].newOwner != nullptr)
			goalang = (ps[pnum].GetActor()->opos.XY() - actor->spr.pos.XY()).Angle();
		else goalang = (ps[pnum].GetActor()->spr.pos.XY() - actor->spr.pos.XY()).Angle();
		angdif = DAngle22_5 * 0.25 * Sgn(deltaangle(actor->spr.Angles.Yaw, goalang).Degrees()); // this looks very wrong...
		actor->spr.Angles.Yaw += angdif;
	}

	if (isRRRA())
	{
		if (a & antifaceplayerslow)
		{
			if (ps[pnum].newOwner != nullptr)
				goalang = ((ps[pnum].GetActor()->opos.XY() - actor->spr.pos.XY()).Angle() + DAngle180);
			else goalang = ((ps[pnum].GetActor()->spr.pos.XY() - actor->spr.pos.XY()).Angle() + DAngle180);
			angdif = DAngle22_5 * 0.25 * Sgn(deltaangle(actor->spr.Angles.Yaw, goalang).Degrees()); // this looks very wrong...
			actor->spr.Angles.Yaw += angdif;
		}

		if ((a & jumptoplayer) == jumptoplayer)
		{
			if (actor->spr.picnum == RTILE_CHEER)
			{
				if (actor->temp_data[0] < 16)
					actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 1.6;
			}
			else
			{
				if (actor->temp_data[0] < 16)
					actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 2;
			}
		}
		if (a & justjump1)
		{
			if (actor->spr.picnum == RTILE_RABBIT)
			{
				if (actor->temp_data[0] < 8)
					actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 2.133;
			}
			else if (actor->spr.picnum == RTILE_MAMA)
			{
				if (actor->temp_data[0] < 8)
					actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 1.83;
			}
		}
		if (a & justjump2)
		{
			if (actor->spr.picnum == RTILE_RABBIT)
			{
				if (actor->temp_data[0] < 8)
					actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 2.667;
			}
			else if (actor->spr.picnum == RTILE_MAMA)
			{
				if (actor->temp_data[0] < 8)
					actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 2.286;
			}
		}
		if (a & windang)
		{
			if (actor->temp_data[0] < 8)
				actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4) * 2.667f);
		}
	}
	else if ((a & jumptoplayer) == jumptoplayer)
	{
		if (actor->temp_data[0] < 16)
			actor->vel.Z -= BobVal(512 + (actor->temp_data[0] << 4)) * 2;
	}


	if (a & face_player_smart)
	{
		DVector2 newpos = ps[pnum].GetActor()->spr.pos.XY() + (ps[pnum].vel.XY() * (4. / 3.));
		goalang = (newpos - actor->spr.pos.XY()).Angle();
		angdif = deltaangle(actor->spr.Angles.Yaw, goalang) * 0.25;
		if (angdif > -DAngle22_5 / 16 && angdif < nullAngle) angdif = nullAngle;
		actor->spr.Angles.Yaw += angdif;
	}

	if (actor->temp_data[1] == 0 || a == 0)
	{
		if ((badguy(actor) && actor->spr.extra <= 0) || (actor->opos.X != actor->spr.pos.X) || (actor->opos.Y != actor->spr.pos.Y))
		{
			if (!actor->isPlayer()) actor->backupvec2();
			SetActor(actor, actor->spr.pos);
		}
		if (badguy(actor) && actor->spr.extra <= 0)
		{
			if (actor->sector()->ceilingstat & CSTAT_SECTOR_SKY)
			{
				if (actor->sector()->shadedsector == 1)
				{
					actor->spr.shade += (16 - actor->spr.shade) >> 1;
				}
				else
				{
					actor->spr.shade += (actor->sector()->ceilingshade - actor->spr.shade) >> 1;
				}
			}
			else
			{
				actor->spr.shade += (actor->sector()->floorshade - actor->spr.shade) >> 1;
			}
		}
		return;
	}

	auto moveptr = &ScriptCode[actor->temp_data[1]];

	if (a & geth) actor->vel.X += (moveptr[0] / 16. - actor->vel.X) * 0.5;
	if (a & getv) actor->vel.Z += (moveptr[1] / 16. - actor->vel.Z) * 0.5;

	if (a & dodgebullet)
		dodge(actor);

	if (!actor->isPlayer())
		alterang(a, actor, pnum);

	if (abs(actor->vel.X) < 6 / 16.) actor->vel.X = 0;

	a = badguy(actor);

	if (actor->vel.X != 0 || actor->vel.Z != 0)
	{
		if (a)
		{
			if (actor->spr.picnum == RTILE_DRONE && actor->spr.extra > 0)
			{
				if (actor->vel.Z > 0)
				{
					float dist = isRRRA() ? 28 : 30;
					float f = getflorzofslopeptr(actor->sector(), actor->spr.pos.X, actor->spr.pos.Y);
					actor->floorz = f;
					if (actor->spr.pos.Z > f - dist)
						actor->spr.pos.Z = f - dist;
				}
				else
				{
					float c = getceilzofslopeptr(actor->sector(), actor->spr.pos.X, actor->spr.pos.Y);
					actor->ceilingz = c;
					if (actor->spr.pos.Z < c + 50)
					{
						actor->spr.pos.Z = c + 50;
						actor->vel.Z = 0;
					}
				}
			}
			if (actor->vel.Z > 0 && actor->floorz < actor->spr.pos.Z)
				actor->spr.pos.Z = actor->floorz;
			if (actor->vel.Z < 0)
			{
				float c = getceilzofslopeptr(actor->sector(), actor->spr.pos.X, actor->spr.pos.Y);
				if (actor->spr.pos.Z < c + 66)
				{
					actor->spr.pos.Z = c + 66;
					actor->vel.Z *= 0.5;
				}
			}
		}

		daxvel = actor->vel.X;
		angdif = actor->spr.Angles.Yaw;

		if (a)
		{
			if (xvel < 960 && actor->spr.scale.X > 0.25 )
			{

				daxvel = -(1024 - xvel) * maptoworld;
				angdif = (ps[pnum].GetActor()->spr.pos.XY() - actor->spr.pos.XY()).Angle();

				if (xvel < 512)
				{
					ps[pnum].vel.X = 0;
					ps[pnum].vel.Y = 0;
				}
				else
				{
					ps[pnum].vel.XY() *= gs.playerfriction - 0.125;
				}
			}
			else if (!actorflag(actor, SFLAG2_FLOATING))
			{
				if (!*(moveptr + 1))
				{
					if (actor->opos.Z != actor->spr.pos.Z || (ud.multimode < 2 && ud.player_skill < 2))
					{
						if ((actor->temp_data[0] & 1) || ps[pnum].actorsqu == actor) return;
						else daxvel *= 2;
					}
					else
					{
						if ((actor->temp_data[0] & 3) || ps[pnum].actorsqu == actor) return;
						else daxvel *= 4;
					}
				}
			}
		}
		if (isRRRA())
		{
			if (actor->sector()->lotag != 1)
			{
				switch (actor->spr.picnum)
				{
				case RTILE_MINIONBOAT:
				case RTILE_HULKBOAT:
				case RTILE_CHEERBOAT:
					daxvel *= 0.5;
					break;
				}
			}
			else if (actor->sector()->lotag == 1)
			{
				switch (actor->spr.picnum)
				{
				case RTILE_BIKERB:
				case RTILE_BIKERBV2:
				case RTILE_CHEERB:
					daxvel *= 0.5;
					break;
				}
			}
		}

		Collision coll;
		actor->movflag = movesprite_ex(actor, DVector3(angdif.ToVector() * daxvel, actor->vel.Z), CLIPMASK0, coll);
	}

	if (a)
	{
		if (actor->sector()->ceilingstat & CSTAT_SECTOR_SKY)
		{
			if (actor->sector()->shadedsector == 1)
			{
				actor->spr.shade += (16 - actor->spr.shade) >> 1;
			}
			else
			{
				actor->spr.shade += (actor->sector()->ceilingshade - actor->spr.shade) >> 1;
			}
		}
		else actor->spr.shade += (actor->sector()->floorshade - actor->spr.shade) >> 1;

		if (actor->sector()->floortexture == mirrortex)
			actor->Destroy();
	}
}

void fakebubbaspawn(DDukeActor *actor, int g_p)
{
	fakebubba_spawn++;
	switch (fakebubba_spawn)
	{
	default:
		break;
	case 1:
		spawn(actor, RTILE_PIG);
		break;
	case 2:
		spawn(actor, RTILE_MINION);
		break;
	case 3:
		spawn(actor, RTILE_CHEER);
		break;
	case 4:
		spawn(actor, RTILE_VIXEN);
		operateactivators(666, &ps[g_p]);
		break;
	}
}

//---------------------------------------------------------------------------
//
// special checks in fall that only apply to RR.
//
//---------------------------------------------------------------------------

static int fallspecial(DDukeActor *actor, int playernum)
{
	int sphit = 0;
	if (isRRRA())
	{
		if (actor->sector()->lotag == 801)
		{
			if (actor->spr.picnum == RTILE_ROCK)
			{
				spawn(actor, RTILE_ROCK2);
				spawn(actor, RTILE_ROCK2);
				addspritetodelete();
			}
			return 0;
		}
		else if (actor->sector()->lotag == 802)
		{
			if (!actor->isPlayer() && badguy(actor) && actor->spr.pos.Z == actor->floorz - FOURSLEIGHT_F)
			{
				spawnguts(actor, PClass::FindActor("DukeJibs6"), 5);
				S_PlayActorSound(SQUISHED, actor);
				addspritetodelete();
			}
			return 0;
		}
		else if (actor->sector()->lotag == 803)
		{
			if (actor->spr.picnum == RTILE_ROCK2)
				addspritetodelete();
			return 0;
		}
	}
	if (actor->sector()->lotag == 800)
	{
		if (actor->spr.picnum == RTILE_AMMO)
		{
			addspritetodelete();
			return 0;
		}
		if (!actor->isPlayer() && (badguy(actor) || actor->spr.picnum == RTILE_HEN || actor->spr.picnum == RTILE_COW || actor->spr.picnum == RTILE_PIG || actor->spr.picnum == RTILE_DOGRUN || actor->spr.picnum == RTILE_RABBIT) && (!isRRRA() || actor->spriteextra < 128))
		{
			actor->spr.pos.Z = actor->floorz - FOURSLEIGHT_F;
			actor->vel.Z = 8000 / 256.;
			actor->spr.extra = 0;
			actor->spriteextra++;
			sphit = 1;
		}
		else if (!actor->isPlayer())
		{
			if (!actor->spriteextra)
				addspritetodelete();
			return 0;
		}
		actor->attackertype = RTILE_SHOTSPARK1;
		actor->hitextra = 1;
	}
	else if (tilesurface(actor->sector()->floortexture) == TSURF_MAGMA)
	{
		if (actor->spr.picnum != RTILE_MINION && actor->spr.pal != 19)
		{
			if ((krand() & 3) == 1)
			{
				actor->attackertype = RTILE_SHOTSPARK1;
				actor->hitextra = 5;
			}
		}
	}	
	return sphit;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void fall_r(DDukeActor* ac, int g_p)
{
	fall_common(ac, g_p, RTILE_JIBS6, RTILE_DRONE, RTILE_BLOODPOOL, RTILE_SHOTSPARK1, 69, 158, fallspecial);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void destroyit(DDukeActor *actor)
{
	int lotag = 0, hitag = 0;
	DDukeActor* spr = nullptr;

	DukeSectIterator it1(actor->sector());
	while (auto a2 = it1.Next())
	{
		if (a2->spr.picnum == RTILE_RRTILE63)
		{
			lotag = a2->spr.lotag;
			spr = a2;
			if (a2->spr.hitag)
				hitag = a2->spr.hitag;
		}
	}
	DukeStatIterator it(STAT_DESTRUCT);
	while (auto a2 = it.Next())
	{
		auto it_sect = a2->sector();
		if (hitag && hitag == a2->spr.hitag)
		{
			DukeSectIterator its(it_sect);
			while (auto a3 = its.Next())
			{
				if (a3->spr.picnum == RTILE_DESTRUCTO)
				{
					a3->attackertype = RTILE_SHOTSPARK1;
					a3->hitextra = 1;
				}
			}
		}
		if (spr && spr->sector() != it_sect)
			if (lotag == a2->spr.lotag)
			{
				auto sect = spr->sector();

				auto destsect = spr->sector();
				auto srcsect = it_sect;

				auto destwal = destsect->walls.Data();
				auto srcwal = srcsect->walls.Data();
				for (unsigned i = 0; i < destsect->walls.Size(); i++, srcwal++, destwal++)
				{
					destwal->setwalltexture(srcwal->walltexture);
					destwal->setovertexture(srcwal->overtexture);
					destwal->shade = srcwal->shade;
					destwal->xrepeat = srcwal->xrepeat;
					destwal->yrepeat = srcwal->yrepeat;
					destwal->xpan_ = srcwal->xpan_;
					destwal->ypan_ = srcwal->ypan_;
					if (isRRRA() && destwal->twoSided())
					{
						destwal->cstat = 0;
						destwal->nextWall()->cstat = 0;
					}
				}
				destsect->setfloorz(srcsect->floorz);
				destsect->setceilingz(srcsect->ceilingz);
				destsect->ceilingstat = srcsect->ceilingstat;
				destsect->floorstat = srcsect->floorstat;
				destsect->setceilingtexture(srcsect->ceilingtexture);
				destsect->ceilingheinum = srcsect->ceilingheinum;
				destsect->ceilingshade = srcsect->ceilingshade;
				destsect->ceilingpal = srcsect->ceilingpal;
				destsect->ceilingxpan_ = srcsect->ceilingxpan_;
				destsect->ceilingypan_ = srcsect->ceilingypan_;
				destsect->setfloortexture(srcsect->floortexture);
				destsect->floorheinum = srcsect->floorheinum;
				destsect->floorshade = srcsect->floorshade;
				destsect->floorpal = srcsect->floorpal;
				destsect->floorxpan_ = srcsect->floorxpan_;
				destsect->floorypan_ = srcsect->floorypan_;
				destsect->visibility = srcsect->visibility;
				destsect->keyinfo = srcsect->keyinfo;
				destsect->lotag = srcsect->lotag;
				destsect->hitag = srcsect->hitag;
				destsect->extra = srcsect->extra;
				destsect->dirty = EDirty::AllDirty;
			}
	}
	it1.Reset(actor->sector());
	while (auto a2 = it1.Next())
	{
		switch (a2->spr.picnum)
		{
		case RTILE_DESTRUCTO:
		case RTILE_RRTILE63:
		case RTILE_TORNADO:
		case RTILE_APLAYER:
		case RTILE_COOT:
			break;
		default:
			a2->Destroy();
			break;
		}
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void mamaspawn(DDukeActor *actor)
{
	if (mamaspawn_count)
	{
		mamaspawn_count--;
		spawn(actor, RTILE_RABBIT);
	}
}

bool spawnweapondebris_r(int picnum)
{
	return true;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void think_r(void)
{
	thinktime.Reset();
	thinktime.Clock();

	movefta();			//ST 2
	tickstat(STAT_PROJECTILE);
	moveplayers();			//ST 10
	movefallers_r();		//ST 12
	tickstat(STAT_MISC, true);

	actortime.Reset();
	actortime.Clock();
	moveactors_r();			//ST 1
	actortime.Unclock();

	moveeffectors_r();		//ST 3
	tickstat(STAT_STANDABLE);
	doanimations();
	tickstat(STAT_FX);				//ST 11

	if (numplayers < 2 && thunderon)
		thunder();

	thinktime.Unclock();
}


END_DUKE_NS
