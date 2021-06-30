#include "probablyegl.h"

static egl_dat initialize_egl(void) {
	egl_dat ed;
	ed.d = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(ed.d != EGL_NO_DISPLAY);
	
	EGLint ret;
	
	ret = eglInitialize(ed.d, NULL, NULL);
	assert(ret == EGL_TRUE);
	
	ret = eglBindAPI(EGL_OPENGL_API);
	assert(ret == EGL_TRUE);
	
	const EGLint attrib_list[] = {
		EGL_ALPHA_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_CONFORMANT, EGL_OPENGL_BIT,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};
	EGLint num_config;
	ret = eglChooseConfig(ed.d, attrib_list, ed.cfg, 1, &num_config);
	assert(ret == EGL_TRUE);
	
	// hxxps://community.arm.com/developer/tools-software/oss-platforms
	// /b/android-blog/posts/check-your-context-if-glcreateshader-returns-0
	// -and-gl_5f00_invalid_5f00_operation
	EGLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE };
	ed.cxt = eglCreateContext(ed.d, ed.cfg[0], EGL_NO_CONTEXT, attribs);
	
	return ed;
}

static x_dat initialize_xwin(void) {
	x_dat xd;
	xd.d = XOpenDisplay(NULL);  // XCB has poor docs
	Window x_drw = XDefaultRootWindow(xd.d);
	XSetWindowAttributes xswa = { 0 };
	xswa.event_mask = KeyPressMask | KeyReleaseMask |
		ResizeRedirectMask | ExposureMask;
	xd.w = XCreateWindow(
		xd.d,
		x_drw,
		10,
		10,
		500,
		300,
		0,
		CopyFromParent,  // 24,
		InputOutput,
		CopyFromParent,
		CWEventMask,
		&xswa
	);
	XMapWindow(xd.d, xd.w);
	XSync(xd.d, false);
	return xd;
}

static void initialize_surface(egl_dat *ed, x_dat *xd) {
	ed->s = eglCreateWindowSurface(ed->d, ed->cfg[0], xd->w, NULL);
	
	int ret;
	ret = eglMakeCurrent(ed->d, ed->s, ed->s, ed->cxt);
	assert(ret == EGL_TRUE);
}

static glsh initialize_glshads(void) {
	glsh sh = { 0 };
	
	sh.ptr = safe_read("./texture1.rgb", &(sh.ptrsz));
	assert(sh.ptr != NULL);
	
	return sh;
}

static bool cleanup(egl_dat *ed, x_dat *xd, glsh *sh) {
	int ret;
	
	assert(sh);
	
	ret = eglMakeCurrent(ed->d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	assert(ret == EGL_TRUE);
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

	return true;
}

void handleKeys(const XEvent *const e, keys *const k) {
	const bool keyState = e->type == KeyPress;
	uint32_t code = e->xkey.keycode;
	
	if (code == 38 || code == 40) {
		if (code == 38)
			k->keyA = keyState;
		if (code == 40)
			k->keyD = keyState;
		if (k->keyA && k->keyD)
			k->keyA = k->keyD = false;
	} else if (code == 25 || code == 39) {
		if (code == 25)
			k->keyW = keyState;
		if (code == 39)
			k->keyS = keyState;
		if (k->keyS && k->keyW)
			k->keyS = k->keyW = false;
	} else if (code == 27) {  // r
		k->keyR = keyState;
	} else if (code == 31 || code == 45) {  // i, k
		if (code == 31)
			k->keyI = keyState;
		if (code == 45)
			k->keyK = keyState;
		if (k->keyI && k->keyK)
			k->keyI = k->keyK = false;
	} else if (code == 44 || code == 46) {  // j, l
		if (code == 44)
			k->keyJ = keyState;
		if (code == 46)
			k->keyL = keyState;
		if (k->keyJ && k->keyL)
			k->keyJ = k->keyL = false;
	} else if (code == 34 || code == 35) {
		if (code == 34)
			k->keyLeftBracket = keyState;
		if (code == 35)
			k->keyRightBracket = keyState;
		if (k->keyLeftBracket && k->keyRightBracket)
			k->keyLeftBracket = k->keyRightBracket = false;
	} else if (code == 113 || code == 114) {
		if (code == 113)
			k->keyLeft = keyState;
		if (code == 114)
			k->keyRight = keyState;
		if (k->keyLeft && k->keyRight)
			k->keyLeft = k->keyRight = false;
	} else if (code == 111 || code == 116) {
		if (code == 111)
			k->keyUp = keyState;
		if (code == 116)
			k->keyDown = keyState;
		if (k->keyUp && k->keyDown)
			k->keyUp = k->keyDown = false;
	} else if (code == 30 || code == 32) {
		if (code == 30)
			k->keyU = keyState;
		if (code == 32)
			k->keyO = keyState;
		if (k->keyU && k->keyO)
			k->keyU = k->keyO = false;
	} else if (code == 26) {
		k->keyE = keyState;
	} else if (code == 28) {
		k->keyT = keyState;
	} else
		fprintf(stdout, "Key %d %s\n", code,
			keyState ? "KeyPress" : "KeyRelease");
}

static bool fn(void) {
	keys k = { 0 };
	
	egl_dat ed = initialize_egl();
	x_dat xd = initialize_xwin();
	initialize_surface(&ed, &xd);
	
	glsh sh = initialize_glshads();
	glViewport(0, 0, 500, 400);
	draw(&k, &sh);
	eglSwapBuffers(ed.d, ed.s);
	
	struct timespec prev = { 0 };
	uint64_t frames = 0, now_xorg_timediff_sec = 0, keyPresses = 0;
	bool seenFirstEvent = false;
	for (;;) {
		XEvent e = { 0 };
		struct timespec now = { 0 };
		
		assert(TIME_UTC == timespec_get(&now, TIME_UTC));
		//sleep(1);
		if (elapsedTimeGreaterThanNS(&prev, &now, 1000000000)) {
			prev = now;
			fprintf(stderr, "DEBUG: %llu frames\n", (long long unsigned)frames);
			frames = 0, keyPresses = 0;
		}
		while (XCheckMaskEvent(xd.d, -1, &e)) {
			if ((e.type == KeyPress || e.type == KeyRelease)) {
				if (e.type == KeyPress && keyPresses++ > 25) {
					fprintf(stderr, "DEBUG: key ignored b/c quota exceeded\n");
					continue;
				}
				if (!seenFirstEvent) {
					now_xorg_timediff_sec = now.tv_sec - e.xkey.time/1000;
					seenFirstEvent = true;
				}
				uint64_t approxX11time_sec = now.tv_sec - now_xorg_timediff_sec;
				uint64_t keyAge = e.xkey.time/1000 - approxX11time_sec;
				if (e.xkey.time/1000 < approxX11time_sec)  // approx = inaccurat
					keyAge = approxX11time_sec - e.xkey.time/1000;
				if (keyAge < 2) {
					if (e.xkey.keycode == 24)  // 'q'
						return cleanup(&ed, &xd, &sh);
					handleKeys(&e, &k);
				} else {
					fprintf(stderr, "DEBUG: key age %llu, ignored\n",
						(long long unsigned)keyAge);
				}
			} else if (e.type == ResizeRequest) {
				//XResizeRequestEvent xrre = e.xresizerequest;
				//int w = xrre.width;
				//int h = xrre.height;
				//glViewport(0, 0, w, h);
			}
		}

		if (!draw(&k, &sh)) {
			cleanup(&ed, &xd, &sh);
			return false;
		}
		eglSwapBuffers(ed.d, ed.s);
		frames++;
	}
	
	return true;
}

int main(int argc, char *argv[], char *const envp[]) {
	argv[0][0] += argc - argc;
	
	if (!fn())
		execve(argv[0], argv, envp);
}
