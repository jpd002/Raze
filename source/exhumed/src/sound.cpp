//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT, sirlemonhead
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
#include "compat.h"
#include "baselayer.h"
#include "build.h"
#include "engine.h"
#include "exhumed.h"
#include "sound.h"
#include "init.h"
#include "object.h"
#include "player.h"
#include "random.h"
#include "snake.h"
#include "trigdat.h"
#include "sequence.h"
#include "cd.h"
#include "sound/s_soundinternal.h"

BEGIN_PS_NS

const char *SoundFiles[kMaxSoundFiles] =
{
  "spl_big",
  "spl_smal",
  "bubble_l",
  "grn_drop",
  "p_click",
  "grn_roll",
  "cosprite",
  "m_chant0",
  "anu_icu",
  "item_reg",
  "item_spe", // 10
  "item_key",
  "torch_on", // 12
  "jon_bnst",
  "jon_gasp",
  "jon_land",
  "jon_gags",
  "jon_fall",
  "jon_drwn",
  "jon_air1",
  "jon_glp1", // 20
  "jon_bbwl",
  "jon_pois",
  "amb_ston",
  "cat_icu",
  "bubble_h",
  "set_land",
  "jon_hlnd",
  "jon_laf2",
  "spi_jump",
  "jon_scub", // 30
  "item_use",
  "tr_arrow",
  "swi_foot",
  "swi_ston",
  "swi_wtr1",
  "tr_fire",
  "m_skull5",
  "spi_atak",
  "anu_hit",
  "fishdies", // 40
  "scrp_icu",
  "jon_wade",
  "amb_watr",
  "tele_1",
  "wasp_stg",
  "res",
  "drum4",
  "rex_icu",
  "m_hits_u",
  "q_tail", // 50
  "vatr_mov",
  "jon_hit3",
  "jon_t_2", // 53
  "jon_t_1",
  "jon_t_5",
  "jon_t_6",
  "jon_t_8",
  "jon_t_4",
  "rasprit1",
  "jon_fdie", // 60
  "wijaf1",
  "ship_1",
  "saw_on",
  "ra_on",
  "amb_ston", // 65
  "vatr_stp", // 66
  "mana1",
  "mana2",
  "ammo",
  "pot_pc1", // 70?
  "pot_pc2",
  "weapon",
  "alarm",
  "tick1",
  "scrp_zap", // 75
  "jon_t_3",
  "jon_laf1",
  "blasted",
  "jon_air2" // 79
};

short nStopSound;
short nStoneSound;
short nSwitchSound;
short nLocalEyeSect;
short nElevSound;
short nCreepyTimer;

bool looped[kMaxSounds];

short StaticSound[kMaxSounds];
int fakesources[] = { 0, 1, 2, 3 };
int swirlysources[4]= { 0, 1, 2, 3 };

int nLocalChan = 0;

//==========================================================================
//
//
//
//==========================================================================

class EXSoundEngine : public SoundEngine
{
    // client specific parts of the sound engine go in this class.
    void CalcPosVel(int type, const void* source, const float pt[3], int channum, int chanflags, FSoundID chanSound, FVector3* pos, FVector3* vel, FSoundChan* chan) override;
    TArray<uint8_t> ReadSound(int lumpnum) override;

public:
    EXSoundEngine()
    {
        int eax = 260;
        TArray<uint8_t> disttable(256, 1);

        for (int i = 0; i < 256; i++)
        {
            if (eax > 65280)
            {
                disttable[i] = 0;
            }
            else
            {
                disttable[i] = 255 - (eax >> 8);

                eax = (eax * eax) >> 8;
            }
        }

        S_Rolloff.RolloffType = ROLLOFF_Custom;
        S_Rolloff.MinDistance = 0;
        S_Rolloff.MaxDistance = 4096; // It's really this big
        Init(disttable, 255);
    }
};

//==========================================================================
//
//
// 
//==========================================================================

TArray<uint8_t> EXSoundEngine::ReadSound(int lumpnum)
{
    auto wlump = fileSystem.OpenFileReader(lumpnum);
    return wlump.Read();
}


//==========================================================================
//
//
//
//==========================================================================

int LoadSound(const char* name)
{
    FString nname(name, 8);
    int sndid = soundEngine->FindSoundNoHash(nname.GetChars());
    if (sndid > 0) return sndid - 1;

    FStringf filename("%s.voc", nname.GetChars());
    auto lump = S_LookupSound(filename);
    if (lump > 0)
    {
        auto &S_sfx = soundEngine->GetSounds();
        S_sfx.Reserve(1);
        int retval = S_sfx.Size() - 2;
        auto check = fileSystem.GetFileData(lump);
        if (check.Size() > 26 && check[26] == 6 && !memcmp("Creative Voice File", check.Data(), 19))
        {
            // This game uses the actual loop point information in the sound data as its only means to check if a sound is looped.
            looped[retval] = true;
        }
        auto& newsfx = S_sfx.Last();
        newsfx.Clear();
        newsfx.name = nname;
        newsfx.lumpnum = lump;
        newsfx.NearLimit = 6;
        newsfx.bTentative = false;
        soundEngine->CacheSound(retval + 1);
        return retval;
    }
    else if (!ISDEMOVER)  // demo tries to load sound files it doesn't have
    {
        Printf("Unable to open sound '%s'!\n", filename.GetChars());
    }
    return -1;
}
//==========================================================================
//
//
//
//==========================================================================

void InitFX(void)
{
    if (soundEngine) return; // just in case.
    soundEngine = new EXSoundEngine;

    auto& S_sfx = soundEngine->GetSounds();
    S_sfx.Resize(1);
    S_sfx[0].Clear(); S_sfx[0].lumpnum = sfx_empty; 
    for (size_t i = 0; i < kMaxSoundFiles; i++)
    {
        StaticSound[i] = LoadSound(SoundFiles[i]);
    }
    soundEngine->HashSounds();
    nCreepyTimer = kCreepyCount;
}


//==========================================================================
//
//
//
//==========================================================================

void GetSpriteSoundPitch(int* pVolume, int* pPitch)
{
    int nSoundSect = nPlayerViewSect[nLocalPlayer];
    int nLocalSectFlags = SectFlag[nSoundSect];
    if (nLocalSectFlags & kSectUnderwater)
    {
        *pVolume >>= 1;
        *pPitch -= 1200;
    }
}

//==========================================================================
//
//
//
//==========================================================================

void BendAmbientSound(void)
{
    soundEngine->EnumerateChannels([](FSoundChan* chan)
        {
            if (chan->SourceType == SOURCE_Ambient)
            {
                soundEngine->SetPitch(chan, (nDronePitch + 11800) / 11025.f);
            }
            return 1;
        });
}

//==========================================================================
//
//
//
//==========================================================================

void PlayLocalSound(short nSound, short nRate, bool unattached, EChanFlags cflags)
{
    if (nSound < 0 || nSound >= kMaxSounds || !soundEngine->isValidSoundId(nSound + 1))
    {
        initprintf("PlayLocalSound: Invalid sound nSound == %i, nRate == %i\n", nSound, nRate);
        return;
    }
    if (looped[nSound]) cflags |= CHANF_LOOP;

    FSoundChan* chan;
    if (!unattached)
    {
        soundEngine->StopSound(SOURCE_None, nullptr, CHAN_BODY);
        chan = soundEngine->StartSound(SOURCE_None, nullptr, nullptr, CHAN_BODY, cflags, nSound + 1, 1.f, ATTN_NONE, nullptr);
    }
    else
    {
        chan = soundEngine->StartSound(SOURCE_None, nullptr, nullptr, CHAN_VOICE, CHANF_OVERLAP|cflags, nSound + 1, 1.f, ATTN_NONE, nullptr);
    }

    if (nRate && chan)
    {
        float ratefac = (11025 + nRate) / 11025.f;
        soundEngine->SetPitch(chan, ratefac);
    }
}

//==========================================================================
//
//
//
//==========================================================================

int LocalSoundPlaying(void)
{
    return soundEngine->EnumerateChannels([](FSoundChan* chan)
        {
            return chan->SourceType == SOURCE_None;
        });
}

//==========================================================================
//
//
//
//==========================================================================

void StopLocalSound(void)
{
    soundEngine->StopSound(SOURCE_None, nullptr, -1);
}

//==========================================================================
//
//
//
//==========================================================================
int nNextFreq;
int nSwirlyFrames;

void StartSwirly(int nActiveSound)
{
    auto &swirly = swirlysources[nActiveSound];

    short nPitch = nNextFreq - RandomSize(9);
    nNextFreq = 25000 - RandomSize(10) * 6;
    if (nNextFreq > 32000)
        nNextFreq = 32000;

    int nVolume = nSwirlyFrames + 1;
    if (nVolume >= 220)
        nVolume = 220;

    soundEngine->StopSound(SOURCE_Swirly, &swirly, -1);
    soundEngine->StartSound(SOURCE_Swirly, &fakesources[nActiveSound-1], nullptr, CHAN_BODY, 0, StaticSound[kSoundMana1]+1, nVolume / 255.f, ATTN_NONE, nullptr, nPitch / 11025.f);
}

//==========================================================================
//
//
//
//==========================================================================

void StartSwirlies()
{
    StopAllSounds();

    nNextFreq = 19000;
    nSwirlyFrames = 0;

    for (int i = 0; i <= 4; i++)
        StartSwirly(i);
}

//==========================================================================
//
//
//
//==========================================================================

void UpdateSwirlies()
{
    nSwirlyFrames++;
    for (int i = 0; i <= 4; i++)
    {
        if (!soundEngine->IsSourcePlayingSomething(SOURCE_Swirly, &swirlysources[i], -1))
            StartSwirly(i);
    }
}

//==========================================================================
//
//
//
//==========================================================================

void SoundBigEntrance(void)
{
    StopAllSounds();
    for (int i = 0; i < 4; i++)
    {
        short nPitch = i * 512 - 1200;
        //pASound->snd_pitch = nPitch;
        soundEngine->StopSound(SOURCE_EXBoss, &fakesources[i], -1);
        soundEngine->StartSound(SOURCE_EXBoss, &fakesources[i], nullptr, CHAN_BODY, 0, StaticSound[kSoundTorchOn]+1, 200 / 255.f, ATTN_NONE, nullptr, nPitch / 11025.f);
    }
}


//==========================================================================
//
//
//
//==========================================================================

void EXSoundEngine::CalcPosVel(int type, const void* source, const float pt[3], int channum, int chanflags, FSoundID chanSound, FVector3* pos, FVector3* vel, FSoundChan* chan)
{
    if (pos != nullptr)
    {
        vec3_t campos;
        if (nSnakeCam > -1)
        {
            Snake* pSnake = &SnakeList[nSnakeCam];
            spritetype* pSnakeSprite = &sprite[pSnake->nSprites[0]];
            campos.x = pSnakeSprite->x;
            campos.y = pSnakeSprite->y;
            campos.z = pSnakeSprite->z;
        }
        else
        {
            campos = { initx, inity, initz };
        }
        auto fcampos = GetSoundPos(&campos);

        if (type == SOURCE_Unattached || type == SOURCE_Ambient)
        {
            pos->X = pt[0];
            pos->Y = pt[1];
            pos->Z = pt[2];
        }
        // Do some angular magic. The original was just 2D panning which in a 3D sound field is not sufficient.
        else if (type == SOURCE_Swirly)
        {
            int which = *(int*)source;
            float phase = ((int)totalclock << (4 + which)) * (M_PI / 1024);
            pos->X = fcampos.X + 256 * cos(phase);
            pos->Z = fcampos.Z + 256 * sin(phase);
        }
        else  if (type == SOURCE_EXBoss)
        {
            int which = *(int*)source;
            *pos = fcampos;
            // Should be positioned in 90� intervals.
            switch (which)
            {
            default:
            case 0: pos->X -= 256; break;
            case 1: pos->Z -= 256; break;
            case 2: pos->X += 256; break;
            case 3: pos->Z += 256; break;
            }
        }
        else if (type == SOURCE_Actor)
        {
            auto actor = (spritetype*)source;
            assert(actor != nullptr);
            if (actor != nullptr)
            {
                *pos = GetSoundPos(&actor->pos);
            }
        }
        if ((chanflags & CHANF_LISTENERZ) && type != SOURCE_None)
        {
            pos->Y = fcampos.Z;
        }
    }
}

//==========================================================================
//
//
//
//==========================================================================

void UpdateSounds()
{
    if (nFreeze)
        return;

    int nLocalSectFlags = SectFlag[nPlayerViewSect[nLocalPlayer]];

    vec3_t pos;
    short ang;
    if (nSnakeCam > -1)
    {
        Snake *pSnake = &SnakeList[nSnakeCam];
        spritetype *pSnakeSprite = &sprite[pSnake->nSprites[0]];
        pos = pSnakeSprite->pos;
        ang = pSnakeSprite->ang;
    }
    else
    {
        pos = { initx, inity, initz };
        ang = inita;
    }
    auto fv = GetSoundPos(&pos);
    SoundListener listener;
    listener.angle = -(float)ang * pi::pi() / 1024; // Build uses a period of 2048.
    listener.velocity.Zero();
    listener.position = GetSoundPos(&pos);
    listener.underwater = false;
    // This should probably use a real environment instead of the pitch hacking in S_PlaySound3D.
    // listenactor->waterlevel == 3;
    //assert(primaryLevel->Zones.Size() > listenactor->Sector->ZoneNumber);
    listener.Environment = 0;// primaryLevel->Zones[listenactor->Sector->ZoneNumber].Environment;
    listener.valid = true;


    soundEngine->SetListener(listener);
    soundEngine->UpdateSounds((int)totalclock);
    soundEngine->EnumerateChannels([](FSoundChan* chan)
        {
            if (!(chan->ChanFlags & (CHANF_UI|CHANF_FORGETTABLE)))
            {
                int nVolume = 255;
                int nPitch = int(chan->Pitch * (11025.f / 128.f)) - 11025;
                GetSpriteSoundPitch(&nVolume, &nPitch);
                soundEngine->SetPitch(chan, (11025 + nPitch) / 11025.f);
                soundEngine->SetVolume(chan, nVolume / 255.f);
            }
            return 0;
        });
}

//==========================================================================
//
//
//
//==========================================================================

int soundx, soundy, soundz;
short soundsect;

void PlayFX2(unsigned short nSound, short nSprite)
{
    if ((nSound&0x1ff) >= kMaxSounds || !soundEngine->isValidSoundId((nSound & 0x1ff)+1))
    {
        initprintf("PlayFX2: Invalid sound nSound == %i, nSprite == %i\n", nSound, nSprite);
        return;
    }

    if (nSprite >= 0)
    {
        nSprite &= 0xfff;
        soundx = sprite[nSprite].x;
        soundy = sprite[nSprite].y;
        soundz = sprite[nSprite].z;
    }

    int nVolume = 255;
    short v10 = (nSound&0xe00)>>9;
    nSound &= 0x1ff;

    int nPitch = 0;
    if (v10) nPitch = -(totalmoves&((1<<v10)-1))*16;

    GetSpriteSoundPitch(&nVolume, &nPitch);

    if (nSprite)
    {
        soundEngine->StartSound(SOURCE_Actor, &sprite[nSprite], nullptr, CHAN_BODY, CHANF_OVERLAP, nSound+1, nVolume / 255.f, ATTN_NORM, nullptr, (11025 + nPitch) / 11025.f);
    }
    else
    {
        vec3_t v = { soundx, soundy, soundz };
        FVector3 vv = GetSoundPos(&v);
        soundEngine->StartSound(SOURCE_Unattached, nullptr, &vv, CHAN_BODY, CHANF_OVERLAP, nSound+1, nVolume / 255.f, ATTN_NONE, nullptr, (11025 + nPitch) / 11025.f);
    }

    // Nuke: added nSprite >= 0 check
    if (nSprite != nLocalSpr && nSprite >= 0 && (sprite[nSprite].cstat&257))
        nCreepyTimer = kCreepyCount;
}

//==========================================================================
//
//
//
//==========================================================================

void PlayFXAtXYZ(unsigned short ax, int x, int y, int z, int nSector)
{
    soundx = x;
    soundy = y;
    soundz = z;
    soundsect = nSector&0x3fff;
    PlayFX2(ax, -1);
}

//==========================================================================
//
//
//
//==========================================================================

void CheckAmbience(short nSector)
{
    if (SectSound[nSector] != -1)
    {
        short nSector2 = SectSoundSect[nSector];
        walltype* pWall = &wall[sector[nSector2].wallptr];
        if (!soundEngine->IsSourcePlayingSomething(SOURCE_Ambient, nullptr, -1))
        {
            vec3_t v = { pWall->x, pWall->y, sector[nSector2].floorz };
            FVector3 vv = GetSoundPos(&v);
            soundEngine->StartSound(SOURCE_Ambient, nullptr, &vv, CHAN_BODY, CHANF_NONE, SectSound[nSector], 1.f, ATTN_NORM);
            return;
        }
        soundEngine->EnumerateChannels([=](FSoundChan* chan)
            {
                if (chan->SourceType == SOURCE_Ambient)
                {
                    FVector3 vv;
                    if (nSector == nSector2)
                    {
                        spritetype* pSprite = &sprite[PlayerList[0].nSprite];
                        vv = GetSoundPos(&pSprite->pos);
                    }
                    else
                    {
                        vec3_t v = { pWall->x, pWall->y, sector[nSector2].floorz };
                        vv = GetSoundPos(&v);
                    }
                    chan->Point[0] = vv.X;
                    chan->Point[1] = vv.Y;
                    chan->Point[2] = vv.Z;
                    return 1;
                }
                return 0;
            });

    }
    else
    {
        soundEngine->StopSound(SOURCE_Ambient, nullptr, -1);
    }
}


//==========================================================================
//
//
//
//==========================================================================

void UpdateCreepySounds()
{
    if (levelnum == 20 || nFreeze)
        return;
    spritetype* pSprite = &sprite[PlayerList[nLocalPlayer].nSprite];
    nCreepyTimer--;
    if (nCreepyTimer <= 0)
    {
        if (nCreaturesLeft > 0 && !(SectFlag[nPlayerViewSect[nLocalPlayer]] & 0x2000))
        {
            int vsi = seq_GetFrameSound(SeqOffsets[kSeqCreepy], totalmoves % SeqSize[SeqOffsets[kSeqCreepy]]);
            if (vsi >= 0 && (vsi & 0x1ff) < kMaxSounds)
            {
                int vdx = (totalmoves + 32) & 31;
                if (totalmoves & 1)
                    vdx = -vdx;
                int vax = (totalmoves + 32) & 63;
                if (totalmoves & 2)
                    vax = -vax;

                PlayFXAtXYZ(vsi, pSprite->x + vdx, pSprite->y + vax, pSprite->z, pSprite->sectnum);
            }
        }
        nCreepyTimer = kCreepyCount;
    }
}


//==========================================================================
//
//
//
//==========================================================================

void StopSpriteSound(short nSprite)
{
    if (nSprite >= 0 && nSprite < MAXSPRITES)
        soundEngine->StopSound(SOURCE_Actor, &sprite[nSprite], -1);
}

void StopAllSounds(void)
{
    soundEngine->StopAllChannels();
}

//==========================================================================
//
//
//
//==========================================================================

void PlayTitleSound(void)
{
    PlayLocalSound(StaticSound[kSoundItemSpecial], 0, false, CHANF_UI);
}

void PlayLogoSound(void)
{
    PlayLocalSound(StaticSound[kSoundJonLaugh2], 7000, false, CHANF_UI);
}

void PlayGameOverSound(void)
{
    PlayLocalSound(StaticSound[kSoundJonLaugh2], 0, false, CHANF_UI);
}

END_PS_NS
