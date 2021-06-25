#ifndef MAYBE_EGL_H
#define MAYBE_EGL_H

#include <EGL/egl.h>
#include <GL/gl.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <float.h>

typedef unsigned char bool;
#define true 1
#define false 0

struct egl_data {
	EGLDisplay d;
	EGLSurface s;
	EGLConfig cfg[1];
	EGLContext cxt;
};
typedef struct egl_data egl_dat;

struct x_data {
	Display *d;
	Window w;
};
typedef struct x_data x_dat;

struct glsh {
	GLuint p;
	GLuint v;
	GLuint f;
};
typedef struct glsh glsh;

struct keys {
	bool keyA, keyD, keyS, keyW, keyR, keyE;
	bool keyI, keyJ, keyK, keyL, keyU, keyO;
	bool keyLeftBracket, keyRightBracket;
	bool keyLeft, keyRight, keyUp, keyDown;
	bool keyT;
};
typedef struct keys keys;

#define SSIZE_MAX ((ssize_t)(~0ULL >> 1))

_Static_assert(sizeof(ssize_t) == 8);
_Static_assert(sizeof(long long) == 8);
_Static_assert(sizeof(GLbyte) == 1);
_Static_assert(sizeof(GLshort) == sizeof(short));
_Static_assert(sizeof(GLint) == sizeof(int));
_Static_assert(sizeof(GLfloat) == sizeof(float));
_Static_assert(sizeof(GLdouble) == sizeof(double));

bool draw(const keys *const);

#endif
