#ifndef MAYBE_EGL_H
#define MAYBE_EGL_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>

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

typedef unsigned char bool;
#define true 1
#define false 0

#define SSIZE_MAX ((ssize_t)(~0ULL >> 1))

_Static_assert(sizeof(ssize_t) == 8);
_Static_assert(sizeof(long long) == 8);

#endif
