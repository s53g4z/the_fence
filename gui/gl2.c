#include "probablyegl.h"
#include "core.h"

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 300;

typedef int Direction;
enum { DIRECTION_HORIZ, DIRECTION_VERT };

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

// Return an array of WorldItems w is colliding with. The caller must free rv
// (but not the pointers stored in rv).
WorldItem **isCollidingWith(WorldItem *const w, size_t *const collisions_len) {
	assert(w->x >= INT_MIN + 1 && w->y  >= INT_MIN + 1);
	w->x--;
	w->y--;
	assert(w->width <= INT_MAX - 2 && w->height <= INT_MAX - 2);
	w->width += 2;
	w->height += 2;
	WorldItem **rv = malloc(sizeof(WorldItem *) * 1);
	*collisions_len = 0;
	size_t collisions_capacity = 1;
	for (size_t i = 0; i < worldItems_len; i++) {
		if (isCollidingHorizontally(w, worldItems[i]) &&
			isCollidingVertically(w, worldItems[i])) {
			if (*collisions_len == collisions_capacity) {
				assert(collisions_capacity <= SIZE_MAX / 2);
				collisions_capacity *= 2;
				WorldItem **newrv = nnrealloc(rv, sizeof(WorldItem *) *
					collisions_capacity);
				rv = newrv;
			}
			worldItems[(*collisions_len)++] = worldItems[i];
		}
	}
	w->x++;
	w->y++;
	w->width -= 2;
	w->height -= 2;
	return rv;
}

int canMoveX(WorldItem *const w) {
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
		size_t collisions_len;
		WorldItem **collisions = isCollidingWith(w, &collisions_len);
		bool shouldBreak = false;
		// calculate if should break
		for (size_t i = 0; i < collisions_len; i++) {
			if ((w->speedX >= 0 && rightOf(w) + 1 == leftOf(collisions[i])) ||
				(w->speedX <= 0 && leftOf(w) - 1 == rightOf(collisions[i]))) {
				shouldBreak = true;
				break;
			}
		}
		free(collisions);
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

int canMoveY(WorldItem *const w) {
	int canMove = 0;
	for (size_t i = 0; i < fabs(w->speedY); i++) {
		
	}
	return 0;
}

static int canMoveTo(WorldItem *const w, const Direction dir) {
	if (dir == DIRECTION_HORIZ)
		return canMoveX(w);
	else if (dir == DIRECTION_VERT)
		return canMoveY(w);
	return 0;
}

static void processInput(const keys *const k) {
	assert(k && SCREEN_WIDTH && SCREEN_HEIGHT);
	
	if (k->keyD) {
		player->speedX = fabs(player->speedX);
		player->x += canMoveTo(player, DIRECTION_HORIZ);
	} else if (k->keyA) {
		assert(player->speedX != INT_MIN);
		if (player->speedX > 0)
			player->speedX *= -1;
		player->x += canMoveTo(player, DIRECTION_HORIZ);
	}
	fprintf(stderr, "Player is at %d, %d\n", player->x, player->y);
}

static void applyGravity() {
	
}

// Opposite of initialize().
void terminate() {
	for (size_t i = 0; i < worldItems_len; i++)
		free(worldItems[i]);
	free(worldItems);
}

static void initialize() {
	worldItems = nnmalloc(sizeof(WorldItem *) * 1);  // space for 1 WorldItem *
	worldItems[0] = worldItem_new(0, 0, 32, 32, 5, 0, NULL);
	
	player = worldItems[0];
	
}

static void core(keys *const k, glsh *const sh) {
	assert(k && sh);
	static bool initialized = false;
	if (!initialized) {
		initialize();
		initialized = true;
	}
	
	processInput(k);
	applyGravity();
}

bool draw(keys *const k, glsh *sh) {
	assert(k && sh);
	core(k, sh);
	return true;
}
