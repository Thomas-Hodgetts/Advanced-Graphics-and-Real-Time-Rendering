#include "ParticleModel.h"

Particle::Particle()
{
	m_Type = "";
	m_TraverseSetting = (TraverseMode)0;
	m_Transform = new Transform();
	m_Velocity.Vector3D_Default();
	m_textureRV = nullptr;
	m_Acceleration = 1;
	m_3DAcceleration = Vector3D(1, 1, 1);
	m_IsGravActive = false;
	m_IsFricActive = true;
	m_IsDragActive = true;
	m_RigidBody = new RigidBody();
}

Particle::Particle(std::string type, Vector3D vel, float energy, float acceleration ,Geometry geometry, Colour colour) : m_Type(type), m_Velocity(vel), m_Energy(energy), m_Acceleration(acceleration) , m_3DAcceleration(acceleration, acceleration, acceleration) ,m_Geometry(geometry), m_Colour(colour)
{
	m_Mass = 1.f;
	m_TraverseSetting = (TraverseMode)0;
	m_textureRV = nullptr;
	m_IsGravActive = false;
	m_IsFricActive = true;
	m_IsDragActive = true;
	m_RigidBody = new RigidBody();
}

Particle::Particle(std::string type, XMFLOAT3 vel , float energy, float acceleration ,Geometry geometry, Colour colour) : m_Type(type), m_Velocity(vel), m_Energy(energy), m_Acceleration(acceleration), m_3DAcceleration(acceleration, acceleration, acceleration) ,m_Geometry(geometry), m_Colour(colour)
{
	m_Mass = 1.f;
	m_TraverseSetting = (TraverseMode)0;
	m_textureRV = nullptr;
	m_IsGravActive = false;
	m_IsFricActive = true;
	m_IsDragActive = true;
	m_RigidBody = new RigidBody();
}


Particle::~Particle()
{
	m_Type = "";
	m_Velocity.Vector3D_Default();
	for each (auto var in m_Relatives)
	{
		delete var;
	}
	m_Relatives.clear();
	delete m_textureRV;
	delete m_Transform;
	if (m_ParentTransform != nullptr)
	{
		delete m_ParentTransform;
	}
	m_textureRV = nullptr;
	m_Transform = nullptr;
	m_ParentTransform = nullptr;
	delete m_RigidBody;
}

void Particle::Draw(ID3D11DeviceContext* device)
{
	//device->IASetVertexBuffers(0, 1, &m_Geometry.vertexBuffer, &m_Geometry.vertexBufferStride, &m_Geometry.vertexBufferOffset);
	//device->IASetIndexBuffer(m_Geometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	//device->DrawIndexed(m_Geometry.numberOfIndices, 0, 0);
}

void Particle::Update(float deltaTime)
{

	// Calculate world matrix

	UpdateNetForce(deltaTime);
	UpdateAcceleration();

	switch (m_TraverseSetting)
	{
	default:
		{		
		ApplyVelocity();
		ResetVelocity(); 
		break;
		}
	case TraverseMode::ConstVel:
		{
			MoveConstVelocity(deltaTime);
			break;
		}
	case TraverseMode::ConstAcc:
		{
			MoveConstAcceleration(deltaTime);

		}
	}



	XMMATRIX scale = XMMatrixScaling(m_Transform->ReturnSca().ReturnX(), m_Transform->ReturnSca().ReturnY(), m_Transform->ReturnSca().ReturnZ());
	XMMATRIX rotation = XMMatrixRotationX(m_Transform->ReturnRot().ReturnX()) * XMMatrixRotationY(m_Transform->ReturnRot().ReturnY()) * XMMatrixRotationZ(m_Transform->ReturnRot().ReturnZ());
	XMMATRIX translation = XMMatrixTranslation(m_Transform->ReturnPos().ReturnX(), m_Transform->ReturnPos().ReturnY(), m_Transform->ReturnPos().ReturnZ());

	XMStoreFloat4x4(&m_World, scale * rotation * translation);

	if (m_ParentTransform != nullptr)
	{
		XMMATRIX scale = XMMatrixScaling(m_ParentTransform->ReturnSca().ReturnX(), m_ParentTransform->ReturnSca().ReturnY(), m_ParentTransform->ReturnSca().ReturnZ());
		XMMATRIX rotation = XMMatrixRotationX(m_ParentTransform->ReturnRot().ReturnX()) * XMMatrixRotationY(m_ParentTransform->ReturnRot().ReturnY()) * XMMatrixRotationZ(m_ParentTransform->ReturnRot().ReturnZ());
		XMMATRIX translation = XMMatrixTranslation(m_ParentTransform->ReturnPos().ReturnX(), m_ParentTransform->ReturnPos().ReturnY(), m_ParentTransform->ReturnPos().ReturnZ());
		XMMATRIX _parent = scale * rotation * translation;
		XMStoreFloat4x4(&m_World, this->ReturnWorldMatrix() * _parent);
	}

	m_RigidBody->UpdateOrientation(deltaTime, &m_Transform->ReturnPos(), &m_World);
}

void Particle::UpdateNetForce(float deltaTime)
{
	m_Forces = Vector3D();
	Vector3D GravitySource = m_Transform->ReturnPos();
	GravitySource.SetY(m_Transform->ReturnPos().ReturnY() - 1);
	CalculateGravity();
	CalculateFriction();
	CalculateDrag(deltaTime);
	m_OldVelocity = m_Velocity;
	m_Velocity += m_Forces;
}

void Particle::CalculateGravity()
{
	if (m_IsGravActive && m_Transform->ReturnPos().ReturnY() != 0)
	{
		Vector3D GravitySource = m_Transform->ReturnPos() + m_CentreOfGravity;
		if (m_CentreOfGravity == Vector3D())
		{
			GravitySource.SetY(m_Transform->ReturnPos().ReturnY() - 0.1);
		}
		Vector3D localDis = m_Transform->ReturnPos() > GravitySource ? m_Transform->ReturnPos() - GravitySource : GravitySource - m_Transform->ReturnPos();
		//Removes risk of dividing by 0
		float temp = 0.001;
		if (localDis.ReturnX() == 0)
			localDis.SetX(temp);
		if (localDis.ReturnY() == 0)
			localDis.SetY(temp);
		if (localDis.ReturnZ() == 0)
			localDis.SetZ(temp);
		Vector3D finalout = Vector3D(temp, m_Mass * m_GravityStrength, temp);
		m_Forces += finalout * (localDis * localDis);
		Debug d;
		d.OutputString("");
	}
}
void Particle::CalculateDrag(float deltaTime)
{
	LaminarFlow(deltaTime);
}

void Particle::LaminarFlow(float deltaTime)
{
	Vector3D dragForceMag;
	dragForceMag.SetX(m_Velocity.ReturnX() * m_DragCoefficient);
	dragForceMag.SetY(m_Velocity.ReturnY() * m_DragCoefficient);
	dragForceMag.SetZ(m_Velocity.ReturnZ() * m_DragCoefficient);
	m_Forces += dragForceMag;
}

void Particle::TurbFlow(float deltaTime)
{
	Vector3D dragForceMag;
	if (m_Velocity.ReturnX() < 3 || m_Velocity.ReturnY() < 3 || m_Velocity.ReturnZ() < 3)
	{
		if (m_Velocity.ReturnX() < 3)
		{
			dragForceMag.SetX(m_Velocity.ReturnX() * m_DragCoefficient);
		}
		if (m_Velocity.ReturnY() < 3)
		{
			dragForceMag.SetY(m_Velocity.ReturnY() * m_DragCoefficient);
		}
		if (m_Velocity.ReturnZ() < 3)
		{
			dragForceMag.SetZ(m_Velocity.ReturnZ() * m_DragCoefficient);
		}
	}
	if (m_Velocity.ReturnX() > 3 || m_Velocity.ReturnY() > 3 || m_Velocity.ReturnZ() > 3)
	{
		if (m_Velocity.ReturnX() > 3)
		{
			dragForceMag.SetX(0.5 * m_DragDensity * m_DragCoefficient * (m_Velocity.ReturnX() / deltaTime) * m_Velocity.ReturnX() * m_Velocity.ReturnX());
		}
		if (m_Velocity.ReturnY() > 3)
		{
			dragForceMag.SetY(0.5 * m_DragDensity * m_DragCoefficient * (m_Velocity.ReturnY() / deltaTime) * m_Velocity.ReturnY() * m_Velocity.ReturnY());
		}
		if (m_Velocity.ReturnZ() > 3)
		{
			dragForceMag.SetZ(0.5 * m_DragDensity * m_DragCoefficient * (m_Velocity.ReturnZ() / deltaTime) * m_Velocity.ReturnZ() * m_Velocity.ReturnZ());
		}
	}
	m_Forces += dragForceMag;
}

void Particle::CalculateFriction()
{
	if (m_IsFricActive && m_Velocity != Vector3D(0, 0, 0))
	{

		Vector3D fricMag;
		Vector3D forces = m_Forces;
		float dnwdForceMag;

		if (m_Velocity.ReturnMagnitude(m_Velocity) <= 0.05f)
		{
			m_Velocity.Vector3D_Default();
		}
		if (m_Transform->ReturnRot() != Vector3D(0.f, m_Transform->ReturnRot().ReturnY(), 0.f))
		{
			Vector3D tanForceMag;

			tanForceMag.SetX(forces.ReturnX() * m_Mass * XMScalarASin(m_Transform->ReturnRot().ReturnX()));
			tanForceMag.SetY(forces.ReturnY() * m_Mass * XMScalarASin(m_Transform->ReturnRot().ReturnY()));
			tanForceMag.SetZ(forces.ReturnZ() * m_Mass * XMScalarASin(m_Transform->ReturnRot().ReturnZ()));

			fricMag.SetX(forces.ReturnX() * m_Mass * forces.ReturnX() * XMScalarACos(m_Transform->ReturnRot().ReturnX()));
			fricMag.SetY(forces.ReturnY() * m_Mass * forces.ReturnY() * XMScalarACos(m_Transform->ReturnRot().ReturnY()));
			fricMag.SetZ(forces.ReturnZ() * m_Mass * forces.ReturnZ() * XMScalarACos(m_Transform->ReturnRot().ReturnZ()));
			dnwdForceMag = tanForceMag.ReturnY() - fricMag.ReturnY();
			m_Forces -= dnwdForceMag;

		}
		else if (!m_IsGravActive)
		{
			fricMag = m_Velocity * m_FricCofficient * m_Mass;
			m_Forces -= fricMag;
		}
		else
		{
			fricMag = m_Forces * m_FricCofficient * m_Mass;
			m_Forces -= fricMag;
		}

	}
}

void Particle::UpdateAcceleration()
{
	m_3DAcceleration = m_Velocity / m_Mass;
}

void Particle::UpdateData(Transform* T, Transform* PT)
{
	m_Transform = T;
	m_ParentTransform = PT;
}


void Particle::MoveConstAcceleration(float deltaTime)
{
	m_OldPosition = m_Transform->ReturnPos();
	m_Transform->SetPos(m_OldPosition + m_Velocity * m_3DAcceleration * (deltaTime + 0.5f * 0.0016f * 0.0016f));
	//m_Transform->SetPos(m_OldPosition + m_Velocity  * (deltaTime + 0.5f * m_Acceleration * 0.0016f * 0.0016f));
	m_OldVelocity = m_Velocity;
	if (m_Velocity.ReturnX() != 0)
	{
		m_Velocity.SetX(m_OldVelocity.ReturnX() + m_3DAcceleration.ReturnX() * deltaTime);
	}
	if (m_Velocity.ReturnY() != 0)
	{
		m_Velocity.SetY(m_OldVelocity.ReturnY() + m_3DAcceleration.ReturnY() * deltaTime);
	}
	if (m_Velocity.ReturnZ() != 0)
	{
		m_Velocity.SetZ(m_OldVelocity.ReturnZ() + m_3DAcceleration.ReturnZ() * deltaTime);
	}
}

void Particle::MoveConstVelocity(float deltaTime)
{

	m_OldPosition = m_Transform->ReturnPos();
	m_Transform->SetPos(m_OldPosition + m_Velocity * deltaTime);

}

void Particle::ApplyVelocity()
{
	m_Transform->SetPos(m_Transform->ReturnPos() += m_Velocity);
}

void Particle::ResetVelocity()
{
	m_Velocity.Vector3D_Default();
}

void Particle::MoveForward(float value)
{
	float z = m_Velocity.ReturnZ();
	m_Velocity.SetZ(z += value);
}

void Particle::MoveBackwards(float value)
{
	float z = m_Velocity.ReturnZ();
	m_Velocity.SetZ(z -= value);
}

void Particle::MoveRight(float value)
{
	float x = m_Velocity.ReturnX();
	m_Velocity.SetX(x += value);
}

void Particle::MoveLeft(float value)
{
	float x = m_Velocity.ReturnX();
	m_Velocity.SetX(x -= value);
}

void Particle::MoveVertically(float value)
{
	float y = m_Velocity.ReturnX();
	m_Velocity.SetX(y += value);
}

void Particle::Rotate(float x)
{

}