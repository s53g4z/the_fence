#include "probablyegl.h"
#include "core.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef int Direction;
enum { LEFT, RIGHT, UP, DOWN, };
typedef int GeneralDirection;
enum { GDIRECTION_HORIZ, GDIRECTION_VERT, };

WorldItem **worldItems;  // array of pointers to WorldItem
size_t worldItems_len;

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
		if ((w->speedX < 0 && leftOf(w) - 1 == 0) || (w->speedX > 0 &&
			rightOf(w) + 1 == SCREEN_WIDTH))
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
			shouldBreak = true;;
		} else if (w->speedY < 0 && topOf(w) - 1 == 0) {
			w->speedY *= -1;  // bonk
			shouldBreak = true;
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
	if (dir == GDIRECTION_HORIZ)
		return canMoveX(w);
	else if (dir == GDIRECTION_VERT)
		return canMoveY(w);
	assert(NULL);
	return 0;
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
	//fprintf(stderr, "Player is at %d, %d\n", player->x, player->y);
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

// Callback for player frame.
void fnpl(WorldItem *self) {
	// do nothing
	assert(self);
	return;
}

// Callback for bot frame.
void fnbot(WorldItem *const self) {
	int moveX = canMoveTo(self, GDIRECTION_HORIZ);
	if (moveX == 0)
		self->speedX *= -1;  // toggle horizontal direction
	else
		self->x += moveX;
}

static void initialize() {
	worldItems_len = 2;
	worldItems = nnmalloc(sizeof(WorldItem *) * worldItems_len);
	worldItems[0] = worldItem_new(0, 0, 32, 32, 9, 1, true,
		"textures/texture3.rgb", fnpl);
	worldItems[1] = worldItem_new(50, 445, 32, 32, 5, 1, false,
		"textures/texture2.rgb", fnbot);
	
	player = worldItems[0];
	
	initialize_prgm();
}

static void drawWorldItems() {
	for (size_t i = 0; i < worldItems_len; i++) {
		const WorldItem *const w = worldItems[i];
		assert(w);
		
		float vertices[] = {
			0+w->x, 0+w->y, 1.0,
			0+w->x, w->y+w->height, 1.0,
			w->x+w->width, 0+w->y, 1.0,
			w->x+w->width, w->y+w->height, 1.0,
		};
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		glEnableVertexAttribArray(0);
		glBindAttribLocation(prgm, 0, "vertices");
		float tcoords[] = {
			0.01, 0.01,
			0.01, 0.99,
			0.99, 0.01,
			0.99, 0.99,
		};
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tcoords);
		glEnableVertexAttribArray(1);
		glBindAttribLocation(prgm, 1, "tcoords");
		assert(glGetError() == GL_NO_ERROR);
		
		//int32_t uniformLoc = glGetUniformLocation(prgm, "texture");
		//if (uniformLoc != -1)
		//	glUniform1i(uniformLoc, w->texnam);  // texture unit zero???
		//glActiveTexture(GL_TEXTURE0 + w->texunit);  // select active tex unit
		glBindTexture(GL_TEXTURE_2D, w->texnam);  // ???
		//glCompileShader(vtx_shdr);
		//glCompileShader(frag_shdr);
		//glLinkProgram(prgm);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		assert(glGetError() == GL_NO_ERROR);
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
	drawWorldItems();
}

bool draw(keys *const k, glsh *sh) {
//	assert(k && sh);
	core(k, sh);
	return true;
}
