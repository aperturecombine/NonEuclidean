#pragma once


#include "cyMatrix.h"
#include <algorithm>
#include <iostream>
#include <algorithm>
#include <vector>

///#include "Object.cpp"

using namespace cy;

# define M_PI 3.1415926


class Camera {
public:
	float yaw = 0.0f;
	float pitch = 0.0f;
	Vec3f forwardDir;// = Vec3f(0, 0, 0);
	Vec3f cameraUp;
	Vec3f camPos;
	Vec3f prevPos;
	float camSpeed = 0.09f;

	Matrix4f viewMat;
	Matrix4f perspectiveMat;

	bool movingF, movingB, movingL, movingR, movingU, movingD;

	bool hasMoved() {
		float threshold = 0.0001f;
		if (std::abs(prevPos.x - camPos.x) > threshold ||
			std::abs(prevPos.y - camPos.y) > threshold ||
			std::abs(prevPos.z - camPos.z) > threshold)
			return true;
		return false;
	}

	void setupCameras(Vec3f pos) {
		camPos = pos;
		viewMat.SetIdentity();
		perspectiveMat.SetPerspective(0.75f, 4.0f / 3.0f, 0.1f, 400.0f);
		forwardDir = Vec3f(0, 0, 1);
		cameraUp = Vec3f(0, 1, 0);
		update();
	}

	void rotate(float dTheta, float dPhi) {
		yaw += dTheta;
		pitch += dPhi;

		if (pitch <= -M_PI/2+0.1f)
			pitch = -M_PI/2+0.1f;
		if (pitch >= M_PI/2-0.1f)
			pitch = M_PI/2-0.1f;
	}

	void move() {
		prevPos = Vec3f(camPos.x, camPos.y, camPos.z);

		if (movingF)
			camPos += camSpeed * forwardDir;
		if (movingB)
			camPos -= camSpeed * forwardDir;
		if (movingL)
			camPos -= camSpeed * forwardDir.Cross(cameraUp);
		if (movingR)
			camPos += camSpeed * forwardDir.Cross(cameraUp);
		if (movingU)
			camPos += camSpeed * cameraUp;
		if (movingD)
			camPos -= camSpeed * cameraUp;

		viewMat.SetView(camPos, camPos + forwardDir, Vec3f(0, 1, 0));
	}

	void update() {
		forwardDir.x = cos(yaw) * cos(pitch);
		forwardDir.y = sin(pitch);
		forwardDir.z = sin(yaw) * cos(pitch);
		//	portal_intersection();

		viewMat.SetView(camPos, camPos + forwardDir, Vec3f(0, 1, 0));
	}

	bool handleCollisions(std::vector<Vec3f> v) {
		if (hasMoved()) {
			for (int i = 0; i < 2; i++) {
				Vec3f p0, p1, p2;
				p2 =  v[i*3 + 0];
				p1 =  v[i*3 + 1];
				p0 =  v[i*3 + 2];
				
				Vec3f tuv =
					Matrix3f(
						prevPos - camPos,
						Vec3f(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z),
						Vec3f(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z)
						).GetInverse()
					* Vec3f(prevPos.x - p0.x, prevPos.y - p0.y, prevPos.z - p0.z);
				float t = tuv.x, u = tuv.y, v = tuv.z;

				float threshold = 0.0001;
				if (t >= -threshold && t < 1+threshold) {
					if (u >= -threshold && u <= 1+threshold && v >= -threshold
						&& v <= 1+threshold && (u + v) <= 1+threshold) {
						return true;
					}
				}
			}
		}
		return false;
	}

	Matrix4f getViewMat() {
		return viewMat;
	}

	Matrix4f getPerspectiveMat() {
		return perspectiveMat;
	}
};