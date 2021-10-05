#pragma once
#include <string>
#include "Debug.h"
#include "Vector3D.h"
#include "Transform.h"
#include "ParticleModel.h"
#include "Structures.h"
#include "Apperance.h"
#include "Object.h"

using namespace DirectX;
using namespace std;



class GameObject : public Object
{
public:
	GameObject(string type, Apperance* apperance, Transform* transform);
	~GameObject();

	// Setters and Getters for position/rotation/scale. Majority of these are legacy functions for compatability


	//Legacy function. will simply call m_Transform->SetPos().
	void SetPosition(XMFLOAT3 position) { m_Transform->SetPos(position); }
	//Legacy function. will simply call m_Transform->SetPos().
	void SetPosition(float x, float y, float z) { m_Transform->SetPos(Vector3D(x, y, z)); }
	//Legacy function. will simply call m_Transform->ReturnPos().
	Vector3D GetPosition() { return m_Transform->ReturnPos(); }
	//Legacy function. will simply call m_Transform->SetSca().
	void SetScale(XMFLOAT3 scale) { m_Transform->SetSca(scale); }
	//Legacy function. will simply call m_Transform->SetSca().
	void SetScale(float x, float y, float z) { m_Transform->SetSca(Vector3D(x, y, z)); }
	//Legacy function. will simply call m_Transform->ReturnSca().
	Vector3D GetScale() { return m_Transform->ReturnSca(); }
	//Legacy function. will simply call m_Transform->SetRot().
	void SetRotation(XMFLOAT3 rotation) { m_Transform->SetRot(rotation); }
	//Legacy function. will simply call m_Transform->SetRot().
	void SetRotation(float x, float y, float z) { m_Transform->SetRot(Vector3D(x,y,z)); }
	//Legacy function. will simply call m_Transform->ReturnRot().
	Vector3D GetRotation() { return m_Transform->ReturnRot(); }
	//returns the name of the object.
	string GetType() const { return _type; }
	//Legacy function. will simply call m_Apperance->ReturnGeo().
	Geometry GetGeometryData() const { return m_Apperance->ReturnGeo(); }
	//Legacy function. will simply call m_Apperance->ReturnMat().
	Material GetMaterial() const { return m_Apperance->ReturnMat(); }
	//Legacy function. will simply call m_Apperance->SetTex().
	void SetTextureRV(ID3D11ShaderResourceView * textureRV) { m_Apperance->SetTex(textureRV); }
	//Legacy function. will simply call m_Apperance->ReturnTex().
	ID3D11ShaderResourceView * GetTextureRV() const { return m_Apperance->ReturnTex(); }
	//Checks if the object has a texture. returns true if so and false if not.
	bool HasTexture() const { return m_Apperance->ReturnTex() ? true : false; }
	//Update function for the game object
	void Update(float t);

	//Public Variables

	//Stores the data to do with the game object's positon
	Transform* m_Transform;
	//Stores Particle information as is responsble for its position
	Particle* m_Particle;
	//Stores and is responsible for drawing the visual data of the game object
	Apperance* m_Apperance;

private:

	string _type;
};

