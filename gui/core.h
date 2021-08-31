//

#ifndef CORE_H
#define CORE_H

struct WorldItem {
	int x, y;
	int width, height;
	float speedX, speedY;
	void *img;
};
typedef struct WorldItem WorldItem;

struct Point {
	int x, y;
};
typedef struct Point Point;

typedef WorldItem Player;

void *nnmalloc(size_t sz);
void *nnrealloc(void *, size_t);
WorldItem *worldItem_new(int, int, int, int, float, float, void *);
int leftOf(const WorldItem *const);
int rightOf(const WorldItem *const);
int topOf(const WorldItem *const);
int bottomOf(const WorldItem *const);

#endif
