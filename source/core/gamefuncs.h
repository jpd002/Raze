#pragma once

#include "gamecontrol.h"
#include "binaryangle.h"
#include "build.h"

// breadth first search, this gets used multiple times throughout the engine, mainly for iterating over sectors.
// Only works on indices, this has no knowledge of the actual objects being looked at.
// All objects of this type operate on the same shared store. Interleaved use is not allowed, nested use is fine.
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
		if (!thelastwall->twoSided()) break;

		wal = thelastwall->nextWall();
		if (walbitmap.Check(wall.IndexOf(wal))) break;
		mark(wal);
		walbitmap.Set(wall.IndexOf(wal));
	}
}


extern int cameradist, cameraclock;

void loaddefinitionsfile(const char* fn, bool cumulative = false, bool maingrp = false);

bool calcChaseCamPos(int* px, int* py, int* pz, DCoreActor* pspr, sectortype** psectnum, binangle ang, fixedhoriz horiz, double const smoothratio);

void PlanesAtPoint(const sectortype* sec, float dax, float day, float* ceilz, float* florz);

int getslopeval(sectortype* sect, int x, int y, int z, int planez);



void setWallSectors();
void GetWallSpritePosition(const tspritetype* spr, vec2_t pos, vec2_t* out, bool render = false);
void GetFlatSpritePosition(const spritetype* spr, vec2_t pos, vec2_t* out, bool render = false);
void GetFlatSpritePosition(const tspritetype* spr, vec2_t pos, vec2_t* out, int* outz = nullptr, bool render = false);
void checkRotatedWalls();
bool sectorsConnected(int sect1, int sect2);
void dragpoint(walltype* wal, int newx, int newy);

// y is negated so that the orientation is the same as in GZDoom, in order to use its utilities.
// The render code should NOT use Build coordinates for anything!

inline double RenderX(int x)
{
	return x * (1 / 16.);
}

inline double RenderY(int y)
{
	return y * (1 / -16.);
}

inline double WallStartX(int wallnum)
{
    return wall[wallnum].pos.X * (1 / 16.);
}

inline double WallStartY(int wallnum)
{
    return wall[wallnum].pos.Y * (1 / -16.);
}

inline double WallEndX(int wallnum)
{
    return wall[wallnum].point2Wall()->pos.X * (1 / 16.);
}

inline double WallEndY(int wallnum)
{
    return wall[wallnum].point2Wall()->pos.Y * (1 / -16.);
}

inline double WallStartX(const walltype* wallnum)
{
    return wallnum->pos.X * (1 / 16.);
}

inline double WallStartY(const walltype* wallnum)
{
    return wallnum->pos.Y * (1 / -16.);
}

inline DVector2 WallStart(const walltype* wallnum)
{
    return { WallStartX(wallnum), WallStartY(wallnum) };
}

inline double WallEndX(const walltype* wallnum)
{
    return wallnum->point2Wall()->pos.X * (1 / 16.);
}

inline double WallEndY(const walltype* wallnum)
{
	return wallnum->point2Wall()->pos.Y * (1 / -16.);
}

inline DVector2 WallEnd(const walltype* wallnum)
{
    return { WallEndX(wallnum), WallEndY(wallnum) };
}

inline DVector2 WallDelta(const walltype* wallnum)
{
    return WallEnd(wallnum) - WallStart(wallnum);
}

inline double PointOnLineSide(double x, double y, double linex, double liney, double deltax, double deltay)
{
    return (x - linex) * deltay - (y - liney) * deltax;
}

inline double PointOnLineSide(const DVector2 &pos, const walltype *line)
{
    return (pos.X - WallStartX(line)) * WallDelta(line).Y - (pos.Y - WallStartY(line)) * WallDelta(line).X;
}

template<class T>
inline double PointOnLineSide(const TVector2<T>& pos, const TVector2<T>& linestart, const TVector2<T>& lineend)
{
    return (pos.X - linestart.X) * (lineend.Y - linestart.Y) - (pos.Y - linestart.Y) * (lineend.X - linestart.X);
}

extern int numshades;

// Return type is int because this gets passed to variadic functions where structs may produce undefined behavior.
inline int shadeToLight(int shade)
{
    shade = clamp(shade, 0, numshades - 1);
    int light = Scale(numshades - 1 - shade, 255, numshades - 1);
    return PalEntry(255, light, light, light);
}

inline void copyfloorpal(spritetype* spr, const sectortype* sect)
{
    if (!lookups.noFloorPal(sect->floorpal)) spr->pal = sect->floorpal;
}

inline void copyfloorpal(tspritetype* spr, const sectortype* sect)
{
	if (!lookups.noFloorPal(sect->floorpal)) spr->pal = sect->floorpal;
}

inline void spriteSetSlope(spritetype* spr, int heinum)
{
    if (spr->cstat & CSTAT_SPRITE_ALIGNMENT_FLOOR)
    {
        spr->xoffset = heinum & 255;
        spr->yoffset = (heinum >> 8) & 255;
        spr->cstat = (spr->cstat & ~CSTAT_SPRITE_ALIGNMENT_MASK) | (heinum != 0 ? CSTAT_SPRITE_ALIGNMENT_SLOPE : CSTAT_SPRITE_ALIGNMENT_FLOOR);
    }
}

inline int spriteGetSlope(const spritetype* spr)
{
    return ((spr->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) != CSTAT_SPRITE_ALIGNMENT_SLOPE) ? 0 : uint8_t(spr->xoffset) + (uint8_t(spr->yoffset) << 8);
}

// same stuff, different flag...
inline int tspriteGetSlope(const tspritetype* spr)
{
	return !(spr->cstat2 & CSTAT2_SPRITE_SLOPE) ? 0 : uint8_t(spr->xoffset) + (uint8_t(spr->yoffset) << 8);
}

inline int32_t tspriteGetZOfSlope(const tspritetype* tspr, int dax, int day)
{
	int heinum = tspriteGetSlope(tspr);
	if (heinum == 0) return tspr->pos.Z;

	int const j = DMulScale(bsin(tspr->ang + 1024), day - tspr->pos.Y, -bsin(tspr->ang + 512), dax - tspr->pos.X, 4);
	return tspr->pos.Z + MulScale(heinum, j, 18);
}


inline int I_GetBuildTime()
{
    return I_GetTime(120);
}

inline int32_t getangle(walltype* wal)
{
    return getangle(
        wal->point2Wall()->pos.X - wal->pos.X,
        wal->point2Wall()->pos.Y - wal->pos.Y);
}

inline TArrayView<walltype> wallsofsector(const sectortype* sec)
{
    return TArrayView<walltype>(sec->firstWall(), sec->wallnum);
}

inline TArrayView<walltype> wallsofsector(int sec)
{
    return wallsofsector(&sector[sec]);
}

// these are mainly meant as refactoring aids to mark function calls to work on.
inline int wallnum(const walltype* wal)
{
	return wall.IndexOf(wal);
}

inline int sectnum(const sectortype* sect)
{
	return sector.IndexOf(sect);
}

inline double SquareDist(double lx1, double ly1, double lx2, double ly2)
{
	double dx = lx2 - lx1;
	double dy = ly2 - ly1;
	return dx * dx + dy * dy;
}

inline double SquareDistToWall(double px, double py, const walltype* wal) 
{
	double lx1 = wal->pos.X;
	double ly1 = wal->pos.Y;
	double lx2 = wal->point2Wall()->pos.X;
	double ly2 = wal->point2Wall()->pos.Y;
	
	double wall_length = SquareDist(lx1, ly1, lx2, ly2);

	if (wall_length == 0) return SquareDist(px, py, lx1, ly1);

	double t = ((px - lx1) * (lx2 - lx1) + (py - ly1) * (ly2 - ly1)) / wall_length;
	t = clamp(t, 0., 1.);
	return SquareDist(px, py, lx1 + t * (lx2 - lx1), ly1 + t * (ly2 - ly1));
}

inline void alignceilslope(sectortype* sect, int x, int y, int z)
{
	sect->setceilingslope(getslopeval(sect, x, y, z, sect->ceilingz));
}

inline void alignflorslope(sectortype* sect, int x, int y, int z)
{
	sect->setfloorslope(getslopeval(sect, x, y, z, sect->floorz));
}
inline void updatesectorneighbor(int32_t const x, int32_t const y, sectortype* * const sect, int32_t maxDistance = MAXUPDATESECTORDIST)
{
	int sectno = *sect? sector.IndexOf(*sect) : -1;
	updatesectorneighbor(x, y, &sectno, maxDistance);
	*sect = sectno < 0? nullptr : &sector[sectno];
}
