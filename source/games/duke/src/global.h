#pragma once

#include "build.h"
#include "duke3d.h"
#include "quotemgr.h"
#include "sounds.h"
#include "constants.h"
#include "types.h"
#include "d_net.h"
#include "serialize_obj.h"
#include "tiletexture.h"

BEGIN_DUKE_NS

extern user_defs ud;

struct DukeGameInfo
{
	// Static constant global state
	float playerfriction;
	float gravity;
	float playerheight;
	float gutsscale;

	int respawnactortime;
	int bouncemineblastradius;
	int respawnitemtime;
	int morterblastradius;
	int numfreezebounces;
	int pipebombblastradius;
	int rpgblastradius;
	int seenineblastradius;
	int shrinkerblastradius;
	int tripbombblastradius;
	int camerashitable;
	int max_player_health;
	int max_armour_amount;
	int lasermode;
	int freezerhurtowner;
	int impact_damage;
	int firstdebris;

	TileInfo tileinfo[MAXTILES]; // This is not from EDuke32.
	ActorInfo actorinfo[MAXTILES];
	int16_t max_ammo_amount[MAX_WEAPONS];
	int16_t weaponsandammosprites[15];
	int displayflags;
};

extern DukeGameInfo gs;

inline TObjPtr<DDukeActor*> camsprite;
inline TObjPtr<DDukeActor*> spriteq[1024];
inline TObjPtr<DDukeActor*> currentCommentarySprite;


extern int otherp; // transient helper, MP only
extern int actor_tog; // cheat state
extern intptr_t apScriptGameEvent[];
extern TArray<int> ScriptCode;
extern int playerswhenstarted;
extern int show_shareware;
extern int screenpeek;

// Variables that must be saved
extern int rtsplaying;

extern player_struct ps[MAXPLAYERS];
extern int spriteqamount;
extern int lastvisinc;
extern animwalltype animwall[MAXANIMWALLS];
extern int numanimwalls;
extern int numclouds;
extern int global_random;
extern int mirrorcnt;
extern int numplayersprites;
extern int spriteqloc;
extern int thunder_brightness;
inline DukeLevel dlevel;

enum animtype_t
{
	anim_floorz,
	anim_ceilingz,
	anim_vertexx,
	anim_vertexy,
};

struct animate
{
	sectortype* sect;
	int target;
	int8_t type;
	float goal;
	float vel;

};

extern TArray<animate> animates;

extern sectortype* clouds[256];
extern float cloudx;
extern float cloudy;
extern int cloudclock;

extern TArray<Cycler> cyclers;
extern TArray<AmbientTags> ambienttags;
extern sectortype* mirrorsector[64];
extern walltype* mirrorwall[64];

extern int wupass;
extern int thunderon;
extern int ufospawn;
extern int ufocnt;
extern int hulkspawn;
extern int lastlevel;

extern sectortype* geosectorwarp[MAXGEOSECTORS];
extern sectortype* geosectorwarp2[MAXGEOSECTORS];
extern sectortype* geosector[MAXGEOSECTORS];
extern float geox[MAXGEOSECTORS];
extern float geoy[MAXGEOSECTORS];
extern float geox2[MAXGEOSECTORS];
extern float geoy2[MAXGEOSECTORS];
extern int geocnt;

extern TArray<DVector2> mspos;
extern int WindTime;
extern DAngle WindDir;
extern short fakebubba_spawn, mamaspawn_count, banjosound;
extern uint8_t enemysizecheat /*raat607*/, pistonsound, chickenphase /* raat605*/, RRRA_ExitedLevel, fogactive;
extern uint32_t everyothertime;
extern player_orig po[MAXPLAYERS];
extern int32_t g_cdTrack;

END_DUKE_NS

#include "inlines.h"

