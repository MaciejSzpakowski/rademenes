#include "types.h"
#include <cstdio>
#include "body.h"
#include "building.h"
#include "gl.h"
#include "map.h"
#include "stream.h"
#include <cmath>

namespace ph
{
	struct vector2
	{
		int x, y;
	};
	
	bool effect(cell* c, int x, int y, body* b)
	{
		if (c) b->effect(c, x, y);
		return true;
	}

	// each walker may hit building 3 times because area to search is 3x3 so by the time they pass
	// the building of interest was in the area of effect 3 times
	void body::effect(cell* c, int x, int y)
	{
		building* b = c->b;

		switch (this->type)
		{
		case bodyType::recruiter:			
			if (this->roam && b && b->type == buildingType::house && b->occupants > 0)
			{
				building* targetb = this->getb();
				if (targetb) targetb->recruit();
			}
			break;
		case bodyType::waterCarrier:
			if (b && b->type == buildingType::house && b->occupants > 0)
			{
				b->water[0] = b->water[1];
			}
			break;
		}
	}

	bool getRoad(cell* c, int x, int y, vector2* dst)
	{
		if (c && c->road)
		{
			if (dst[0].x == LONG_MAX) dst[0] = { x,y };
			else if (dst[1].x == LONG_MAX) dst[1] = { x,y };
			else if (dst[2].x == LONG_MAX) dst[2] = { x,y };
			else if (dst[3].x == LONG_MAX) dst[3] = { x,y };
		}

		return true;
	}

	uint initSprite(int x, int y, float r, float g, float b)
	{
		float color[3] = { r,g,b };
		return gl::addSprite(color, x, y, 0.3f, 0.8f, 0.8f);
	}

	void body::init()
	{
		this->initGraphics();
	}

	void body::init(bodyType type, int x, int y, building* target)
	{
		this->id = map.getId();
		this->type = type;
		this->live = true;
		this->x = x;
		this->y = y;
		this->stamina[0] = 40;
		this->checkpoint[0] = LONG_MAX;
		this->checkpoint[1] = LONG_MAX;
		this->dir[0] = 0;
		this->dir[1] = 0;		
		this->roam = false;
		this->sprite = LONG_MAX;
		this->hasEffect = false;

		if (target)
		{
			this->targetb[0] = target->x;
			this->targetb[1] = target->y;
			this->target[0] = target->door[0];
			this->target[1] = target->door[1];
			this->targetbId = target->id;
		}
		else
		{
			this->targetb[0] = LONG_MAX;
			this->targetb[1] = LONG_MAX;
			this->target[0] = LONG_MAX;
			this->target[1] = LONG_MAX;
			this->targetbId = 0;
		}

		switch (this->type)
		{
		case bodyType::immigrant:
			target->immigrants += 1;
			break;
		case bodyType::recruiter:
		case bodyType::waterCarrier:
		case bodyType::fire:
			this->roam = true;
			this->hasEffect = true;
			break;
		}

		this->initGraphics();
	}

	void body::initGraphics()
	{
		switch (this->type)
		{
		case bodyType::immigrant:
			this->sprite = initSprite(x, y, 1, 0, 1);
			break;
		case bodyType::recruiter:
			this->sprite = initSprite(x, y, 0, 0, 0);
			break;
		case bodyType::waterCarrier:
			this->sprite = initSprite(x, y, 0.5f, 0.5f, 1);
			break;
		case bodyType::fire:
			this->sprite = initSprite(x, y, 1.0f, 0.5f, 0.5f);
			break;
		}
	}

	void body::initDir()
	{
		if (this->target[0] > this->x)
		{
			this->dir[0] = 1;
			this->dir[1] = 0;
			this->checkpoint[0] = this->target[0];
			this->checkpoint[1] = this->y;
		}
		else if (this->target[0] < this->x)
		{
			this->dir[0] = -1;
			this->dir[1] = 0;
			this->checkpoint[0] = this->target[0];
			this->checkpoint[1] = this->y;
		}
		else if (this->target[1] > this->y)
		{
			this->dir[0] = 0;
			this->dir[1] = 1;
			this->checkpoint[0] = this->x;
			this->checkpoint[1] = this->target[1];
		}
		else if (this->target[1] < this->y)
		{
			this->dir[0] = 0;
			this->dir[1] = -1;
			this->checkpoint[0] = this->x;
			this->checkpoint[1] = this->target[1];
		}
	}

	void body::arrive()
	{
		cell* c = map.at(this->targetb[0], this->targetb[1]);
		building* b = c ? c->b : nullptr;

		// check if targetb is still valid
		switch (this->type)
		{
		case bodyType::immigrant:
			if (b && b->live && b->type == buildingType::house && 
				b->occupants < b->maxOccupants)
			{
				map.citizens += 1;
				map.employees += 1;
				map.unemployed += 1;
				b->occupants += 1;
				b->immigrants -= 1;
			}
			break;
		case bodyType::recruiter:
			if (b && b->live && b->id == this->targetbId)
			{
				b->recruiters -= 1;
			}
			break;
		case bodyType::waterCarrier:
		case bodyType::fire:
			if (b && b->live && b->id == this->targetbId)
			{
				b->workers -= 1;
			}
		}

		this->remove();
	}

	void body::action()
	{
		// always remove stamina
		if (this->roam && this->stamina[0] > 0)
			this->stamina[0] -= 1;

		// move
		if (this->dir[0] != 0 || this->dir[1] != 0)
		{
			this->x += this->dir[0];
			this->y += this->dir[1];
			gl::updateSprite(this->sprite, this->x, this->y);
		}

		// effect
		if (hasEffect)
		{
			map.getBorder(this->x - 1, this->y - 1, 3, 3, (CELLIT)ph::effect, this);
		}
		
		// direction
		if (this->roam && this->stamina[0] > 0)
		{
			vector2 directions[] = { {LONG_MAX,LONG_MAX },{LONG_MAX,LONG_MAX },{LONG_MAX,LONG_MAX },{LONG_MAX,LONG_MAX } };
			map.getBorderNoCorners(this->x - 1, this->y - 1, 3, 3, (CELLIT)getRoad, directions);
			uint roadCount = 0;
			for (uint i = 0; i < 4; i++)
			{
				if (directions[i].x != LONG_MAX)
					roadCount += 1;
			}

			// first step
			if (this->dir[0] == 0 && this->dir[1] == 0)
			{
				gl::shuffleArray((byte*)directions, 4, 8);
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != LONG_MAX)
					{
						this->dir[0] = directions[i].x - this->x;
						this->dir[1] = directions[i].y - this->y;
						break;
					}
				}
			}
			// keep going
			else if (roadCount == 2)
			{
				int prev[] = { this->x - this->dir[0], this->y - this->dir[1] };
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != LONG_MAX && directions[i].x != prev[0] && directions[i].y != prev[1])
					{
						this->dir[0] = directions[i].x - this->x;
						this->dir[1] = directions[i].y - this->y;
						break;
					}
				}
			}
			// branch; same as keep going except shuffle
			else if (roadCount > 2)
			{
				int prev[] = { this->x - this->dir[0], this->y - this->dir[1] };
				gl::shuffleArray((byte*)directions, 4, 8);
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != LONG_MAX && directions[i].x != prev[0] && directions[i].y != prev[1])
					{
						this->dir[0] = directions[i].x - this->x;
						this->dir[1] = directions[i].y - this->y;
						break;
					}
				}
			}
			// dead end; same as keep going except allow going back
			else if (roadCount == 1)
			{
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != LONG_MAX)
					{
						this->dir[0] = directions[i].x - this->x;
						this->dir[1] = directions[i].y - this->y;
						break;
					}
				}
			}
			// something is wrong
			else
			{
				this->roam = false;
				this->initDir();
			}
		}
		else if (this->roam)
		{
			this->roam = false;
			this->initDir();
		}
		else if (this->targetb[0] != LONG_MAX && this->x == this->target[0] && this->y == this->target[1])
		{
			this->arrive();
		}
		else if (this->checkpoint[0] != LONG_MAX && this->x == this->checkpoint[0] && this->y == this->checkpoint[1])
		{
			this->initDir();
		}
		else if(this->dir[0] == 0 && this->dir[1] == 0)
		{
			this->initDir();
		}

		// validate building
		if (this->targetbId != 0)
		{
			cell* c = map.at(this->targetb[0], this->targetb[1]);
			if (!c || !c->b || c->b->id != this->targetbId)
				this->remove();
		}
	}

	void body::remove()
	{
		this->live = false;
		gl::removeSprite(this->sprite);

		switch (this->type)
		{
		case bodyType::immigrant:
			break;
		}
	}

	building* body::getb()
	{
		cell* c = map.at(this->targetb[0], this->targetb[1]);
		if (!c) return nullptr;
		building* b = c->b;
		if (!b) return nullptr;
		massert(b->live);
		return b->id == this->targetbId ? b : nullptr;
	}

	void body::serialize(stream* s)
	{
		s->writeByte(this->live);

		if (!this->live) return;

		s->writeUint64(this->id);
		s->write(this->targetb, 8);
		s->writeUint64(this->targetbId);
		s->writeInt32((int)this->type);
		s->writeInt32(this->x);
		s->writeInt32(this->y);
		s->write(this->dir, 8);
		s->write(this->target, 8);
		s->write(this->checkpoint, 8);
		s->write(this->stamina, 8);
		s->writeByte(this->roam);
	}

	void body::deserialize(stream* s)
	{
		this->live = s->readByte();
		if (!this->live) return;

		this->id = s->readUint64();
		s->read(this->targetb, 8);
		this->targetbId = s->readUint64();
		s->read(&this->type, 4);
		this->x = s->readInt32();
		this->y = s->readInt32();
		s->read(this->dir, 8);
		s->read(this->target, 8);
		s->read(this->checkpoint, 8);
		s->read(this->stamina, 8);
		this->roam = s->readByte();

		this->init();
	}
}