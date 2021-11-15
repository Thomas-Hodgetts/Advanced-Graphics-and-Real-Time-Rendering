#pragma once
#include "Vector3D.h"
#include "Structures.h"
#include "Quaternion.h"
#include <math.h>  
class RigidBody
{
public:

	enum RigidBodyShape
	{
		Square,
		Sphere
	};


	//Caculates a Torque Vector and returns it
	Vector3D TorqueVect(Vector3D* RelPos, Vector3D* ForceVector);
	//Calculates a Torque Vector
	void TorqueVect(Vector3D* RelPos);
	//Calculates angular acceleration
	void AngularAcceleration(Vector3D* RelPos, Vector3D* ForceVector);
	//Dampens the Angular Acceleration
	void AngularDampaning(float deltaTime);
	//Calculates Angular Velocity
	void AngularVelocity(float deltaTime);
	//Uses the quaternion class to update the orientation of rigid body
	void UpdateOrientation(float deltaTime, Vector3D* pos, XMFLOAT4X4* world);
	//
	void SetRigidType(RigidBodyShape r, float x, float y, float z, float mass) { SetUpTensor(r, x, y, z, mass); };


protected:

private:

	void SetUpTensor(RigidBodyShape type, float x, float y, float z, float mass);

	float CalculateCubeRigidBod(float x, float y, float mass);
	float CalculateSphereRigidBod(float mass);


	XMFLOAT3X3 m_InertialTensor;
	float m_ADamping = 0.99;
	XMFLOAT3X3 m_AngularVelocity;
	Vector3D m_ForceVector;
	Quaternion m_Orientation;

};

