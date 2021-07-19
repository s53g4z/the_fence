#include "probablyegl.h"

static void print_shader_log(uint32_t shader) {
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

static uint32_t shader_try(void) {
	static bool ran = false;
	static uint32_t prgm;
	if (ran)
		return prgm;
	else
		ran = true;
	// get shader text
	ssize_t vtxTXTlen = 0, fragTXTlen = 0;
	const char *const vtxTXT = safe_read("./vtx2.txt", &vtxTXTlen);
	const char *const fragTXT = safe_read("./frag2.txt", &fragTXTlen);
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
	prgm = glCreateProgram();
	glAttachShader(prgm, vtxShdr);
	glAttachShader(prgm, fragShdr);
	glBindAttribLocation(prgm, 2, "vPos");  // must be done prior to link
	glLinkProgram(prgm);
	glGetProgramiv(prgm, GL_LINK_STATUS, &status);
	assert(GL_TRUE == status);
	
	// use program?
	glUseProgram(prgm);
	assert(glGetError() == GL_NO_ERROR);
	
	return prgm;
}

static void set_vertex_attribs() {
	static bool run = true;
	if (run)
		run = false;
	else
		return;
	
	static float vertices[] = {
		// XY plane
		-3.1, 3.1, 0.0,
		-3.1, -3.1, 0.0,
		3.1, 3.1, 0.0,
		3.1, -3.1, 0.0,
		
		// XZ plane
		-3.1, 0.0, -3.1,
		-3.1, 0.0, 3.1,
		3.1, 0.0, -3.1,
		3.1, 0.0, 3.1,
		
		// YZ plane
		0.0, 3.1, 3.1,
		0.0, 3.1, -3.1,
		0.0, -3.1, 3.1,
		0.0, -3.1, -3.1,
	};
	
	// note: glGetAttribLocation(prgm, nam) returns the bound index
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(2);
}

static void set_uniforms(uint32_t prgm, keys *const k) {	
	static float x = 0, y = 0, z = 0;  // translation in Units(TM)
	static float rx = -0.5, ry = 0.5, rz = 0;  // rotation in radians
	static float scale = 0.25;  // scale in Units(TM)
	
	if (k->keyLeft)
		x += 0.1;
	if (k->keyRight)
		x -= 0.1;
	if (k->keyUp)
		y -= 0.1;
	if (k->keyDown)
		y += 0.1;
	if (k->keyLeftBracket)
		z -= 0.1;
	if (k->keyRightBracket)
		z += 0.1;
	
	if (k->keyI)
		rx += 0.1;
	if (k->keyK)
		rx -= 0.1;
	if (k->keyJ)
		ry += 0.1;
	if (k->keyL)
		ry -= 0.1;  // bug: rotation is not smooth
	
	if (k->keyU)
		scale *= 0.9;
	if (k->keyO)
		scale *= 1.1;
	
	if (k->keyR)
		x = y = z = 0, rx = -0.5, ry = 0.5, rz = 0, scale = 0.25;
	int translationVecLocation = glGetUniformLocation(prgm, "translationVec");
	glUniform4f(translationVecLocation, x, y, z, 0);
	int rotationVecLocation = glGetUniformLocation(prgm, "rotationVec");
	glUniform4f(rotationVecLocation, rx, ry, rz, 0);
	int scaleFloatLocation = glGetUniformLocation(prgm, "scaleFloat");
	glUniform1f(scaleFloatLocation, scale);
}

static uint32_t texID[2];

static void set_texture(uint32_t prgm, glsh *sh) {
	static bool run = true;
	if (run)
		run = false;
	else
		return;
	
	for (int i = 0; i < 2; i++) {
		glGenTextures(1, &(texID[i]));
		glBindTexture(GL_TEXTURE_2D, texID[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		assert(sqrt(sh->ptrarr_elemsz[i] / 3) == 64);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB,
			sqrt(sh->ptrarr_elemsz[i] / 3),
			sqrt(sh->ptrarr_elemsz[i] / 3),
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			sh->ptrarr[i]
		);
	}

	free(sh->ptrarr_elemsz);
	for (size_t i = 0; i < sh->ptrarr_len; i++) {
		free(sh->ptrarr[i]);
	}
	free(sh->ptrarr);
	//uint32_t samplerLoc = glGetUniformLocation(prgm, "sTexture");
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texID[0]);
	//glUniform1i(samplerLoc, 0);
}

static void actualDraw(keys *const k, glsh *sh) {
	uint32_t prgm = shader_try();
	set_vertex_attribs();
	set_uniforms(prgm, k);
	set_texture(prgm, sh);
	
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	int useTextureLocation = glGetUniformLocation(prgm, "useTexture");
	uint32_t samplerLoc = glGetUniformLocation(prgm, "sTexture");
	
	// draw XY plane
	glUniform1i(useTextureLocation, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID[0]);
	glUniform1i(samplerLoc, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	// draw YZ plane
	glUniform1i(useTextureLocation, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID[1]);
	glUniform1i(samplerLoc, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	
	// draw XZ plane
	int fragColorLocation = glGetUniformLocation(prgm, "fragColor");
	glUniform4f(fragColorLocation, 0, 0, 1, 1);  // set colour here
	glUniform1i(useTextureLocation, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	
	assert(glGetError() == GL_NO_ERROR);
}

bool draw(keys *const k, glsh *sh) {
	assert(k && sh);
	actualDraw(k, sh);
	return true;
}
