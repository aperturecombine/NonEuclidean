#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glfw/glfw3.h"

#include "Camera.cpp"

#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"
#include "lodepng.h"
#include "Object.cpp"
#include "Light.cpp"
#include <iostream>
#include <fstream>
#include <vector> 
#include <chrono>

using namespace cy;

# define M_PI 3.1415926
# define T_PI 6.2831853

Camera cam;

// Mouse stuff
Vec2f lastPosL;
Vec2f lastPosR;

int WIDTH = 1024;
int HEIGHT = 768;

GLSLProgram program, simpleShader;

std::vector<Object> drawables;
std::vector<std::pair<Object, Object>> portals;

bool movingF, movingB, movingL, movingR, movingU, movingD;

bool allClipping;

Vec3f lightPos;

float sign(float a) {
	if (a > 0.0f)
		return 1.0f;
	if (a < 0.0f)
		return -1.0f;
	return 0.0f;
}

void drawPortals(Object p1, Object p2) {
	Matrix4f V = cam.getViewMat();
	Matrix4f P = cam.getPerspectiveMat();

	Vec3f camPos2 = cam.camPos + (p2.position - p1.position);

	Matrix4f viewMat;
	viewMat.SetView(camPos2, camPos2 + cam.forwardDir, Vec3f(0, 1, 0));

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);

	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

	glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);

	glStencilMask(0xFF);

	p1.draw(&V, &P, &program, &lightPos, &cam.camPos, true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glStencilMask(0x00);
	glStencilFunc(GL_EQUAL, 1, 0xFF);

	for (int i = 0; i < drawables.size(); i++) {
		if(allClipping)
			drawables[i].draw(&viewMat, &P, &program, &lightPos, &cam.camPos);
		else
			drawables[i].draw(&viewMat, &P, &program, &lightPos, &cam.camPos, true);
	}

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFF);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

	p1.draw(&V, &P, &program, &lightPos, &cam.camPos);
}

void update() {
	cam.move();

	for (int i = 0; i < portals.size(); i++) {
		Object p1 = portals[i].first;
		Object p2 = portals[i].second;
		if (cam.handleCollisions(p1.getVertices())) {
			cam.camPos += (p2.position - p1.position);
		}
		else if (cam.handleCollisions(p2.getVertices())) {
			cam.camPos += (p1.position - p2.position);
		}
	}
	cam.update();

	glutPostRedisplay();
}

void mouseMoveFunc(int x, int y) {
	glutWarpPointer(WIDTH / 2, HEIGHT / 2);
	float dPhi = -(y - HEIGHT / 2) / 200.0f;
	float dTheta = (x - WIDTH / 2) / 200.0f;

	lastPosL.x = x;
	lastPosL.y = y;

	cam.rotate(dTheta, dPhi);
}

void initScene1() {
	drawables.clear();
	portals.clear();

	drawables.push_back(Object("base.obj", Vec3f(0, 0, -10), Vec3f(0.6, 0.5, 0.5)));
	drawables.push_back(Object("base.obj", Vec3f(0, 0, 10), Vec3f(0.5, 0.5, 0.6)));
	drawables.push_back(Object("teapot.obj", Vec3f(-5, 0, -9), Vec3f(0.7, 0.7, 0.5)));
	drawables.push_back(Object("cube.obj", Vec3f(-3, 1.01, 9), Vec3f(0.5, 0.7, 0.5)));
	drawables.push_back(Object("cube.obj", Vec3f(3, 1.01, -9), Vec3f(0.9, 0.9, 0.9)));
	drawables.push_back(Object("cube.obj", Vec3f(3, 1.01, 9), Vec3f(0.1, 0.1, 0.2)));
	drawables.push_back(Object("frame.obj", Vec3f(0, 0, 10), Vec3f(0.7, 0.7, 0.3)));
	drawables.push_back(Object("frame.obj", Vec3f(0, 0, -10), Vec3f(0.7, 0.7, 0.3)));

	Object portal1 = Object("portal.obj", Vec3f(0, 0, 10), Vec3f(0.7, 0.7, 0.3));
	Object portal2 = Object("portal.obj", Vec3f(0, 0, -10), Vec3f(0.7, 0.7, 0.3));

	portals.push_back(std::pair<Object, Object>(portal1, portal2));
}


void renderScene() {
	glClearColor(0.24f, 0.72f, 0.96f, 1.0f);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0xFF);


	for (int i = 0; i < portals.size(); i++) {
		drawPortals(portals[i].first, portals[i].second);
		drawPortals(portals[i].second, portals[i].first);
	}

	glDisable(GL_STENCIL_TEST);
	glStencilMask(0x00);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glDepthFunc(GL_ALWAYS);

	glClear(GL_DEPTH_BUFFER_BIT);

	auto viewMat = cam.getViewMat();
	auto persMat = cam.getPerspectiveMat();

	for (int i = 0; i < portals.size(); i++) {
		portals[i].first.draw(&viewMat, &persMat, &program, &lightPos, &cam.camPos);
		portals[i].second.draw(&viewMat, &persMat, &program, &lightPos, &cam.camPos);
	}

	glDepthFunc(GL_LESS);

	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);

	glStencilFunc(GL_LEQUAL, 0, 0xFF);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < drawables.size(); i++)
		drawables[i].draw(&viewMat, &persMat, &program, &lightPos, &cam.camPos);
	glutSwapBuffers();
}

void handleKeypress(unsigned char key, int x, int y) {
	if (key == 27)
		glutLeaveMainLoop();

	if (key == 'w')
		cam.movingF = true;
	if (key == 's')
		cam.movingB = true;
	if (key == 'a')
		cam.movingL = true;
	if (key == 'd')
		cam.movingR = true;
	if (key == ' ')
		cam.movingU = true;
	if (key == 'z')
		cam.movingD = true;

	if (key == '1') {
		initScene1();
	}

	if (key == 'p')
		allClipping = !allClipping;
}
void handleKeypressUp(unsigned char key, int x, int y) {
	if (key == 'w')
		cam.movingF = false;
	if (key == 's')
		cam.movingB = false;
	if (key == 'a')
		cam.movingL = false;
	if (key == 'd')
		cam.movingR = false;
	if (key == ' ')
		cam.movingU = false;
	if (key == 'z')
		cam.movingD = false;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("CS6610 Project 8 - Tesselations");

	glutDisplayFunc(renderScene);
	glutIdleFunc(update);
	glutKeyboardFunc(handleKeypress);
	glutKeyboardUpFunc(handleKeypressUp);
	glutPassiveMotionFunc(mouseMoveFunc);

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		return -1;

	glEnable(GL_CLIP_DISTANCE0);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	cam.setupCameras(Vec3f(5, 5, 0));

	lightPos = Vec3f(20, 20, -20);

	GLSLShader vertShader, fragShader;
	program.CreateProgram();
	vertShader.CompileFile("shaders/program.vert", GL_VERTEX_SHADER);
	fragShader.CompileFile("shaders/program.frag", GL_FRAGMENT_SHADER);
	program.AttachShader(vertShader);
	program.AttachShader(fragShader);
	program.Link();
	program.Bind();

	GLSLShader simpleVertShader, simpleFragShader;
	simpleShader.CreateProgram();
	simpleVertShader.CompileFile("shaders/shaderSingleColor.vert", GL_VERTEX_SHADER);
	simpleFragShader.CompileFile("shaders/shaderSingleColor.frag", GL_FRAGMENT_SHADER);
	simpleShader.AttachShader(simpleVertShader);
	simpleShader.AttachShader(simpleFragShader);
	simpleShader.Link();
	simpleShader.Bind();

	initScene1();

	glutMainLoop();
}