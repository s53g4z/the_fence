#include "probablyegl.h"

// Overwrite v with its normalised (unit vector) form.
void normalise(float v[3]) {
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
void crossProduct(float u[3], float v[3], float normal[3]) {
	normal[0] = u[1]*v[2] - v[1]*u[2];
	normal[1] = u[2]*v[0] - v[2]*u[0];
	normal[2] = u[0]*v[1] - v[0]*u[1];
}

void fakeGluPerspective(void) {
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

void print_curr_mv_matrix(float params[4][4]) {
	for (size_t c = 0; c < 4; c++)
		for (size_t r = 0; r < 4; r++)
			fprintf(stderr, "%+.2f %s", params[r][c], (r+1)%4 == 0 ? "\n" : "");
	fprintf(stderr, "\n");
}

char *safe_read(const char *const filename, ssize_t *has_read) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;
	
	char *buf = malloc(1);
	ssize_t bufsiz = 1;
	for (;;) {
		if (*has_read > SSIZE_MAX - (ssize_t)buf) {
			free(buf);
			return NULL;
		}
		ssize_t got = read(fd, buf + *has_read, bufsiz - *has_read);
		if (got == 0)  // EOF
			return buf;
		if (got < 0) {
			free(buf);
			return NULL;
		}
		if (*has_read > SSIZE_MAX - got) {
			free(buf);
			return NULL;
		}
		*has_read += got;
		assert(bufsiz >= *has_read);
		if (bufsiz == *has_read) {
			if (bufsiz > SSIZE_MAX / 2) {
				free(buf);
				return NULL;
			}
			bufsiz *= 2;
			char *bigger = realloc(buf, bufsiz);
			if (!bigger) {
				free(buf);
				return NULL;
			}
			buf = bigger;
		}
	}
	
	return NULL;
}

bool elapsedTimeGreaterThanNS(struct timespec *const prev,
	struct timespec *const now, int64_t ns) {
	if (now->tv_sec - prev->tv_sec != 0)
		return true;  // lots of time has elapsed already
	return now->tv_nsec - prev->tv_nsec > ns;
}
