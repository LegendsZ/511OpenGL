#include "Camera.h"

Camera* Camera::camera = nullptr;

void Camera::makeCamera(VECTOR3D* eye, VECTOR3D* center, VECTOR3D* up) {
	if (!camera) {
		camera = new Camera(eye, center, up);
	}
}