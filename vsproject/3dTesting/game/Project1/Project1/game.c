#include "gameHeader\game.h"
#include <stdio.h>
#include <stdint.h>
#include <CommonEngine.h>


EXPORT void init_game(void* p)
{
	printf("game inited!");
}


EXPORT void update_game(void* p)
{
	Engine* eng = (Engine*)p;
	printf("game updated!");

	if (/*!mouse_init ||*/ eng->inputs.inputsDisabled)
	{

		update_engine_camera(&eng->camera, eng->inputs.mousePos, eng->inputs.mousePos);
	}
	else
	{
		update_engine_camera(&eng->camera, eng->inputs.mousePos, eng->inputs.lastMousepos);
	}
}

EXPORT void dispose_game(void* p)
{
	printf("game disposed!");
}