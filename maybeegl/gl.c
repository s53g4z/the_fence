#include "probablyegl.h"

// Overwrite v with its normalised (unit vector) form.
static void normalise(float v[3]) {
	float len = sqrt(pow(v[0],2) + pow(v[1],2) + pow(v[2],2));
	if (len == 0) {
		fprintf(stderr, "WARN: normalise(): using len = 1\n");
		len = 1;  // will give the wrong answer ...
	}
	for (size_t i = 0; i < 3; i++) {
		v[i] /= len;
	}
}

// Return u*v in the third argument.
static void crossProduct(float u[3], float v[3], float normal[3]) {
	normal[0] = u[1]*v[2] - v[1]*u[2];
	normal[1] = u[2]*v[0] - v[2]*u[0];
	normal[2] = u[0]*v[1] - v[0]*u[1];
}

static void fakeGluPerspective(void) {
	const float fovy = 45, aspect = 1, zNear = 0.99, zFar = 425;
	const float f = 1.0/(tan(fovy/2));
	const float matrix[] = {
		f/aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (zFar+zNear)/(zNear-zFar), (2*zFar*zNear)/(zNear-zFar),
		0, 0, -1, 0,
	};
	glMultMatrixf(matrix);
}

static void print_curr_mv_matrix(float params[4][4]) {
	for (size_t c = 0; c < 4; c++)
		for (size_t r = 0; r < 4; r++)
			fprintf(stderr, "%+.2f %s", params[r][c], (r+1)%4 == 0 ? "\n" : "");
	fprintf(stderr, "\n");
}

static size_t has_matrix = 0;  // # of matrices on the stack

static bool handleInput(const keys *const k) {
	static float m = 1;  // movement scale
	
	glMatrixMode(GL_MODELVIEW);
	
	// rotate around origin
	if (k->keyD || k->keyA || k->keyW || k->keyS) {
		if (k->keyD) {
			glRotatef(20*m , 0, 1, 0);
		}
		if (k->keyA) {
			glRotatef(-20*m, 0, 1, 0);
		}
		if (k->keyW) {
			glRotatef(-20*m, 1, 0, 0);
		}
		if (k->keyS) {
			glRotatef(20*m, 1, 0, 0);
		}
	}
	
	if (k->keyR) {  // reset everything
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		fakeGluPerspective();
		m = 1;
	}
	if (k->keyE) {
		return false;
	}
	
	if (k->keyK)
		glTranslatef(0, 0, -0.1*m);
	if (k->keyI)
		glTranslatef(0, 0, 0.1*m);
	if (k->keyL)
		glTranslatef(-0.1*m, 0, 0);
	if (k->keyJ)
		glTranslatef(0.1*m, 0, 0);
	if (k->keyU)
		glTranslatef(0, 0.1*m, 0);
	if (k->keyO)
		glTranslatef(0, -0.1*m, 0);
	
	if (k->keyT) {
		float params[4][4];
		glGetFloatv(GL_MODELVIEW_MATRIX, (float *)params);
		print_curr_mv_matrix(params);
	}
	
	if (k->keyLeftBracket) {
		glPushMatrix();
		has_matrix++;
	}
	if (k->keyRightBracket) {
		if (has_matrix) {
			glPopMatrix();
			has_matrix--;
		} else
			fprintf(stderr, "DEBUG: WARN: tried to pop empty matrix stack\n");
	}
	
	return true;
}

bool draw(const keys *const k) {
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);
	glDisable(GL_POLYGON_STIPPLE);
	static bool firstrun = true;
	if (firstrun) {
		firstrun = false;
		fakeGluPerspective();
	}
	glViewport(0, 0, 300, 300);

	if (!handleInput(k))
		return false;
	
	const uint8_t pixmap[] = {
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF,
		0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00,
		0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00,
		0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0xFF,0x00,0xFF, 0xFF,0x00,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	};
	glDisable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 17, 0, GL_RGB, GL_UNSIGNED_BYTE, pixmap);
	
	static uint32_t list = (uint32_t)-1;
	if (list == (uint32_t)-1) {
		list = glGenLists(1);
		glNewList(list, GL_COMPILE);  // mapping the window generates 2nd call
			glEnable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
				glColor3f(1.0, 0.0, 0.0);  // red
				glTexCoord2s(0, 0);
				glVertex3f(-5, 0, 5);  // XZ plane
				glTexCoord2s(1, 0);
				glVertex3f(5, 0, 5);
				glTexCoord2s(1, 1);
				glVertex3f(5, 0, -5);
				glTexCoord2s(0, 1);
				glVertex3f(-5, 0, -5);
			glEnd();
			
			glEnable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
				glColor3f(0.0, 1.0, 0.0);  // green
				glTexCoord2s(0, 0);
				glVertex3f(-5, -5, 0.0);  // XY plane
				glTexCoord2s(5, 0);
				glVertex3f(5, -5, 0.0);
				glTexCoord2s(5, 5);
				glVertex3f(5, 5, 0.0);
				glTexCoord2s(0, 5);
				glVertex3f(-5, 5, 0.0);
			glEnd();
			
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
				glColor3f(0.0, 0.0, 1.0);  // blue
				glVertex3f(0.0, -5, 5);  // YZ plane
				glVertex3f(0.0, -5, -5);
				glVertex3f(0.0, 5, -5);
				glVertex3f(0.0, 5, 5);
			glEnd();
		glEndList();
	} else
		glCallList(list);
	
	glPointSize(10);
	glColor3f(1.0, 1.0, 0.5);
	glBegin(GL_POINTS);
	for (size_t i = 0; i < 100; i++)
		glVertex2f(i / 10.0, i % 10 == 0 ? 0.1 : 0);
	glEnd();
	
	glPushMatrix();
	glRotatef(90, 0, 0, 1);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 1.0);
		glVertex3f(10, 0, 0);
		glColor3f(0.0, 1.0, 1.0);
		glVertex3f(10, -10, 0);
		glVertex3f(50, 0, 0);
	glEnd();
	glPopMatrix();
	
	glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 1.0);
		glVertex3f(10, 0, 0);
		glColor3f(0.0, 1.0, 1.0);
		glVertex3f(10, -10, 0);
		glVertex3f(50, 0, 0);
	glEnd();
	
	const uint8_t bitmap[] = {
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF,
		
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
	};
	
	glRasterPos3s(0, 0, 0);
	glBitmap(32, 32, 0, 0, 0, 0, bitmap);
	
	glFlush();
	assert(glGetError() == GL_NO_ERROR);
	
	return true;
}
