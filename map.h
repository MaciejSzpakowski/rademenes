#pragma once

namespace ph
{
	struct cell
	{
		building* b;
		bool road;
		uint spriteIndex;
		uint roadSpriteIndex;
		bool moisture;
		cellType type;		
	};

	struct smap
	{
		ulong id;
		int citizens;
		int employees, employeed, unemployed;
		building buildings[MAX_BUILDINGS];
		building* entrance;
		body bodies[MAX_BODIES];
		cell grid[50][50];
		int GRID_SIZE;
		int HALF;

		void init();
		cell* at(int x, int y);
		void put(building* b, int x, int y);
		void action();
		void getArea(int x, int y, int w, int h, CELLIT it, void* data);
		void getBorder(int x, int y, int w, int h, CELLIT it, void* data);
		void getBorderNoCorners(int x, int y, int w, int h, CELLIT it, void* data);
		body* addBody();
		void remove(building* b);
		void serialize();
		void deserialize();
		ulong getId();
		void render(float tickProgress);
		void addRoad(int x, int y);
		building* addBuilding();
		void removeRoad(int x, int y);
	};

	extern smap map;
}