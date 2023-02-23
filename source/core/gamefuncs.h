#pragma once

#include "gamecontrol.h"
#include "gamestruct.h"
#include "build.h"
#include "coreactor.h"
#include "intrect.h"
#include "geometry.h"
#include "c_cvars.h"

extern IntRect viewport3d;

EXTERN_CVAR(Bool, hw_hightile)
EXTERN_CVAR(Bool, hw_models)
EXTERN_CVAR(Float, gl_texture_filter_anisotropic)
EXTERN_CVAR(Int, gl_texture_filter)
extern bool hw_int_useindexedcolortextures;
EXTERN_CVAR(Bool, hw_useindexedcolortextures)
EXTERN_CVAR(Bool, r_voxels)

inline int leveltimer;
inline int Numsprites;
inline int display_mirror;
inline int randomseed;
inline int g_visibility = 512, g_relvisibility = 0;

constexpr int SLOPEVAL_FACTOR = 4096;

enum
{
	CLIPMASK0 = (1 << 16) + 1,
	CLIPMASK1 = (256 << 16) + 64
};

//==========================================================================
//
// breadth first search, this gets used multiple times throughout the engine, mainly for iterating over sectors.
// Only works on indices, this has no knowledge of the actual objects being looked at.
// All objects of this type operate on the same shared store. Interleaved use is not allowed, nested use is fine.
//
//==========================================================================

class BFSSearch
{
	static inline TArray<unsigned> store;

	unsigned bitpos;
	unsigned startpos;
	unsigned curpos;

public:
	enum { EOL = ~0u };
	BFSSearch(unsigned datasize, unsigned startnode)
	{
		bitpos = store.Size();
		unsigned bitsize = (datasize + 31) >> 5;
		store.Reserve(bitsize);
		memset(&store[bitpos], 0, bitsize*4);

		startpos = store.Size();
		curpos = startpos;
		Set(startnode);
		store.Push(startnode);
	}

	// This allows this object to just work as a bit array
	// which is useful for using its shared storage.
	BFSSearch(unsigned datasize)
	{
		bitpos = store.Size();
		unsigned bitsize = (datasize + 31) >> 5;
		store.Reserve(bitsize);
		memset(&store[bitpos], 0, bitsize * 4);
	}

	~BFSSearch()
	{
		store.Clamp(bitpos);
	}

	bool Check(unsigned index) const
	{
		return !!(store[bitpos + (index >> 5)] & (1 << (index & 31)));
	}

	void Set(unsigned index)
	{
		store[bitpos + (index >> 5)] |= (1 << (index & 31));
	}


private:
public:
	unsigned GetNext()
	{
		curpos++;
		if (curpos <= store.Size())
			return store[curpos-1];
		else
			return ~0;
	}

	void Rewind()
	{
		curpos = startpos;
	}

	void Add(unsigned elem)
	{
		if (!Check(elem))
		{
			Set(elem);
			store.Push(elem);
		}
	}
};

class BFSSectorSearch : public BFSSearch
{
public:

	BFSSectorSearch(const sectortype* startnode) : BFSSearch(sector.Size(), sector.IndexOf(startnode))
	{
	}

	bool Check(const sectortype* index) const
	{
		return BFSSearch::Check(sector.IndexOf(index));
	}

	void Set(const sectortype* index)
	{
		BFSSearch::Set(sector.IndexOf(index));
	}

	sectortype* GetNext()
	{
		unsigned ret = BFSSearch::GetNext();
		return ret == EOL? nullptr : &sector[ret];
	}

	void Add(sectortype* elem)
	{
		BFSSearch::Add(sector.IndexOf(elem));
	}
};

//==========================================================================
//
// scans all vertices equivalent with a given spot and performs some work on them.
//
//==========================================================================

template<class func>
void vertexscan(walltype* startwall, func mark)
{
	BFSSearch walbitmap(wall.Size());

	// first pass: scan the the next-in-loop of the partner
	auto wal = startwall;
	do
	{
		mark(wal);
		walbitmap.Set(wall.IndexOf(wal));
		if (wal->nextwall < 0) break;
		wal = wal->nextWall()->point2Wall();
	} while (!walbitmap.Check(wall.IndexOf(wal)));

	// second pass: scan the partner of the previous-in-loop.
	wal = startwall;
	while (true)
	{
		auto thelastwall = wal->lastWall();
		// thelastwall can be null here if the map is bogus. 
		if (!thelastwall || thelastwall->nextwall < 0) break;
		wal = thelastwall->nextWall();
		if (walbitmap.Check(wall.IndexOf(wal))) break;
		mark(wal);
		walbitmap.Set(wall.IndexOf(wal));
	}
}


//==========================================================================
//
// 
//
//==========================================================================

inline void dragpoint(walltype* startwall, const DVector2& pos)
{
	vertexscan(startwall, [&](walltype* wal)
		{
			wal->move(pos);
			wal->sectorp()->exflags |= SECTOREX_DRAGGED;
		});
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

extern float cameradist, cameraclock;

bool calcChaseCamPos(DVector3& ppos, DCoreActor* pspr, sectortype** psectnum, const DRotator& angles, float const interpfrac, float const backamp);
int getslopeval(sectortype* sect, const DVector3& pos, float bazez);
bool cansee(const DVector3& start, sectortype* sect1, const DVector3& end, sectortype* sect2);
float intersectSprite(DCoreActor* actor, const DVector3& start, const DVector3& direction, DVector3& result, float maxfactor);
float intersectWallSprite(DCoreActor* actor, const DVector3& start, const DVector3& direction, DVector3& result, float maxfactor, bool checktex = false);
float intersectFloorSprite(DCoreActor* actor, const DVector3& start, const DVector3& direction, DVector3& result, float maxfactor);
float intersectSlopeSprite(DCoreActor* actor, const DVector3& start, const DVector3& direction, DVector3& result, float maxfactor);
float checkWallHit(walltype* wal, EWallFlags flagmask, const DVector3& start, const DVector3& direction, DVector3& result, float maxfactor);
float checkSectorPlaneHit(sectortype* sec, const DVector3& start, const DVector3& direction, DVector3& result, float maxfactor);
void neartag(const DVector3& start, sectortype* sect, DAngle angle, HitInfoBase& result, float neartagrange, int tagsearch);
int testpointinquad(const DVector2& pt, const DVector2* quad);
int hitscan(const DVector3& start, const sectortype* startsect, const DVector3& vect, HitInfoBase& hitinfo, unsigned cliptype, float maxrange = -1);

bool checkRangeOfWall(walltype* wal, EWallFlags flagmask, const DVector3& pos, float maxdist, float* theZs);
bool checkRangeOfFaceSprite(DCoreActor* itActor, const DVector3& pos, float maxdist, float* theZs);
bool checkRangeOfWallSprite(DCoreActor* itActor, const DVector3& pos, float maxdist, float* theZs);
bool checkRangeOfFloorSprite(DCoreActor* itActor, const DVector3& pos, float maxdist, float& theZ);
void getzrange(const DVector3& pos, sectortype* sect, float* ceilz, CollisionBase& ceilhit, float* florz, CollisionBase& florhit, float walldist, uint32_t cliptype);

bool checkOpening(const DVector2& inpos, float z, const sectortype* sec, const sectortype* nextsec, float ceilingdist, float floordist, bool precise = false);
int pushmove(DVector3& pos, sectortype** pSect, float walldist, float ceildist, float flordist, unsigned cliptype);
tspritetype* renderAddTsprite(tspriteArray& tsprites, DCoreActor* actor);

inline int pushmove(DVector2& pos, float z, sectortype** pSect, float walldist, float ceildist, float flordist, unsigned cliptype)
{
	auto vect = DVector3(pos, z);
	auto result = pushmove(vect, pSect, walldist, ceildist, flordist, cliptype);
	pos = vect.XY();
	return result;
}




int FindBestSector(const DVector3& pos);


tspritetype* renderAddTsprite(tspriteArray& tsprites, DCoreActor* actor);

void setWallSectors();
void GetWallSpritePosition(const spritetypebase* spr, const DVector2& pos, DVector2* out, bool render = false);
void GetFlatSpritePosition(DCoreActor* spr, const DVector2& pos, DVector2* out, float* outz = nullptr, bool render = false);
void GetFlatSpritePosition(const tspritetype* spr, const DVector2& pos, DVector2* out, float* outz, bool render = false);

enum class EClose
{
	Outside,
	InFront,
	Behind
};
EClose IsCloseToLine(const DVector2& vect, const DVector2& start, const DVector2& end, float walldist);
EClose IsCloseToWall(const DVector2& vect, walltype* wal, float walldist);

bool sectorsConnected(int sect1, int sect2);
int inside(float x, float y, const sectortype* sect);
int insidePoly(float x, float y, const DVector2* points, int count);

enum {
	NT_Lotag = 1,
	NT_Hitag = 2,
	NT_NoSpriteCheck = 4
};


//==========================================================================
//
// slope getter stuff (many wrappers, one worker only)
//
//==========================================================================

void calcSlope(const sectortype* sec, float xpos, float ypos, float* pceilz, float* pflorz);

//==========================================================================
//
// for the renderer
//
//==========================================================================

inline void PlanesAtPoint(const sectortype* sec, float dax, float day, float* pceilz, float* pflorz)
{
	float f, c;
	calcSlope(sec, dax, day, &c, &f);
	if (pceilz) *pceilz = -float(c);
	if (pflorz) *pflorz = -float(f);
}


//==========================================================================
//
// for the game engine
//
//==========================================================================

template<class Vector>
inline void calcSlope(const sectortype* sec, const Vector& pos, float* ceilz, float* florz)
{
	calcSlope(sec, pos.X, pos.Y, ceilz, florz);
}

inline float getceilzofslopeptr(const sectortype* sec, float dax, float day)
{
	float c;
	calcSlope(sec, dax, day, &c, nullptr);
	return c;
}
inline float getflorzofslopeptr(const sectortype* sec, float dax, float day)
{
	float f;
	calcSlope(sec, dax, day, nullptr, &f);
	return f;
}
template<class Vector>
inline float getceilzofslopeptr(const sectortype* sec, const Vector& pos)
{
	return getceilzofslopeptr(sec, pos.X, pos.Y);
}
template<class Vector>
inline float getflorzofslopeptr(const sectortype* sec, const Vector& pos)
{
	return getflorzofslopeptr(sec, pos.X, pos.Y);
}

//==========================================================================
//
// slope setters
//
//==========================================================================

inline void alignceilslope(sectortype* sect, const DVector3& pos)
{
	sect->setceilingslope(getslopeval(sect, pos, sect->ceilingz));
}

inline void alignflorslope(sectortype* sect, const DVector3& pos)
{
	sect->setfloorslope(getslopeval(sect, pos, sect->floorz));
}

//==========================================================================
//
// slope sprites
//
//==========================================================================

inline void spriteSetSlope(DCoreActor* actor, int heinum)
{
	if (actor->spr.cstat & CSTAT_SPRITE_ALIGNMENT_FLOOR)
	{
		actor->spr.xoffset = heinum & 255;
		actor->spr.yoffset = (heinum >> 8) & 255;
		actor->spr.cstat = (actor->spr.cstat & ~CSTAT_SPRITE_ALIGNMENT_MASK) | (heinum != 0 ? CSTAT_SPRITE_ALIGNMENT_SLOPE : CSTAT_SPRITE_ALIGNMENT_FLOOR);
	}
}

inline int spriteGetSlope(DCoreActor* actor)
{
	return ((actor->spr.cstat & CSTAT_SPRITE_ALIGNMENT_MASK) != CSTAT_SPRITE_ALIGNMENT_SLOPE) ? 0 : uint8_t(actor->spr.xoffset) + (int8_t(actor->spr.yoffset) << 8);
}

// same stuff, different flag...
inline int tspriteGetSlope(const tspritetype* spr)
{
	return !(spr->clipdist & TSPR_SLOPESPRITE) ? 0 : uint8_t(spr->xoffset) + (int8_t(spr->yoffset) << 8);
}

inline float spriteGetZOfSlopef(const spritetypebase* tspr, const DVector2& pos, int heinum)
{
	if (heinum == 0) return tspr->pos.Z;
	return tspr->pos.Z + heinum * -tspr->Angles.Yaw.ToVector().dot(pos - tspr->pos.XY()) * (1.f / SLOPEVAL_FACTOR);
}

//==========================================================================
//
// end of slopes
//
//==========================================================================


enum EFindNextSector
{
	Find_Floor = 0,
	Find_Ceiling = 1,
	
	Find_Down = 0,
	Find_Up = 2,
	
	Find_Safe = 4,
	
	Find_CeilingUp = Find_Ceiling | Find_Up,
	Find_CeilingDown = Find_Ceiling | Find_Down,
	Find_FloorUp = Find_Floor | Find_Up,
	Find_FloorDown = Find_Floor | Find_Down,
};
sectortype* nextsectorneighborzptr(sectortype* sectp, float startz, int flags);
int isAwayFromWall(DCoreActor* ac, float delta);


// important note: This returns positive for 'in front' with renderer coordinates.
// Due to Build's inverted coordinate system it will return negative for 'in front' there.
inline float PointOnLineSide(const DVector2 &pos, const walltype *line)
{
	return (pos.X - line->pos.X) * line->delta().Y - (pos.Y - line->pos.Y) * line->delta().X;
}


extern int numshades;

// Return type is int because this gets passed to variadic functions where structs may produce undefined behavior.
inline int shadeToLight(int shade)
{
	shade = clamp(shade, 0, numshades - 1);
	int light = Scale(numshades - 1 - shade, 255, numshades - 1);
	return PalEntry(255, light, light, light);
}

inline void copyfloorpal(tspritetype* spr, const sectortype* sect)
{
	if (!lookups.noFloorPal(sect->floorpal)) spr->pal = sect->floorpal;
}

inline int I_GetBuildTime()
{
	return I_GetTime(120);
}

// these are mainly meant as refactoring aids to mark function calls to work on.
inline int wallindex(const walltype* wal)
{
	return wall.IndexOf(wal);
}

inline int sectindex(const sectortype* sect)
{
	return sector.IndexOf(sect);
}

inline DVector2 NearestPointOnWall(float px, float py, const walltype* wal, bool clamp = true)
{
	return NearestPointOnLine(px, py, wal->pos.X, wal->pos.Y, wal->point2Wall()->pos.X, wal->point2Wall()->pos.Y, clamp);
}

inline float SquareDistToWall(float px, float py, const walltype* wal, DVector2* point = nullptr) 
{
	auto pt = NearestPointOnWall(px, py, wal);
	if (point) *point = pt;
	return SquareDist(px, py, pt.X, pt.Y);
}

float SquareDistToSector(float px, float py, const sectortype* sect, DVector2* point = nullptr);

inline float BobVal(int val)
{
	return g_sinbam((unsigned)val << 21);
}

inline float BobVal(float val)
{
	return g_sinbam(xs_CRoundToUInt(val * (1 << 21)));
}

inline DAngle GetMinPitch()
{
	return !cl_clampedpitch ? (DAngle90 - minAngle) : gi->playerPitchMin();
}

inline DAngle GetMaxPitch()
{
	return !cl_clampedpitch ? (minAngle - DAngle90) : gi->playerPitchMax();
}

inline DAngle ClampViewPitch(const DAngle pitch)
{
	return clamp(pitch, GetMaxPitch(), GetMinPitch());
}

inline void setFreeAimVelocity(float& vel, float& zvel, const DAngle pitch, const float zvspeed)
{
	vel *= pitch.Cos();
	zvel = pitch.Sin() * zvspeed;
}

#include "updatesector.h"
