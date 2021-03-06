#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/type_ptr.hpp>
#include "vertices.h"
#include "patches.h"

#define BUFFER_OFFSET(offset)   ((GLvoid*) (offset))

using namespace glm;

const int NumTimesToSubdivide = 3;
const int NumTriangles = (NumTeapotPatches * 64);
const int NumVertices = (NumTriangles * 12);

vec4 points[NumVertices];

GLuint InitShader(const char* vShaderFile, const char* fShaderFile);
GLuint Projection;

enum { X = 0, Y = 1, Z = 2 };

int Index = 0;

void transpose(vec4 a[4][4])
{
	for (int i = 0; i < 4; i++) {
		for (int j = i; j < 4; j++) {
			vec4 t = a[i][j];
			a[i][j] = a[j][i];
			a[j][i] = t;
		}
	}
}

void divide_curve(vec4 c[4], vec4 r[4], vec4 l[4]) {

	int i;
	vec4 t;
	for (i = 0; i < 3; i++) {
		l[0][i] = c[0][i];
		r[3][i] = c[3][i];
		l[1][i] = (c[1][i] + c[0][i]) / 2;
		r[2][i] = (c[2][i] + c[3][i]) / 2;
		t[i] = (l[1][i] + r[2][i]) / 2;
		l[2][i] = (t[i] + l[1][i]) / 2;
		r[1][i] = (t[i] + r[2][i]) / 2;
		l[3][i] = r[0][i] = (l[2][i] + r[1][i]) / 2;
	}
	for (i = 0; i < 4; i++)
		l[i][3] = r[i][3] = 1.0;

}

void draw_patch(vec4 p[4][4]) {

	points[Index++] = p[0][0];
	points[Index++] = p[3][0];
	points[Index++] = p[3][3];
	points[Index++] = p[0][0];
	points[Index++] = p[3][3];
	points[Index++] = p[0][3];

}

void divide_patch(vec4 p[4][4], int n) {

	vec4 q[4][4], r[4][4], s[4][4], t[4][4];
	vec4 a[4][4], b[4][4];
	int k;
	if (n == 0) {
		draw_patch(p);
	}
	else
	{
		for (k = 0; k < 4; k++)
			divide_curve(p[k], a[k], b[k]);
		transpose(a);
		transpose(b);
		for (k = 0; k < 4; k++) {
			divide_curve(a[k], q[k], r[k]);
			divide_curve(b[k], s[k], t[k]);
		}

		divide_patch(q, n - 1);
		divide_patch(r, n - 1);
		divide_patch(s, n - 1);
		divide_patch(t, n - 1);
	}



}

void init(void) {

	for (int n = 0; n < NumTeapotPatches; n++) {
		vec4 patch[4][4];

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				vec3& v = vertices[indices[n][i][j]];
				patch[i][j] = vec4(v[X], v[Y], v[Z], 1.0);

			}
		}

		divide_patch(patch, NumTimesToSubdivide);

	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * NumVertices, points, GL_STATIC_DRAW);

	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	Projection = glGetUniformLocation(program, "Projection");

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(1.0, 1.0, 1.0, 1.0);

}

void reshape(int width, int height) {

	glViewport(0, 0, width, height);

	GLfloat left = -4.0, right = 4.0;
	GLfloat bottom = -3.0, top = 5.0;
	GLfloat zNear = -10.0, zFar = 10.0;

	GLfloat aspect = GLfloat(width) / height;

	if (aspect > 0) {
		left *= aspect;
		right *= aspect;

	}
	else {
		bottom /= aspect;
		bottom /= aspect;
	}

	mat4 projection = ortho(left, right, bottom, top, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_FALSE, value_ptr(projection));

}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glFinish();

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("3D Utah Teapot");

	glewInit();
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutMainLoop();

	return EXIT_SUCCESS;
}