#include "probablyegl.h"
#include "core.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
int gScrollOffset = 0;

typedef int Direction;
enum { LEFT, RIGHT, UP, DOWN, GDIRECTION_HORIZ, GDIRECTION_VERT };
typedef int GeneralDirection;

WorldItem **worldItems;  // array of pointers to WorldItem
size_t worldItems_len, worldItems_cap;

static stl lvl;

Player *player;

// Return true if c is between a and b.
bool isBetween(const int c, const int a, const int b) {
	assert(a <= b);
	return (a <= c && c <= b);
}

bool isCollidingHorizontally(const WorldItem *const w,
	const WorldItem *const v) {
	return v != w && (
		isBetween(topOf(v), topOf(w), bottomOf(w)) ||
		isBetween(bottomOf(v), topOf(w), bottomOf(w)) ||
		(topOf(w) >= topOf(v) && bottomOf(w) <= bottomOf(v)) ||
		(topOf(w) <= topOf(v) && bottomOf(w) >= bottomOf(v))
	);
}

bool isCollidingVertically(const WorldItem *const w,
	const WorldItem *const v) {
	return v != w && (
		isBetween(leftOf(v), leftOf(w), rightOf(w)) ||
		isBetween(rightOf(v), leftOf(w), rightOf(w)) ||
		(leftOf(w) >= leftOf(v) && rightOf(w) <= rightOf(v)) ||
		(leftOf(w) <= leftOf(v) && rightOf(w) >= rightOf(v))
	);
}

// iCW helper. Expand w in either the horizontal or vertical direction.
void iCW_expand_w(WorldItem *const w, GeneralDirection dir) {
	assert(w->x >= INT_MIN + 1 && w->y  >= INT_MIN + 1);
	if (dir == GDIRECTION_HORIZ)
		w->x--;
	else
		w->y--;
	assert(w->width <= INT_MAX - 2 && w->height <= INT_MAX - 2);
	if (dir == GDIRECTION_HORIZ)
		w->width += 2;
	else
		w->height += 2;
}

// iCW helper. Shrink w in either the horizontal or vertical direction.
void iCW_shrink_w(WorldItem *const w, GeneralDirection dir) {
	if (dir == GDIRECTION_HORIZ)
		w->x++;
	else
		w->y++;
	if (dir == GDIRECTION_HORIZ)
		w->width -= 2;
	else
		w->height -= 2;
}

// Return an array of WorldItems w is colliding with. The caller must free rv
// (but not the pointers stored in rv).
WorldItem **isCollidingWith(WorldItem *const w, size_t *const collisions_len,
	GeneralDirection gdir) {
	assert(gdir == GDIRECTION_HORIZ || gdir == GDIRECTION_VERT);
	iCW_expand_w(w, gdir);
	WorldItem **rv = malloc(sizeof(WorldItem *) * 1);
	*collisions_len = 0;
	size_t collisions_capacity = 1;
	for (size_t i = 0; i < worldItems_len; i++) {
		if (!isCollidingHorizontally(w, worldItems[i]) ||
			!isCollidingVertically(w, worldItems[i]))
			continue;
		
		if (*collisions_len == collisions_capacity) {
			assert(collisions_capacity <= SIZE_MAX / 2);
			collisions_capacity *= 2;
			WorldItem **newrv = nnrealloc(rv, sizeof(WorldItem *) *
				collisions_capacity);
			rv = newrv;
		}
		rv[(*collisions_len)++] = worldItems[i];
	}
	iCW_shrink_w(w, gdir);
	return rv;
}

// Helper for canMoveTo.
int canMoveX(WorldItem *const w) {
	if (w->speedX == 0)
		return 0;
	int canMove = 0;
	if (w->speedX > 0)
		assert(w->x <= INT_MAX - w->speedX);
	else
		assert(w->x >= INT_MIN - w->speedX);
	for (size_t i = 0; i < fabs(w->speedX); i++) {
		float origX = w->x;
		if (w->speedX > 0)
			w->x += i;
		else
			w->x -= i;
		bool shouldBreak = false;
		//if ((w->speedX < 0 && leftOf(w) - 1 == 0) || (w->speedX > 0 &&
		//	rightOf(w) + 1 == SCREEN_WIDTH))
		//	shouldBreak = true;
		if (w == player && ((w->speedX < 0 && leftOf(w) - 1 == 0) ||
			(gScrollOffset + SCREEN_WIDTH >= lvl.width * 32 &&
			w->speedX > 0 && rightOf(w) + 1 == SCREEN_WIDTH)))
			shouldBreak = true;
		else {
			size_t collisions_len;
			WorldItem **colls = isCollidingWith(w, &collisions_len,
				GDIRECTION_HORIZ);
			for (size_t i = 0; i < collisions_len; i++) {
				if ((w->speedX > 0 && rightOf(w) + 1 == leftOf(colls[i])) ||
					(w->speedX < 0 && leftOf(w) - 1 == rightOf(colls[i]))) {
					shouldBreak = true;
					break;
				}
			}
			free(colls);
		}
		w->x = origX;
		if (shouldBreak)
			break;
		if (w->speedX > 0)
			canMove++;
		else
			canMove--;
	}
	return canMove;
}

// Helper for canMoveTo.
int canMoveY(WorldItem *const w) {
	if (w->speedY == 0)
		return 0;
	int canMove = 0;  // todo: add overflow assertions
	for (size_t i = 0; i < fabs(w->speedY); i++) {
		int origY = w->y;
		if (w->speedY > 0)
			w->y += i;
		else
			w->y -= i;
		bool shouldBreak = false;
		if (w->speedY > 0 && bottomOf(w) + 1 == SCREEN_HEIGHT) {
			shouldBreak = true;
		//} else if (w->speedY < 0 && topOf(w) - 1 == 0) {
		//	w->speedY *= -1;  // bonk
		//	shouldBreak = true;
		} else {
			size_t collisions_len;
			WorldItem **colls = isCollidingWith(w, &collisions_len,
				GDIRECTION_VERT);
			for (size_t i = 0; i < collisions_len; i++) {
				if (w->speedY > 0 && bottomOf(w) + 1 == topOf(colls[i])) {
					shouldBreak = true;
					break;
				} else if (w->speedY < 0 &&
					topOf(w) - 1 == bottomOf(colls[i])) {
					w->speedY *= -1;  // bonk
					shouldBreak = true;
					break;
				}
			}
			free(colls);
		}
		w->y = origY;
		if (shouldBreak)
			break;
		if (w->speedY > 0)
			canMove++;
		else
			canMove--;
	}
	return canMove;
}

static int canMoveTo(WorldItem *const w, const GeneralDirection dir) {
	assert(dir == GDIRECTION_HORIZ || dir == GDIRECTION_VERT);
	if (dir == GDIRECTION_HORIZ)
		return canMoveX(w);
	else //if (dir == GDIRECTION_VERT)
		return canMoveY(w);
	//assert(NULL);
	//return 0;
}

static void player_toggle_size() {
	if (player->height == 32) {
		player->height = 64;
		player->y -= 32;
	} else {
		player->height = 32;
		player->y += 32;
	}
}

void deletefrom_worldItems(int index) {
	assert(index > 0 && (size_t)index < worldItems_len);
	free(worldItems[index]);
	memmove(&(worldItems[index]), &(worldItems[index+1]),
		(worldItems_len - index - 1) * sizeof(WorldItem *));
	worldItems_len--;
}

void maybeScrollScreen() {
	if (player->x <= SCREEN_WIDTH / 3 ||
		gScrollOffset + SCREEN_WIDTH >= lvl.width * 32)
		return;
	
	int diff = player->x - SCREEN_WIDTH / 3;
	for (size_t i = 0; i < worldItems_len; i++) {
		WorldItem *const w = worldItems[i];
		w->x -= diff;
	}
	gScrollOffset += diff;
	for (size_t i = 0; i < worldItems_len; i++) {
		const WorldItem *const w = worldItems[i];
		if (w->x < -100)
			deletefrom_worldItems(i--);
	}
}

static void processInput(const keys *const k) {
	assert(k && SCREEN_WIDTH && SCREEN_HEIGHT);
	
	if (k->keyD) {
		player->speedX = fabs(player->speedX);
		player->x += canMoveTo(player, GDIRECTION_HORIZ);
	}
	if (k->keyA) {
		assert(player->speedX != INT_MIN);
		if (player->speedX > 0)
			player->speedX *= -1;
		player->x += canMoveTo(player, GDIRECTION_HORIZ);
	}
	if (k->keyW)
		player->speedY = -10;
	if (k->keyR)
		player_toggle_size();
	
	maybeScrollScreen();
}

// Run the frame functions of elements of worldItems.
static void applyFrame() {
	for (size_t i = 0; i < worldItems_len; i++)
		worldItems[i]->frame(worldItems[i]);
}

static void applyGravity() {
	for (size_t i = 0; i < worldItems_len; i++) {
		WorldItem *const w = worldItems[i];
		if (w->gravity == false)  // not affected by gravity
			continue;
		int moveY = canMoveTo(w, GDIRECTION_VERT);
		if (moveY == 0)
			w->speedY = 1;
		else {
			w->speedY++;
			//w->speedY *= 2;
			w->y += moveY;
		}
	}
}

// Opposite of initialize().
void terminate() {
	for (size_t i = 0; i < worldItems_len; i++)
		free(worldItems[i]);
	free(worldItems);
	lrFailCleanup(NULL, &lvl);
}

static uint32_t prgm;
static uint32_t vtx_shdr;
static uint32_t frag_shdr;

static void printShaderLog(uint32_t shdr) {
	int log_len;
	glGetShaderiv(shdr, GL_INFO_LOG_LENGTH, &log_len);
	if (log_len > 0) {
		char *log = nnmalloc(log_len);
		glGetShaderInfoLog(shdr, log_len, NULL, log);
		fprintf(stderr, "SHDR_LOG: %s\n", log);
		free(log);
		//assert(NULL);
	}
}

static void initialize_prgm() {
	prgm = glCreateProgram();
	assert(prgm);
	vtx_shdr = glCreateShader(GL_VERTEX_SHADER);
	frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
	assert(vtx_shdr && frag_shdr);
	ssize_t src_len;
	char *const vtx_src = safe_read("shaders/vtx3.txt", &src_len);
	glShaderSource(vtx_shdr, 1, (const char *const *)&vtx_src, (int *)&src_len);
	free(vtx_src);
	char *const frag_src = safe_read("shaders/frag3.txt", &src_len);
	glShaderSource(frag_shdr, 1, (const char *const *)&frag_src,
		(int *)&src_len);
	free(frag_src);
	glAttachShader(prgm, vtx_shdr);
	glAttachShader(prgm, frag_shdr);
	
	glCompileShader(vtx_shdr);
	glCompileShader(frag_shdr);
	printShaderLog(vtx_shdr);
	printShaderLog(frag_shdr);
	
	glLinkProgram(prgm);
	glUseProgram(prgm);
	if (glGetError() != GL_NO_ERROR) {
		int32_t log_len;
		glGetProgramiv(prgm, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len) {
			char *log = nnmalloc(log_len);
			glGetProgramInfoLog(prgm, log_len, NULL, log);
			fprintf(stderr, "PRGM_LOG: %s\n", log);
			free(log);
			assert(NULL);
		}
	}
}

// Callback for doing nothing.
void fnret(WorldItem *self) {
	assert(self);
	return;
}

// Callback for player frame.
void fnpl(WorldItem *self) {
	// do nothing
	assert(self);
	return;
}

// Callback for bot frame.
void fnbot(WorldItem *const self) {
	int moveX = canMoveTo(self, GDIRECTION_HORIZ);
	if (moveX == 0) {
		self->speedX *= -1;  // toggle horizontal direction
		uint32_t tmp = self->texnam;
		self->texnam = self->texnam2;
		self->texnam2 = tmp;
	}
	else
		self->x += moveX;
}

// Push a WorldItem onto worldItems. (auto-inits and grows the array)
void pushto_worldItems(const WorldItem *const v) {
	WorldItem *const w = (WorldItem *const)v;
	if (worldItems_cap == 0) {  // uninitialized
		worldItems_cap = 1;
		worldItems = nnmalloc(worldItems_cap * sizeof(WorldItem *));
	}
	if (worldItems_len == worldItems_cap) {  // need to double capacity
		assert(worldItems_cap <= SIZE_MAX / 2);
		worldItems_cap *= 2;
		worldItems = nnrealloc(worldItems, worldItems_cap * sizeof(WorldItem *));
	}
	worldItems[worldItems_len++] = w;
}

static uint32_t gTextureNames[256];

void must(unsigned long long condition) {
	if (!condition)
		exit(1);
}

// Mirror a 64 * 64 * 3 array of {char r, g, b}.
void mirrorTexelImg(void *imgmem) {
	struct texel {
		char r, g, b;
	};
	struct texel *img = (struct texel *)imgmem;
	for (int height = 0; height < 64; height++)
		for (int width = 0; width < 32; width++) {
			struct texel tmp = img[height * 64 + width];
			img[height * 64 + width] = img[height * 64 + 64 - width - 1];
			img[height * 64 + 64 - width - 1] = tmp;
		}
}

// Upload the file specified by imgnam to the texture specified by texnam to
// make texnam usable by the GL.
void initGLTextureNam(const uint32_t texnam, const char *const imgnam,
	bool mirror) {
	ssize_t has_read;
	char *imgmem = safe_read(imgnam, &has_read);
	assert(has_read == 64 * 64 * 3);
	if (mirror) {  // flip-flop the image
		mirrorTexelImg(imgmem);
	}
	// Do NOT switch the active texture unit!
	// See https://web.archive.org/web/20210905013830/https://users.cs.jmu.edu/b
	//     ernstdh/web/common/lectures/summary_opengl-texture-mapping.php
	//glActiveTexture(GL_TEXTURE0 + w->texunit);  bug
	glBindTexture(GL_TEXTURE_2D, texnam);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		64,
		64,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		imgmem
	);
	free(imgmem);
	assert(glGetError() == GL_NO_ERROR);
}

static void maybeInitgTextureNames() {
	static bool ran = false;
	if (ran)
		return;
	ran = true;
	
	glGenTextures(256, gTextureNames);
	assert(glGetError() == GL_NO_ERROR);
	
	// todo: connect the rest of the valid texturename indexes to files
	initGLTextureNam(gTextureNames[10], "textures/snow4.data", false);
	initGLTextureNam(gTextureNames[11], "textures/snow5.data", false);
	initGLTextureNam(gTextureNames[12], "textures/snow6.data", false);
	initGLTextureNam(gTextureNames[13], "textures/snow7.data", false);
	initGLTextureNam(gTextureNames[15], "textures/snow9.data", false);
	initGLTextureNam(gTextureNames[14], "textures/snow8.data", false);
	initGLTextureNam(gTextureNames[20], "textures/snow14.data", false);
	initGLTextureNam(gTextureNames[21], "textures/snow15.data", false);
	initGLTextureNam(gTextureNames[22], "textures/snow16.data", false);
	initGLTextureNam(gTextureNames[23], "textures/snow17.data", false);
	initGLTextureNam(gTextureNames[25], "textures/texture2.rgb", false);
	initGLTextureNam(gTextureNames[26], "textures/bonus2.data", false);
	initGLTextureNam(gTextureNames[44], "textures/coin1.data", false);
	initGLTextureNam(gTextureNames[77], "textures/brick0.data", false);
	assert(glGetError() == GL_NO_ERROR);
}

void drawGLvertices(const float *const vertices, const uint32_t texnam);

// Draw a tile?
static void paintTile(uint8_t tileID, int x, int y) {
	if (x < -100) {
		fprintf(stderr, "skipping a paintTile\n");
		return;
	}
	maybeInitgTextureNames();
	
	const float vertices[] = {
		x+00, y+00, 1.0,
		x+00, y+32, 1.0,
		x+32, y+00, 1.0,
		x+32, y+32, 1.0,
	};
	
	return drawGLvertices(vertices, gTextureNames[tileID]);
}

// Handy convenience function to make a new block.
const WorldItem *worldItem_new_block(int x, int y) {
	return worldItem_new(x, y, 32, 32, 0, 0, false, NULL, fnret, false);
}

// Draw some nice (non-interactive) scenery.
void paintTM(uint8_t **tm) {
	const size_t nTilesScrolledOver = gScrollOffset / 32;
	const bool playerIsBetweenTiles = gScrollOffset % 32 != 0 &&
		gScrollOffset + SCREEN_WIDTH < lvl.width * 32 ? 1 : 0;
	for (int h = 0; h < 480 / 32; h++)
		for (size_t w = nTilesScrolledOver;
			w < nTilesScrolledOver + 640 / 32 + playerIsBetweenTiles;
			w++) {
			int x = w * 32 - gScrollOffset;  // screen coordinates
			int y = h * 32;  // ibid
			paintTile(tm[h][w], x, y);
		}
}

static void initialize() {
	pushto_worldItems(worldItem_new(0, 0, 32, 32, 9, 1, true,
		"textures/texture3.rgb", fnpl, false));
	player = worldItems[0];
	
	initialize_prgm();
	
	lvl = levelReader("level1.stl");
	if (lvl.hdr) {
		stlPrinter(&lvl);
		if (lvl.height != 15)
			fprintf(stderr, "WARN: lvl.height is unrecognized (%d)\n", lvl.height);
		for (size_t i = 0; i < lvl.objects_len; i++) {
			WorldItem *w = NULL;
			stl_obj *obj = &lvl.objects[i];
			if (obj->type == SNOWBALL) {
				w = worldItem_new(obj->x, obj->y, 32, 32, -3, 1, true,
					"textures/snowball.data", fnbot, true);  // debug obj->x
			} else
				continue;
			pushto_worldItems(w);
		}
		// Load in the interactives all at once, one time.
		// (Painting for interactives happens elsewhere, repeatedly.)
		for (int h = 0; h < lvl.height; h++)
			for (int w = 0; w < lvl.width; w++) {
				uint8_t tileID = lvl.interactivetm[h][w];
				int x = w * 32 - gScrollOffset;  // screen coordinates
				int y = h * 32;  // ibid
				if (tileID == 14 || tileID == 22 || tileID == 26 || tileID == 77 ||
					tileID == 83 || tileID == 102 || tileID == 23 || tileID == 20 ||
					tileID == 21 || tileID == 13 || tileID == 15 || tileID == 10 ||
					tileID == 11 || tileID == 12)
					pushto_worldItems(worldItem_new_block(x, y));
				else if (tileID == 44) {
					//const WorldItem *const w = worldItem_new(
					//pushto_worldItems(w);  // todo: 44 is a coin
				}
			}
	}
}

// Return true if w is off-screen.
bool isOffscreen(const WorldItem *const w) {
	int top, bottom, left, right;
	
	top = w->y;
	bottom = w->y + w->height;
	left = w->x;
	right = w->x + w->width;
	
	return (top > SCREEN_HEIGHT || bottom < 0 ||
		left > SCREEN_WIDTH || right < 0);
}

void drawGLvertices(const float *const vertices, const uint32_t texnam) {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(0);
	glBindAttribLocation(prgm, 0, "vertices");
	
	static const float tcoords[] = {
		0.001, 0.001,
		0.001, 0.999,
		0.999, 0.001,
		0.999, 0.999,
	};
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tcoords);
	glEnableVertexAttribArray(1);
	glBindAttribLocation(prgm, 1, "tcoords");
	assert(glGetError() == GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_2D, texnam);  // ???

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	must(glGetError() == GL_NO_ERROR);
}

static void drawWorldItems() {
	for (size_t i = 0; i < worldItems_len; i++) {
		const WorldItem *const w = worldItems[i];
		assert(w);
		if (isOffscreen(w) || w->texnam == (uint32_t)-1)
			continue;
		
		const float vertices[] = {
			0+w->x, 0+w->y, 1.0,
			0+w->x, w->y+w->height, 1.0,
			w->x+w->width, 0+w->y, 1.0,
			w->x+w->width, w->y+w->height, 1.0,
		};
		drawGLvertices(vertices, w->texnam);
	}
}

static void clearScreen() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void core(keys *const k, glsh *const sh) {
	assert(k && sh);
	static bool initialized = false;
	if (!initialized) {
		initialize();
		initialized = true;
	}
	
	processInput(k);
	applyFrame();
	applyGravity();
	
	clearScreen();
	paintTM(lvl.backgroundtm);
	paintTM(lvl.interactivetm);
	drawWorldItems();
}

bool draw(keys *const k, glsh *sh) {
//	assert(k && sh);
	core(k, sh);
	return true;
}
