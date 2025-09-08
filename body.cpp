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
	
	void* body::operator new(size_t count)
	{
		for (int i = 0; i < MAX_BODIES; i++)
		{
			if (!map.bodies[i].is(FLAG_LIVE))
				return map.bodies + i;
		}

		return nullptr;
	}

	bool body::is(int flag)
	{
		return (this->flags & flag) == flag;
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

	void body::effect(cell* c, int x, int y)	{	}

	bool getRoad(cell* c, int x, int y, vec2i* dst)
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
		this->stamina.cur = staminaMap[(int)type];
		this->dir.zero();
		this->sprite = LONG_MAX;
		this->targetb = target;
		this->target.set(NOPOS, NOPOS);
		this->targetbId = 0;
		this->pathIt = -1;
		this->waitCounter = 0;

		if (this->targetb)
		{
			this->target = target->door;
			this->targetbId = target->id;
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
		case bodyType::hunter:
			this->sprite = initSprite(x, y, 0, 1, 0.5f);
			break;
		}
	}

	void body::initDir()
	{
		if (this->pathIt >= 0)
		{
			int dx = this->path[pathIt].x - this->x;
			if (dx < 0)
				this->dir.x = -1;
			else if (dx > 0)
				this->dir.x = 1;
			else
				this->dir.x = 0;
			int dy = this->path[pathIt].y - this->y;
			if (dy < 0)
				this->dir.y = -1;
			else if (dy > 0)
				this->dir.y = 1;
			else
				this->dir.y = 0;
		}
		else
		{
			if (this->targeta)
				this->target.set(&this->targeta->pos);

			if (this->target.x > this->x)
				this->dir.x = 1;
			else if (this->target.x < this->x)
				this->dir.x = -1;
			else
				this->dir.x = 0;

			if (this->target.y > this->y)
				this->dir.y = 1;
			else if (this->target.y < this->y)
				this->dir.y = -1;
			else
				this->dir.y = 0;
		}
	}

	void body::arrive()	{	}

	void body::action()	{	}

	void body::damage(int hit, body* source)
	{
		this->hp -= hit;

		if (this->hp < 1)
		{
			if (this->is(BODY_ANIMAL))
			{
				float col[] = { 1,0,0 };
				gl::updateSprite(this->sprite, this->x, this->y, 0.3f, col, 0.5f, 0.5f);
				this->dir.zero();
				this->state = ANIMAL_DEAD;
			}
			this->flags |= BODY_DEAD;
		}
		else if(this->is(BODY_GAME))
		{
			this->runAwayFrom(source);
		}
	}

	void body::remove()
	{
		this->flags = 0;
		gl::removeSprite(this->sprite);
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

	immigrant::immigrant() {}

	void immigrant::action()
	{
		// validate building
		// remove if building is gone
		if (this->targetb)
		{
			if (!this->targetb->is(FLAG_LIVE) || this->targetb->id != this->targetbId)
				this->remove();
		}

		this->x += this->dir.x;
		this->y += this->dir.y;
		gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);

		this->initDir();

		if (this->x == this->target.x && this->y == this->target.y)
		{
			this->arrive();
		}
	}

	void immigrant::init(bodyType type, int x, int y, building* target)
	{
		body::init(type,x,y,target);
		this->target.x = target->x;
		this->target.y = target->y;
		this->targetb->immigrants += 1;
	}

	void immigrant::arrive()
	{
		if (this->targetb->is(FLAG_LIVE) && this->targetbId == this->targetb->id &&
			this->targetb->occupants.cur < this->targetb->occupants.max)
		{
			map.citizens += 1;
			map.employees += 1;
			map.unemployed += 1;
			this->targetb->occupants.cur += 1;
			this->targetb->immigrants -= 1;
		}

		this->remove();
	}

	void walker::arrive()
	{
		if (this->targetb->is(FLAG_LIVE) && this->targetb->id == this->targetbId)
		{
			this->targetb->workers.cur -= 1;
		}

		this->remove();
	}

	void walker::action()
	{
		// actual logic
		// if service walker cannot find wayback upon returning that it's destroyed right away
		// if service walker started return and then there is some obstruction (road was destroyed and building was placed) then it's destroyed when hits obstruction
		// if service walker started return and road was destroyed (but no obstruction), it remembers the path and walks over the empty space
		// actually when return starts, entire path back is computed so even if all roads are removed but space is empty, walker will walk back just fine
		// return will be computed even if it's very long (because original short path was destroyed)

		// validate building
		// remove if building is gone
		if (this->targetb)
		{
			if (!this->targetb->is(FLAG_LIVE) || this->targetb->id != this->targetbId)
				this->remove();
		}

		// move
		if (this->dir.x != 0 || this->dir.y != 0)
		{
			if (this->pathIt >= 0)
			{
				cell* cnext = map.at(this->x + this->dir.x, this->y + this->dir.y);
				// actual logic: path interrupted, remove
				if (cnext->b && !cnext->b->is(BUILDING_WALKABLE))
				{
					this->arrive();
					return;
				}
			}

			this->x += this->dir.x;
			this->y += this->dir.y;
			gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);
		}

		if (this->stamina.cur > 0)
		{
			this->stamina.cur -= 1;
			vec2i directions[] = { {NOPOS,NOPOS },{NOPOS,NOPOS },{NOPOS,NOPOS },{NOPOS,NOPOS } };
			map.getBorderNoCorners(this->x - 1, this->y - 1, 3, 3, (CELLIT)getRoad, directions);
			uint roadCount = 0;
			for (uint i = 0; i < 4; i++)
			{
				if (directions[i].x != NOPOS)
					roadCount += 1;
			}

			// first step
			if (this->dir.x == 0 && this->dir.y == 0)
			{
				gl::shuffleArray((byte*)directions, 4, 8);
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != NOPOS)
					{
						this->dir.x = directions[i].x - this->x;
						this->dir.y = directions[i].y - this->y;
						break;
					}
				}
			}
			// keep going
			else if (roadCount == 2)
			{
				int prev[] = { this->x - this->dir.x, this->y - this->dir.y };
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != NOPOS && directions[i].x != prev[0] && directions[i].y != prev[1])
					{
						this->dir.x = directions[i].x - this->x;
						this->dir.y = directions[i].y - this->y;
						break;
					}
				}
			}
			// branch; same as keep going except shuffle
			else if (roadCount > 2)
			{
				int prev[] = { this->x - this->dir.x, this->y - this->dir.y };
				gl::shuffleArray((byte*)directions, 4, 8);
				for (uint i = 0; i < 4; i++)
				{
					if (directions[i].x != NOPOS && directions[i].x != prev[0] && directions[i].y != prev[1])
					{
						this->dir.x = directions[i].x - this->x;
						this->dir.y = directions[i].y - this->y;
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
						this->dir.x = directions[i].x - this->x;
						this->dir.y = directions[i].y - this->y;
						break;
					}
				}
			}
			// something is wrong
			else
			{
				massert(false, "walker unhandled state 1");
			}
		}
        // init path
		else if (this->stamina.cur < 1 && this->pathIt == -1)
		{
			body b;
			b.x = 7;

			int pos[2] = { x,y };
			// actual logic: if cannot find path when init return then remove
			int pathlen = getPathRoad(&this->pos, &this->target, this->path);
			if (pathlen == 0)
			{
				this->arrive();
				return;
			}

			this->pathIt = pathlen - 1;
			this->initDir();
		}
		// arrive
		else if (this->x == this->target.x && this->y == this->target.y)
		{
			this->arrive();
			return;
		}
		else if (this->pathIt >= 0 && this->path[pathIt].x == this->x && this->path[pathIt].y == this->y)
		{
			massert(this->pathIt > 0, "path run off");
			this->pathIt -= 1;
			this->initDir();
		}

		// effect
		// so far all effect have range of 2
		map.getArea(this->x - 2, this->y - 2, 5, 5, (CELLIT)ph::effect, this);
	}

	recruiter::recruiter() {}

	void recruiter::arrive()
	{
		if (this->targetb->is(FLAG_LIVE) && this->targetb->id == this->targetbId)
		{
			this->targetb->recruiters -= 1;
		}

		this->remove();
	}

	void recruiter::effect(cell* c, int x, int y)
	{
		building* b = c->b;

		if (this->pathIt == -1 && b && b->is(buildingType::house) && b->occupants.cur > 0)
		{
			massert(this->targetb, "targetb is null");
			if (this->targetb && this->targetb->is(FLAG_LIVE) && this->targetb->id == this->targetbId)
				targetb->recruit();
		}
	}

	waterCarrier::waterCarrier() {}

	void waterCarrier::effect(cell* c, int x, int y)
	{
		building* b = c->b;

		if (b && b->is(buildingType::house) && b->occupants.cur > 0)
		{
			b->water.cur = b->water.max;
		}
	}

	fireMarshal::fireMarshal() {}

	void fireMarshal::effect(cell* c, int x, int y)
	{
		building* b = c->b;

		if (b && b->is(BUILDING_FLAMABLE))
		{
			b->fire.cur = b->fire.max;
		}
	}

	architect::architect() {}

	void architect::effect(cell* c, int x, int y)
	{
		building* b = c->b;

		if (b && b->is(BUILDING_COLLAPSABLE))
		{
			b->collapse.cur = b->collapse.max;
		}
	}

	hunter::hunter() {}

	void hunter::init(bodyType type, int x, int y, building* target)
	{
		body::init(type, x, y, target);
		this->state = HUNTER_SEARCH;
	}

	void hunter::action()
	{
		// verify building
		if (!this->targetb->is(FLAG_LIVE) || this->targetb->id != this->targetbId)
		{
			this->remove();
			return;
		}

		if (this->dir.x != 0 || this->dir.y != 0)
		{
			this->pos.add(&this->dir);
			gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);
		}

		// find new target
		if (this->state == HUNTER_SEARCH)
		{
			// TODO
			// actual logic: hunter always picks prey that is the closest
			// sometimes it will change target if prey runs away too far
			for (uint i = 0; i < MAX_BODIES; i++)
			{
				body* b = map.bodies + i;

				if (b->is(FLAG_LIVE | BODY_GAME))
				{
					if (b->state == ANIMAL_DEAD && !b->is(BODY_GAME_BEING_COLLECTED))
					{
						this->state = HUNTER_COLLECT;
						b->flags |= BODY_GAME_BEING_COLLECTED;
						this->targeta = map.bodies + i;
						this->targetaId = b->id;
						this->initDir();
						return;
					}
					else if (b->state != ANIMAL_DEAD)
					{
						this->state = HUNTER_PURSUE;
						this->targeta = map.bodies + i;
						this->targetaId = b->id;
						this->initDir();
						return;
					}
				}
			}
		}
		// chase target
		else if (this->state == HUNTER_PURSUE)
		{
			// verify
			if (this->targeta->id != this->targetaId || this->targeta->state == ANIMAL_DEAD)
			{
				this->dir.zero();
				this->targeta = nullptr;
				this->targetaId = 0;
				this->state = HUNTER_SEARCH;
				return;
			}

			if (this->waitCounter > 0)
			{
				this->waitCounter -= 1;
			}
			else if (abs(this->targeta->x - this->x) + abs(this->targeta->y - this->y) < 3)
			{
				this->waitCounter = HUNTER_SHOT_AND_WAIT;
				this->dir.zero();
				// SHOOT, can hit only if it's not moving
				if (!(this->targeta->state == ANIMAL_WALK))
				{
					this->targeta->damage(HUNTER_ATTACK_POWER, this);
				}
			}
			else
			{
				this->initDir();
			}

			// check if it's dead
			if (this->targeta->state == ANIMAL_DEAD)
			{
				massert(!this->targeta->is(BODY_GAME_BEING_COLLECTED),"this is already collected");
				this->targeta->flags |= BODY_GAME_BEING_COLLECTED;
				this->state = HUNTER_COLLECT;
			}
		}
		else if (this->state == HUNTER_COLLECT)
		{
			if (this->pos.equals(&this->targeta->pos))
			{
				this->dir.zero();
				this->state = HUNTER_RETURN;
				this->target = this->targetb->door;			
				this->targeta->remove();
				this->targeta = nullptr;
				return;
			}

			this->initDir();
		}
		// return to lodge with meat
		// actual logic: hunter go to the door of hunting lodge
		else if (this->state == HUNTER_RETURN)
		{
			if (this->pos.equals(&this->target))
			{
				this->arrive();
				return;
			}

			this->initDir();
		}		
	}

	void hunter::arrive()
	{
		if (this->targetb->is(FLAG_LIVE) && this->targetb->id == this->targetbId)
		{
			this->targetb->workers.cur -= 1;
			this->targetb->storage[GOODS_PRODUCE_SLOT].qty += 100;
		}
		this->remove();
	}

	animal::animal() {}

	void animal::init(bodyType type, int x, int y, building* target)
	{
		body::init(type, x, y, target);
		this->hp = 100;
		this->flags |= BODY_ANIMAL | BODY_GAME;
		this->waitCounter = gl::rand(MAX_ANIMAL_MOVE_COUNTER, MAX_ANIMAL_MOVE_COUNTER + 10); // actual logic: wait time between moves is random
		this->state = ANIMAL_CHILLIN;
	}

	void animal::stop()
	{
		this->flags -= BODY_RUN_AWAY;
		this->waitCounter = MAX_ANIMAL_MOVE_COUNTER;
		this->state = ANIMAL_CHILLIN;
		this->dir.zero();
	}

	bool animal::stopMovingIfObstruction()
	{
		massert(this->dir.x != 0 || this->dir.y != 0, "dir is 0");
		massert(this->is(BODY_ANIMAL), "this is not animal");
		cell* cnext = map.at(this->x + this->dir.x, this->y + this->dir.y);
		if (!cnext || cnext->b && !cnext->b->is(BUILDING_WALKABLE) || cnext->type == cellType::water && !this->is(BODY_WATER_ANIMAL))
		{
			this->stop();
			return true;
		}

		return false;
	}

	void body::runAwayFrom(body* b)
	{
		if (this->is(BODY_GAME))
		{
			this->stamina.cur = 4;
			this->state = ANIMAL_WALK;
			this->flags |= BODY_RUN_AWAY;

			// TODO: better target selection
			// check for obstacles
			// currently, simply run away in opposite dir
			if (!this->pos.equals(&b->pos))
			{
				this->dir.set(&this->pos);
				this->dir.sub(&b->pos);
				this->dir.norm();
			}
			else
			{
				this->dir.set(1, 0);
			}					
		}
	}

	void animal::action()
	{
		massert(this->state, "bad state");

		if (this->is(BODY_DEAD)) return;

		// check at the beginning before move
		if (this->state == ANIMAL_WALK)
		{
			if (this->stopMovingIfObstruction()) return;
		}

		if (this->state == ANIMAL_CHILLIN)
		{
			this->waitCounter -= 1;

			// find a new target
			if (this->waitCounter < 1)
			{
				this->state = ANIMAL_WALK;
				this->stamina.cur = gl::rand(1, 4); // actual logic: this is random between 1 and 3
				int r = gl::rand(0, 4);
				switch (r)
				{
				case 0:
					this->dir.set(-1, 0);
					break;
				case 1:
					this->dir.set(1, 0);
					break;
				case 2:
					this->dir.set(0, -1);
					break;
				case 3:
					this->dir.set(0, 1);
					break;
				}
			}
		}
		else if (this->state == ANIMAL_WALK)
		{
			massert(this->dir.x != 0 || this->dir.y != 0, "dir is 0");
			this->pos.add(&this->dir);
			gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);

			this->stamina.cur -= 1;

			if (this->stamina.cur < 1)
			{
				this->stop();
				return;
			}
		}

		// check at the end after move
		if (this->state == ANIMAL_WALK)
		{
			if (this->stopMovingIfObstruction()) return;
		}
	}

	delivery::delivery() {}

	void delivery::action()
	{
		if (!this->targetb)
		{
			// FIND TARGET
			// TODO, should be nearest by default
			for (uint i = 0; i < MAX_BUILDINGS; i++)
			{
				building* bi = map.buildings + i;
				int ri = (int)this->resourceType;
				// consumer like potter or school
				if (bi->consumes == this->resourceType && 
					bi->storage[GOODS_CONSUME_SLOT].qty + bi->storage[GOODS_CONSUME_SLOT].incoming < bi->storage[GOODS_CONSUME_SLOT].max)
				{
					this->targetb = bi;
					bi->storage[GOODS_CONSUME_SLOT].incoming += this->resource;
				}
				else if (bi->is(buildingType::granary) || bi->is(buildingType::storage))
				{
					for (uint i = 0; i < 8; i++)
					{
						// empty slot
						if (bi->storage[i].qty == 0 && bi->storage[i].incoming == 0)
						{
							bi->storage[i].incoming += this->resource;
							bi->storage[i].type = this->resourceType;
							this->targetb = bi;
							this->targetbId = bi->id;
							this->target.set(&bi->door);
						}
						// slot of the same type with space left
						else if (bi->storage[i].incoming + bi->storage[i].qty < bi->storage[i].max && bi->storage[i].type == this->resourceType)
						{
							bi->storage[i].incoming += this->resource;
							this->targetb = bi;
							this->targetbId = bi->id;
							this->target.set(&bi->door);
						}
					}
				}

				// found new target, stop looking
				if (this->targetb)
					break;
			}
		}

		// no new target, dont do anything else
		if (!this->targetb)
			return;

		// validate building
		// remove if building is gone
		if (this->targetb)
		{
			if (!this->targetb->is(FLAG_LIVE) || this->targetb->id != this->targetbId)
				this->remove();
		}

		this->x += this->dir.x;
		this->y += this->dir.y;
		gl::updateSprite(this->sprite, this->x + 0.15f, this->y + 0.15f);

		this->initDir();

		if (this->x == this->target.x && this->y == this->target.y)
		{
			this->arrive();
		}
	}

	void delivery::init(bodyType type, int x, int y, building* target)
	{
		body::init(type, x, y, target);
		this->target.x = target->x;
		this->target.y = target->y;
		this->targetb->immigrants += 1;
	}

	void delivery::arrive()
	{
		building* b = this->targetb;

		if (b->is(FLAG_LIVE) && this->targetbId == b->id)
		{
			if (b->is(buildingType::granary) || b->is(buildingType::storage))
			{
				// first try to find slot with existing goods but not full
				for (uint i = 0; i < 8; i++)
				{
					if (b->storage[i].qty < b->storage[i].max && b->storage[i].type == this->resourceType)
					{
						int amountTransferred = __min(b->storage[i].max - b->storage[i].qty, this->resource);
						b->storage[i].qty += amountTransferred;
						b->storage[i].incoming -= amountTransferred;
						this->resource -= amountTransferred;
						break;
					}
				}
				// then try to find empty slot
				for (uint i = 0; i < 8; i++)
				{
					if (b->storage[i].qty == 0)
					{
						massert(this->resource <= b->storage[i].max, "delivery carries more than 400 resource");

						b->storage[i].qty += this->resource;
						b->storage[i].incoming -= this->resource;
						this->resource = 0;
						b->storage[i].type = this->resourceType;
						break;
					}
				}
			}
		}

		if (this->resource == 0)
		{
			this->remove();
		}
	}
}