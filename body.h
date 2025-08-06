#pragma once

namespace ph
{
	struct body
	{
		ulong id;
		ulong targetbId;
		ulong flags;
		int targetb[2];
		bodyType type;
		int x, y;
		int path[MAX_PATH][2];
		int pathIt;
		int dir[2];
		int target[2];
		int checkpoint[2];
		int stamina[2];
		int animalMoveCounter;
		int hp;

		uint sprite;

		void init(bodyType type, int x, int y, building* target);
		/// <summary>
		/// used by deserialize
		/// </summary>
		void init();
		void initGraphics();
		void initDir();
		void arrive();
		void action();
		void remove();
		void serialize(stream* s);
		void deserialize(stream* s);
		void effect(cell* c, int x, int y);
		building* getb();
		bool is(int flag);
		bool is(bodyType type);
		void animalAction();
		bool animalStopMovingIfObstruction();
	};
}