#include "types.h"
#include "gl.h"
#include "building.h"
#include "body.h"
#include "map.h"
#include <cstdio>
#include <cmath>
#include "stream.h"

namespace ph
{
	int buildingCost[] = { 
		0,2,1,10,150,
		0,6,0,50,5,8,
		6,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
	};
	int buildingSize[][2] =
	{
		{0,0},{1,1},{1,1},{2,2},{3,3},
		{1,1},{1,1},{1,1},{4,4},{2,2},
		{2,2},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
		{1,1},{1,1},{1,1},{1,1},{1,1},
	};
	int housemaxOccupantsMap[] = { 0, 5, 7, 9 };

	uint initSprite(int x, int y, int w, int h, float r, float g, float b)
	{
		float color[3] = { r,g,b };
		return gl::addSprite(color, x, y, 0.4f, w, h);
	}

	bool building::tryDoor(cell* c, int x, int y)
	{
		if (c->road)
		{
			// skip corners
			if (x == this->x - 1 && y == this->y - 1 || x == this->x + this->w && y == this->y - 1 ||
				x == this->x - 1 && y == this->y + this->h || x == this->x + this->w && y == this->y + this->h)
				return false;

			this->door.set(x, y);
			return true;
		}

		return false;
	}

	bool initDoor(cell* c, int x, int y, building* b)
	{
		return !b->tryDoor(c, x, y);
	}

	bool recalcDoors(cell* c, int x, int y, building* b)
	{
		if (c->b && c->b->is(BUILDING_HASDOOR))
		{
			// this building has no door
			if (isNopos(&c->b->door))
			{
				c->b->updateDoor();
			}
			// check if door got obstructed
			else
			{
				cell* c2 = map.at(&c->b->door);
				if(c2->b) c->b->updateDoor();
			}
		}

		return true;
	}

	void building::updateDoor()
	{
		this->door.set(NOPOS, NOPOS);

		map.getBorder(this->x - 1, this->y - 1, this->w + 2, this->h + 2, (CELLIT)initDoor, this);
	}

	void addBuildingGrid(buildingType type, int x, int y, int w, int h)
	{
		for (uint i = 0; i < w; i++)
		{
			for (uint j = 0; j < h; j++)
			{
				building* b = map.addBuilding();
				b->init(type, x + i, y + j);
			}
		}
	}

	void building::init()
	{
		this->initGraphics();
		map.put(this, x, y);

		switch (type)
		{
		case buildingType::entry:
			map.entrance = this;
			break;
		}
	}

	void building::initGraphics()
	{
		if (this->sprite)
			gl::removeSprite(this->sprite);

		switch (this->type)
		{
		case buildingType::house:
			this->sprite = gl::addSprite((spriteType)(spriteType::h1 - 1 + this->houseLevel), (float)x, y, 0.4f, w, h); // initSprite(x, y, w, h, 1.0f, 0.5f, 0);
			break;
		case buildingType::waterSupply:
			this->sprite = initSprite(x, y, w, h, 0.5f, 0.5f, 1);
			break;
		case buildingType::statueLarge:
			this->sprite = initSprite(x, y, w, h, 0.5f, 0.5f, 0.5f);
			break;
		case buildingType::entry:
			this->sprite = initSprite(x, y, w, h, 1, 1, 1);
			break;
		case buildingType::fireHouse:
			this->sprite = initSprite(x, y, w, h, 1, 0.5f, 0.5f);
			break;
		case buildingType::fire:
			this->sprite = initSprite(x, y, w, h, 1, 1, 0);
			break;
		case buildingType::granary:
			this->sprite = initSprite(x, y, w, h, 0, 1, 0.5f);
			break;
		case buildingType::huntingLodge:
			this->sprite = initSprite(x, y, w, h, 0, 1, 0.5f);
			break;
		case buildingType::bazaar:
			this->sprite = initSprite(x, y, w, h, 0, 1, 0);
			break;
		case buildingType::architect:
			this->sprite = initSprite(x, y, w, h, 36/255.0f, 49/255.0f, 54/255.0f);
			break;
		case buildingType::rubble:
			this->sprite = initSprite(x, y, w, h, 0.25f, 0.25f, 0.25f);
			break;
		}
	}

	void building::init(buildingType type, int x, int y)
	{
		map.deben -= buildingCost[(int)type];
		this->id = map.getId();
		this->type = type;
		this->flags = FLAG_LIVE;
		this->x = x;
		this->y = y;
		this->door.set(NOPOS, NOPOS);
		this->occupants.zero();
		this->recruiters = 0;
		this->workers.set(0, 1);
		this->recruiterCounter = 0;
		this->workerCounter = MAX_WORKERCOUNTER;
		this->employementCounter = 0;
		this->fire.set(1000, 1000);
		this->collapse.set(1000, 1000);
		this->w = buildingSize[(int)type][0];
		this->h = buildingSize[(int)type][1];

		if (type == buildingType::house)
		{
			for (uint i = 0; i < MAX_RESOURCE_TYPES; i++)
				this->resources[i].set(0, 20000);
		}
		else
		{
			for (uint i = 0; i < MAX_RESOURCE_TYPES; i++)
				this->resources[i].set(0, 500);
		}

		switch (type)
		{
		case buildingType::house:
			this->occupants.max = 5;
			this->immigrants = 0;
			this->water.set(0, 500);
			this->houseLevel = 1;			
			this->flags |= BUILDING_FLAMABLE;
			break;
		case buildingType::waterSupply:
			this->occupants.max = 5;
			this->flags |= BUILDING_HASDOOR | BUILDING_WORKPLACE;
			this->walkerType = bodyType::waterCarrier;
			break;
		case buildingType::fireHouse:
			this->occupants.max = 6;
			this->flags |= BUILDING_HASDOOR | BUILDING_WORKPLACE;
			this->walkerType = bodyType::fire;
			break;
		case buildingType::entry:
			this->flags |= BUILDING_WALKABLE;
			map.entrance = this;
			break;
		case buildingType::fire:
			this->fire.set(200, 200);
			break;
		case buildingType::granary:
			this->occupants.max = 12;
			this->flags |= BUILDING_HASDOOR | BUILDING_WORKPLACE | BUILDING_FLAMABLE | BUILDING_COLLAPSABLE;
			break;
		case buildingType::huntingLodge:
			this->raw = goods::game;
			this->occupants.max = 6;
			this->workers.max = 3;
			this->flags |= BUILDING_HASDOOR | BUILDING_WORKPLACE | BUILDING_FLAMABLE | BUILDING_RESOURCE_GATHER;
			this->walkerType = bodyType::hunter;
			break;
		case buildingType::bazaar:
			this->occupants.max = 5;
			this->flags |= BUILDING_HASDOOR | BUILDING_WORKPLACE | BUILDING_FLAMABLE;
			break;
		case buildingType::architect:
			this->occupants.max = 5;
			this->flags |= BUILDING_HASDOOR | BUILDING_WORKPLACE;
			this->walkerType = bodyType::architect;
			break;
		case buildingType::animalSpawn:
			this->workerCounter = MAX_ANIMAL_SPAWN_COUNTER;
			this->occupants.max = MAX_ANIMAL_COUNT;
			break;
		}

		this->initGraphics();
		map.put(this, x, y);

		if (this->is(BUILDING_HASDOOR))
			this->updateDoor();
	}

	void building::action()
	{
		// TODO actual logic, if house has no road access then it disappears (no matter if it's occupied or not)
		// immigrant for house
		if (type == buildingType::house && map.entrance && //map.vacancies > map.immigrants && 
			this->immigrants < this->occupants.max - this->occupants.cur)
		{
			body* b = new immigrant();
			if (b)
				b->init(bodyType::immigrant, map.entrance->x, map.entrance->y, this);
		}
		// recruiter
		if (this->is(BUILDING_WORKPLACE) && this->door.x != LONG_MAX && 
			this->employementCounter < EMPLOYEECOUNTER_RECRUITER_THRESHOLD &&
			this->recruiters < MAX_RECRUITERS)
		{
			if (this->recruiterCounter < 1)
			{
				body* b = new recruiter();
				if (b)
				{
					this->recruiterCounter = MAX_RECRUITERCOUNTER;
					this->recruiters += 1;
					b->init(bodyType::recruiter, this->door.x, this->door.y, this);
				}
			}
			else
			{
				this->recruiterCounter -= 1;
			}
		}
		// walker aka service delivery
		if (this->is(BUILDING_WORKPLACE) && this->door.x != LONG_MAX && 
			this->workers.cur < this->workers.max && this->occupants.cur > 1)
		{
			if (this->workerCounter < 1 && (!this->is(BUILDING_RESOURCE_GATHER) || 
				this->resources[(int)this->raw].cur + this->workers.cur * 100 < this->resources[(int)this->raw].max))
			{
				body* b = nullptr;

				switch (this->type)
				{
				case buildingType::waterSupply:
					b = new waterCarrier();
					break;
				case buildingType::architect:
					b = new architect();
					break;
				case buildingType::fireHouse:
					b = new fireMarshal();
					break;
				case buildingType::huntingLodge:
					b = new hunter();
					break;
				}

				if (b)
				{
					this->workerCounter = this->type == buildingType::huntingLodge ? 2 : MAX_WORKERCOUNTER;
					this->workers.cur += 1;
					b->init(this->walkerType, this->door.x, this->door.y, this);
				}
			}
			else
			{
				float empRatio = (float)this->occupants.cur / (float)this->occupants.max;
				if (empRatio == 1.0f) workerCounter -= 4;
				else if (empRatio > 0.5f) workerCounter -= 2;
				else if (empRatio > 0.25f) workerCounter -= 1;
			}
		}

		// employementCounter and rebalance employees
		if (this->is(BUILDING_WORKPLACE) && this->occupants.cur > 0)
		{
			// more employees than citizens (because house destroyed or people left)
			if (map.employeed + map.unemployed > map.employees)
			{
				int removeEmployees = map.employeed + map.unemployed - map.employees;
				if (removeEmployees > this->occupants.cur) removeEmployees = this->occupants.cur;
				map.employeed -= removeEmployees;
				this->occupants.cur -= removeEmployees;

				if (this->occupants.cur == 0) this->workerCounter = MAX_WORKERCOUNTER;
			}

			if (this->employementCounter > 0)
			{
				this->employementCounter -= 1;
				this->recalcEmployees();
			}
		}

		// house consume resources and evolve
		if (this->type == buildingType::house && this->occupants.cur > 0)
		{
			this->evolveHouse();
			this->water.cur -= this->occupants.cur;
			if (this->water.cur < 0) this->water.cur = 0;
			// rebalance citizes
			if (this->occupants.cur > this->occupants.max)
			{
				int removePeople = this->occupants.cur - this->occupants.max;
				this->occupants.cur -= removePeople;
				map.citizens -= removePeople;
				map.employees -= removePeople;
			}
		}

		// fire
		if (this->is(BUILDING_FLAMABLE))
		{
			this->fire.cur -= 1;
			if (this->fire.cur < 0)
			{
				this->burnDown();
				return;
			}
		}
		else if (this->is(buildingType::fire))
		{
			this->fire.cur -= 1;
			if (this->fire.cur < 0)
			{
				int x = this->x;
				int y = this->y;
				this->remove();
				building* b = map.addBuilding();
				b->init(buildingType::rubble, x, y);
			}
		}

		// collapse
		if (this->is(BUILDING_COLLAPSABLE))
		{
			this->collapse.cur -= 1;
			if (this->collapse.cur < 0)
			{
				this->buildingCollapsed();
				return;
			}
		}

		if (this->is(buildingType::animalSpawn) && this->occupants.cur < this->occupants.max)
		{
			this->workerCounter -= 1;
			if (this->workerCounter < 1)
			{
				this->workerCounter = MAX_ANIMAL_SPAWN_COUNTER;

				// find random spot to spawn animal
				while (true)
				{
					int rx = gl::rand(-3, 3);
					int ry = gl::rand(-3, 3);
					cell* c = map.at(this->x + rx, this->y + ry);
					// try again if out of bounds, building, water (non water animal)
					if (!c || c->b || c->type == cellType::water && !this->is(BUILDING_WATER_ANIMAL_SPAWNER))
						continue;

					body* b = new animal();
					b->init(this->animalType, this->x + rx, this->y + ry, this);
					this->occupants.cur += 1;
					break;
				}
			}
		}
	}

	void building::buildingCollapsed()
	{
		int w = this->w;
		int h = this->h;
		int x = this->x;
		int y = this->y;
		this->remove();

		addBuildingGrid(buildingType::rubble, x, y, w, h);
	}

	void building::burnDown()
	{
		int w = this->w;
		int h = this->h;
		int x = this->x;
		int y = this->y;
		this->remove();

		addBuildingGrid(buildingType::fire, x, y, w, h);
	}

	void building::remove()
	{
		map.remove(this);
		gl::removeSprite(this->sprite);

		switch (this->type)
		{
		case buildingType::house:
			map.citizens -= this->occupants.cur;
			map.employees -= this->occupants.cur;
			break;
		}

		if (this->is(BUILDING_WORKPLACE))
		{
			map.employeed -= this->occupants.cur;
			map.unemployed += this->occupants.cur;
		}

		this->flags = 0;
	}

	void building::serialize(stream* it)
	{
		it->writeByte(this->is(FLAG_LIVE));

		if (!this->is(FLAG_LIVE)) return;

		/*it->writeUint64(this->id);
		it->writeInt32(this->x);
		it->writeInt32(this->y);
		it->writeInt32(this->w);
		it->writeInt32(this->h);
		it->writeInt32((int)this->type);
		it->writeInt32(this->occupants);
		it->writeInt32(this->occupants.max);
		it->writeInt32(this->immigrants);
		it->writeInt32(this->recruiters);
		it->writeInt32(this->workers);
		it->writeInt32(this->recruiterCounter);
		it->writeInt32(this->workerCounter);
		it->writeInt32(this->door.x);
		it->writeInt32(this->door.y);*/
		//it->writeByte(this->walkable);
		//it->writeByte(this->hasDoor);
		//it->writeByte(this->workplace);
	}

	void building::deserialize(stream* s)
	{
		//this->live = s->readByte();
		//if (!this->live) return;

		/*this->id = s->readUint64();
		this->x = s->readInt32();
		this->y = s->readInt32();
		this->w = s->readInt32();
		this->h = s->readInt32();
		this->type = (buildingType)s->readInt32();
		this->occupants = s->readInt32();
		this->occupants.max = s->readInt32();
		this->immigrants = s->readInt32();
		this->recruiters = s->readInt32();
		this->workers = s->readInt32();
		this->recruiterCounter = s->readInt32();
		this->workerCounter = s->readInt32();
		this->door.x = s->readInt32();
		this->door.y = s->readInt32();*/
		//this->walkable = s->readByte();
		//this->hasDoor = s->readByte();
		//this->workplace = s->readByte();

		this->init();
	}

	void building::recruit()
	{
		// actual logic
		// the moment recruiter gets employee access it fills all positions (unless not enough employees)
		// so building will gain and loose all employees at once (not incrementally)
		this->employementCounter += EMPLOYEMENTCOUNTER_INCREMENT;
		if (this->employementCounter > MAX_EMPLOYEMENTCOUNTER) this->employementCounter = MAX_EMPLOYEMENTCOUNTER;
		this->recalcEmployees();
	}

	void building::recalcEmployees()
	{
		massert(this->is(BUILDING_WORKPLACE));

		// hire
		if (this->employementCounter > 0 && this->occupants.cur < this->occupants.max)
		{
			int needed = this->occupants.max - this->occupants.cur;
			int newEmployees = needed < map.unemployed ? needed : map.unemployed;
			this->occupants.cur += newEmployees;
			map.unemployed -= newEmployees;
			map.employeed += newEmployees;
		}
		// quit
		else if (this->employementCounter < 1 && this->occupants.cur > 0)
		{
			this->workerCounter = MAX_WORKERCOUNTER;
			map.unemployed += this->occupants.cur;
			map.employeed -= this->occupants.cur;
			this->occupants.cur = 0;
		}
	}

	bool countFoodTypes(building* house)
	{
		int res = 0;

		for (uint i = (int)goods::game; i <= (int)goods::fish; i++)
		{
			if (house->resources[i].cur > 0) res += 1;
		}

		return res;
	}

	void building::evolveHouse()
	{
		massert(this->type == buildingType::house && this->occupants.cur > 0, "bad house");
		int oldLevel = this->houseLevel;
		
		// only one level at a time
		switch (this->houseLevel)
		{
		case 1:
			if (this->water.cur > 0)
				this->houseLevel += 1;
			break;
		case 2:
			if (this->water.cur < 1)
				this->houseLevel -= 1;
			else if (countFoodTypes(this) > 0)
				this->houseLevel += 1;
			break;
		case 3:
			if (countFoodTypes(this) == 0)
				this->houseLevel -= 1;
			break;
		}

		if (this->houseLevel != oldLevel)
		{
			this->initGraphics();
			this->occupants.max = housemaxOccupantsMap[this->houseLevel];
			this->water.max = this->occupants.max * 100;
			if (this->water.cur > this->water.max) this->water.cur = this->water.max;
		}
	}

	bool building::is(int flag)
	{
		return this->flags & flag;
	}

	bool building::is(buildingType type)
	{
		return this->type == type;
	}
}