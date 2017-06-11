#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_LightShader = 0;
	m_Light = 0;
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
	if(!m_D3D)
	{
		return false;
	}

	// Initializacja obiektu Direct3D, ifnormacjami o wysokoœci, szerokoœci.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	//Stworzenie obiektu kamery.
	m_Camera = new CameraClass;
	if(!m_Camera)
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
	result = m_Model->Initialize(m_D3D->GetDevice(), "../Engine/data/cube.txt", L"../Engine/data/stone01.png", L"../Engine/data/dirt01.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Stwórz obiek light shader.
	m_LightShader = new LightShaderClass;
	if(!m_LightShader)
	{
		return false;
	}

	// Initializuj obiekt light shader.
	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	//Stwórz light obiekt.
	m_Light = new LightClass;
	if(!m_Light)
	{
		return false;
	}

	// Initializuj light obiekt.
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}


void GraphicsClass::Shutdown()
{
	//Zwalnianie pamiêci.
	if(m_Light)
	{
		delete m_Light;
		m_Light = 0;
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
	static float rotation = 0.0f;


	// Aktualizacji zmiennych po ka¿dej klatce.
	rotation += (float)D3DX_PI * 0.08f;
	if(rotation > 360.0f)
	{
		rotation -= 360.0f;
	}
	
	// Renderuj grafikê sceny.
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
	bool result;


	// Czyœc bufer do pocz¹tkowej sceny.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Stwó tablicê widoku bazuj¹æ na pozycji kamery
	m_Camera->Render();

	// Pobierz œwiat, widok i tablice projekcyjne z obiektów kamery i d3d.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	// Obracaj macierz œwiata.
	D3DXMatrixRotationY(&worldMatrix, rotation);

	//Umieœæ modelowe vertexów i bufory indeksu , aby przygotowaæ je do rysowania.
	m_Model->Render(m_D3D->GetDeviceContext());

	// Renderuj model u¿ywaj¹c light shader.
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
								   m_Model->GetTextureArray(), m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if(!result)
	{
		return false;
	}

	// Poka¿ wyrenderowan¹ scenê na ekranie.
	m_D3D->EndScene();

	return true;
}