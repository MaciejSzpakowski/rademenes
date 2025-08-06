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
	// water 25-27, fire 42-51
	int staminaMap[] = { 
		0,42,25,42,0,
		42,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0,0,0,
		0,0,0 
	};

	struct vector2
	{
		int x, y;
	};
	
	bool body::is(int flag)
	{
		return this->flags & flag;
	}

	bool body::is(bodyType type)
	{
		return this->type == type;
	}

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
			if (this->is(BODY_ROAM) && b && b->is(buildingType::house) && b->occupants > 0)
			{
				building* targetb = this->getb();
				if (targetb) targetb->recruit();
			}
			break;
		case bodyType::waterCarrier:
			if (b && b->is(buildingType::house) && b->occupants > 0)
			{
				b->water[0] = b->water[1];
			}
			break;
		case bodyType::fire:
			if (b && b->is(BUILDING_FLAMABLE))
			{
				b->fire[0] = b->fire[1];
			}
			break;
		case bodyType::architect:
			if (b && b->is(BUILDING_COLLAPSABLE))
			{
				b->collapse[0] = b->collapse[1];
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
		return gl::addSprite(color, x, y, 0.3f, 0.5f, 0.5f);
	}

	void body::init()
	{
		this->initGraphics();
	}

	void body::init(bodyType type, int x, int y, building* target)
	{
		this->flags = FLAG_LIVE;
		this->id = map.getId();
		this->type = type;
		this->x = x;
		this->y = y;
		this->stamina[0] = staminaMap[(int)type];
		this->checkpoint[0] = NOPOS;
		this->checkpoint[1] = NOPOS;
		this->dir[0] = 0;
		this->dir[1] = 0;
		this->sprite = LONG_MAX;

		if (target)
		{
			this->targetb[0] = target->x;
			this->targetb[1] = target->y;
			this->target[0] = this->is(bodyType::immigrant) ? target->x : target->door[0];
			this->target[1] = this->is(bodyType::immigrant) ? target->y : target->door[1];
			this->targetbId = target->id;
		}
		else
		{
			this->targetb[0] = NOPOS;
			this->targetb[1] = NOPOS;
			this->target[0] = NOPOS;
			this->target[1] = NOPOS;
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
		case bodyType::architect:
			this->flags |= BODY_ROAM | BODY_HAS_EFFECT;
			break;
		case bodyType::ostrich:
			this->flags |= BODY_ANIMAL;
			this->animalMoveCounter = gl::rand(MAX_ANIMAL_MOVE_COUNTER, MAX_ANIMAL_MOVE_COUNTER + 3); // actual logic: wait time between moves is random
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
		case bodyType::architect:
			this->sprite = initSprite(x, y, 36 / 255.0f, 49 / 255.0f, 54 / 255.0f);
			break;
		case bodyType::ostrich:
			this->sprite = initSprite(x, y, 1, 1, 1);
			break;
		}
	}

	void body::initDir()
	{
		// if service walker cannot find wayback upon returning that it's destroyed right away
		// if service walker started return and then there is some obstruction (road was destroyed and building was placed) then it's destroyed when hits obstruction
		// if service walker started return and road was destroyed (but no obstruction), it remembers the path and walks over the empty space
		// actually when return starts, entire path back is computed so even if all roads are removed by space is empty, walker will walk back just fine
		// return will be computed even if it's very long (for example original path was destroyed)

		if (this->is(BODY_RETURN))
		{
			int dx = this->path[pathIt][0] - this->x;
			if (dx < 0)
				this->dir[0] = -1;
			else if (dx > 0)
				this->dir[0] = 1;
			else
				this->dir[0] = 0;
			int dy = this->path[pathIt][1] - this->y;
			if (dy < 0)
				this->dir[1] = -1;
			else if (dy > 0)
				this->dir[1] = 1;
			else
				this->dir[1] = 0;
		}
		else
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
	}

	void body::arrive()
	{
		cell* c = map.at(this->targetb[0], this->targetb[1]);
		building* b = c ? c->b : nullptr;

		// check if targetb is still valid
		switch (this->type)
		{
		case bodyType::immigrant:
			if (b && b->is(FLAG_LIVE) && b->is(buildingType::house) && 
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
			if (b && b->is(FLAG_LIVE) && b->id == this->targetbId)
			{
				b->recruiters -= 1;
			}
			break;
		case bodyType::waterCarrier:
		case bodyType::fire:
		case bodyType::architect:
			if (b && b->is(FLAG_LIVE) && b->id == this->targetbId)
			{
				b->workers -= 1;
			}
			break;
		case bodyType::ostrich:
			this->animalMoveCounter = MAX_ANIMAL_MOVE_COUNTER;
			this->dir[0] = 0;
			this->dir[1] = 0;
			break;
		}

		if(!this->is(BODY_ANIMAL))
			this->remove();
	}

	bool body::animalStopMovingIfObstruction()
	{
		if (this->dir[0] != 0 || this->dir[1] != 0)
		{
			cell* cnext = map.at(this->x + this->dir[0], this->y + this->dir[1]);
			if (this->is(BODY_ANIMAL) && (!cnext || cnext->b || cnext->type == cellType::water && !this->is(BODY_WATER_ANIMAL)))
			{
				this->arrive();
				return true;
			}
		}

		return false;
	}

	void body::animalAction()
	{
		// check at the beginning before move
		if (this->animalStopMovingIfObstruction()) return;

		if (this->animalMoveCounter > 0)
		{
			this->animalMoveCounter -= 1;

			// find a new target
			if (this->animalMoveCounter < 1)
			{
				this->stamina[0] = gl::rand(1, 4); // actual logic: this is random between 1 and 3
				int r = gl::rand(0, 4);
				switch (r)
				{
				case 0:
					dir[0] = -1;
					dir[1] = 0;
					break;
				case 1:
					dir[0] = 1;
					dir[1] = 0;
					break;
				case 2:
					dir[0] = 0;
					dir[1] = -1;
					break;
				case 3:
					dir[0] = 0;
					dir[1] = 1;
					break;
				}
			}
		}
		else if (this->dir[0] != 0 || this->dir[1] != 0)
		{
			this->x += this->dir[0];
			this->y += this->dir[1];
			gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);

			this->stamina[0] -= 1;

			if (this->stamina[0] < 1)
			{
				this->arrive();
				return;
			}
		}

		// check at the end after move
		this->animalStopMovingIfObstruction();
	}

	void body::action()
	{
		if (this->is(BODY_ANIMAL))
		{
			this->animalAction();
			return;
		}

		// always remove stamina
		if (this->is(BODY_ROAM) && this->stamina[0] > 0)
			this->stamina[0] -= 1;

		// move
		if (this->dir[0] != 0 || this->dir[1] != 0)
		{

			if (this->is(BODY_RETURN))
			{
				cell* cnext = map.at(this->x + this->dir[0], this->y + this->dir[1]);
				// actual logic: path interrupted, remove
				if (cnext->b && !cnext->b->is(BUILDING_WALKABLE))
				{
					this->arrive();
					return;
				}
			}

			this->x += this->dir[0];
			this->y += this->dir[1];
			gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);
		}

		// effect
		if (this->is(BODY_HAS_EFFECT))
		{
			// so far all effect have range of 2
			map.getBorder(this->x - 2, this->y - 2, 5, 5, (CELLIT)ph::effect, this);
		}
		
		if (this->is(BODY_ROAM) && this->stamina[0] > 0)
		{
			vector2 directions[] = { {NOPOS,NOPOS },{NOPOS,NOPOS },{NOPOS,NOPOS },{NOPOS,NOPOS } };
			map.getBorderNoCorners(this->x - 1, this->y - 1, 3, 3, (CELLIT)getRoad, directions);
			uint roadCount = 0;
			for (uint i = 0; i < 4; i++)
			{
				if (directions[i].x != NOPOS)
					roadCount += 1;
			}

			// first step
			if (this->dir[0] == 0 && this->dir[1] == 0)
			{
				gl::shuffleArray((byte*)directions, 4, 8);
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != NOPOS)
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
					if (directions[i].x != NOPOS && directions[i].x != prev[0] && directions[i].y != prev[1])
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
					if (directions[i].x != NOPOS && directions[i].x != prev[0] && directions[i].y != prev[1])
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
					if (directions[i].x != NOPOS)
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
				REMOVE_FLAG(BODY_ROAM)
				this->initDir();
			}
		}
		// stamina is 0
		else if (this->is(BODY_ROAM))
		{
			massert(this->stamina[0] <= 0, "stamina is positive");
			REMOVE_FLAG(BODY_ROAM)			

			int pos[2] = { x,y };
			// actual logic: if cannot find path when init return then remove
			int pathlen = getPathRoad(pos, this->target, this->path);
			if (pathlen == 0)
			{
				this->arrive();
				return;
			}

			this->flags |= BODY_RETURN;
			this->pathIt = pathlen -= 1;
			this->initDir();
		}
		else if (this->targetb[0] != NOPOS && this->x == this->target[0] && this->y == this->target[1])
		{
			this->arrive();
		}
		else if (this->is(BODY_RETURN) && this->path[pathIt][0] == this->x && this->path[pathIt][1] == this->y)
		{
			massert(this->pathIt > 0, "path run off");
			this->pathIt -= 1;
			this->initDir();
		}
		else if (this->checkpoint[0] != NOPOS && this->x == this->checkpoint[0] && this->y == this->checkpoint[1])
		{
			this->initDir();
		}
		else if(!this->is(BODY_ANIMAL) && this->dir[0] == 0 && this->dir[1] == 0)
		{
			this->initDir();
		}

		// validate building
		// remove if building is gone
		if (this->targetbId != 0)
		{
			cell* c = map.at(this->targetb[0], this->targetb[1]);
			if (!c->b || c->b->id != this->targetbId)
				this->remove();
		}
	}

	void body::remove()
	{
		this->flags = 0;
		gl::removeSprite(this->sprite);
	}

	building* body::getb()
	{
		cell* c = map.at(this->targetb[0], this->targetb[1]);
		if (!c) return nullptr;
		building* b = c->b;
		if (!b) return nullptr;
		massert(b->is(FLAG_LIVE));
		return b->id == this->targetbId ? b : nullptr;
	}

	void body::serialize(stream* s)
	{
		/*s->writeByte(this->live);

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
		s->writeByte(this->roam);*/
	}

	void body::deserialize(stream* s)
	{
		/*this->live = s->readByte();
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

		this->init();*/
	}
}