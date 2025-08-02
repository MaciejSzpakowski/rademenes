#pragma once

typedef unsigned __int64 ulong;
typedef unsigned int uint;
typedef unsigned char byte;
typedef void(__stdcall* VOIDPROC)();
typedef void(__stdcall* KEYDOWNPROC)(int key);

#define massert(cond,msg) gl::_assert(cond, __FUNCTION__## " " ## msg)

#define MAX_BODIES 1000
#define MAX_BUILDINGS 1000
#define MAX_GRID_SIZE 500
#define MAX_WALKERS 4
#define MAX_RECRUITERS 1
#define MAX_EMPLOYEMENTCOUNTER 1000
#define EMPLOYEECOUNTER_RECRUITER_THRESHOLD (MAX_EMPLOYEMENTCOUNTER / 2)
#define MAX_RECRUITERCOUNTER 10
#define EMPLOYEMENTCOUNTER_INCREMENT 70
#define MAX_WORKERCOUNTER 40

namespace ph
{
	enum class editorBuildType : int
	{
		empty, water, rock, gold, sand, tree, reed, meadow, field, entrance
	};

	enum class cellType : int
	{
		empty, water, rock, gold, sand, tree, reed, meadow, field
	};

	enum spriteType
	{
		grass, water, rock, gold, sand, road, h1, h2, h3
	};

	enum class buildingType : int
	{
		none, house, road, waterSupply, statueLarge, immigrantEntry, fireHouse, fire, 
		granary, huntingLodge, bazaar, architect, rubble
	};

	enum class bodyType : int
	{
		immigrant, recruiter, waterCarrier, fire, ostrich, architect, delivery, sklepikarz, hunter
	};

	struct stream;
	struct building;
	struct body;
	struct smap;
	struct cell;
	typedef bool(__stdcall* CELLIT)(cell* c, int x, int y, void* data);
	extern float gameTime;
	struct mousepos { float fworldx; float fworldy; int worldx; int worldy; };
	extern struct mousepos mouse;
}