#pragma once

namespace ph
{
	struct building
	{
		ulong id;
		ulong flags;
		int x, y, w, h;
		buildingType type;
		int occupants, maxOccupants, immigrants, recruiters, workers, maxWorkers;
		int recruiterCounter, workerCounter;
		int employementCounter;
		int door[2];
		int water[2];
		int fire[2];
		int collapse[2];
		int houseLevel;
		uint sprite;
		bodyType walkerType;
		
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