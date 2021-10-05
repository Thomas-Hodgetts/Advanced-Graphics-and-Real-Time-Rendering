#pragma once
#include "ImaginaryObject.h"
#include "Transform.h"

//Framework's camera class. Child of type imaginary object and so can be used in flotillas
class Camera : public ImaginaryObject 
{
	private:
		// Private attributes to store the camera position and view 
		// volume

		//Position
		XMFLOAT4 m_Eye;
		//Focus
		XMFLOAT4 m_At;
		//Shows which way is up
		XMFLOAT4 m_Up;
		//Shows Which way is right
		XMFLOAT4 m_Right;

		FLOAT m_WindowWidth;
		FLOAT m_WindowHeight;
		FLOAT m_NearDepth;
		FLOAT m_FarDepth;

		// attributes to hold the view and projection matrices which 
		// will be passed to the shader

		XMFLOAT4X4 m_View;
		XMFLOAT4X4 m_Projection;
		float Sensitivity = 110.5;


	public:


		Camera(XMFLOAT4 position, XMFLOAT4 at, XMFLOAT4 up, FLOAT
			windowWidth, FLOAT windowHeight, FLOAT
			nearDepth, FLOAT farDepth);

		//to stop error C2512
		Camera() = default;
		~Camera();

		//SET//

		void SetEye(XMFLOAT4 pos) { m_Eye = pos; };
		void SetAt(XMFLOAT4 At) { m_At = At; };
		void SetUp(XMFLOAT4 Up) { m_Up = Up ;};
		void SetRight(XMFLOAT4 Right) { m_Right = Right ;};
		void SetLookTo(XMVECTOR Forwardvector);


		//GET//

		XMVECTOR GetEye() { return XMLoadFloat4(&m_Eye); };
		XMVECTOR GetAt() { return XMLoadFloat4(&m_At); };
		XMVECTOR GetUp() { return XMLoadFloat4(&m_Up); };
		XMVECTOR GetRight() { return XMLoadFloat4(&m_Right); };

		//Movement//

		//Takes the mouse input and makes it useful to the class
		void ManageMouseMovement(MouseData MD);

		Transform m_Transform;

		//Moves the Camera along the Z vector
		void Walk(float Direction);
		//Moves the Camera along the Z vector
		void Strafe(float Direction);
		//Rotates the Camera on the X axis
		void Pitch(float Angle);
		//Rotates the Camera on the Y axis
		void Yaw(float Angle);

		//Lens Effects//

		//To be fixed or removed
		void Zoom();
		void Unzoom();


		//Updates the Porjection and view of the object
		void Update();

		//Returns a struct Camera data.
		CameraData ReturnCamData();

		//Returns the View and Projection matrix and a struct
		CameraBufferData ReturnViewPlusProjection();

		//Here to reset Camera if the window is resized. must be called manually
		void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	};


