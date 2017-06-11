#include "modelclass.h"


ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_TextureArray = 0;
	m_model = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, char* modelFilename, WCHAR* textureFilename1, WCHAR* textureFilename2)
{
	bool result;


	// Za�adowanie danych dla modelu
	result = LoadModel(modelFilename);
	if (!result)
	{
		return false;
	}

	//Initializacja vertexa and bufora index�w.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// Za�adowanie tekstur dla tego modelu
	result = LoadTexture(device, textureFilename1, textureFilename2);
	if (!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	// Zwolnienie pami�ci tekstur modelu.
	ReleaseTexture();

	// Wy��czenie vertex i bufora indeks�w.
	ShutdownBuffers();

	// Zwolnienie pami�ci modelu.
	ReleaseModel();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Przygotowania vertexu i bufora indeks�w do rysowania.
	RenderBuffers(deviceContext);

	return;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

// jest tablica tekst�w poniewa� mamy dwie grafiki, kt�re si� przenikaj�
ID3D11ShaderResourceView** ModelClass::GetTextureArray()
{
	return m_TextureArray->GetTextureArray();
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// Stworzenie tablicy vertex.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Stworzenie tablicy indeks�w.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Za�aduj tablice vertexa i tablic� indeks�w z danymi.
	for (i = 0; i<m_vertexCount; i++)
	{
		vertices[i].position = D3DXVECTOR3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = D3DXVECTOR2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = D3DXVECTOR3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		indices[i] = i;
	}

	// Ustawiamy opis dla statycznego vertex buffera.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType)* m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Podaj struktur� do wska�nika wierzcho�k�w.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Stworzenia bufora vertex�w.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Ustawiamy opis dla statycznego buffera indeks�w.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long)* m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Podaj struktur� do wska�nika dla index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Stw�rz bufor indeks�w.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Zwolnienie tablicy , gdy bufory wierzcho�kowe i indeksy zosta�y utworzone i za�adowane.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	//Zwolnienie pami�ci bufera indeks�w.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Zwolnienie pamieci bufora verteks�w.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	//Ustaw krok naprz�d i przesuni�cie bufora.
	stride = sizeof(VertexType);
	offset = 0;

	// Ustaw bufor vertexa jako aktywny w assemblerze wej�ciowym, aby m�g� by� renderowany
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Ustaw bufor indeksu jako aktywny w assemblerze wej�ciowym, aby m�g� by� renderowany
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Ustaw typ, kt�ry powinien by� renderowany z tego buforu vertexa, w tym przypadku tr�jk�ty.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename1, WCHAR* filename2)
{
	bool result;


	// Create the texture object.
	m_TextureArray = new TextureArrayClass;
	if (!m_TextureArray)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_TextureArray->Initialize(device, filename1, filename2);
	if (!result)
	{
		return false;
	}

	return true;
}


void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if (m_TextureArray)
	{
		m_TextureArray->Shutdown();
		delete m_TextureArray;
		m_TextureArray = 0;
	}

	return;
}


bool ModelClass::LoadModel(char* filename)
{
	ifstream fin;
	char input;
	int i;


	// Otw�rz plik z modele.
	fin.open(filename);

	// Je�eli nie mo�na otworzy� pliku.
	if (fin.fail())
	{
		return false;
	}

	// Pobierz warto�� vertex�w.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Stw�rzy model u�ywaj�c liczby z pliku.
	m_model = new ModelType[m_vertexCount];
	if (!m_model)
	{
		return false;
	}

	// Pobierz informacj� z pocz�tku pliku.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Czytaj dane vertexa.
	for (i = 0; i<m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// Zamknij plik.
	fin.close();

	return true;
}


void ModelClass::ReleaseModel()
{
	if (m_model)
	{
		delete[] m_model;
		m_model = 0;
	}

	return;
}