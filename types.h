#pragma once

typedef unsigned __int64 ulong;
typedef unsigned int uint;
typedef unsigned char byte;
typedef void(__stdcall* VOIDPROC)();
typedef void(__stdcall* KEYDOWNPROC)(int key);

#define massert(cond,msg) gl::_assert(cond, __FUNCTION__## " " ## msg)

#define HUNTER_SHOT_AND_WAIT 3
#define MAX_GODS 5
#define MAX_ENTERTAINMENT 4
#define HUNTER_ATTACK_POWER 40
#define MAX_ANIMAL_SPAWN_COUNTER 20
#define NOPOS LONG_MAX
#define MAX_ANIMAL_COUNT 7 // actual value, animals per spawn point
#define MAX_ANIMAL_MOVE_COUNTER 40
#define MAX_RESOURCE_TYPES 31
#define MAX_PATH 30
#define MAX_BODIES 5000
#define MAX_BUILDINGS 5000
#define MAX_GRID_SIZE 500
#define MAX_WALKERS 4
#define MAX_RECRUITERS 1
#define MAX_EMPLOYEMENTCOUNTER 1000
#define EMPLOYEECOUNTER_RECRUITER_THRESHOLD (MAX_EMPLOYEMENTCOUNTER / 2)
#define MAX_RECRUITERCOUNTER 10
#define EMPLOYEMENTCOUNTER_INCREMENT 70
#define MAX_WORKERCOUNTER 40
#define FLAG_LIVE 0x1
#define BODY_ROAM 0x2
#define BODY_HAS_EFFECT 0x4
#define BODY_RETURN 0x8
#define BODY_ANIMAL 0x10
#define BODY_MOVING 0x20
#define BODY_WATER_ANIMAL 0x40
#define BODY_GAME 0x80
#define BODY_GAME_BEING_COLLECTED 0x100
#define BODY_DEAD 0x200
#define BODY_RUN_AWAY 0x400
#define BUILDING_WALKABLE 0x2
#define BUILDING_HASDOOR 0x4
#define BUILDING_WORKPLACE 0x8
#define BUILDING_FLAMABLE 0x10
#define BUILDING_COLLAPSABLE 0x20
#define BUILDING_WATER_ANIMAL_SPAWNER 0x40
// make sure buildings like hunter lodge, reed gatherer dont dispatch gatherers if they are full
#define BUILDING_RESOURCE_GATHER 0x80
#define REMOVE_FLAG(flag) this->flags ^= (flag);
#define HUNTER_SEARCH 0x2
#define HUNTER_PURSUE 0x4
#define HUNTER_ATTACK 0x8
#define HUNTER_COLLECT 0x10
#define HUNTER_RETURN 0x20
#define ANIMAL_CHILLIN 0x1
#define ANIMAL_WALK 0x2
#define ANIMAL_DEAD 0x4
#define ANIMAL_ATTACKING 0x8

namespace ph
{
	struct vec2i
	{
		union
		{
			int x, cur;
		};

		union
		{
			int y, max;
		};

		void set(int a, int b);
		void set(vec2i* v);
		void add(vec2i* v);
		void sub(vec2i* v);
		bool equals(vec2i* v);
		void zero();
		/// <summary>
		/// components becomes 1
		/// </summary>
		void norm();
	};

	enum class editorBuildType : int
	{
		empty, water, rock, gold, sand, tree, reed, meadow, field, entrance
	};

	enum class cellType : int
	{
		empty, water, rock, gold, sand, 
		tree, reed, meadow, field
	};

	enum spriteType
	{
		grass, water, rock, gold, sand, road, h1, h2, h3
	};

	enum class buildingType : int
	{
		none, house, road, waterSupply, statueLarge, 
		entry, exit, fireHouse, fire, granary, huntingLodge, 
		bazaar, architect, rubble, animalSpawn
	};

	enum class bodyType : int
	{
		immigrant, recruiter, waterCarrier, fire, ostrich, 
		architect, delivery, sklepikarz, hunter
	};

	enum class goods : int
	{
		// food
		game, grain, meat, lettuce, chickpeas, pomegranates, figs, fish,
		// finished
		pottery, beer, jewels, ivory, incense, linen, brick, papyrus, weapons, chariot,
		// raw
		clay, barley, flax, gems, reeds, wood, copper, gold, straw,
		// stone
		plainstone, limestone, granite, sandstone
	};

	struct stream;
	struct building;
	struct body;
	struct smap;
	struct cell;
	/// <summary>
	/// 'c' is guaranteed to exist, x and y are 'c' coords, data is passthru;
	/// if function returns false then stop iteration
	/// </summary>
	typedef bool(__stdcall* CELLIT)(cell* c, int x, int y, void* data);
	extern float gameTime;
	struct mousepos { float fworldx; float fworldy; int worldx; int worldy; };
	extern struct mousepos mouse;
	extern int buildingSize[][2];
	/// <summary>
	/// returns number of checkpoints
	/// </summary>
	int getPathRoad(vec2i* start, vec2i* end, vec2i* path);
	bool isNopos(vec2i* v);
}