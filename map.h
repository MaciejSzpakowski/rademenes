#pragma once

namespace ph
{
	struct cell
	{
		ulong visited;
		building* b;
		uint spriteIndex;
		uint roadSpriteIndex;
		cellType type;
		bool moisture;
		bool road;
	};

	struct smap
	{
		ulong id;
		int citizens;
		int employees, employeed, unemployed;
		building buildings[MAX_BUILDINGS];
		building* entrance;
		building* huntingSpot[10];
		building* fishingSpot[10];
		body bodies[MAX_BODIES];
		cell grid[50][50];
		int GRID_SIZE;
		int HALF;
		int deben;


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
		/// <summary>
		/// checks if cell is acceptable, does nothing if it's not
		/// </summary>
		void addRoad(int x, int y);
		building* addBuilding();
		void removeRoad(int x, int y);
		void fillMoisture();
		bool areaHasBody(int x, int y, int w, int h);
	};

	extern smap map;
}