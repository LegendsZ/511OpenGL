#pragma once
#include <vector>
#include <gl/glut.h>
#include "VECTOR3D.h"
#include <iostream>

class Camera {
public:
	static Camera* camera;
	VECTOR3D* eye;
	VECTOR3D* center;
	VECTOR3D* up;

	void look() {
		//gluLookAt(eye->x, eye->y, eye->z, center->x + eye->x, center->y + eye->y, center->z + eye->z, up->x, up->y, up->z);
		gluLookAt(eye->x, eye->y, eye->z, center->x, center->y, center->z, up->x, up->y, up->z);
	}

	static void makeCamera(VECTOR3D* eye = new VECTOR3D(0.0f, 0.0f, 0.0f), VECTOR3D* center = new VECTOR3D(0.0f, 0.0f, -1.0f), VECTOR3D* up = new VECTOR3D(0.0f, 1.0f, 0.0f));

private:
	Camera(VECTOR3D* eye, VECTOR3D* center, VECTOR3D* up) :
		eye(eye), center(center), up(up) {}
};

