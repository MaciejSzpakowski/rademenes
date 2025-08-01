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
	int len = 7;
	int index = 0;
	char modeDesc[20];
	buildingType options[] = {buildingType::none, buildingType::road, 
		buildingType::house, buildingType::waterSupply, buildingType::immigrantEntry, 
		buildingType::statueLarge, buildingType::fireHouse};
	const char* desc[] = { "none","road","house","water supply","entry","L statue","fire house" };

	bool checkForEptyFields(cell* c, int x, int y, bool* data)
	{
		if (!c || c->b || c->road)
		{
			*data = false;
			return false;
		}

		return true;
	}

	void select(int delta)
	{
		index += delta;
		if (index < 0) index = len - 1;
		if (index >= len) index = 0;
	}

	buildingType getBuilding()
	{
		return options[index];
	}

	const char* getDesc()
	{
		return desc[index];
	}

	void leftClickCallback()
	{
		if (getBuilding() == buildingType::none) return;

		if (getBuilding() == buildingType::road)
		{
			map.addRoad(mouse.worldx, mouse.worldy);
		}
		else
		{
			cell* c = map.at(mouse.worldx, mouse.worldy);
			if (!c) return;
			building* b = nullptr;
			int size[2];
			b->getSize(getBuilding(), size);
			bool goodToGo = true;
			map.getArea(mouse.worldx, mouse.worldy, size[0], size[1], (CELLIT)checkForEptyFields, &goodToGo);

			if (goodToGo)
			{
				b = map.addBuilding();
				b->init(getBuilding(), mouse.worldx, mouse.worldy);
			}
		}
	}

	void rightClickCallback()
	{
		cell* c = map.at(mouse.worldx, mouse.worldy);
		if (c && c->b) c->b->remove();
		if (c && c->road) map.removeRoad(mouse.worldx, mouse.worldy);
	}
}