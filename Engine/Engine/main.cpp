#include "systemclass.h"

//punkt startowy aplikacji 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	SystemClass* System;
	bool result;


	// Stworzenie obiektu, kt�ry uruchamia wszystkie potrzebne obiekty do otwarcia okna, renderowania.
	System = new SystemClass;
	if (!System)
	{
		return 0;
	}

	// Initializacja i uruchomienie obiektu System
	result = System->Initialize();



	if (result)
	{
		System->Run();
	}

	//Zwolnienie pamieci obiekt�w stworzonych w obiekcie System i zamkni�cie okna.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}