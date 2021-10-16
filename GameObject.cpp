#include "GameObject.h"

GameObject::GameObject(string type, Apperance* apperance, Transform* transform) : m_Apperance(apperance), _type(type), m_Transform(transform)
{
	m_Particle = new Particle(_type, Vector3D(), 0, 2, m_Apperance->ReturnGeo(), Colour());
	m_Particle->UpdateData(m_Transform, nullptr);
	m_Particle->SetUnlimitedEnergy(true);
	m_ObjType = ObjectType::GameObj;
}

GameObject::~GameObject()
{
	delete m_Transform;
	delete m_Particle;
	delete m_Apperance;
	m_Transform = nullptr;
	m_Particle = nullptr;
	m_Apperance = nullptr;

}

void GameObject::Draw(DrawObjectsStruct DOS)
{
	m_Apperance->Draw(DOS); 
}

void GameObject::Update(float t)
{
	m_Particle->Update(t);
}
