#include "probablyegl.h"

static bool ordinary_matrix_on_stack = false;

static bool handleInput(keys *const k) {
	static float rotatedX = 0, rotatedY = 0;
	
	glMatrixMode(GL_MODELVIEW);
	
	// calculate rotate
	if (k->keyD)
		rotatedY += 5;
	else if (k->keyA)
		rotatedY += -5;
	else if (k->keyW)
		rotatedX += -5;
	else if (k->keyS)
		rotatedX += 5;
	
	// do rotate
	glPushMatrix();
	ordinary_matrix_on_stack = true;
	glRotatef(rotatedY, 0, 1, 0);
	glRotatef(rotatedX, 1, 0, 0);
	
	if (k->keyR) {  // reset everything
		if (ordinary_matrix_on_stack)
			glPopMatrix();
		ordinary_matrix_on_stack = false;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		fakeGluPerspective();
		rotatedX = rotatedY = 0;
	}
	
	if (k->keyE) {
		return false;  // trigger execve
	}
	
	if (k->keyK || k->keyI || k->keyL || k->keyJ || k->keyU || k->keyO) {
		if (ordinary_matrix_on_stack)
			glPopMatrix();  // pop a normal matrix
		if (k->keyK)
			glTranslatef(0, 0, -0.1);
		if (k->keyI)
			glTranslatef(0, 0, 0.1);
		if (k->keyL)
			glTranslatef(-0.1, 0, 0);
		if (k->keyJ)
			glTranslatef(0.1, 0, 0);
		if (k->keyU)
			glTranslatef(0, 0.1, 0);
		if (k->keyO)
			glTranslatef(0, -0.1, 0);
		if (ordinary_matrix_on_stack)
			glPushMatrix();  // re-save the normal matrix
		glRotatef(rotatedY, 0, 1, 0);
		glRotatef(rotatedX, 1, 0, 0);
	}
	
	if (k->keyT) {
		float params[4][4];
		glGetFloatv(GL_MODELVIEW_MATRIX, (float *)params);
		print_curr_mv_matrix(params);
	}
	
	memset(k, 0x00, sizeof(*k));
	
	return true;
}

static bool textured = false;

void planeXZ(glsh *sh) {
	if (!textured) {
		glEnable(GL_TEXTURE_2D);
		uint32_t texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB,
			(uint32_t)sqrt(sh->ptrsz / 3.0),  // three bytes per pixel
			(uint32_t)sqrt(sh->ptrsz / 3.0),  // i.e., (64*64) * 3
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			sh->ptr
		);
		free(sh->ptr);
		//sh->ptr = NULL;
		glBindTexture(GL_TEXTURE_2D, texture);
		textured = true;
	}
	
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glRotatef(90, 0, 0, 1);
	glColor3f(0, 1, 0);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 0, 1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 0, 1);
		glTexCoord2f(1, 1);
		glVertex3f(1, 0, -1);
		glTexCoord2f(0, 1);
		glVertex3f(-1, 0, -1);
	glEnd();
	glPopMatrix();
	
	glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
		glTexCoord2f(0, 0);
		glVertex3f(-2, 0, 2);
		glTexCoord2f(1, 0);
		glVertex3f(2, 0, 2);
		glTexCoord2f(0, 1);
		glVertex3f(-2, 0, -2);
		
		glTexCoord2f(0, 1);
		glVertex3f(-2, 0.1, -2);
		glTexCoord2f(1, 0);
		glVertex3f(2, 0.1, 2);
		glTexCoord2f(1, 1);
		glVertex3f(2, 0.1, -2);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	
	assert(glGetError() == GL_NO_ERROR);
}

bool draw(keys *const k, glsh* sh) {
	glEnable(GL_STENCIL_TEST);
	glClearColor(0, 0, 0, 1);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glPolygonMode(GL_BACK, GL_LINE);
	glDisable(GL_POLYGON_STIPPLE);
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	static bool firstrun = true;
	if (firstrun) {
		firstrun = false;
		fakeGluPerspective();
	}
	glViewport(0, 0, 300, 300);
	
	// sea of 0, GL_QUAD of 1
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_ZERO, GL_REPLACE, GL_REPLACE);  // first arg doesn't matter
	
	glColor3f(1, 0, 1);
	glBegin(GL_QUADS);
		glVertex3f(-0.5, -0.5, 0);
		glVertex3f(0.5, -0.5, 0);
		glVertex3f(0.5, 0.5, 0);
		glVertex3f(-0.5, 0.5, 0);
	glEnd();
	
	// draw on the sea of 0
	glStencilFunc(GL_EQUAL, 0, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	if (!handleInput(k))
		return false;  // execve
	
	glColor3f(1, 0, 0);
	glBegin(GL_QUADS);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 0, -1);
		glVertex3f(-1, 0, -1);
		glVertex3f(-1, 0, 1);
	glEnd();
	
	planeXZ(sh);
	
	if (ordinary_matrix_on_stack) {  // clean stack from input handling
		glPopMatrix();
	}
	glFlush();
	return true;
}
