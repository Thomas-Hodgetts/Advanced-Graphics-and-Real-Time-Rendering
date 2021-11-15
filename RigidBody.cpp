#include "RigidBody.h"

Vector3D RigidBody::TorqueVect(Vector3D* RelPos, Vector3D* ForceVector)
{
	return ForceVector->CrossProductCalculation(*RelPos);
}

void RigidBody::TorqueVect(Vector3D* RelPos)
{
	m_ForceVector.CrossProductCalculation(*RelPos);
}

float RigidBody::CalculateSphereRigidBod(float mass)
{
	float returns;
	//float returns = pow(x, 2) + pow(y, 2);
	return  float(0.4) * mass;

}
float RigidBody::CalculateCubeRigidBod(float x, float y, float mass)
{
	float returns = pow(x, 2) + pow(y, 2);
	return float(0.083) * (returns * mass);
}

void RigidBody::SetUpTensor(RigidBodyShape type, float x, float y, float z, float mass)
{
	switch (type)
	{
	default:

		m_InertialTensor._11 = 0.f;
		m_InertialTensor._12 = 0.f;
		m_InertialTensor._13 = 0.f;
		m_InertialTensor._21 = 0.f;
		m_InertialTensor._22 = 0.f;
		m_InertialTensor._23 = 0.f;
		m_InertialTensor._31 = 0.f;
		m_InertialTensor._32 = 0.f;
		m_InertialTensor._33 = 0.f;
		break;
	case 00:
	{
		m_InertialTensor._11 = CalculateCubeRigidBod(y,z,mass);
		m_InertialTensor._12 = 0.f;
		m_InertialTensor._13 = 0.f;
		m_InertialTensor._21 = 0.f;
		m_InertialTensor._22 = CalculateCubeRigidBod(x, z, mass);
		m_InertialTensor._23 = 0.f;
		m_InertialTensor._31 = 0.f;
		m_InertialTensor._32 = 0.f;
		m_InertialTensor._33 = CalculateCubeRigidBod(x, y, mass);;
		break;
	}
	case 01:
	{
		m_InertialTensor._11 = 0.f;
		m_InertialTensor._12 = 0.f;
		m_InertialTensor._13 = 0.f;
		m_InertialTensor._21 = 0.f;
		m_InertialTensor._22 = 0.f;
		m_InertialTensor._23 = 0.f;
		m_InertialTensor._31 = 0.f;
		m_InertialTensor._32 = 0.f;
		m_InertialTensor._33 = 0.f;
		break;
	}
	}
}


void RigidBody::AngularVelocity(float deltaTime)
{
	XMStoreFloat3x3(&m_AngularVelocity, XMLoadFloat3x3(&m_AngularVelocity) * deltaTime);
}
void RigidBody::AngularAcceleration(Vector3D* RelPos, Vector3D* ForceVector)
{
	XMStoreFloat3x3(&m_AngularVelocity, XMMatrixInverse(&XMMatrixDeterminant(XMLoadFloat3x3(&m_InertialTensor)),XMLoadFloat3x3(&m_InertialTensor)));
	XMVector3Transform(TorqueVect(RelPos, ForceVector).ConvertToXMvector3(), XMLoadFloat3x3(&m_AngularVelocity));
}
void RigidBody::AngularDampaning(float deltaTime)
{
	XMStoreFloat3x3(&m_AngularVelocity, XMLoadFloat3x3(&m_AngularVelocity) * pow(m_ADamping, deltaTime));
}

void RigidBody::UpdateOrientation(float deltaTime, Vector3D* pos, XMFLOAT4X4* world)
{
	XMVECTOR Sca, Rot, trans;
	XMMatrixDecompose(&Sca, &Rot, &trans, XMLoadFloat3x3(&m_AngularVelocity));
	m_Orientation.addScaledVector(Vector3D(Rot), deltaTime);
	m_Orientation.normalise();
	CalculateTransformMatrixRowMajor(XMLoadFloat4x4(world), *pos, m_Orientation);
}