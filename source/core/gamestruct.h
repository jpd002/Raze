#pragma once

bool System_WantGuiCapture();	// During playing this tells us whether the game must be paused due to active GUI elememts.

#include <stdint.h>
#include "vectors.h"
#include "engineerrors.h"
#include "stats.h"
#include "packet.h"
#include "serializer.h"
#include "inputstate.h"
#include "maptypes.h"

class FSerializer;
struct FRenderViewpoint;
struct sectortype;
struct tspritetype;
class DCoreActor;
struct MapRecord;

struct GameStats
{
	int kill, tkill;
	int secret, tsecret;
	int timesecnd;
	int frags;
};

struct FNewGameStartup
{
	MapRecord* Map;
	int Episode;
	int Level;
	int Skill;
};

struct FSavegameInfo
{
	const char *savesig;
	int minsavever;
	int currentsavever;
};

enum EMenuSounds : int;
struct MapRecord;
struct TilesetBuildInfo;

extern cycle_t drawtime, actortime, thinktime, gameupdatetime;

struct GeoEffect
{
	sectortype** geosectorwarp;
	sectortype** geosectorwarp2;
	sectortype** geosector;
	float* geox;
	float* geoy;
	float* geox2;
	float* geoy2;
	int geocnt;

};

struct GameInterface
{
	virtual const char* Name() { return "$"; }
	virtual ~GameInterface() {}
	virtual bool GenerateSavePic() { return false; }
	virtual void app_init() = 0;
	virtual void LoadTextureInfo(TilesetBuildInfo& info) {}
	virtual void SetupSpecialTextures(TilesetBuildInfo&) {}
	virtual void loadPalette();
	virtual void clearlocalinputstate() {}
	virtual void UpdateScreenSize() {}
	virtual void FreeLevelData();
	virtual void FreeGameData() {}
	virtual void PlayHudSound() {}
	virtual GameStats getStats() { return {}; }
	virtual void MainMenuOpened() {}
	virtual void MenuOpened() {}
	virtual void MenuClosed() {}
	virtual void MenuSound(EMenuSounds snd) {}
	virtual bool CanSave() { return true; }
	virtual void CustomMenuSelection(int menu, int item) {}
	virtual bool StartGame(FNewGameStartup& gs) { return true; }
	virtual FSavegameInfo GetSaveSig() { return { "", 0, 0}; }
	virtual float SmallFontScale() { return 1; }
	virtual void SerializeGameState(FSerializer& arc) {}
	virtual void DrawPlayerSprite(const DVector2& origin, bool onteam) {}
	virtual void SetAmbience(bool on) {}
	virtual std::pair<DVector3, DAngle> GetCoordinates() { return {}; }
	virtual void ExitFromMenu() { throw CExitEvent(0); }
	virtual void GetInput(ControlInfo* const hidInput, float const scaleAdjust, InputPacket* packet = nullptr) {}
	virtual void UpdateSounds() {}
	virtual void ErrorCleanup() {}
	virtual void Startup() {}
	virtual void DrawBackground() {}
	virtual void Render() {}
	virtual void Ticker() {}
	virtual int GetPlayerChecksum(int pnum) { return 0x12345678 + pnum; }
	virtual const char *CheckCheatMode() { return nullptr; }
	virtual const char* GenericCheat(int player, int cheat) = 0;
	virtual void NextLevel(MapRecord* map, int skill) {}
	virtual void NewGame(MapRecord* map, int skill, bool special = false) {}
	virtual void LevelCompleted(MapRecord* map, int skill) {}
	virtual bool DrawAutomapPlayer(const DVector2& mxy, const DVector2& cpos, const DAngle cang, const DVector2& xydim, const float czoom, float const interpfrac) { return false; }
	virtual DAngle playerPitchMin() { return DAngle::fromDeg(57.375f); }
	virtual DAngle playerPitchMax() { return DAngle::fromDeg(-57.375f); }
	virtual void WarpToCoords(float x, float y, float z, DAngle a) {}
	virtual void ToggleThirdPerson() { }
	virtual void SwitchCoopView() { Printf("Unsupported command\n"); }
	virtual void ToggleShowWeapon() { Printf("Unsupported command\n"); }
	virtual void processSprites(tspriteArray& tsprites, const DVector3& view, DAngle viewang, float interpfrac) = 0;
	virtual void UpdateCameras(float smoothratio) {}
	virtual void EnterPortal(DCoreActor* viewer, int type) {}
	virtual void LeavePortal(DCoreActor* viewer, int type) {}
	virtual bool GetGeoEffect(GeoEffect* eff, sectortype* viewsector) { return false; }
	virtual int Voxelize(int sprnum) { return -1; }
	virtual void AddExcludedEpisode(const FString& episode) {}
	virtual int GetCurrentSkill() { return -1; }
	virtual bool IsQAVInterpTypeValid(const FString& type) { return false; }
	virtual void AddQAVInterpProps(const int res_id, const FString& interptype, const bool loopable, const TMap<int, TArray<int>>&& ignoredata) { }
	virtual void RemoveQAVInterpProps(const int res_id) { }
	virtual bool WantEscape() { return false; }
	virtual void StartSoundEngine() = 0;

	virtual FString statFPS()
	{
		FString output;

		output.AppendFormat("Actor think time: %.3f ms\n", actortime.TimeMS());
		output.AppendFormat("Total think time: %.3f ms\n", thinktime.TimeMS());
		output.AppendFormat("Game Update: %.3f ms\n", gameupdatetime.TimeMS());
		output.AppendFormat("Draw time: %.3f ms\n", drawtime.TimeMS());

		return output;
	}


};

extern GameInterface* gi;


void ImGui_Begin_Frame();

