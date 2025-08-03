#include "types.h"
#include "building.h"
#include "body.h"
#include "map.h"
#include "gl.h"
#include <cstdio>
#include <cstring>
#include "stream.h"
#include <cmath>

namespace ph
{	
	smap map;

	bool checkBounds(int x, int y, int w, int h)
	{
		return !(x < 0 || (x + w - 1) >= map.GRID_SIZE || y < 0 || (y + h - 1) >= map.GRID_SIZE);
	}

	cell* smap::at(int x, int y)
	{
		if (!checkBounds(x, y, 1, 1)) return nullptr;

		return &grid[x][y];
	}

	void smap::getArea(int x, int y, int w, int h, CELLIT it, void* data)
	{
		// int because it can be negative (out of bounds check will catch it)
		for (int i = x; i < x + w; i++)
		{
			for (int j = y; j < y + h; j++)
			{
				bool outOfBounds = !(i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE);
				cell* c = !outOfBounds ? &this->grid[i][j] : nullptr;
				// run only if c exists
				if (c && !it(c, i, j, data)) return;
			}
		}
	}

	void smap::getBorder(int x, int y, int w, int h, CELLIT it, void* data)
	{
		// int because it can be negative (out of bounds check will catch it)
		for (int i = x; i < x + w; i++)
		{
			for (int j = y; j < y + h; j++)
			{
				// skip inside
				if (i != x && i != x + w - 1 && j > y) j = y + h - 1;

				bool outOfBounds = !(i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE);
				cell* c = !outOfBounds ? &grid[i][j] : nullptr;
				// run only if c exists
				if (c && !it(c, i, j, data)) return;
			}
		}
	}

	void smap::getBorderNoCorners(int x, int y, int w, int h, CELLIT it, void* data)
	{
		// this wont work for 1x1, 1x2, 2x2
		if (w == 1 && h == 1 || w == 1 && h == 2 || w == 2 && h == 1 || w == 2 && h == 2) return;

		// int because it can be negative (out of bounds check will catch it)
		for (int i = x; i < x + w; i++)
		{
			for (int j = y; j < y + h; j++)
			{
				// skip inside
				if (i != x && i != x + w - 1 && j > y) j = y + h - 1;
				// skip corners
				if (i == x && j == y || i == x && j == y + h - 1 || i == x + w - 1 && j == y || i == x + w - 1 && j == y + h - 1) continue;

				bool outOfBounds = !(i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE);
				cell* c = !outOfBounds ? &grid[i][j] : nullptr;
				// run only if c exists
				if (c && !it(c, i, j, data)) return;
			}
		}
	}

	void smap::put(building* b, int x, int y)
	{
		if (!checkBounds(x, y, b->w, b->h))
			gl::error(__FUNCTION__ ". Out of bounds");

		for (uint i = x; i < b->w + x; i++)
		{
			for (uint j = y; j < b->h + y; j++)
			{
				if (grid[i][j].b)
				{
					gl::error(__FUNCTION__". Not empty");
				}

				grid[i][j].b = b;
			}
		}
	}

	void smap::remove(building* b)
	{
		for (uint i = b->x; i < b->w + b->x; i++)
		{
			for (uint j = b->y; j < b->h + b->y; j++)
			{
				massert(grid[i][j].b, "Empty");

				grid[i][j].b = nullptr;
			}
		}
	}

	bool roadAddedCallback(cell* c, int x, int y, void* _)
	{
		if (c && c->b && c->b->is(BUILDING_HASDOOR) && c->b->door[0] == LONG_MAX && !c->b->is(buildingType::house))
		{
			c->b->updateDoor();
		}

		return true;
	}

	void smap::addRoad(int x, int y)
	{
		cell* c = at(x, y);
		if (!c || c->b || c->road || c->type != cellType::empty) return;
		c->road = true;
		float color[3] = { 1,1,1 };
		c->roadSpriteIndex = gl::addSprite(color, x, y, 0.85f, 1, 1);

		// recalc roads, maybe some buildings gained access to road
		getBorder(x - 1, y - 1, 3, 3, roadAddedCallback, nullptr);
	}

	bool roadRemovedCallback(cell* c, int x, int y, void* _)
	{
		if (c && c->b && c->b->is(BUILDING_HASDOOR) && c->b->door[0] != LONG_MAX && !c->b->is(buildingType::house))
		{
			cell* currentDoor = map.at(c->b->door[0], c->b->door[1]);
			if (!currentDoor->road)
				c->b->updateDoor();
		}

		return true;
	}

	void smap::removeRoad(int x, int y)
	{
		cell* c = at(x, y);
		if (!c || !c->road) return;
		c->road = false;
		gl::removeSprite(c->roadSpriteIndex);

		// recalc roads, maybe some buildings lost access to road
		getBorder(x - 1, y - 1, 3, 3, roadRemovedCallback, nullptr);
	}

	building* smap::addBuilding()
	{
		for (uint i = 0; i < MAX_BUILDINGS; i++)
		{
			if (!this->buildings[i].is(FLAG_LIVE)) return map.buildings + i;
		}

		return nullptr;
	}

	bool fillMoistureCallback(cell* c, int x, int y, void* _)
	{
		// TODO need to make it faster
		if (c && !c->moisture)
		{
			c->moisture = true;
			float col[] = { 0.75f,0.75f,1 };
			gl::addSprite(col, x, y, 0.99f, 1, 1);
		}

		return true;
	}

	void smap::fillMoisture()
	{
		for (uint i = 0; i < GRID_SIZE; i++)
		{
			for (uint j = 0; j < GRID_SIZE; j++)
			{
				if (grid[i][j].type == cellType::water)
				{
					getArea(i - 6, j - 6, 13, 13, fillMoistureCallback, nullptr);
				}
			}
		}
	}

	void smap::init()
	{
		this->id = 0;
		this->citizens = 0;
		this->employees = 0;
		this->unemployed = 0;
		this->employeed = 0;
		this->entrance = nullptr;
		this->GRID_SIZE = 50;
		this->HALF = 25;
		this->deben = 5000;

		for (uint i = 0; i < MAX_BUILDINGS; i++)
		{
			buildings[i].flags = 0;
		}

		for (uint i = 0; i < MAX_BODIES; i++)
		{
			bodies[i].flags = 0;
		}

		for (uint i = 0; i < GRID_SIZE; i++)
		{
			for (uint j = 0; j < GRID_SIZE; j++)
			{
				grid[i][j] = {};
				grid[i][j].spriteIndex = -1;
				grid[i][j].roadSpriteIndex = -1;
				grid[i][j].type = cellType::empty;
			}
		}

		// mock deserialize
		float waterc[] = { 0,0,0.5f };

		for (uint i = 0; i < 10; i++)
		{
			for (uint j = 0; j < 10; j++)
			{
				grid[i][j].type = cellType::water;
				gl::addSprite(waterc, i, j, 0.99f, 1, 1);
			}
		}

		grid[25][25].type = cellType::water;
		gl::addSprite(waterc, 25, 25, 0.99f, 1, 1);

		this->fillMoisture();

		//deserialize();
	}

	void smap::action()
	{
		char buf[100];

		for (uint i = 0; i < MAX_BUILDINGS; i++)
		{
			building* b = buildings + i;

			if (!b->is(FLAG_LIVE)) continue;

			b->action();
		}

		for (uint i = 0; i < MAX_BODIES; i++)
		{
			body* b = bodies + i;

			if (!b->is(FLAG_LIVE)) continue;

			b->action();
		}
	}

	void smap::render(float tickProgress)
	{
		for (uint i = 0; i < MAX_BODIES; i++)
		{
			body* b = bodies + i;

			if (!b->is(FLAG_LIVE) || b->dir[0] == 0 && b->dir[1] == 0) continue;

			gl::updateSprite(b->sprite, b->x + b->dir[0] * tickProgress + 0.15f, b->y + b->dir[1] * tickProgress + 0.15f);
		}
	}

	body* smap::addBody()
	{
		for (uint i = 0; i < MAX_BODIES; i++)
		{
			if (!bodies[i].is(FLAG_LIVE)) return bodies + i;
		}

		return nullptr;
	}

	void smap::serialize()
	{
		stream s;
		s.openWriteFileStream("map.dat");
		s.writeUint64(this->id);
		s.writeInt32(this->citizens);
		s.writeInt32(this->employees);
		s.writeInt32(MAX_BUILDINGS);
		s.writeInt32(MAX_BODIES);

		// write ids first
		for (uint i = 0; i < MAX_BUILDINGS; i++)
			s.writeUint64(buildings[i].id);

		for (uint i = 0; i < MAX_BODIES; i++)
			s.writeUint64(bodies[i].id);

		for (uint i = 0; i < MAX_BUILDINGS; i++)
			buildings[i].serialize(&s);

		for (uint i = 0; i < MAX_BODIES; i++)
			bodies[i].serialize(&s);

		s.writeInt32(GRID_SIZE);
		s.writeInt32(GRID_SIZE);
		for (uint i = 0; i < GRID_SIZE; i++)
		{
			for (uint j = 0; j < GRID_SIZE; j++)
				s.writeByte(grid[i][j].road);
		}

		s.closeFileStream();
		gl::notify("Saved");
	}

	void smap::deserialize()
	{
		stream s;
		if (!s.openReadFileStream("./maps/map.dat"))
			return;

		this->id = s.readUint64();
		this->citizens = s.readInt32();
		this->employees = s.readInt32();
		int maxBuildings = s.readInt32();
		int maxBodies = s.readInt32();
		massert(maxBuildings > 0, "bad maxBuildings");
		massert(maxBodies > 0, "bad maxBodies");
		for (uint i = 0; i < maxBuildings; i++)
			buildings[i].id = s.readUint64();

		for (uint i = 0; i < maxBodies; i++)
			bodies[i].id = s.readUint64();

		for (uint i = 0; i < maxBuildings; i++)
			buildings[i].deserialize(&s);

		for (uint i = 0; i < maxBodies; i++)
			bodies[i].deserialize(&s);

		uint gridi = s.readInt32();
		uint gridj = s.readInt32();
		massert(gridi > 0, "bad gridi");
		massert(gridj > 0, "bad gridj");
		for (uint i = 0; i < gridi; i++)
		{
			for (uint j = 0; j < gridj; j++)
			{
				if(s.readByte())
					addRoad(i, j);
			}
		}
	}

	ulong smap::getId()
	{
		this->id += 1;
		return this->id;
	}
}