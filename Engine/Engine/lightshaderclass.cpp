#include "lightshaderclass.h"


LightShaderClass::LightShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;
	m_lightBuffer = 0;
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


	// Initializacja vertexa and pixel shadera.
	result = InitializeShader(device, hwnd, L"../Engine/light.vs", L"../Engine/light.ps");
	if(!result)
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
	D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView** textureArray, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColor)
{
	bool result;


	// Ustawienie parametr�w shadera do renderowania.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, textureArray, lightDirection, diffuseColor);
	if(!result)
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
	D3D11_BUFFER_DESC lightBufferDesc;


	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Kompiluj vertex shadera kod.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// Informacja w razie b��du.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// Je�eli jest b��d, ale brakuje wiadomo�ci, to program nie potrafi� znale�� plik shadera
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Kompiluj kod pixel shadera.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "LightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// Informacja w razie b��du.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// Je�eli jest b��d, ale brakuje wiadomo�ci, to program nie potrafi� znale�� plik shadera
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Stw�rz vertex shader z bufera.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Stw�rz pixel shader z bufera.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}


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
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
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
	if(FAILED(result))
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
	if(FAILED(result))
	{
		return false;
	}

	// Konfiguruj opis dynamicznego buforu sta�ego �wiat�a, kt�ry znajduje si� w pixel shaderze.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Utw�rz ci�g bufora, dzi�ki czemu mo�emy uzyska� dost�p do sta�ego bufora vertexa z tej klasy.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void LightShaderClass::ShutdownShader()
{
	// Zwolnij pami�� sta�ego bufera.
	if(m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	// Zwolnij pami�� sta�ego bufera.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Zwolnij pami�� stanu samplera.
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Zwolnij layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Zwolnij pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Zwolnij vertex shader.
	if(m_vertexShader)
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

	// Otw�rz plik.
	fout.open("shader-error.txt");

	// Zapisz informacj� o b��dzie.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Zamknij plik.
	fout.close();

	//Zwolnij pami��.
	errorMessage->Release();
	errorMessage = 0;

	// Poka� informacj� dla u�ytkownika, gdzie ma szuka� informacji o b��dzie.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool LightShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
										   D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView** textureArray, D3DXVECTOR3 lightDirection, 
										   D3DXVECTOR4 diffuseColor)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;


	// Transponuj tablice by przygotowa� je na shadery.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Zablokuj sta�y bufer by mo�na do neigo co� doda�.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Pobierz wska�nik do danych w st��ym buforze.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Przekopiowanie tablic do sta��go bufora.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Odblokowanie sta�ego bufora.
    deviceContext->Unmap(m_matrixBuffer, 0);

	//Ustaw pozycj� sta�ego bufora w vertex shaderze.
	bufferNumber = 0;

	// Ustaw sta�y bufer w vertex shaderze z nowymi warto�ciami.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Ustaw textury w pixel shaderze.
	deviceContext->PSSetShaderResources(0, 2, textureArray);

	// Zablokuj sta�y bufor light by mo�na by�o do niego co� doda�.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Pobierz wska�nik do danych w sta�ym buforze.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Skopiuj zmienne do sta�ego buforar.
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;

	// Odblokuj sta�y bufor.
	deviceContext->Unmap(m_lightBuffer, 0);

	bufferNumber = 0;

	// Ustaw light buffer w pixel shaderze z nowymi warto�ciami.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

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

	// Renderuj tr�jk�t.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}