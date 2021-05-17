#pragma once
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glfw/glfw3.h"

#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyVector.h"
#include "cyGL.h"

#include "Light.cpp"
#include "Camera.cpp"

using namespace cy;

class Object {
public:
	Matrix4f modelMat;
	GLSLProgram* program;
	GLuint vao, vbo, vnbo;
	TriMesh mesh;
	Vec3f position;
	Vec3f color;
	std::vector<Vec3f> worldV;

	Object() {}

	Object(const char* modelFn, Vec3f pos, Vec3f diffuseColor) {
		modelMat.SetTranslation(pos);
		std::vector<Vec3f> v;
		std::vector<Vec3f> vn;
		mesh.LoadFromFileObj(modelFn, false);

		for (int i = 0; i < mesh.NF(); i++) {
			for (int t = 0; t < 3; t++) {
				v.push_back(mesh.V(mesh.F(i).v[t]));
				Vec3f newVec = mesh.V(mesh.F(i).v[t]) + pos;
				//std::cout << newVec.x << " " << newVec.y << " " << newVec.z << std::endl;
				worldV.push_back(newVec);
				
				vn.push_back(mesh.VN(mesh.FN(i).v[t]));
			}
		}
		//std::cout << std::endl;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vec3f), &v[0], GL_STATIC_DRAW);

		glGenBuffers(1, &vnbo);
		glBindBuffer(GL_ARRAY_BUFFER, vnbo);
		glBufferData(GL_ARRAY_BUFFER, vn.size() * sizeof(cy::Vec3f), &vn[0], GL_STATIC_DRAW);

		color = diffuseColor;
		position = pos;
	}

	void draw(Matrix4f* V, Matrix4f* P, GLSLProgram* program,
		Vec3f* lightPos, Vec3f* camPos, bool isClipping=false) { 

		glUseProgram(program->GetID());

		modelMat.SetTranslationComponent(position);

		Matrix4f MVP = *P * *V * modelMat;
		glUniformMatrix4fv(glGetUniformLocation(program->GetID(), "MVP"), 1, GL_FALSE, &MVP[0]);
		glUniformMatrix4fv(glGetUniformLocation(program->GetID(), "modelMat"), 1, GL_FALSE, &modelMat[0]);


		if (isClipping && camPos->x > 0) {
			glUniform1i(glGetUniformLocation(program->GetID(), "isClipping"), 1);
		}
		else if (isClipping && camPos->x <= 0) {
			glUniform1i(glGetUniformLocation(program->GetID(), "isClipping"), 2);
		}
		else if (!isClipping) {
			glUniform1i(glGetUniformLocation(program->GetID(), "isClipping"), 0);
		}

		Matrix3f MVit = (modelMat).GetSubMatrix3().GetInverse().GetTranspose();

		glUniformMatrix3fv(glGetUniformLocation(program->GetID(), "MVit"), 1, GL_FALSE, &MVit[0]);

		glUniform3f(glGetUniformLocation(program->GetID(), "diffuseColor"), color[0], color[1], color[2]);
		//std::cout << "lightpos: " << lightPos->x << " " << lightPos->y << " " << lightPos->z << std::endl;

		glUniform3f(glGetUniformLocation(program->GetID(), "lightPos"), lightPos->x, lightPos->y, lightPos->z);

		glBindVertexArray(vao);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vnbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	std::vector<Vec3f> getVertices() {
		return worldV;
	}
};