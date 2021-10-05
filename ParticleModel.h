#pragma once
#include "RigidBody.h"
#include "Transform.h"
#include <Vector>
#include <cstdlib>


enum TraverseMode
{	
	ConstVel = 0,
	ConstAcc
};

class ParticleModel
{
};


//Base Particle. Relies on Vector3D class & Rigidbody class
class Particle : RigidBody
{
public:
	//Default Constructor. nothing is set
	Particle();
	//Constructor. Takes Vector3D class for velocity and position.
	Particle(std::string type, Vector3D vel, float energy, float acceleration, Geometry geometry, Colour colour);
	//Constructor. Takes XMFLOAT3 class for velocity and position.
	Particle(std::string type, XMFLOAT3 vel, float energy, float acceleration, Geometry geometry, Colour colour);

	//Defualt Constructor.
	~Particle();
	

	virtual void UpdateData(Transform* T, Transform* PT);
	//Update function for 
	virtual void Update(float deltaTime);
	//Draw method for particles
	virtual void Draw(ID3D11DeviceContext* device);

	//Set

	//Sets the position of the particle.
	void SetPos(Vector3D pos) { m_Transform->SetPos(pos); };
	//Sets the position of the particle.
	void SetPos(XMFLOAT3 pos) { m_Transform->SetPos(pos); };
	//Adds forces to the velocity of the particle.
	void SetVel(Vector3D vel) { m_Velocity += vel; };
	//Adds forces to the velocity of the particle.
	void SetVel(XMFLOAT3 vel) { m_Velocity += vel; };
	//Resets the velocity for any objects without friction 
	void ResetVel() { m_Velocity.Vector3D_Default(); };
	//Sets the Type of particle name;
	void SetType(std::string type) { m_Type = type; };
	//Sets the relatives of the particle
	void SetRelatives(Particle* relatives[]) { /*Complete*/ };
	//Sets the geomerty
	void SetGeometry(Geometry geo) { m_Geometry = geo; };
	//Sets the energy
	void SetEnergy(float value) { m_Energy = value; };
	//Sets the texture
	void SetTex(ID3D11ShaderResourceView* Tex) { m_textureRV = Tex; };
	//Sets the Forces
	//void AddForces(Forces* forces) { m_Forces.push_back(forces); };
	//Sets Unlimited energy
	void SetUnlimitedEnergy(bool Choice) { m_Energy = Choice; };
	//Sets the Acceleration of the particle
	void SetAcceleration(float value) { m_Acceleration = value; };
	//Set Traverse mode. 0 is constant Velocity, 1 is constant Acceleration
	void SetTraverseMode(int mode) { m_TraverseSetting = (TraverseMode)mode; };
	//Sets the mass of the object
	void SetMass(float mass) { m_Mass = mass; };
	//Sets gravity strength
	void SetGravity(float gravstrength) { m_GravityStrength = gravstrength; };
	//Sets gravity on or off
	void SetGravityState(bool Choice) { m_IsGravActive = Choice; };
	//Sets Drag on and off
	void SetDragState(bool Choice) { m_IsDragActive = Choice; };
	//Sets friction on and off
	void SetFrictionState(bool Choice) { m_IsFricActive = Choice; };
	//Sets the drag coefficent
	void SetDragCoefficient(float value) { m_DragCoefficient = value; };
	//Sets the point of gravity
	void SetCentreOfGravity(Vector3D value) { m_CentreOfGravity = value; };
	//Sets the point of gravity
	void SetCentreOfGravity(XMFLOAT3 value) { m_CentreOfGravity = value; };

	//Return
	Particle* ReturnRelatives() { return m_Relatives[0]; };
	//Return the position of the particle.
	Vector3D ReturnPos() { return m_Transform->ReturnPos(); };
	//Return the Velocity of the particle.
	Vector3D ReturnVel() { return m_Velocity; };
	//Sets the Type of particle name;
	std::string ReturnType() { return m_Type; };
	//Returns particles energy
	float ReturnEnergy() { return m_Energy; };
	//Returns Geometry
	Geometry ReturnGeometry() { return m_Geometry; };
	//Returns the world position.
	XMMATRIX ReturnWorldMatrix() const { return XMLoadFloat4x4(&m_World); }
	//Return the Acceleration value
	float ReturnAcceleration() { return m_Acceleration; };
	//Returns the mass of the object
	float ReturnMass() { return m_Mass; };
	//returns the gravity's strength
	float ReturnGravityStrength() { return m_GravityStrength; };

	//Check function

	//Checks to see if the object moves using constant velocity. Can be changed using SetTraverseMode()
	bool UsingConstVel() const { return m_TraverseSetting == 0 ? true : false; };
	//Checks to see if the object moves using constant Acceleration. Can be changed using SetTraverseMode()
	bool UsingConstAcc() const { return m_TraverseSetting == 1 ? true : false; };
	//Returns the result of if gravity is active
	bool IsGavityActive() { return m_IsGravActive; };
	//Returns the result of if friction is active
	bool IsFrictionActive() { return m_IsFricActive; };
	//Returns the result of if drag is active
	bool IsDragActive() { return m_IsDragActive; };

	//Movement Functions


	//Default Movement Function		
	void MoveConstVelocity(float deltaTime);
	void MoveConstAcceleration(float deltaTime);

	//Z axis

	//Moves the Game object forward. Distance is determined by float value. Temp Recommended Values being 0.00002
	void MoveForward(float value);

	//Moves the Game object backwards. Distance is determined by float value. Temp Recommended Values being 0.00002
	void MoveBackwards(float value);

	//X axis

	//Moves the Game object left. Distance is determined by float value. Temp Recommended Values being 0.00002
	void MoveLeft(float value);

	//Moves the Game object right. Distance is determined by float value. Temp Recommended Values being 0.00002
	void MoveRight(float value);

	//Y axis

	//Moves the Game object Vertically. Distance is determined by float value. 
	void MoveVertically(float value);


	//RotateFunctions

	//temp func
	void Rotate(float x);


	//Math
	void ApplyVelocity();
	//Resets velocity value to 0
	void ResetVelocity();
	//Updates the net force of the particle
	void UpdateNetForce(float deltaTime);
	//Updates the avalue 
	void UpdateAcceleration();
	//Calculates Gravity
	void CalculateGravity();
	//Calculates Friction
	void CalculateFriction();
	//Calculates Drag
	void CalculateDrag(float deltaTime);


protected:

	//Forces
	void LaminarFlow(float deltaTime);
	void TurbFlow(float deltaTime);

private:
	//A string that stores the type of particle
	std::string m_Type;
	//Old particle position
	Vector3D m_OldPosition;
	//Old particle velocity 
	Vector3D m_OldVelocity;
	//Velocity of the particle
	Vector3D m_Velocity;
	//Total amount of forces in one variable
	Vector3D m_Forces;
	//Centre of gravity
	Vector3D m_CentreOfGravity;
	//3D acceleration
	Vector3D m_3DAcceleration;
	//Acceleration of the Particle
	float m_Acceleration;
	//Particles World
	XMFLOAT4X4 m_World;
	//Parent's Transform
	Transform* m_ParentTransform;
	//Particle's transform
	Transform* m_Transform;
	//Particles Energy
	float m_Energy;
	//Texture applied to the particle
	ID3D11ShaderResourceView* m_textureRV;
	//Local Geomerty
	Geometry m_Geometry;
	//Local Colour
	Colour m_Colour;
	//Controls unlimited energy
	bool m_UnlimitedEnergy;
	//Controls the traverse mode
	TraverseMode m_TraverseSetting;
	//A list of particles that this particle might be used to form a mesh with, e.g. nearby particles
	std::vector<Particle*> m_Relatives;
	//Mass of the Particle
	float m_Mass;
	//Gravities Strength
	float m_GravityStrength = -9.81;
	//Controls Gravity
	bool m_IsGravActive;
	//Controls Friction
	bool m_IsFricActive;
	//Controls Drag
	bool m_IsDragActive;
	//Drag coefficent
	float m_DragCoefficient = 0.3;
	//The density used 
	float m_DragDensity;
	//The friction coefficent
	float m_FricCofficient = 0.3;
	//Objects Rigidbody
	RigidBody* m_RigidBody;

};

