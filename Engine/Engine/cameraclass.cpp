#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}


D3DXVECTOR3 CameraClass::GetPosition()
{
	return D3DXVECTOR3(m_positionX, m_positionY, m_positionZ);
}


D3DXVECTOR3 CameraClass::GetRotation()
{
	return D3DXVECTOR3(m_rotationX, m_rotationY, m_rotationZ);
}


void CameraClass::Render()
{
	D3DXVECTOR3 up, position, lookAt;
	float yaw, pitch, roll;
	D3DXMATRIX rotationMatrix;


	// Ustaw wektor, kt�ry skieruje si� ku g�rze.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Ustaw pozycj� kamery na �wiecie.
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	// Ustaw domy�ln� lokalizacj� kamery.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Ustaw odchylenie (o� Y), skok (o� X) i obroty (o� Z) w radianach.
	pitch = m_rotationX * 0.0174532925f;
	yaw   = m_rotationY * 0.0174532925f;
	roll  = m_rotationZ * 0.0174532925f;

	// Utw�rz macierz rotacji z warto�ci odchylenia, skoku i obrotu.
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

	// Przekszta�� wektor lookAt i up przez matryc� rotacji, aby widok zosta� prawid�owo obr�cony w kierunku pocz�tku.
	D3DXVec3TransformCoord(&lookAt, &lookAt, &rotationMatrix);
	D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

	// Przet�umacz obr�cone po�o�enie kamery w po�o�enie spogl�daj�cego.
	lookAt = position + lookAt;

	// Na koniec utw�rz macierz widoku z trzech zaktualizowanych wektor�w.
	D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);

	return;
}


void CameraClass::GetViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}