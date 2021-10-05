#pragma once
#include "Loader.h"
#include "Structures.h"



class Object
{
public: 

	//Basic Update function on objects
	virtual void Update();
	//Basic draw function on objects
	virtual void Draw(ID3D11DeviceContext* IC, ID3D11Buffer* _pCB, ConstantBuffer cb);
	//TBD
	virtual void ReturnObjectType();
	//Not used by default Physcial Object. Defined by user
	virtual void SetSubResources();

protected:

	Loader c_SystemLoad;

};

