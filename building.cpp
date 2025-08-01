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
	uint initSprite(int x, int y, int w, int h, float r, float g, float b)
	{
		float color[3] = { r,g,b };
		return gl::addSprite(color, x, y, 0.4f, w, h);
	}

	bool building::tryDoor(cell* c, int x, int y)
	{
		if (c && (!c->b || c->b->walkable) && this->type == buildingType::house)
		{
			this->door[0] = x;
			this->door[1] = y;
			return true;
		}
		else if (c && c->road)
		{
			// skip corners
			if (x == this->x - 1 && y == this->y - 1 || x == this->x + this->w && y == this->y - 1 ||
				x == this->x - 1 && y == this->y + this->h || x == this->x + this->w && y == this->y + this->h)
				return false;

			this->door[0] = x;
			this->door[1] = y;
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
		if (c && c->b && c->b->hasDoor)
		{
			// this building has no door
			if (c->b->door[0] == LONG_MAX)
			{
				c->b->updateDoor();
			}
			// check if door got obstructed
			else
			{
				cell* c2 = map.at(c->b->door[0], c->b->door[1]);
				if(c2->b) c->b->updateDoor();
			}
		}

		return true;
	}

	void building::updateDoor()
	{
		this->door[0] = LONG_MAX;
		this->door[1] = LONG_MAX;

		map.getBorder(this->x - 1, this->y - 1, this->w + 2, this->h + 2, (CELLIT)initDoor, this);
	}

	void building::setSize(int w, int h)
	{
		this->w = w;
		this->h = h;
	}

	void building::init()
	{
		this->initGraphics();
		map.put(this, x, y);

		switch (type)
		{
		case buildingType::immigrantEntry:
			map.entrance = this;
			break;
		}
	}

	void building::initGraphics()
	{
		switch (this->type)
		{
		case buildingType::house:
			this->sprite = initSprite(x, y, w, h, 1.0f, 0.5f, 0);
			break;
		case buildingType::waterSupply:
			this->sprite = initSprite(x, y, w, h, 0.5f, 0.5f, 1);
			break;
		case buildingType::statueLarge:
			this->sprite = initSprite(x, y, w, h, 0.5f, 0.5f, 0.5f);
			break;
		case buildingType::immigrantEntry:
			this->sprite = initSprite(x, y, w, h, 1, 1, 1);
			break;
		case buildingType::fireHouse:
			this->sprite = initSprite(x, y, w, h, 1, 0.5f, 0.5f);
			break;
		case buildingType::fire:
			this->sprite = initSprite(x, y, w, h, 1, 1, 0);
			break;
		}
	}

	void building::init(buildingType type, int x, int y)
	{
		this->id = map.getId();
		this->type = type;
		this->live = true;
		this->walkable = false;
		this->x = x;
		this->y = y;
		this->door[0] = LONG_MAX;
		this->door[1] = LONG_MAX;
		this->hasDoor = false;
		this->occupants = 0;
		this->workplace = false;
		this->recruiters = 0;
		this->workers = 0;
		this->recruiterCounter = 0;
		this->workerCounter = MAX_WORKERCOUNTER;
		this->employementCounter = 0;
		this->maxWorkers = 1;
		this->flamable = false;

		switch (type)
		{
		case buildingType::house:
			this->setSize(1, 1);
			this->maxOccupants = 5;
			this->immigrants = 0;
			this->hasDoor = true;
			this->water[0] = 0;
			this->water[1] = 500;
			this->houseLevel = 1;
			this->fire[0] = 1000;
			this->fire[1] = 1000;
			this->flamable = true;
			break;
		case buildingType::waterSupply:
			this->setSize(2, 2);
			this->maxOccupants = 5;
			this->hasDoor = true;
			this->workplace = true;
			this->walkerType = bodyType::waterCarrier;
			break;
		case buildingType::fireHouse:
			this->setSize(1, 1);
			this->maxOccupants = 6;
			this->hasDoor = true;
			this->workplace = true;
			this->walkerType = bodyType::fire;
			break;
		case buildingType::statueLarge:
			this->setSize(3, 3);
			break;
		case buildingType::immigrantEntry:
			this->setSize(1, 1);
			this->walkable = true;
			map.entrance = this;
			break;
		case buildingType::fire:
			this->fire[0] = 200;
			this->fire[1] = 200;
			break;
		}

		this->initGraphics();
		map.put(this, x, y);

		if (this->hasDoor)
			this->updateDoor();

		// recalc doors around
		map.getBorder(this->x - 1, this->y - 1, this->w + 2, this->h + 2, (CELLIT)recalcDoors, nullptr);
	}

	void building::action()
	{
		// immigrant for house
		if (type == buildingType::house && map.entrance && //map.vacancies > map.immigrants && 
			this->immigrants < this->maxOccupants - this->occupants)
		{
			body* b = map.addBody();
			if (b)
				b->init(bodyType::immigrant, map.entrance->x, map.entrance->y, this);
		}
		// recruiter
		if (this->workplace && this->door[0] != LONG_MAX && (this->employementCounter < EMPLOYEECOUNTER_RECRUITER_THRESHOLD) &&
			this->recruiters < MAX_RECRUITERS)
		{
			if (this->recruiterCounter < 1)
			{
				body* b = map.addBody();
				if (b)
				{
					this->recruiterCounter = MAX_RECRUITERCOUNTER;
					this->recruiters += 1;
					b->init(bodyType::recruiter, this->door[0], this->door[1], this);
				}
			}
			else
			{
				this->recruiterCounter -= 1;
			}
		}
		// worker
		if (this->workplace && this->door[0] != LONG_MAX && this->workers < this->maxWorkers && this->occupants > 1)
		{
			if (this->workerCounter < 1)
			{
				body* b = map.addBody();
				if (b)
				{
					this->workerCounter = MAX_WORKERCOUNTER;
					this->workers += 1;
					b->init(this->walkerType, this->door[0], this->door[1], this);
				}
			}
			else
			{
				float empRatio = (float)this->occupants / (float)this->maxOccupants;
				if (empRatio == 1.0f) workerCounter -= 4;
				else if (empRatio > 0.5f) workerCounter -= 2;
				else if (empRatio > 0.25f) workerCounter -= 1;
			}
		}

		// employementCounter and rebalance employees
		if (this->workplace && this->occupants > 0)
		{
			// more employees than citizens (because house destroyed or people left)
			if (map.employeed + map.unemployed > map.employees)
			{
				int removeEmployees = map.employeed + map.unemployed - map.employees;
				if (removeEmployees > this->occupants) removeEmployees = this->occupants;
				map.employeed -= removeEmployees;
				this->occupants -= removeEmployees;

				if (this->occupants == 0) this->workerCounter = MAX_WORKERCOUNTER;
			}

			if (this->employementCounter > 0)
			{
				this->employementCounter -= 1;
				this->recalcEmployees();
			}
		}

		// house consume resources and evolve
		if (this->type == buildingType::house && this->occupants > 0)
		{
			this->evolveHouse();
			this->water[0] -= this->occupants;
			if (this->water[0] < 0) this->water[0] = 0;
			// rebalance citizes
			if (this->occupants > this->maxOccupants)
			{
				int removePeople = this->occupants - this->maxOccupants;
				this->occupants -= removePeople;
				map.citizens -= removePeople;
				map.employees -= removePeople;
			}
		}

		// fire
		if (this->flamable)
		{
			this->fire[0] -= 1;
			if (this->fire[0] < 0)
				this->burnDown();
		}
		else if (this->type == buildingType::fire)
		{
			this->fire[0] -= 1;
			if (this->fire[0] < 0)
				this->remove();
		}
	}

	void building::burnDown()
	{
		int w = this->w;
		int h = this->h;
		int x = this->x;
		int y = this->y;
		this->remove();
		building* b = map.addBuilding();
		massert(b);
		b->w = w;
		b->h = h;
		b->init(buildingType::fire, x, y);
	}

	void building::remove()
	{
		this->live = false;
		map.remove(this);
		gl::removeSprite(this->sprite);

		switch (this->type)
		{
		case buildingType::house:
			map.citizens -= this->occupants;
			map.employees -= this->occupants;
			break;
		}

		if (this->workplace)
		{
			map.employeed -= this->occupants;
			map.unemployed += this->occupants;
		}
	}

	void building::serialize(stream* it)
	{
		it->writeByte(this->live);

		if (!this->live) return;

		it->writeUint64(this->id);
		it->writeInt32(this->x);
		it->writeInt32(this->y);
		it->writeInt32(this->w);
		it->writeInt32(this->h);
		it->writeInt32((int)this->type);
		it->writeInt32(this->occupants);
		it->writeInt32(this->maxOccupants);
		it->writeInt32(this->immigrants);
		it->writeInt32(this->recruiters);
		it->writeInt32(this->workers);
		it->writeInt32(this->recruiterCounter);
		it->writeInt32(this->workerCounter);
		it->writeInt32(this->door[0]);
		it->writeInt32(this->door[1]);
		it->writeByte(this->walkable);
		it->writeByte(this->hasDoor);
		it->writeByte(this->workplace);
	}

	void building::deserialize(stream* s)
	{
		this->live = s->readByte();
		if (!this->live) return;

		this->id = s->readUint64();
		this->x = s->readInt32();
		this->y = s->readInt32();
		this->w = s->readInt32();
		this->h = s->readInt32();
		this->type = (buildingType)s->readInt32();
		this->occupants = s->readInt32();
		this->maxOccupants = s->readInt32();
		this->immigrants = s->readInt32();
		this->recruiters = s->readInt32();
		this->workers = s->readInt32();
		this->recruiterCounter = s->readInt32();
		this->workerCounter = s->readInt32();
		this->door[0] = s->readInt32();
		this->door[1] = s->readInt32();
		this->walkable = s->readByte();
		this->hasDoor = s->readByte();
		this->workplace = s->readByte();

		this->init();
	}

	void building::getSize(buildingType type, int* size)
	{
		switch (type)
		{
		case buildingType::house:
			size[0] = 1;
			size[1] = 1;
			break;
		case buildingType::waterSupply:
			size[0] = 2;
			size[1] = 2;
			break;
		case buildingType::statueLarge:
			size[0] = 3;
			size[1] = 3;
			break;
		case buildingType::immigrantEntry:
			size[0] = 1;
			size[1] = 1;
			break;
		}
	}

	void building::recruit()
	{
		// logic note
		// in pharaoh, the moment recruiter gets employee access it fills all positions (unless not enough employees)
		// so building will gain and loose all employees at once (not incrementally)
		this->employementCounter += EMPLOYEMENTCOUNTER_INCREMENT;
		if (this->employementCounter > MAX_EMPLOYEMENTCOUNTER) this->employementCounter = MAX_EMPLOYEMENTCOUNTER;
		this->recalcEmployees();
	}

	void building::recalcEmployees()
	{
		massert(this->workplace);

		// hire
		if (this->employementCounter > 0 && this->occupants < this->maxOccupants)
		{
			int needed = this->maxOccupants - this->occupants;
			int newEmployees = needed < map.unemployed ? needed : map.unemployed;
			this->occupants += newEmployees;
			map.unemployed -= newEmployees;
			map.employeed += newEmployees;
		}
		// quit
		else if (this->employementCounter == 0 && this->occupants > 0)
		{
			this->workerCounter = MAX_WORKERCOUNTER;
			map.unemployed += this->occupants;
			map.employeed -= this->occupants;
			this->occupants = 0;
		}
	}

	void building::evolveHouse()
	{
		massert(this->type == buildingType::house && this->occupants > 0, "bad house");
		int oldLevel = this->houseLevel;
		int maxOccupantsMap[] = { 0, 5, 7 };

		switch (this->houseLevel)
		{
		case 1:
			if (this->water[0] > 0)
				this->houseLevel += 1;
			break;
		case 2:
			if (this->water[0] == 0)
				this->houseLevel -= 1;
			break;
		}

		if (this->houseLevel != oldLevel)
		{
			this->maxOccupants = maxOccupantsMap[this->houseLevel];
			this->water[1] = this->maxOccupants * 100;
			if (this->water[0] > this->water[1]) this->water[0] = this->water[1];
		}
	}
}