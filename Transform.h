#pragma once
#include "Vector3D.h"

//Contains position information and basic controls
class Transform
{
public:
	//Constructors
	Transform();
	Transform(Vector3D _position, Vector3D _rotation, Vector3D _scale);
	Transform(XMFLOAT3 _position, XMFLOAT3 _rotation, XMFLOAT3 _scale);

	~Transform();


	//Z axis

	//Moves the Game object forward.
	void MoveForward();
	//Moves the Game object forward. Distance is determined by float value.
	void MoveForward(float value);

	//Moves the Game object backwards.
	void MoveBackwards();
	//Moves the Game object backwards. Distance is determined by float value.
	void MoveBackwards(float value);

	//X axis

	//Moves the Game object left.
	void MoveLeft();
	//Moves the Game object left. Distance is determined by float value.
	void MoveLeft(float value);

	//Moves the Game object right.
	void MoveRight();
	//Moves the Game object right. Distance is determined by float value.
	void MoveRight(float value);


	//Set

	//Set Postion Vector
	void SetPos(Vector3D vec) {m_Position = vec ;};
	//Set Postion Vector. This takes XMFLOAT 3
	void SetPos(XMFLOAT3 vec) {m_Position = vec ;};

	//Set Rotation Vector
	void SetRot(Vector3D vec) { m_Rotation = vec; };
	//Set Rotation Vector. This takes XMFLOAT 3
	void SetRot(XMFLOAT3 vec) { m_Rotation = vec; };
	//Set Rotation Vector. This takes XMVector3
	void SetRot(XMVECTOR vec) { m_Rotation = vec; };

	//Set Scale Vector
	void SetSca(Vector3D vec) { m_Scale = vec; };
	//Set Scale Vector. This takes XMFLOAT 3
	void SetSca(XMFLOAT3 vec) { m_Scale = vec; };

	//Return

	//Returns the transforms position
	Vector3D ReturnPos() { return m_Position; };
	//Returns the transforms rotation
	Vector3D ReturnRot() { return m_Rotation; };
	//Returns the transforms scale
	Vector3D ReturnSca() { return m_Scale; };

private:
	Vector3D m_Position;
	Vector3D m_Rotation;
	Vector3D m_Scale;
};

