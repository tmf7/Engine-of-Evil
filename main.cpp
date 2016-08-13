//**********************************************
//*
//*Copyright Thomas Freehill July 29 2016
//*This is just garbage code I'm using to learn
//*how I'd like to structure my future code
//*and to get back in the rhythm of 
//*readability and perfomance
//*
//**********************************************

#include "Game.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	Game evil;

	if (!evil.Init())
		return 0;

	evil.GetEntities()->Spawn();

	while (evil.Run());

	evil.Shutdown(NULL);

	return 0;
}




