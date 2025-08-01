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

namespace ph
{
	const int buildIndexLen = 10;
	const char* buildDesc[] = { "clear","water","rock","gold","sand","tree","reed","meadow","field","entrance" };

	void editor::init()
	{
		this->buildIndex = 0;
		map.init();
	}

	void editor::run()
	{
		sprintf(this->textbuffer, "Editor %4d %4d %s", mouse.worldx, mouse.worldy, buildDesc[this->buildIndex]);
		gl::setText(0, this->textbuffer);
	}

	void editor::destroy()
	{

	}

	void serialize()
	{
		stream s;
		s.openWriteFileStream("./maps/map.dat");
		s.writeInt32(map.GRID_SIZE);
		for (uint i = 0; i < map.GRID_SIZE; i++)
		{
			for (uint j = 0; j < map.GRID_SIZE; j++)
			{
				cell* c = map.at(i, j);
				massert(c,"cell is null");
				if (c->b && c->b->type == buildingType::immigrantEntry)
					s.writeInt32((int)editorBuildType::entrance);
				else
					s.writeInt32((int)c->type);
			}
		}
	}

	void deserialize()
	{
		stream s;
		if (!s.openReadFileStream("./maps/map.dat")) return;

		gl::clearSprites();
		map.GRID_SIZE = s.readInt32();
		map.init();

		for (uint i = 0; i < map.GRID_SIZE; i++)
		{
			for (uint j = 0; j < map.GRID_SIZE; j++)
			{
				cell* c = map.at(i, j);
				massert(c, "cell is null");
				int data = s.readInt32();
				if (data == (int)editorBuildType::entrance)
				{
					building* b = map.addBuilding();
					b->init(buildingType::immigrantEntry, i, j);
				}
				else if(data > 0)
				{
					c->type = (cellType)data;

					if (c->type == cellType::water)
					{
						float col[3] = { 0,0,0.5f };
						c->spriteIndex = gl::addSprite(col, i, j, 0.99f, 1, 1);
					}
				}
			}
		}
	}

	void editor::keyboard()
	{
		if (gl::isKeyPressed('Q'))
		{
			this->buildIndex -= 1;
			if (this->buildIndex < 0) this->buildIndex = buildIndexLen - 1;
		}
		else if (gl::isKeyPressed('E'))
		{
			this->buildIndex += 1;
			if (this->buildIndex >= buildIndexLen) this->buildIndex = 0;
		}
		else if (gl::isKeyDown(1))
		{
			cell* c = map.at(mouse.worldx, mouse.worldy);
			if (!c) return;

			c->type = cellType::empty;
			if (c->spriteIndex != -1) gl::removeSprite(c->spriteIndex);
			c->spriteIndex = -1;
			if (c->b) c->b->remove();

			if (this->buildIndex == 0)
			{
				c->type = (cellType)this->buildIndex;
			}
			else if (this->buildIndex >= (int)cellType::water && this->buildIndex <= (int)cellType::field)
			{
				c->type = (cellType)this->buildIndex;

				if (this->buildIndex == (int)cellType::water)
				{
					float col[3] = { 0,0,0.5f };
					c->spriteIndex = gl::addSprite(col, mouse.worldx, mouse.worldy, 0.99f, 1, 1);
				}
			}
			else if (this->buildIndex == (int)editorBuildType::entrance)
			{
				building* b = map.addBuilding();
				b->init(buildingType::immigrantEntry, mouse.worldx, mouse.worldy);
			}
		}
		else if (gl::isKeyPressed(0x74)) // f5
		{
			serialize();
		}
		else if (gl::isKeyPressed(0x75)) // f6
		{
			deserialize();
		}
	}

	void editor::mouseMove()
	{

	}
}