#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_LightShader = 0;
	m_Light1 = 0;
	m_Light2 = 0;
	m_Light3 = 0;
	m_Light4 = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;


	// Stworzenie obiektu Direct3D.
	m_D3D = new D3DClass;
	if (!m_D3D)
	{
		return false;
	}

	// Initializacja obiektu Direct3D, inormacjami o wysokoœci, szerokoœci, czy na pe³en ekran itd.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	//Stworzenie obiektu kamery.
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// Ustawienie pocz¹tkowej pozycji kamery.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	
	// Stworzenie obiektu.
	m_Model = new ModelClass;
	if(!m_Model)
	{
		return false;
	}

	// Initializuj model.
	result = m_Model->Initialize(m_D3D->GetDevice(), "../Engine/data/cube.txt", L"../Engine/data/box.png", L"../Engine/data/grass.png");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create the light shader object.
	m_LightShader = new LightShaderClass;
	if(!m_LightShader)
	{
		return false;
	}

	// Stwórz obiek light shader.
	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// Initializuj obiekt light shader.
	//Stwórz obiekt pierwszego œwiat³a.
	m_Light1 = new LightClass;
	if (!m_Light1)
	{
		return false;
	}

	// Initializuje obiekt pierwszego œwiat³a
	m_Light1->SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
	m_Light1->SetPosition(-10.0f, 10.0f, -5.0f);

	//Stwórz obiekt drugiego œwiat³a.
	m_Light2 = new LightClass;
	if (!m_Light2)
	{
		return false;
	}

	// Initializuje obiekt drugiego œwiat³a
	m_Light2->SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);
	m_Light2->SetPosition(10.0f, 10.0f, -5.0f);

	//Stwórz obiekt trzeciego œwiat³a.
	m_Light3 = new LightClass;
	if (!m_Light3)
	{
		return false;
	}

	// Initializuje obiekt trzeciego œwiat³a
	m_Light3->SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);
	m_Light3->SetPosition(-10.0f, -10.0f, -5.0f);

	//Stwórz obiekt czwartego œwiat³a.
	m_Light4 = new LightClass;
	if (!m_Light4)
	{
		return false;
	}

	// Initializuje obiekt czwartego œwiat³a
	m_Light4->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light4->SetPosition(10.0f, -10.0f, -5.0f);

	return true;
}


void GraphicsClass::Shutdown()
{
	// Uwolnienie pamiêci obiektów œwiat³a.
	if (m_Light1)
	{
		delete m_Light1;
		m_Light1 = 0;
	}

	if (m_Light2)
	{
		delete m_Light2;
		m_Light2 = 0;
	}

	if (m_Light3)
	{
		delete m_Light3;
		m_Light3 = 0;
	}

	if (m_Light4)
	{
		delete m_Light4;
		m_Light4 = 0;
	}

	if(m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	if(m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}


bool GraphicsClass::Frame()
{
	bool result;
	static float rotation = 0.1f;


	// Aktualizacji zmiennych po ka¿dej klatce
	rotation += (float)D3DX_PI * 0.003f;
	if(rotation > 360.0f)
	{
		rotation -= 360.0f;
	}
	
	// Renderuj grafikê sceny
	result = Render(rotation);

	if(!result)
	{
		return false;
	}

	return true;
}

bool GraphicsClass::Render(float rotation)
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3DXVECTOR4 diffuseColor[4];
	D3DXVECTOR4 lightPosition[4];
	bool result;

	// Utwórz tablicê kolorów rozproszonych z czterech œwiate³ kolorów.
	diffuseColor[0] = m_Light1->GetDiffuseColor();
	diffuseColor[1] = m_Light2->GetDiffuseColor();
	diffuseColor[2] = m_Light3->GetDiffuseColor();
	diffuseColor[3] = m_Light4->GetDiffuseColor();

	// Utwórz tablicê pozycji œwiat³a z czterech pozycji œwiat³a.
	lightPosition[0] = m_Light1->GetPosition();
	lightPosition[1] = m_Light2->GetPosition();
	lightPosition[2] = m_Light3->GetPosition();
	lightPosition[3] = m_Light4->GetPosition();

	// Czyœc bufer dla pocz¹tkowej sceny.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Stwórz tablicê widoku bazuj¹æ na pozycji kamery
	m_Camera->Render();

	// Obracaj macierz œwiata.
	D3DXMatrixRotationY(&worldMatrix, rotation);

	// Pobierz œwiat, widok i tablice projekcyjne z obiektów kamery i d3d.
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	// Obracaj macierz œwiata.
	D3DXMatrixRotationY(&worldMatrix, rotation);

	//Umieœæ bufory vertexów i bufory indeksu , aby przygotowaæ je do rysowania.
	m_Model->Render(m_D3D->GetDeviceContext());

	// Renderuj model u¿ywaj¹c light shader.
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
		m_Model->GetTextureArray(), diffuseColor, lightPosition);
	if(!result)
	{
		return false;
	}

	// Poka¿ wyrenderowan¹ scenê na ekranie.
	m_D3D->EndScene();

	return true;
}