#pragma once
namespace ph
{
	struct screen
	{
		char textbuffer[400];

		virtual void init() = 0;
		virtual void run() = 0;
		virtual void destroy() = 0;
		virtual void keyboard() = 0;
		virtual void mouseMove() = 0;
	};

	struct game : public screen
	{
		float tickLength;
		float lastTick;

		virtual void init();
		virtual void run();
		virtual void destroy();
		virtual void keyboard();
		virtual void mouseMove();
	};

	struct editor : public screen
	{
		int buildIndex;

		virtual void init();
		virtual void run();
		virtual void destroy();
		virtual void keyboard();
		virtual void mouseMove();
	};
}