#include "probablyegl.h"
#include "core.h"

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
#ifndef USE_GLES2
	const float fovy = 45, aspect = 1, zNear = 0.99, zFar = 425;
	const float f = 1.0/(tan(fovy/2));
	const float matrix[] = {
		f/aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (zFar+zNear)/(zNear-zFar), (2*zFar*zNear)/(zNear-zFar),
		0, 0, -1, 0,
	};
	glMultMatrixf(matrix);
#endif
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
	*has_read = 0;
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

struct coord {
	float x;
	float y;
};

// Helper fn for glPrintNum.
void calculate_texture_coordinates(int dig, struct coord tc[4]) {
	if (dig == 0) {
		tc[0] = (struct coord) { .x = 0.33, .y = 0.05 };
		tc[1] = (struct coord) { .x = 0.66, .y = 0.05 };
		tc[2] = (struct coord) { .x = 0.66, .y = 0.02 + 0.33/2 };
		tc[3] = (struct coord) { .x = 0.33, .y = 0.02 + 0.33/2 };
	} else {
		float tox = (dig-1)%3 * 0.33;  // texture offset x
		float toy = 0.33;  // texture offset y
		if (dig >= 1 && dig <= 3) {
			toy = 0.66;
		} else if ( dig >= 7 && dig <= 9) {
			toy = 0.00;
		}
		tc[0] = (struct coord) { .x = 0.00 + tox, .y = 0.00 + toy };
		tc[1] = (struct coord) { .x = 0.33 + tox, .y = 0.00 + toy };
		tc[2] = (struct coord) { .x = 0.33 + tox, .y = 0.33 + toy };
		tc[3] = (struct coord) { .x = 0.00 + tox, .y = 0.33 + toy };
	}
}

#ifdef USING_FULL_OPENGL
// Helper fn for glPrintNum.
void render_numbers(struct coord tc[4], ssize_t offsetX) {
	glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, textures[1]);
	glBegin(GL_QUADS);
		glColor3f(0.0, 1.0, 0.0);  // color backup in case texturing fails
		glTexCoord2f(tc[0].x, tc[0].y);
		glVertex3f(0 + offsetX, 0, -1);  // lower left
		glTexCoord2f(tc[1].x, tc[1].y);
		glVertex3f(1 + offsetX, 0, -1);  // lower right
		glTexCoord2f(tc[2].x, tc[2].y);
		glVertex3f(1 + offsetX, 1, -1);  // upper right
		glTexCoord2f(tc[3].x, tc[3].y);
		glVertex3f(0 + offsetX, 1, -1);  // upper left
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

// Print a number to the left of the origin.
void glPrintNum(uint64_t num, uint32_t textures[2]) {
	glBindTexture(GL_TEXTURE_2D, textures[1]);  // the number texture
	
	ssize_t offsetX = -1;
	for (;;) {
		int dig = num % 10;
		num /= 10;
		
		struct coord tc[4];  // texture coordinates to use
		calculate_texture_coordinates(dig, tc);
		render_numbers(tc, offsetX);
		
		if (num == 0)
			break;
		offsetX -= 1;
	}
}
#endif

// Never-null malloc().
void *nnmalloc(size_t sz) {
	void *rv = malloc(sz);
	if (!rv)
		exit(12);
	return rv;
}

// Never-null realloc().
void *nnrealloc(void *oldptr, size_t newsz) {
	void *rv = realloc(oldptr, newsz);
	if (!rv)
		exit(12);
	return rv;
}

WorldItem *worldItem_new(int x, int y, int wi, int h, float spx, float spy,
	bool gravity, char *imgnam, void(*frame)(WorldItem *const), bool ff) {
	//static int tunit = 0;
	
	assert(wi > 0 && h > 0);
	WorldItem *w = nnmalloc(sizeof(WorldItem));
	w->x = x;
	w->y = y;
	w->width = wi;
	w->height = h;
	w->speedX = spx;
	w->speedY = spy;
	w->frame = frame;
	w->gravity = gravity;
	//w->texunit = tunit++;
	
	if (imgnam) {
		glGenTextures(1, &w->texnam);
		initGLTextureNam(w->texnam, imgnam, false);
		if (ff) {
			glGenTextures(1, &w->texnam2);
			initGLTextureNam(w->texnam2, imgnam, true);
		}
	} else
		w->texnam = (uint32_t)-1;
	
	return w;
}

int leftOf(const WorldItem *const w) {
	return w->x;
}

int rightOf(const WorldItem *const w) {
	assert(w->x < INT_MAX - w->width);
	return w->x + w->width;
}

int topOf(const WorldItem *const w) {
	return w->y;
}

int bottomOf(const WorldItem *const w) {
	assert(w->y < INT_MAX - w->height);
	return w->y + w->height;
}

char_stack char_stack_init() {
	char_stack ch_stack;
	ch_stack.arr_len = 0;
	ch_stack.arr_capacity = 1;
	ch_stack.arr = nnmalloc(ch_stack.arr_capacity);
	return ch_stack;
}

void char_stack_destroy(char_stack *const ch_stack) {
	free(ch_stack->arr);
	ch_stack->arr_len = ch_stack->arr_capacity = 0;
}

void char_stack_print(const char_stack *const ch_stack) {
	for (size_t i = 0; i < ch_stack->arr_len; i++)
		fprintf(stderr, "[%c] ", ch_stack->arr[i]);
	fprintf(stderr, "\n");
}

size_t char_stack_len(const char_stack *const ch_stack) {
	return ch_stack->arr_len;
}

void char_stack_push(char_stack *const ch_stack, char ch) {
	assert(ch_stack->arr_capacity != 0);  // UAF guard
	if (ch_stack->arr_len == ch_stack->arr_capacity) {
		assert(ch_stack->arr_capacity < SIZE_MAX / 2);
		ch_stack->arr_capacity *= 2;
		ch_stack->arr = nnrealloc(ch_stack->arr, ch_stack->arr_capacity);
	}
	ch_stack->arr[(ch_stack->arr_len)++] = ch;
}

char char_stack_pop(char_stack *const ch_stack) {
	assert(ch_stack->arr_capacity != 0 && ch_stack->arr_len > 0);
	return ch_stack->arr[--(ch_stack->arr_len)];
}

static const char *nextSectionFrom(const char *ptr, const char *const end, 
	size_t *const sect_len) {
	*sect_len = 0;
	const char *start;
	
	while (ptr != end && *ptr != '(')
		ptr++;
	if (ptr == end)
		return NULL;  // no sect found
	
	start = ptr++;
	
	// until the next paren
	while (ptr != end && *ptr != '(' && *ptr != ')')
		ptr++;
	if (ptr == end)
		return NULL;  // malformed?
	
	*sect_len = ptr - start;
	return start;
}

bool isWhitespace(char ch) {
	return ch == ' ' || ch == '\t' || ch == '\n';
}

static void trimWhitespace(const char **section, size_t *section_len) {
	while (isWhitespace(**section)) {
		(*section)++;
		(*section_len)--;
	}
	while (isWhitespace(*(*section + *section_len - 1)))
		(*section_len)--;
}

// levelReader failure cleanup. Returns with lvl->hdr = false.
stl lrFailCleanup(const char *const level_orig, stl *lvl) {
	free((char *)level_orig);
	
	free(lvl->author);
	free(lvl->name);
	free(lvl->background);
	free(lvl->music);
	free(lvl->particle_system);
	free(lvl->theme);
	for (int h = 0; h < lvl->height; h++) {
		free(lvl->interactivetm[h]);
		free(lvl->backgroundtm[h]);
		free(lvl->foregroundtm[h]);
	}
	free(lvl->interactivetm);
	free(lvl->backgroundtm);
	free(lvl->foregroundtm);
	free(lvl->objects);
	memset(lvl, 0xe5, sizeof(*lvl));
	
	lvl->hdr = false;
	return *lvl;
}

void stlPrinter(const stl *const lvl) {
	fprintf(stderr, "hdr: %s\n", lvl->hdr ? "true" : "false");
	fprintf(stderr, "version: %d\n", lvl->version);
	fprintf(stderr, "author: %s\n", lvl->author);
	fprintf(stderr, "name (of the level): %s\n", lvl->name);
	fprintf(stderr, "width: %d\n", lvl->width);
	fprintf(stderr, "height: %d\n", lvl->height);
	fprintf(stderr, "start_pos: %d, %d\n", lvl->start_pos_x, lvl->start_pos_y);
	fprintf(stderr, "background: %s\n", lvl->background);
	fprintf(stderr, "music: %s\n", lvl->music);
	fprintf(stderr, "time: %d\n", lvl->time);
	fprintf(stderr, "gravity: %d\n", lvl->gravity);
	fprintf(stderr, "particle_system: %s\n", lvl->particle_system);
	fprintf(stderr, "theme: %s\n", lvl->theme);
	fprintf(stderr, "objects {\n");
	for (size_t i = 0; i < lvl->objects_len; i++) {
		char *str = "u n k n o w n ";
		int type = lvl->objects[i].type;
		switch(type) {
			case STL_INVALID_OBJ: str = "STL_INVALID_OBJ"; break;
			case STL_NO_MORE_OBJ: str = "STL_NO_MORE_OBJ"; break;
			case SNOWBALL: str = "SNOWBALL"; break;
			case MRICEBLOCK: str = "MRICEBLOCK"; break;
			case MRBOMB: str = "MRBOMB"; break;
			case STALACTITE: str = "STALACTITE"; break;
			case BOUNCINGSNOWBALL: str = "BOUNCINGSNOWBALL"; break;
			case FLYINGSNOWBALL: str = "FLYINGSNOWBALL"; break;
			case MONEY: str = "MONEY"; break;
		}
		fprintf(stderr, "\t%s at %d, %d\n", str, lvl->objects[i].x,
			lvl->objects[i].y);
	}
	fprintf(stderr, "}\n");
	fprintf(stderr, "objects_len/cap: %lu/%lu\n", lvl->objects_len,
		lvl->objects_cap);
}

stl levelReader(const char *const filename) {
	stl lvl = { 0 };
	
	ssize_t level_len_ss;
	char *file = safe_read(filename, &level_len_ss);
	if (!file || level_len_ss < 1) {
		lvl.hdr = false;
		return lvl;
	}
	file = nnrealloc(file, level_len_ss + 1);  // for *scanf()
	file[level_len_ss] = '\0';  // ibid
	const char *level = file;
	file = NULL;  // dont use again
	const char *const level_orig = level;
	size_t level_len = level_len_ss;
	
	const char *section = NULL;  // you have the level, now get a section
	size_t section_len;
	while ((section = nextSectionFrom(level, level + level_len, &section_len))) {
		const char *const section_orig = section;
		const size_t section_len_orig = section_len;
		trimWhitespace(&section, &section_len);
		
		if (!lvl.hdr) {
			if (*section++ != '(')
				return lrFailCleanup(level_orig, &lvl);
			else
				section_len--;
			trimWhitespace(&section, &section_len);
			const char *const su_level_str = "supertux-level";
			if (section_len >= strlen(su_level_str) &&
				0 == strncmp(section, su_level_str, strlen(su_level_str))) {
				lvl.hdr = true;
			} else
				return lrFailCleanup(level_orig, &lvl);
		} else {
			if (*section++ != '(')
				return lrFailCleanup(level_orig, &lvl);
			else
				section_len--;
			trimWhitespace(&section, &section_len);  // optional for well-formed??

			if (nextWordIs("version", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.version) || lvl.version != 1)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("author", &section, &section_len)) {
				if (!writeStrTo(&lvl.author, &section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("name", &section, &section_len)) {
				if (!writeStrTo(&lvl.name, &section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("width", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.width) || lvl.width < 0)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("height", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.height) || lvl.height < 0)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("start_pos_x", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.start_pos_x) ||
					lvl.start_pos_x < 0)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("start_pos_y", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.start_pos_y) ||
					lvl.start_pos_y < 0)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("background", &section, &section_len)) {
				if (!writeStrTo(&lvl.background, &section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("music", &section, &section_len)) {
				if (!writeStrTo(&lvl.music, &section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("time", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.time) || lvl.time < 0)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("gravity", &section, &section_len)) {
				if (1 != sscanf(section, "%d", &lvl.gravity) || lvl.gravity < 0)
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("particle_system", &section, &section_len)) {
				if (!writeStrTo(&lvl.particle_system, &section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("theme", &section, &section_len)) {
				if (!writeStrTo(&lvl.theme, &section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("interactive-tm", &section, &section_len)) {
				if (!parseTM(&lvl.interactivetm, lvl.width, lvl.height,
					&section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("background-tm", &section, &section_len)) {
				if (!parseTM(&lvl.backgroundtm, lvl.width, lvl.height,
					&section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("foreground-tm", &section, &section_len)) {
				if (!parseTM(&lvl.foregroundtm, lvl.width, lvl.height,
					&section, &section_len))
					return lrFailCleanup(level_orig, &lvl);
			} else if (nextWordIs("objects", &section, &section_len)) {
				// note: objects[_len|_cap]
				if (lvl.objects_cap == 0)
					init_lvl_objects(&lvl);
				for (;;) {
					stl_obj obj = getSTLobj(&section, &section_len);
					if (obj.type == STL_NO_MORE_OBJ)
						break;
					else if (obj.type == STL_INVALID_OBJ)
						return lrFailCleanup(level_orig, &lvl);
					pushto_lvl_objects(&lvl, &obj);
				}
			}
		}
		
		level_len = level_len - (section_orig + section_len_orig - level);
		level = section_orig + section_len_orig;
	}
	
	free((char *)level_orig);
	return lvl;
}

// Construct the stl's objects member.
void init_lvl_objects (stl *const lvl) {
	assert(lvl->objects_cap == 0);
	lvl->objects_cap = 1;
	lvl->objects = nnmalloc(lvl->objects_cap * sizeof(stl_obj));
}

// Push to stl's objects member.
void pushto_lvl_objects(stl *const lvl, stl_obj *obj) {
	if (lvl->objects_len == lvl->objects_cap) {
		assert(lvl->objects_cap < SIZE_MAX / 2);
		lvl->objects_cap *= 2;
		lvl->objects = nnrealloc(lvl->objects,
			lvl->objects_cap * sizeof(stl_obj));
	}
	(lvl->objects)[lvl->objects_len++] = *obj;
}

// Parse past a '(' in search of an stl_obj. Return true on success, else marks
// the object as invalid and returns false.
bool parse_paren(stl_obj *const obj,
	const char **section, size_t *const section_len) {
	(*section_len)--;
	if (*(*section)++ == '(')
		return true;
	obj->type = STL_INVALID_OBJ;
	return false;
}

// Return the length of n written out as an ASCII string. Works on 0 < n <= 9999
int intAsStrLen(int n) {
	assert(n > 0 && n <= 9999);
	if (n > 999)
		return 4;
	else if (n > 99)
		return 3;
	else if (n > 9)
		return 2;
	else
		return 1;
}

// Helper for get_stl_obj. Scans for a ([x|y] [0-9]+) and returns true or else
// marks the object as invalid and returns false.
bool scanForObjComponent(stl_obj *const obj, const char *x_or_y,
	const char **section, size_t *const section_len) {
	assert(*x_or_y == 'x' || *x_or_y == 'y');
	
	if (!parse_paren(obj, section, section_len))
		return false;
	int n;
	if (!nextWordIs(x_or_y, section, section_len) ||
		1 != sscanf(*section, "%d", &n)) {
		obj->type = STL_INVALID_OBJ;
		return false;
	}
	if (n < 0)
		n = 0;
	if (*x_or_y == 'x')
		obj->x = n;
	else if (*x_or_y == 'y')
		obj->y = n;
	int nAsStrLen = intAsStrLen(n);
	*section += nAsStrLen;
	*section_len -= nAsStrLen;
	(*section_len)--;
	if (*(*section)++ != ')') {
		obj->type = STL_INVALID_OBJ;
		return false;
	}
	return true;
}

// Helper for getSTLobj. On success, true is returned and *section is moved
// past the rest of the object.
bool consumeRestOfObj(stl_obj *const obj, const enum stl_obj_type type,
	const char **section, size_t *const section_len) {
	obj->type = type;
	if (!scanForObjComponent(obj, "x", section, section_len))
		return false;
	trimWhitespace(section, section_len);
	if (!scanForObjComponent(obj, "y", section, section_len))
		return false;
	trimWhitespace(section, section_len);
	(*section_len)--;
	if (*(*section)++ != ')') {
		obj->type = STL_INVALID_OBJ;
		return false;
	}
	return true;
}

// Return a populated stl_obj. If a well-formed object is found, *section is 
// moved past the object.
stl_obj getSTLobj(const char **section, size_t *const section_len) {
	stl_obj obj = { 0 };
	obj.type = STL_INVALID_OBJ;  // type is not known yet
	
	trimWhitespace(section, section_len);
	if (**section == ')') {
		obj.type = STL_NO_MORE_OBJ;
		return obj;
	}
	if (!parse_paren(&obj, section, section_len))
		return obj;
	if (nextWordIs("snowball", section, section_len)) {
		consumeRestOfObj(&obj, SNOWBALL, section, section_len);
	} else if (nextWordIs("mriceblock", section, section_len)) {
		consumeRestOfObj(&obj, MRICEBLOCK, section, section_len);
	} else if (nextWordIs("mrbomb", section, section_len)) {
		consumeRestOfObj(&obj, MRBOMB, section, section_len);
	} else if (nextWordIs("stalactite", section, section_len)) {
		consumeRestOfObj(&obj, STALACTITE, section, section_len);
	} else if (nextWordIs("bouncingsnowball", section, section_len)) {
		consumeRestOfObj(&obj, BOUNCINGSNOWBALL, section, section_len);
	} else if (nextWordIs("flyingsnowball", section, section_len)) {
		consumeRestOfObj(&obj, FLYINGSNOWBALL, section, section_len);
	} else if (nextWordIs("money", section, section_len)) {
		consumeRestOfObj(&obj, MONEY, section, section_len);
	}
	return obj;
}

// Debugging function. Print a TM to stderr.
void printTM(uint8_t **const tm, const int width,
	const int height) {
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++)
			fprintf(stderr, "%.3d ", tm[h][w]);
		fprintf(stderr, "\n");
	}
}

// Parse a TM using *section to fill *ptm with char values.
bool parseTM(uint8_t ***ptm, const int width, const int height,
	const char **section, size_t *const section_len) {
	*ptm = nnmalloc(height * sizeof(uint8_t *));
	for (int h = 0; h < height; h++) {
		(*ptm)[h] = nnmalloc(width * sizeof(uint8_t));
		memset((*ptm)[h], 0x00, width * sizeof(uint8_t));
	}
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++) {
			trimWhitespace(section, section_len);
			int ch;
			if (1 != sscanf(*section, "%u", &ch) || ch > 255)
				return false;
			(*ptm)[h][w] = (uint8_t)ch;
			
			int lengthOfchAsAStr = 1;
			if (ch > 99)
				lengthOfchAsAStr = 3;
			else if (ch > 9)
				lengthOfchAsAStr = 2;
			*section_len -= lengthOfchAsAStr;
			
			*section += lengthOfchAsAStr;
		}
	//printTM(*ptm, width, height);
	return true;
}

// levelReader helper. Read a word from *section and write it to *destination.
bool writeStrTo(char **const destination, const char **section,
	size_t *const section_len) {
	if (*(*section)++ != '"')
		return false;
	else
		(*section_len)--;
	size_t dest_len = 0;
	while (*section_len > 0 && **section != '"') {
		dest_len++;
		(*section)++;
		(*section_len)--;
	}
	if (*section_len == 0)
		return false;
	*destination = nnmalloc(dest_len + 1);
	strncpy(*destination, *section - dest_len,
		dest_len);
	(*destination)[dest_len] = '\0';
	if (*(*section)++ != '"')
		return false;
	trimWhitespace(section, section_len);
	if (*(*section) != ')')
		return false;
	return true;
}

// levelReader helper. Move section past the word, and trim any whitespace.
// Return true on success. Parse failure leaves section[_len] unmodified.
bool nextWordIs(const char *const word, const char **section,
	size_t *const section_len) {
	if (*section_len < strlen(word) ||
		0 != strncmp(*section, word, strlen(word)) ||
		!isWhitespace(*(*section + strlen(word))))
		return false;
	
	*section += strlen(word);
	*section_len -= strlen(word);
	trimWhitespace(section, section_len);
	return true;
}
