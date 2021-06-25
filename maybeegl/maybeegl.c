#include "maybeegl.h"

static egl_dat initialize_egl(void) {
	egl_dat ed;
	ed.d = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(ed.d != EGL_NO_DISPLAY);
	
	EGLint ret;
	
	ret = eglInitialize(ed.d, NULL, NULL);
	assert(ret == EGL_TRUE);
	
	ret = eglBindAPI(EGL_OPENGL_ES_API);
	assert(ret == EGL_TRUE);
	
	const EGLint attrib_list[] = {
		EGL_ALPHA_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint num_config;
	ret = eglChooseConfig(ed.d, attrib_list, ed.cfg, 1, &num_config);
	assert(ret == EGL_TRUE);
	
	// hxxps://community.arm.com/developer/tools-software/oss-platforms
	// /b/android-blog/posts/check-your-context-if-glcreateshader-returns-0
	// -and-gl_5f00_invalid_5f00_operation
	EGLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	ed.cxt = eglCreateContext(ed.d, ed.cfg[0], EGL_NO_CONTEXT, attribs);
	
	return ed;
}

static x_dat initialize_xwin(void) {
	x_dat xd;
	xd.d = XOpenDisplay(NULL);  // XCB has poor docs
	Window x_drw = XDefaultRootWindow(xd.d);
	XSetWindowAttributes xswa = { 0 };
	xswa.event_mask = KeyPressMask | KeyReleaseMask |
		ExposureMask | VisibilityChangeMask;
	xd.w = XCreateWindow(
		xd.d,
		x_drw,
		10,
		10,
		500,
		400,
		0,
		CopyFromParent,  // 24,
		InputOutput,
		CopyFromParent,
		CWEventMask,
		&xswa
	);
	XMapWindow(xd.d, xd.w);
	XFlush(xd.d);
	return xd;
}

static void initialize_surface(egl_dat *ed, x_dat *xd) {
	ed->s = eglCreateWindowSurface(ed->d, ed->cfg[0], xd->w, NULL);
	
	int ret;
	ret = eglMakeCurrent(ed->d, ed->s, ed->s, ed->cxt);
	assert(ret == EGL_TRUE);
}

// Print a GL program's information log.
static void gl_print_info_log(GLuint gl_p) {
	int llen;
	glGetProgramiv(gl_p, GL_INFO_LOG_LENGTH, &llen);
	if (!llen)
		return;
	
	GLchar *log = malloc(llen);
	glGetProgramInfoLog(gl_p, llen, NULL, log);
	fprintf(stderr, "LOG: %s\n", log);
	free(log);
}

// Print a GL program's information log if the program has errors.
static bool gl_verify_prog(GLuint gl_p) {
	GLint status;
	bool ret = true;
	
	glGetProgramiv(gl_p, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
		ret = false;
	gl_print_info_log(gl_p);

	glValidateProgram(gl_p);
	glGetProgramiv(gl_p, GL_VALIDATE_STATUS, &status);
	if (status != GL_TRUE)
		ret = false;
	gl_print_info_log(gl_p);
	
	return ret;
}

// Return buf with a null terminator appended. (nbytes_total is the buf fill.)
static char *buf_add_null_terminator(char *buf, ssize_t *bufsiz,
	const ssize_t nbytes_total) {
	if (*bufsiz <= nbytes_total) {
		if (*bufsiz > SSIZE_MAX - 1) {
			free(buf);
			return NULL;
		}
		void *newbuf = realloc(buf, ++(*bufsiz));
		if (!newbuf) {
			free(buf);
			return NULL;
		}
		buf = newbuf;
	}
	buf[nbytes_total] = '\0';
	return buf;
}

// Double the size of buf. Return true on success.
static bool double_sz_of_buf(char **buf, ssize_t *bufsiz) {
	if (*bufsiz > SSIZE_MAX / 2) {
		free(*buf);
		return false;
	}
	*bufsiz = *bufsiz * 2;
	void *newbuf = realloc(*buf, *bufsiz);
	if (!newbuf) {
		free(*buf);
		return false;
	}
	*buf = newbuf;
	return true;
}

// Return a heap pointer to the contents of filename.
static char *getShaderFromFile(const char *const filename) {
	int fd = open(filename, O_RDONLY);
	assert(0 < fd);
	
	char *buf = malloc(1);
	buf[0] = '\0';
	ssize_t bufsiz = 1, nbytes_total = 0;
	for (;;) {
		ssize_t nbytes_read = read(fd, buf + nbytes_total, bufsiz - nbytes_total);
		assert(0 <= nbytes_read);
		if (nbytes_total > SSIZE_MAX - nbytes_read) {
			free(buf);
			return NULL;
		}
		nbytes_total += nbytes_read;
		if (0 == nbytes_read)
			return buf_add_null_terminator(buf, &bufsiz, nbytes_total);
		assert(bufsiz >= nbytes_total);
		while (bufsiz <= nbytes_total)
			if (!double_sz_of_buf(&buf, &bufsiz))
				return NULL;
	}
	
	return NULL;
}

//static uint8_t randomChar(void) {
//	return (rand() % (255 - 0 + 1)) + 0;
//}

static void *genRandomArray() {
	//srand(time(NULL));
	struct rgb {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
	size_t arr_sz = sizeof(struct rgb) * 64 * 64;
	uint8_t *arr_ = malloc(arr_sz);
	memset(arr_, 0x00, arr_sz);
	
	// make the texture green
	for (size_t r = 0; r <64; r++)
		for (size_t c = 0; c < 64; c++) {
			struct rgb *row = (struct rgb *)
				(arr_ + r * 64 * sizeof(struct rgb));
			struct rgb *col = (struct rgb *)
				((uint8_t *)row + c * sizeof(struct rgb));
			*col = (struct rgb){0,255,0};
		}
	// make the bottom row blue
	for (size_t i = 0; i < 64; i++) {
		struct rgb *texel = (struct rgb *)(arr_ + i * sizeof(struct rgb));
		*texel = (struct rgb){0,0,255};
	}
	
	return arr_;
}

static glsh initialize_glshads() {
	glsh sh;
	
	sh.p = glCreateProgram();
	sh.v = glCreateShader(GL_VERTEX_SHADER);
	sh.f = glCreateShader(GL_FRAGMENT_SHADER);
	assert(sh.p && sh.v && sh.f);
	
	const char *const string1 = getShaderFromFile("vertex.txt");
	const char *const string2 = getShaderFromFile("frag.txt");

//	fprintf(stderr, "%s\n\n", glGetString(GL_VERSION));
//	fprintf(stderr, "%s\n", string1);
//	fprintf(stderr, "%s\n", string2);

	glActiveTexture(GL_TEXTURE0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	char *rand_arr = genRandomArray();
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		64,
		64,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		rand_arr
	);

	glShaderSource(sh.v, 1, &string1, NULL);
	glShaderSource(sh.f, 1, &string2, NULL);
	free((char *)string1);
	free((char *)string2);
	glCompileShader(sh.v);
	glCompileShader(sh.f);

	glAttachShader(sh.p, sh.v);
	glAttachShader(sh.p, sh.f);
//	fprintf(stderr, "DEBUG: GL_MAX_VERTEX_ATTRIBS is %d\n",
//		GL_MAX_VERTEX_ATTRIBS);
	glBindAttribLocation(sh.p, 9, "vpos");
	glLinkProgram(sh.p);
	
	gl_verify_prog(sh.p);
	
	glUseProgram(sh.p);  // ???
	
	return sh;
}

static void draw() {
	GLfloat strip[] = {
		-0.5, -0.5, 0.0,
		0.5, -0.5, 0.0,
		-0.5, 0.5, 0.0,
		0.5, 0.5, 0.0,
		0.0, 0.85, 0.0
	};
	glViewport(0, 0, 500, 400);
	glClearColor(1, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	//glUseProgram(gl_p);
	glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 0, strip);
	glEnableVertexAttribArray(9);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
}

static void cleanup(egl_dat *ed, x_dat *xd, glsh *sh) {
	int ret;
	
	glDeleteShader(sh->v);
	glDeleteShader(sh->f);
	glDeleteProgram(sh->p);
	
	ret = eglDestroyContext(ed->d, ed->cxt);
	assert(ret != EGL_FALSE && ret != EGL_BAD_DISPLAY &&
		ret != EGL_NOT_INITIALIZED && ret != EGL_BAD_CONTEXT);
	ret = eglDestroySurface(ed->d, ed->s);
	assert(ret != EGL_FALSE && ret != EGL_BAD_DISPLAY &&
		ret != EGL_NOT_INITIALIZED && ret != EGL_BAD_SURFACE);
	ret = eglTerminate(ed->d);
	assert(ret != EGL_FALSE && ret != EGL_BAD_DISPLAY);
	
	ret = XDestroyWindow(xd->d, xd->w);
	assert(ret != BadWindow);
	ret = XCloseDisplay(xd->d);
	assert(ret != BadGC);

}

static void fn(void) {
	egl_dat ed = initialize_egl();
	x_dat xd = initialize_xwin();
	initialize_surface(&ed, &xd);
	
	glsh sh = initialize_glshads();
	
	for (;;) {
		XEvent e;
		XNextEvent(xd.d, &e);
		//fprintf(stderr, "DEBUG: event received\n");
		if (e.type == Expose) {
			//XExposeEvent xee = e.xexpose;
			//fprintf(stderr, "DEBUG: %d %d %d %d %d\n", xee.x, xee.y,
			//	xee.width, xee.height, xee.count);
		} else if (e.type == KeyPress || e.type == KeyRelease) {
			XKeyEvent xke = e.xkey;
			if (xke.keycode == 24) {  // 'q'
				return cleanup(&ed, &xd, &sh);
			}
		}

		draw(sh.p);
		eglSwapBuffers(ed.d, ed.s);
	}
}

int main(int argc, char *argv[]) {
	argv[0][0] += argc - argc;
	
	fn();
}
