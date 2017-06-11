#include "inputclass.h"


InputClass::InputClass()
{
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


void InputClass::Initialize()
{
	int i;
	

	// Initializacja wszystkich klawiszy jako nie wciœniete
	for(i=0; i<256; i++)
	{
		m_keys[i] = false;
	}

	return;
}


void InputClass::KeyDown(unsigned int input)
{
	//Na wciœniêcie klawisza
	m_keys[input] = true;
	return;
}


void InputClass::KeyUp(unsigned int input)
{
	// Po puszczeniu klawisza znów nie jest on wciœniêty.
	m_keys[input] = false;
	return;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// Pobranie informacji o klawiszu.
	return m_keys[key];
}