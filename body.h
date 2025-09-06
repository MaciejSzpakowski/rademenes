#pragma once

namespace ph
{
	struct body
	{
		ulong id;
		ulong targetbId;
		ulong flags;
		ulong targetaId;
		bodyType type;
		ulong state;
		union
		{
			struct { int x, y; };
			vec2i pos;
		};
		vec2i path[MAX_PATH];
		int pathIt;
		vec2i dir;
		vec2i target;
		vec2i stamina;
		int hp;
		int waitCounter;
		int resource;
		goods resourceType;

		building* targetb;
		body* targeta;
		uint sprite;

		void* operator new(size_t count);
		virtual void init(bodyType type, int x, int y, building* target);
		/// <summary>
		/// used by deserialize
		/// </summary>
		void init();
		void initGraphics();
		void initDir();
		virtual void arrive();
		virtual void action();
		void remove();
		void serialize(stream* s);
		void deserialize(stream* s);
		// each walker may hit building 3 times because area to search is 3x3 so by the time they pass
		// the building of interest was in the area of effect 3 times
		virtual void effect(cell* c, int x, int y);
		bool is(int flag);
		bool is(bodyType type);
		/// <summary>
		/// temporarily used instead of initdir
		/// </summary>
		void damage(int hit, body* source);
		void runAwayFrom(body* b);
	};

	struct immigrant : public body
	{
		/// <summary>
		/// need to override default ctor because default ctor ereases everything including dynamic binding method table
		/// </summary>
		immigrant();
		virtual void init(bodyType type, int x, int y, building* target);
		virtual void action();
		virtual void arrive();
	};

	struct walker : public body
	{
		virtual void action();
		virtual void arrive();
	};

	struct recruiter : public walker
	{
		recruiter();
		virtual void effect(cell* c, int x, int y);
		virtual void arrive();
	};

	struct waterCarrier : public walker
	{
		waterCarrier();
		virtual void effect(cell* c, int x, int y);
	};

	struct fireMarshal : public walker
	{
		fireMarshal();
		virtual void effect(cell* c, int x, int y);
	};

	struct architect : public walker
	{
		architect();
		virtual void effect(cell* c, int x, int y);
	};

	struct hunter : public body
	{
		hunter();
		virtual void init(bodyType type, int x, int y, building* target);
		virtual void action();
		virtual void arrive();
	};

	struct animal : public body
	{
		animal();
		virtual void init(bodyType type, int x, int y, building* target);
		virtual void action();
		void stop();
		bool stopMovingIfObstruction();
	};

	struct delivery : public body
	{
		virtual void init(bodyType type, int x, int y, building* target);
		virtual void action();
		virtual void arrive();
	};
}