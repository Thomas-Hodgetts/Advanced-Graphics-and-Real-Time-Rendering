#pragma once
#include "Loader.h"
#include "Structures.h"



class Object
{
public: 

	//Basic Update function on objects
	virtual void Update();
	//Basic draw function on objects
	void Draw(DrawObjectsStruct DOS);
	//TBD
	virtual void ReturnObjectType();
	//Not used by default Physcial Object. Defined by user
	virtual void SetSubResources();
	//Denotes what type of object it is
	ObjectType m_ObjType;
protected:
	Loader c_SystemLoad;

};

