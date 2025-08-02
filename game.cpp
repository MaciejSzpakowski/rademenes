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
	void game::init()
	{
		tickLength = 0.25f;
		map.init();
		builder::init();
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
			sprintf(it, "door %d %d empcnt %d occ %d reccnt %d rec %d wrkcnt %d wrk %d wtr %d fir %d col %d",
				b->door[0], b->door[1], b->employementCounter, b->occupants, b->recruiterCounter,
				b->recruiters, b->workerCounter, b->workers, b->water[0], b->fire[0], b->collapse[0]);
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