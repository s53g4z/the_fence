#include "probablyegl.h"

static bool ordinary_matrix_on_stack = false;

static bool handleInput(keys *const k) {
	glPushMatrix();
	ordinary_matrix_on_stack = true;
	static float rotatedX = 0, rotatedY = 0;
	glRotatef(rotatedY, 0, 1, 0);
	glRotatef(rotatedX, 1, 0, 0);
	
	static struct timespec prev = { 0 };
	struct timespec now = { 0 };
		
	assert(TIME_UTC == timespec_get(&now, TIME_UTC));
	if (elapsedTimeGreaterThanNS(&prev, &now, 1000000000 / 60)) {
		prev = now;
	} else {
		return true;  // rate limit input
	}

	glMatrixMode(GL_MODELVIEW);
	
	// calculate rotate (for next time)
	if (k->keyD)
		rotatedY += 5;
	else if (k->keyA)
		rotatedY += -5;
	else if (k->keyW)
		rotatedX += -5;
	else if (k->keyS)
		rotatedX += 5;
	
	// do rotate
	//glPushMatrix();
	//ordinary_matrix_on_stack = true;
	
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
	
	//memset(k, 0x00, sizeof(*k));
	
	return true;
}

void genTextures(glsh *sh, uint32_t *textures) {
	static bool textured = false;
	if (textured)
		return;
	textured = true;
	
	glDisable(GL_TEXTURE_2D);
	glGenTextures(sh->ptrarr_len, textures);
	
	for (size_t i = 0; i < sh->ptrarr_len; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB,
			(uint32_t)sqrt(sh->ptrarr_elemsz[i] / 3.0),  // 3 bytes per texel
			(uint32_t)sqrt(sh->ptrarr_elemsz[i] / 3.0),  // i.e., (64*64) * 3
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			sh->ptrarr[i]
		);
	}
	
	for (size_t i = 0; i < sh->ptrarr_len; i++) {
		free(sh->ptrarr[i]);
	}
	free(sh->ptrarr);
	free(sh->ptrarr_elemsz);
	sh->ptrarr_len = 0;
}

void draw_planes(uint32_t textures[]) {
	
	glColor3f(1, 0, 0);  // a red XZ plane
	glBegin(GL_QUADS);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 0, -1);
		glVertex3f(-1, 0, -1);
		glVertex3f(-1, 0, 1);
	glEnd();
	
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glRotatef(90, 0, 0, 1);
	glColor3f(0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glBegin(GL_QUADS);  // a textured YZ plane (b/c of glRotate)
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
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);  // a textured XZ "plane" made out of two triangles
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
	}

void make_stencil(void) {
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
}

void print_shader_log(uint32_t shader) {
	int logLen;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen == 0)
		return;
	char *log = malloc(logLen);
	assert(log);
	
	glGetShaderInfoLog(shader, logLen, NULL, log);
	fprintf(stderr, "SHDR_LOG: %s\n", log);
	free(log);
	assert(NULL);
}

void shader_try(void) {
	static bool ran = false;
	if (ran)
		return;
	else
		ran = true;
	// get shader text
	ssize_t vtxTXTlen = 0, fragTXTlen = 0;
	const char *const vtxTXT = safe_read("./vtx1.txt", &vtxTXTlen);
	const char *const fragTXT = safe_read("./frag1.txt", &fragTXTlen);
	assert(vtxTXT && fragTXT);
	
	// create and build shaders
	uint32_t vtxShdr = glCreateShader(GL_VERTEX_SHADER);
	uint32_t fragShdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vtxShdr, 1, &vtxTXT, (const int *)&vtxTXTlen);
	glShaderSource(fragShdr, 1, &fragTXT, (const int *)&fragTXTlen);
	glCompileShader(vtxShdr);
	int status;
	glGetShaderiv(vtxShdr, GL_COMPILE_STATUS, &status);
	if(GL_TRUE != status)
		print_shader_log(vtxShdr);
	glCompileShader(fragShdr);
	glGetShaderiv(fragShdr, GL_COMPILE_STATUS, &status);
	if(GL_TRUE != status)
		print_shader_log(fragShdr);
	
	// create and link program
	uint32_t prgm = glCreateProgram();
	glAttachShader(prgm, vtxShdr);
	glAttachShader(prgm, fragShdr);
	glLinkProgram(prgm);
	glGetProgramiv(prgm, GL_LINK_STATUS, &status);
	assert(GL_TRUE == status);
	
	// use program?
	glUseProgram(prgm);
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
		glViewport(0, 0, 300, 300);
	}
	if (!handleInput(k))
		return false;  // execve

	static uint32_t *textures = NULL;
	if (!textures) {
		textures = malloc(sizeof(uint32_t) * sh->ptrarr_len);
		genTextures(sh, textures);
	}

	shader_try();

	make_stencil();
	

	draw_planes(textures);
		
	if (ordinary_matrix_on_stack) {  // clean stack from input handling
		glPopMatrix();
		ordinary_matrix_on_stack = false;
	}
	glFlush();
	return true;
}
