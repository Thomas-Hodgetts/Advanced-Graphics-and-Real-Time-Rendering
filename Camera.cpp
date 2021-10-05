#include "Camera.h"


//Pages 500 onwards

Camera::Camera(XMFLOAT4 position, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_Eye = position;
	m_At = at;
	m_Up = up;
	m_Right = XMFLOAT4(1,0,0,0);
	m_WindowHeight = windowHeight;
	m_WindowWidth = windowWidth;
	m_NearDepth = nearDepth;
	m_FarDepth = farDepth;
	XMStoreFloat4x4(&m_View, XMMatrixLookAtLH(XMLoadFloat4(&m_Eye), XMLoadFloat4(&m_At), XMLoadFloat4(&m_Up)));
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, m_WindowWidth / m_WindowHeight, m_NearDepth, m_FarDepth));

}

Camera::~Camera()
{

}


void Camera::SetLookTo(XMVECTOR Forwardvector)
{
	XMVECTOR Pos = GetEye();
	XMVECTOR Target = Pos + Forwardvector;
	XMStoreFloat4x4(&m_View, XMMatrixLookToLH(XMLoadFloat4(&m_Eye), XMLoadFloat4(&m_At), XMLoadFloat4(&m_Up)));
}

void Camera::Zoom()
{
	if (m_Eye.x < m_At.x)
	{
		XMStoreFloat4x4(&m_View, XMMatrixTranslation(1.f, 0.f, 0.f));
	}
	if (m_Eye.y < m_At.y)
	{
		XMStoreFloat4x4(&m_View, XMMatrixTranslation(0.f, 1.f, 0.f));
	}
	if (m_Eye.z < m_At.z)
	{
		XMStoreFloat4x4(&m_View, XMMatrixTranslation(0.f, 0.f, 1.f));
	}
}

void Camera::Unzoom()
{
	XMStoreFloat4x4(&m_View, XMMatrixTranslation(0.0f, 0.f, -1));
}

//Book
void Camera::Walk(float Direction)
{
	XMVECTOR Speed = XMVectorReplicate(Direction);
	XMVECTOR NewLook = GetAt();
	XMVECTOR Pos = GetEye();
	XMStoreFloat4(&m_Eye, XMVectorMultiplyAdd(Speed, NewLook, Pos));
}
//Book
void Camera::Strafe(float Direction)
{
	XMVECTOR Speed = XMVectorReplicate(Direction);
	XMVECTOR Right = GetRight();
	XMVECTOR Pos = GetEye();
	XMStoreFloat4(&m_Eye, XMVectorMultiplyAdd(Speed, Right, Pos));
}
//Book
void Camera::Pitch(float Angle)
{
	XMMATRIX Rotation = XMMatrixRotationAxis(GetRight(), Angle);
	XMStoreFloat4(&m_Up, XMVector3TransformNormal(GetUp(), Rotation));
	XMStoreFloat4(&m_At, XMVector3TransformNormal(GetAt(), Rotation));
}
//Book
void Camera::Yaw(float Angle)
{
	XMMATRIX Rotation = XMMatrixRotationY(Angle);
	XMStoreFloat4(&m_Right, XMVector3TransformNormal(GetRight(), Rotation));
	XMStoreFloat4(&m_Up, XMVector3TransformNormal(GetUp(), Rotation));
	XMStoreFloat4(&m_At, XMVector3TransformNormal(GetAt(), Rotation));
}

void Camera::Update()
{
	if (m_At.x == 0 && m_At.y == 0 && m_At.z == 0);
	{
		m_At.z =  1;
	}
	XMStoreFloat4x4(&m_View, XMMatrixLookToLH(XMLoadFloat4(&m_Eye), XMLoadFloat4(&m_At), XMLoadFloat4(&m_Up)));
}

void Camera::ManageMouseMovement(MouseData MD)
{
	float dx = XMConvertToRadians(0.05f * (MD.xPos - MD.PrevXPos));
	float dy = XMConvertToRadians(0.05f * (MD.yPos - MD.PrevYPos));
	Pitch(dy);
	Yaw(dx);


}

CameraBufferData Camera::ReturnViewPlusProjection()
{
	CameraBufferData CBD;
	CBD.m_projection = m_Projection;
	CBD.m_view = m_View;
	return CBD;
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_NearDepth = nearDepth;
	m_FarDepth = farDepth;
	m_WindowHeight = windowHeight;
	m_WindowWidth = windowWidth;
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, m_WindowWidth / (FLOAT)m_WindowHeight, m_NearDepth, m_FarDepth));
}

CameraData 	Camera::ReturnCamData()
{
	CameraData CD;
	CD.m_eye = m_Eye;
	CD.m_at = m_At;
	CD.m_up = m_Up;
	return CD;
}