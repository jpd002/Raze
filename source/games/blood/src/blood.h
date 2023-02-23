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

#include "build.h"
#include "gamestruct.h"
#include "mapinfo.h"
#include "d_net.h"
#include "serialize_obj.h"

#include "common_game.h"
#include "fx.h"
#include "gameutil.h"
#include "db.h"

#include "actor.h"
#include "ai.h"
#include "aistate.h"
#include "aiunicult.h"
#include "callback.h"
#include "db.h"
#include "endgame.h"
#include "eventq.h"
#include "gib.h"
#include "globals.h"
#include "levels.h"
#include "misc.h"
#include "player.h"
#include "seq.h"
#include "sound.h"
#include "triggers.h"
#include "view.h"
#include "nnexts.h"
#include "player.h"
#include "misc.h"
#include "sectorfx.h"
#include "bloodactor.h"


BEGIN_BLD_NS

struct INIDESCRIPTION {
	const char* pzName;
	const char* pzFilename;
	const char** pzArts;
	int nArts;
};

struct INICHAIN {
	INICHAIN* pNext;
	char zName[BMAX_PATH];
	INIDESCRIPTION* pDescription;
};

extern INICHAIN* pINIChain;

extern int gNetPlayers;
extern int blood_globalflags;

void QuitGame(void);
void PreloadCache(void);
void ProcessFrame(void);
void ScanINIFiles(void);
void EndLevel(bool);

struct MIRROR
{
	int type;
	int link;
	DVector3 diff;
	int mynum;
};

extern MIRROR mirror[16];
extern int mirrorcnt, mirrorsector, mirrorwall[4];

inline bool DemoRecordStatus(void)
{
	return false;
}

inline bool VanillaMode()
{
	return false;
}
void sndPlaySpecialMusicOrNothing(int nMusic);

struct GameInterface : public ::GameInterface
{
	const char* Name() override { return "Blood"; }
	void app_init() override;
	void SerializeGameState(FSerializer& arc) override;
	void loadPalette() override;
	void clearlocalinputstate() override;
	bool GenerateSavePic() override;
	void FreeLevelData() override;
	void FreeGameData() override;
	FSavegameInfo GetSaveSig() override;
	void MenuOpened() override;
	void MenuClosed() override;
	bool CanSave() override;
	std::pair<DVector3, DAngle> GetCoordinates() override;
	void UpdateSounds() override;
	void GetInput(ControlInfo* const hidInput, float const scaleAdjust, InputPacket* packet = nullptr) override;
	void Ticker() override;
	void DrawBackground() override;
	void Startup() override;
	void Render() override;
	const char* GenericCheat(int player, int cheat) override;
	void NewGame(MapRecord* sng, int skill, bool) override;
	void NextLevel(MapRecord* map, int skill) override;
	void LevelCompleted(MapRecord* map, int skill) override;
	bool DrawAutomapPlayer(const DVector2& mxy, const DVector2& cpos, const DAngle cang, const DVector2& xydim, const float czoom, float const interpfrac) override;
	DAngle playerPitchMin() override { return DAngle::fromDeg(54.575f); }
	DAngle playerPitchMax() override { return DAngle::fromDeg(-43.15f); }
	void WarpToCoords(float x, float y, float z, DAngle a) override;
	void ToggleThirdPerson() override;
	void SwitchCoopView() override;
	void ToggleShowWeapon() override;
	void processSprites(tspriteArray& tsprites, const DVector3& view, DAngle viewang, float interpfrac) override;
	void EnterPortal(DCoreActor* viewer, int type) override;
	void LeavePortal(DCoreActor* viewer, int type) override;
	void LoadTextureInfo(TilesetBuildInfo& info) override;
	void SetupSpecialTextures(TilesetBuildInfo&) override;
	int GetCurrentSkill() override;
	bool IsQAVInterpTypeValid(const FString& type) override;
	void AddQAVInterpProps(const int res_id, const FString& interptype, const bool loopable, const TMap<int, TArray<int>>&& ignoredata) override;
	void RemoveQAVInterpProps(const int res_id) override;
	void StartSoundEngine() override;

	GameStats getStats() override;
};

END_BLD_NS
