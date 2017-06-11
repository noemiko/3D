#include "systemclass.h"


SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;


	// Initializacja wysokoœci i szerokoœci zanim przeka¿emy zmienne do fuknkcji.
	screenWidth = 0;
	screenHeight = 0;

	// Initializacja windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Stworzenie obiektu input, który zajmuje siê zczytywanie tego co wyklika u¿ytkownik
	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}

	//Initializacja obiektu do inputów 
	m_Input->Initialize();

	// Stworzenie obiektu grafiki, który zajmuje siê renderowaniem grafiki dla ca³ej aplikacji
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	// Initializacaja obiektu grafiki - szerokoœci¹, wysokoœci¹ i obiektem input
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
	{
		return false;
	}

	return true;
}

// Zwalnianie pamiêci i zakmniêcie okna
void SystemClass::Shutdown()
{
	// Zwolnienie pamiêci po obiekcie grafiki
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// Zwolnienie pamiêci po obiekcie inputów
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// Zamkniêcie okna
	ShutdownWindows();

	return;
}

// uruchomienie obiektów do stworzenia grafiki
void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initializacja struktury pamiêcie
	ZeroMemory(&msg, sizeof(MSG));

	// Pêtla dopóki nie bêdzie wiadomoœci o przerwaniu
	done = false;
	while (!done)
	{
		// Sprawdzanie wiadomoœci od okna
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Je¿eli jest wiadomoœc o zakoñczeniu wtedy pêtla jest przerywana
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Kolejna klatka jest tworzona
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}

	}

	return;
}


bool SystemClass::Frame()
{
	bool result;


	//Sprawdza czy u¿ytkownik wcisn¹³ esc, aby wyjœæ
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// Przetwarzanie ramek dla graficznego obiektu
	result = m_Graphics->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// Sprawdzenie czy klawisz zostawi³ wciœniety
	case WM_KEYDOWN:
	{
					   // Przes³anie tej informacji do obiektu zajmuj¹cego siê inputami
					   m_Input->KeyDown((unsigned int)wparam);
					   return 0;
	}

		// Sprawdzenie czy klawisz zosta³ zwolniony
	case WM_KEYUP:
	{
					 // Przeslanie tej informacji do obiektu input
					 m_Input->KeyUp((unsigned int)wparam);
					 return 0;
	}

		// Inne informacje o klawiszach nic nie wywo³aj¹
	default:
	{
			   return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Pobierz wskaŸnik
	ApplicationHandle = this;

	// Pobierz instancjê aplikacji
	m_hinstance = GetModuleHandle(NULL);

	// Nazwa aplikacji
	m_applicationName = L"Projekt 3D";

	// Ustawl klasê okna z defaultowymi ustawieniami
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Rejestruj klasê okna.
	RegisterClassEx(&wc);

	// Okreœl rozdzielczoœæ ekranu pulpitu klienta.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// ustawienia wyswietalnia keranu w zale¿noœci od wybranych opcji
	if (FULL_SCREEN)
	{
		// ustaw maksymalny rozmiar okna
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Ustaw pe³en ekran
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Ustaw pozycjê okna na lewy górny róg 
		posX = posY = 0;
	}
	else
	{
		// Ustawienie szerokoœci i wysokoœci
		screenWidth = 800;
		screenHeight = 600;

		// Wyœrodkój okno na ekranie
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Stwórz okno
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Poka¿ okno na przedzie
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Schowaj kursor myszy
	ShowCursor(false);

	return;
}


void SystemClass::ShutdownWindows()
{
	//Poka¿ kursor muszy
	ShowCursor(true);

	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Usuñ okno
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// usuñ instancje aplikacji
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Zwolnienie pamiêci
	ApplicationHandle = NULL;

	return;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Sprawdzenie czy okno zosta³o zniszczone
	case WM_DESTROY:
	{
					   PostQuitMessage(0);
					   return 0;
	}

		// Sprawdzenie czy okno zosta³o zamkniête
	case WM_CLOSE:
	{
					 PostQuitMessage(0);
					 return 0;
	}

		// Wszystkie inne wiadomoœci idê do message handlera
	default:
	{
			   return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}