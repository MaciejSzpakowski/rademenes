#include "types.h"
#include "building.h"
#include "body.h"
#include "map.h"
#include "gl.h"
#include <cstdio>
#include <cstring>
#include "stream.h"
#include <cmath>

namespace ph::builder
{
	float spriteColor[] = { 0.5f,0.5f ,0.5f };
	uint sprite1, sprite2;
	int index = 0;
	char modeDesc[20];
	const int optionsLen = 11;
	buildingType options[optionsLen] = { buildingType::none, buildingType::road,
		buildingType::house, buildingType::waterSupply, buildingType::immigrantEntry,
		buildingType::statueLarge, buildingType::fireHouse, buildingType::granary ,
		buildingType::bazaar , buildingType::huntingLodge , buildingType::architect };
	const char* desc[] = { "none","road","house","water supply","entry","large statue","fire house","granary","bazaar","hunting lodge","architect" };
	int startClick[2];
	enum class mouseState : int
	{
		def, roadDrag, houseDrag
	};
	mouseState mState = mouseState::def;

	void init()
	{
		index = 0;
		sprite1 = gl::addSprite(spriteColor, 0, 0, 0.9f, 0, 0);
		sprite2 = gl::addSprite(spriteColor, 0, 0, 0.9f, 0, 0);
	}

	bool checkForEptyFields(cell* c, int x, int y, bool* data)
	{
		if (!c || c->b || c->road || c->type != cellType::empty)
		{
			*data = false;
			return false;
		}

		return true;
	}

	bool checkForMoisture(cell* c, int x, int y, bool* data)
	{
		if (c && c->moisture)
		{
			*data = true;
			return false;
		}

		return true;
	}

	void select(int delta)
	{
		index += delta;
		if (index < 0) index = optionsLen - 1;
		if (index >= optionsLen) index = 0;
	}

	buildingType getBuilding()
	{
		return options[index];
	}

	const char* getDesc()
	{
		return desc[index];
	}

	void keyboard()
	{
		if (gl::isKeyPressed(1) && mState == mouseState::def && options[index] == buildingType::road)
		{
			startClick[0] = mouse.worldx;
			startClick[1] = mouse.worldy;
			mState = mouseState::roadDrag;
		}
		else if (gl::isKeyPressed(1) && mState == mouseState::def && options[index] == buildingType::house)
		{
			startClick[0] = mouse.worldx;
			startClick[1] = mouse.worldy;
			mState = mouseState::houseDrag;
		}
		else if (gl::isKeyDown(1) && mState == mouseState::houseDrag)
		{
			int x = startClick[0] < mouse.worldx ? startClick[0] : mouse.worldx;
			int y = startClick[1] < mouse.worldy ? startClick[1] : mouse.worldy;
			int w = startClick[0] < mouse.worldx ? mouse.worldx - startClick[0] : startClick[0] - mouse.worldx;
			int h = startClick[1] < mouse.worldy ? mouse.worldy - startClick[1] : startClick[1] - mouse.worldy;

			gl::updateSprite(sprite1, x, y, 0.9f, spriteColor, w + 1, h + 1);
		}
		else if (gl::isKeyDown(1) && mState == mouseState::roadDrag)
		{
			int x1 = startClick[0] < mouse.worldx ? startClick[0] : mouse.worldx;
			int y1 = startClick[1];
			int w1 = startClick[0] < mouse.worldx ? mouse.worldx - startClick[0] : startClick[0] - mouse.worldx;

			gl::updateSprite(sprite1, x1, y1, 0.9f, spriteColor, w1 + 1, 1);

			int x2 = mouse.worldx;
			int y2 = startClick[1] < mouse.worldy ? startClick[1] : mouse.worldy;
			int h2 = startClick[1] < mouse.worldy ? mouse.worldy - startClick[1] : startClick[1] - mouse.worldy;

			gl::updateSprite(sprite2, x2, y2, 0.9f, spriteColor, 1, h2 + 1);
		}
		else if (!gl::isKeyDown(1) && mState == mouseState::roadDrag)
		{
			int it[2] = { startClick[0],startClick[1] };
			map.addRoad(it[0], it[1]);
			while (it[0] != mouse.worldx || it[1] != mouse.worldy)
			{
				if (it[0] < mouse.worldx)
					it[0] += 1;
				else if (it[0] > mouse.worldx)
					it[0] -= 1;
				else if (it[1] < mouse.worldy)
					it[1] += 1;
				else if (it[1] > mouse.worldy)
					it[1] -= 1;

				map.addRoad(it[0], it[1]);
			}

			mState = mouseState::def;
		}
		else if (!gl::isKeyDown(1) && mState == mouseState::houseDrag)
		{
			int fromx, fromy, tox, toy;

			if (mouse.worldx < startClick[0])
			{
				fromx = mouse.worldx;
				tox = startClick[0];
			}
			else
			{
				tox = mouse.worldx;
				fromx = startClick[0];
			}

			if (mouse.worldy < startClick[1])
			{
				fromy = mouse.worldy;
				toy = startClick[1];
			}
			else
			{
				toy = mouse.worldy;
				fromy = startClick[1];
			}

			for (int i = fromx; i <= tox; i++)
			{
				for (int j = fromy; j <= toy; j++)
				{
					cell* c = map.at(i, j);
					if (!c || c->b || c->road) continue;
					building* b = map.addBuilding();
					b->init(buildingType::house, i, j);
				}
			}

			mState = mouseState::def;
		}
		else
		{
			gl::updateSprite(sprite1, mouse.worldx, mouse.worldy, 0.9f, spriteColor, 1, 1);
			gl::updateSprite(sprite2, 0, 0, 0.9f, spriteColor,0,0);
		}
	}

	void leftClickCallback()
	{
		if (getBuilding() == buildingType::none) return;

		if (getBuilding() == buildingType::road || getBuilding() == buildingType::house)
		{
		}
		else
		{
			cell* c = map.at(mouse.worldx, mouse.worldy);
			if (!c) return;
			building* b = nullptr;
			int size[2];
			b->getSize(getBuilding(), size);
			bool goodToGo = true;
			// check for obstructions
			map.getArea(mouse.worldx, mouse.worldy, size[0], size[1], (CELLIT)checkForEptyFields, &goodToGo);
			if (!goodToGo) return;

			// watersource only on moisture
			if (getBuilding() == buildingType::waterSupply)
			{
				goodToGo = false;
				map.getArea(mouse.worldx, mouse.worldy, size[0], size[1], (CELLIT)checkForMoisture, &goodToGo);
			}

			if (!goodToGo) return;

			b = map.addBuilding();
			b->init(getBuilding(), mouse.worldx, mouse.worldy);
		}
	}

	void rightClickCallback()
	{
		if (mState != mouseState::def)
		{
			mState = mouseState::def;
			return;
		}

		cell* c = map.at(mouse.worldx, mouse.worldy);
		if (c && c->b && c->b->type != buildingType::fire) c->b->remove();
		if (c && c->road) map.removeRoad(mouse.worldx, mouse.worldy);
	}
}