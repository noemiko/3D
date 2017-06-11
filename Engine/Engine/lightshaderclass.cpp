#include "lightshaderclass.h"

LightShaderClass::LightShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;
	m_lightColorBuffer = 0;
	m_lightPositionBuffer = 0;
}

LightShaderClass::LightShaderClass(const LightShaderClass& other)
{
}

LightShaderClass::~LightShaderClass()
{
}

bool LightShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initializacja vertexa i pixel shadera.
	result = InitializeShader(device, hwnd, L"../Engine/light.vs", L"../Engine/light.ps");
	if (!result)
	{
		return false;
	}

	return true;
}


void LightShaderClass::Shutdown()
{
	// Wy�aczenie vertexa and pixel shadera i powi�zanych obiekt�w.
	ShutdownShader();

	return;
}


bool LightShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
	D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView** textureArray, D3DXVECTOR4 diffuseColor[],
	D3DXVECTOR4 lightPosition[])
{
	bool result;


	// Ustawienie parametr�w shadera do renderowania.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, textureArray, diffuseColor, lightPosition);
	if (!result)
	{
		return false;
	}

	// Renderuj przygotowane bufery z shaderem.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool LightShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightColorBufferDesc;
	D3D11_BUFFER_DESC lightPositionBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Kompiluj kod vertex shadera
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		// Informacja w razie b��du.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// Je�eli jest b��d, ale brakuje wiadomo�ci, to program nie potrafi� znale�� plik shadera.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Kompiluj kod pixel shadera.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "LightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		// Je�eli jest b��d,shadero to pokazuje wiadomo��.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// Je�eli jest b��d, ale brakuje wiadomo�ci, to program nie potrafi� znale�� plik shadera.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Stw�rz vertex shader z bufera.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
		&m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Stw�rz pixel shader z bufera.

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
		&m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}


	// Stw�rz opis layoutu 
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// Policz elemnety w layoucie.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Stworz vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// Zwolnij pami�� vartex shadera i pixel shadera, bo nie s� wi�cej potrzebne
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	// Utw�rz opis stanu samplera tekstur.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Stw�rz stan texture samplera.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}


	// Konfiguruj opis sta�ego bufora dynamicznego, kt�ry znajduje si� w vertex shaderze
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Utw�rz ci�g bufora, dzi�ki czemu mo�emy uzyska� dost�p do sta�ego bufora vertexa z tej klasy.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Konfiguruj opis dynamicznego buforu sta�ego �wiat�a, kt�ry znajduje si� w pixel shaderze.
	lightColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightColorBufferDesc.ByteWidth = sizeof(LightColorBufferType);
	lightColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightColorBufferDesc.MiscFlags = 0;
	lightColorBufferDesc.StructureByteStride = 0;

	// Utw�rz ci�g bufora, dzi�ki czemu mo�emy uzyska� dost�p do sta�ego bufora vertexa z tej klasy.
	result = device->CreateBuffer(&lightColorBufferDesc, NULL, &m_lightColorBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Konfiguracja opisu sta�ego bufora dynamicznego, kt�ry znajduje si� w vertex shaderze.
	lightPositionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightPositionBufferDesc.ByteWidth = sizeof(LightPositionBufferType);
	lightPositionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightPositionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightPositionBufferDesc.MiscFlags = 0;
	lightPositionBufferDesc.StructureByteStride = 0;

	// Utw�rz ci�g bufora, dzi�ki czemu mo�emy uzyska� dost�p do sta�ego bufora vertexa z tej klasy.
	result = device->CreateBuffer(&lightPositionBufferDesc, NULL, &m_lightPositionBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void LightShaderClass::ShutdownShader()
{
	// Uwalnianie pami�ci
	if (m_lightColorBuffer)
	{
		m_lightColorBuffer->Release();
		m_lightColorBuffer = 0;
	}
	if (m_lightPositionBuffer)
	{
		m_lightPositionBuffer->Release();
		m_lightPositionBuffer = 0;
	}

	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}


void LightShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Pobierz wska�nik do wiadomo�ci o b��dzie.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Pobierz d�ugo�� wiadomo�ci.
	bufferSize = errorMessage->GetBufferSize();

	// Otw�rz plik
	fout.open("shader-error.txt");

	// Wpisz wiadomo�� b��du
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Zamknij plik.
	fout.close();

	// Uwolnij pami��.
	errorMessage->Release();
	errorMessage = 0;

	// Poka� informacj� dla u�ytkownika, gdzie ma szuka� informacji o b��dzie.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool LightShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
	D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView** textureArray,
	D3DXVECTOR4 diffuseColor[], D3DXVECTOR4 lightPosition[])
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightPositionBufferType* dataPtr2;
	LightColorBufferType* dataPtr3;
	unsigned int bufferNumber;


	// Transponuj tablice by przygotowa� je na shadery.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Zablokuj sta�y bufer by mo�na do neigo co� doda�.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Pobierz wska�nik do danych w st��ym buforze.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Przekopiowanie tablic do sta�ego bufora.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	// Odblokowanie sta�ego bufora.

	deviceContext->Unmap(m_lightColorBuffer, 0);

	//Ustaw pozycj� sta�ego bufora w vertex shaderze.
	bufferNumber = 0;

	// Ustaw sta�y bufer w vertex shaderze z nowymi warto�ciami.
	deviceContext->PSSetShaderResources(0, 2, textureArray);
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Zablokuj sta�y bufor light by mo�na by�o do niego co� doda�.
	result = deviceContext->Map(m_lightPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}
	// Pobierz wska�nik do danych w sta�ym buforze.
	dataPtr2 = (LightPositionBufferType*)mappedResource.pData;

	// Skopiuj zmienne do sta�ego buforar.
	dataPtr2->lightPosition[0] = lightPosition[0];
	dataPtr2->lightPosition[1] = lightPosition[1];
	dataPtr2->lightPosition[2] = lightPosition[2];
	dataPtr2->lightPosition[3] = lightPosition[3];
	// Odblokuj sta�y bufor
	deviceContext->Unmap(m_lightPositionBuffer, 0);

	bufferNumber = 1;

	// Ustaw light buffer w pixel shaderze z nowymi warto�ciami.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_lightPositionBuffer);

	deviceContext->PSSetShaderResources(0, 1, textureArray);

	// Zablokuj sta�y bufer by mo�na do niego dodawa�.
	result = deviceContext->Map(m_lightColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Pobierz wska�nik do danych w sta�ym buforze.
	dataPtr3 = (LightColorBufferType*)mappedResource.pData;

	// KOpiuj kolor �wiat�a do sta�ego bufora.
	dataPtr3->diffuseColor[0] = diffuseColor[0];
	dataPtr3->diffuseColor[1] = diffuseColor[1];
	dataPtr3->diffuseColor[2] = diffuseColor[2];
	dataPtr3->diffuseColor[3] = diffuseColor[3];

	
	// Odblokuj sta�y bufor.
	deviceContext->Unmap(m_lightColorBuffer, 0);

	// Ustaw pozycj� w tablicy sta�ego buforu w vertex shaderze 
	bufferNumber = 0;

	// Ustaw tablic� sta�ego buforu w vertex shaderze z aktualizowanymi warto�ciami
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Ustaw tablic� sta�ego buforu w pixel shaderze z aktualizowanymi warto�ciami
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightColorBuffer);

	return true;
}


void LightShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	// Ustaw vertex and pixel shader kt�ry b�dzie renderowa� tr�jk�t.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Ustaw stan samplera w pixel shaderze.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Renderuj tr�jk�ty.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}