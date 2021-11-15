#pragma once
#include "GameObject.h"
#include "PhysicalObject.h"
#include "ImaginaryObject.h"
#include "ThreadVector.h"


//A flotilla is a group of objects with similar resource needs and are used to be batch rendered or just stored.
class Flotilla
{
public:
	Flotilla();
	~Flotilla();

	//Adds an object to the flotilla
	void AddObject(Object* newObject);
	//Clears the entire flotilla (NOT FUNCTIONAL)
	void ClearObjects();
	//Draws all the objects in the flotilla.
	void Draw(DrawObjectsStruct DOS);
	//Calls the update function of all of the objects in a flotilla leader
	void Update();
	//Returns a specified object
	Object* ReturnObject(int Placement);
	//Returns the Size of the local vector
	int ReturnVectorSize();
	//For the Flotilla Draw Function
	bool Drawable = true;
	//For the Flotilla Update Function. ONLY APPLY IF NO DATA IS PASSED THROUGH
	bool AutoUpdate = true;
	//Name of the flotilla
	std::string Name;

private:

	std::vector<Object*> m_Objects;

};


class ThreadFlotilla
{
public:
	ThreadFlotilla();
	~ThreadFlotilla();

	//Adds an object to the flotilla
	void AddObject(Object* newObject);
	//Clears the entire flotilla (NOT FUNCTIONAL)
	void ClearObjects();
	//Draws all the objects in the flotilla.
	void Draw(ID3D11DeviceContext* IC, ID3D11Buffer* _pCB, ConstantBuffer cb);
	//Calls the update function of all of the objects in a flotilla leader
	void Update();
	//Returns a specified object
	Object* ReturnObject(int Placement);
	//Returns the Size of the local vector
	int ReturnVectorSize();
	//For the Flotilla Draw Function
	bool Drawable = true;
	//For the Flotilla Update Function. ONLY APPLY IF NO DATA IS PASSED THROUGH
	bool AutoUpdate = true;
	//Name of the flotilla
	std::string Name;


private:

	ThreadVector<Object*> m_Objects;

};
