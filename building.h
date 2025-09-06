#pragma once

namespace ph
{
	struct building
	{
		ulong id;
		ulong flags;
		union
		{
			struct { int x, y; };
			vec2i pos;
		};
		union
		{
			struct { int w, h; };
			vec2i size;
		};
		buildingType type;
		vec2i occupants, workers, water, fire, collapse, religion[MAX_GODS], health, entertainment[MAX_ENTERTAINMENT];
		int immigrants, recruiters;
		int recruiterCounter, workerCounter, employementCounter, delivery;
		vec2i door;
		int houseLevel;
		goods raw;
		goods finished;
		vec2i resources[MAX_RESOURCE_TYPES];
		bodyType walkerType;
		bodyType animalType;


		uint sprite;
		
		void action();
		void init(buildingType type, int x, int y);
		/// <summary>
		/// used by deserialize
		/// </summary>
		void init();
		bool tryDoor(cell* c, int x, int y);
		void updateDoor();
		void remove();
		void initGraphics();
		void serialize(stream* s);
		void deserialize(stream* s);
		void recruit();
		void recalcEmployees();
		void evolveHouse();
		void burnDown();
		void buildingCollapsed();
		bool is(int flag);
		bool is(buildingType type);
	};
}