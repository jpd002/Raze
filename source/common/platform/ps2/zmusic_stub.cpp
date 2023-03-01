#include <zmusic.h>

const char* ZMusic_GetLastError()
{
	return "Not implemented";
}

// Sets callbacks for functionality that the client needs to provide.
void ZMusic_SetCallbacks(const ZMusicCallbacks* callbacks)
{

}

// Sets GenMidi data for OPL playback. If this isn't provided the OPL synth will not work.
void ZMusic_SetGenMidi(const uint8_t* data)
{

}

// Set default bank for OPN. Without this OPN only works with custom banks.
void ZMusic_SetWgOpn(const void* data, unsigned len)
{

}

// Set DMXGUS data for running the GUS synth in actual GUS mode.
void ZMusic_SetDmxGus(const void* data, unsigned len)
{

}

EMIDIType ZMusic_IdentifyMIDIType(uint32_t* id, int size)
{
	return MIDI_MIDI;
}

ZMusic_MidiSource ZMusic_CreateMIDISource(const uint8_t* data, size_t length, EMIDIType miditype)
{
	return 0;
}

zmusic_bool ZMusic_MIDIDumpWave(ZMusic_MidiSource source, EMidiDevice devtype, const char* devarg, const char* outname, int subsong, int samplerate)
{
	return 0;
}

ZMusic_MusicStream ZMusic_OpenSong(ZMusicCustomReader* reader, EMidiDevice device, const char* Args)
{
	return 0;
}

ZMusic_MusicStream ZMusic_OpenSongFile(const char *filename, EMidiDevice device, const char* Args)
{
	return 0;
}

ZMusic_MusicStream ZMusic_OpenSongMem(const void *mem, size_t size, EMidiDevice device, const char* Args)
{
	return 0;
}

ZMusic_MusicStream ZMusic_OpenCDSong(int track, int cdid)
{
	return 0;
}

const ZMusicMidiOutDevice *ZMusic_GetMidiDevices(int *pAmount)
{
	return nullptr;
}

zmusic_bool ZMusic_FillStream(ZMusic_MusicStream stream, void* buff, int len)
{
	return 0;
}

zmusic_bool ZMusic_Start(ZMusic_MusicStream song, int subsong, zmusic_bool loop)
{
	return 0;
}

void ZMusic_Pause(ZMusic_MusicStream song)
{

}

void ZMusic_Resume(ZMusic_MusicStream song)
{

}

void ZMusic_Update(ZMusic_MusicStream song)
{

}

zmusic_bool ZMusic_IsPlaying(ZMusic_MusicStream song)
{
	return 0;
}

void ZMusic_Stop(ZMusic_MusicStream song)
{

}

void ZMusic_Close(ZMusic_MusicStream song)
{

}

zmusic_bool ZMusic_SetSubsong(ZMusic_MusicStream song, int subsong)
{
	return 0;
}

zmusic_bool ZMusic_IsLooping(ZMusic_MusicStream song)
{
	return 0;
}

int ZMusic_GetDeviceType(ZMusic_MusicStream song)
{
	return 0;
}

zmusic_bool ZMusic_IsMIDI(ZMusic_MusicStream song)
{
	return 0;
}

void ZMusic_VolumeChanged(ZMusic_MusicStream song)
{

}

zmusic_bool ZMusic_WriteSMF(ZMusic_MidiSource source, const char* fn, int looplimit)
{
	return 0;
}

void ZMusic_GetStreamInfo(ZMusic_MusicStream song, SoundStreamInfo *info)
{

}

zmusic_bool ChangeMusicSettingInt(EIntConfigKey key, ZMusic_MusicStream song, int value, int* pRealValue)
{
	return 0;
}

zmusic_bool ChangeMusicSettingFloat(EFloatConfigKey key, ZMusic_MusicStream song, float value, float* pRealValue)
{
	return 0;
}

zmusic_bool ChangeMusicSettingString(EStringConfigKey key, ZMusic_MusicStream song, const char* value)
{
	return 0;
}

const char *ZMusic_GetStats(ZMusic_MusicStream song)
{
	return "";
}

struct SoundDecoder* CreateDecoder(const uint8_t* data, size_t size, zmusic_bool isstatic)
{
	return nullptr;
}

void SoundDecoder_GetInfo(struct SoundDecoder* decoder, int* samplerate, ChannelConfig* chans, SampleType* type)
{

}

size_t SoundDecoder_Read(struct SoundDecoder* decoder, void* buffer, size_t length)
{
	return 0;
}

void SoundDecoder_Close(struct SoundDecoder* decoder)
{

}

void FindLoopTags(const uint8_t* data, size_t size, uint32_t* start, zmusic_bool* startass, uint32_t* end, zmusic_bool* endass)
{

}

int ZMusic_GetADLBanks(const char* const** pNames)
{
	return 0;
}
