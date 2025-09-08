#include "types.h"
#include "building.h"
#include "body.h"
#include "map.h"
#include "gl.h"
#include <cstdio>
#include <cstring>
#include "stream.h"
#include <cmath>
#include "screens.h"
#include "builder.h"

namespace ph
{
	void vec2i::set(int _x, int _y)
	{
		this->x = _x;
		this->y = _y;
	}

	void vec2i::set(vec2i* v)
	{
		this->x = v->x;
		this->y = v->y;
	}

	void vec2i::add(vec2i* v)
	{
		this->x += v->x;
		this->y += v->y;
	}

	void vec2i::sub(vec2i* v)
	{
		this->x -= v->x;
		this->y -= v->y;
	}

	bool vec2i::equals(vec2i* v)
	{
		return this->x == v->x && this->y == v->y;
	}

	void vec2i::zero()
	{
		this->x = 0;
		this->y = 0;
	}

	void game::init()
	{
		tickLength = 0.25f;
		map.init();
		builder::init();
	}

	void vec2i::norm()
	{
		if (this->x < 0) this->x = -1;
		else if (this->x > 0) this->x = 1;
		if (this->y < 0) this->y = -1;
		else if (this->y > 0) this->y = 1;
	}

	bool isNopos(vec2i* v)
	{
		return v->x == NOPOS;
	}

	void game::run()
	{
		if (gameTime - lastTick > tickLength)
		{
			lastTick = gameTime;
			map.action();
		}

		cell* hover = map.at(mouse.worldx, mouse.worldy);

		char* it = textbuffer;
		sprintf(it, "%4d", mouse.worldx); it += 4;
		sprintf(it, "%4d", mouse.worldy); it += 4;
		sprintf(it, " %20s ", builder::getDesc()); it += 22;
		sprintf(it, "cit %3d|", map.citizens); it += 8;
		sprintf(it, "emp %3d|", map.employees); it += 8;
		sprintf(it, "emd %3d|", map.employeed); it += 8;
		sprintf(it, "unm %3d|", map.unemployed); it += 8;
		sprintf(it, "db %3d|", map.deben); it += 7;
		sprintf(it, "\n"); it += 1;

		if (hover && hover->b)
		{
			building* b = hover->b;
			sprintf(it, "door %d %d empcnt %d occ %d reccnt %d rec %d wrkcnt %d wrk %d wtr %d fir %d col %d res %d",
				b->door.x, b->door.y, b->employementCounter, b->occupants, b->recruiterCounter,
				b->recruiters, b->workerCounter, b->workers, b->water.cur, b->fire.cur, b->collapse.cur, b->storage[0].qty);
		}
		else
		{
			sprintf(it, " ");
		}

		gl::setText(0, textbuffer);

		float tickProgress = (gameTime - lastTick) / tickLength;
		map.render(tickProgress);
	}

	void game::destroy()
	{

	}

	void game::keyboard()
	{
		builder::keyboard();

		if (gl::isKeyPressed(1))
		{
			ph::builder::leftClickCallback();
		}
		else if (gl::isKeyPressed(2))
		{
			ph::builder::rightClickCallback();
		}
		else if (gl::isKeyPressed('Q'))
		{
			ph::builder::select(-1);
		}
		else if (gl::isKeyPressed('E'))
		{
			ph::builder::select(1);
		}
	}

	void game::mouseMove()
	{

	}
}