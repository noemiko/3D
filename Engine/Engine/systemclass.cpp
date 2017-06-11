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


	// Initializacja wysoko�ci i szeroko�ci zanim przeka�emy zmienne do fuknkcji.
	screenWidth = 0;
	screenHeight = 0;

	// Initializacja windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Stworzenie obiektu input, kt�ry zajmuje si� zczytywanie tego co wyklika u�ytkownik
	m_Input = new InputClass;
	if(!m_Input)
	{
		return false;
	}

	//Initializacja obiektu do input�w 
	m_Input->Initialize();

	// Stworzenie obiektu grafiki, kt�ry zajmuje si� renderowaniem grafiki dla ca�ej aplikacji
	m_Graphics = new GraphicsClass;
	if(!m_Graphics)
	{
		return false;
	}

	// Initializacaja obiektu grafiki - szeroko�ci�, wysoko�ci� i obiektem input
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if(!result)
	{
		return false;
	}
	
	return true;
}

// Zwalnianie pami�ci i zakmni�cie okna
void SystemClass::Shutdown()
{
	// Zwolnienie pami�ci po obiekcie grafiki
	if(m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// Zwolnienie pami�ci po obiekcie input�w
	if(m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// Zamkni�cie okna
	ShutdownWindows();
	
	return;
}

// uruchomienie obiekt�w do stworzenia grafiki
void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initializacja struktury pami�cie
	ZeroMemory(&msg, sizeof(MSG));
	
	// P�tla dop�ki nie b�dzie wiadomo�ci o przerwaniu
	done = false;
	while(!done)
	{
		// Sprawdzanie wiadomo�ci od okna
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Je�eli jest wiadomo�c o zako�czeniu wtedy p�tla jest przerywana
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Kolejna klatka jest tworzona
			result = Frame();
			if(!result)
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


	//Sprawdza czy u�ytkownik wcisn�� esc, aby wyj��
	if(m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// Przetwarzanie ramek dla graficznego obiektu
	result = m_Graphics->Frame();
	if(!result)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch(umsg)
	{
		// Sprawdzenie czy klawisz zostawi� wci�niety
		case WM_KEYDOWN:
		{
			// Przes�anie tej informacji do obiektu zajmuj�cego si� inputami
			m_Input->KeyDown((unsigned int)wparam);
			return 0;
		}

		// Sprawdzenie czy klawisz zosta� zwolniony
		case WM_KEYUP:
		{
			// Przeslanie tej informacji do obiektu input
			m_Input->KeyUp((unsigned int)wparam);
			return 0;
		}

		// Inne informacje o klaiwszach nic nie wywo�aj�
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


	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"Engine";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth  = 800;
		screenHeight = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}


void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}