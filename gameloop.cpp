#include <cstdio>
#include "types.h"
#include "gl.h"
#include <cmath>
#include "building.h"
#include "body.h"
#include "map.h"
#include "builder.h"
#include "screens.h"

namespace ph
{	
	screen* current;
	game theGame;
	editor theEditor;

	void gameloop()
	{
		current->run();
	}

	void mouseMove()
	{
		current->mouseMove();
	}

	void keyDown()
	{
		current->keyboard();

		if (gl::isKeyDown(0x70) && current != &theGame) // F1
		{
			gl::centerCamera();
			gl::clearSprites();
			current->destroy();
			current = &theGame;
			current->init();
		}
		else if (gl::isKeyDown(0x71) && current != &theEditor) // F2
		{
			gl::centerCamera();
			gl::clearSprites();
			current->destroy();
			current = &theEditor;
			current->init();
		}
	}

	void start()
	{
		gl::init();
		current = &theEditor;
		current->init();
		gl::run(gameloop, mouseMove, keyDown);
	}
}