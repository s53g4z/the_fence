#include "probablyegl.h"

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

uint32_t shader_try(void) {
	static bool ran = false;
	if (ran)
		return (uint32_t)(uint64_t)NULL;
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
	uint32_t prgm = glCreateProgram();
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
	
//	static float vertices1[] = {
//		0.0, 0.0, 0.0,
//		1.0, 0.0, 0.0,
//		0.0, 1.0, 0.0,
//		1.0, 1.0, 0.0
//	};
	static float vertices[] = {
		0.0, 0.0, 0.0,
		.1000, 0.0, 0.0,
		0.0, .1000, 0.0,
		.1000, .1000, 0.0
	};
	
	// note: glGetAttribLocation(prgm, nam) returns the bound index
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(2);
}

void set_uniforms(uint32_t prgm) {
	int fragColorLocation = glGetUniformLocation(prgm, "fragColor");
	glUniform4f(fragColorLocation, 0, 1, 0, 1);  // set colour here
}

void set_texture(uint32_t prgm, glsh *sh) {
	uint32_t texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	assert(sqrt(sh->ptrarr_elemsz[1] / 3) == 64);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		sqrt(sh->ptrarr_elemsz[1] / 3),
		sqrt(sh->ptrarr_elemsz[1] / 3),
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		sh->ptrarr[1]
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	uint32_t samplerLoc = glGetUniformLocation(prgm, "sTexture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(samplerLoc, 0);
}

void actualDraw(glsh *sh) {
	uint32_t prgm = shader_try();
	set_vertex_attribs();
	set_uniforms(prgm);
	set_texture(prgm, sh);
	
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

bool draw(keys *const k, glsh *sh) {
	assert(k && sh);
	actualDraw(sh);
	return true;
}
