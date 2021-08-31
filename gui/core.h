//

#ifndef CORE_H
#define CORE_H

struct WorldItem {
	int x, y;
	int width, height;
	float speedX, speedY;
//	int texunit;
	uint32_t texnam;
	bool gravity;
	void (*frame)(struct WorldItem *const w);
};
typedef struct WorldItem WorldItem;

struct Point {
	int x, y;
};
typedef struct Point Point;

typedef WorldItem Player;

void *nnmalloc(size_t sz);
void *nnrealloc(void *, size_t);
WorldItem *worldItem_new(int, int, int, int, float, float, bool, char *,
	void (*)(WorldItem *const));
int leftOf(const WorldItem *const);
int rightOf(const WorldItem *const);
int topOf(const WorldItem *const);
int bottomOf(const WorldItem *const);

#endif
