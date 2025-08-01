#pragma once

namespace ph
{
	struct body
	{
		ulong id;
		int targetb[2];
		ulong targetbId;
		bodyType type;
		int x, y;
		int path[10][2];
		int dir[2];
		int target[2];
		int checkpoint[2];
		int stamina[2];
		uint sprite;
		bool live;
		bool roam;
		bool hasEffect;

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
	};
}