#include <stdio.h>
#include <string.h>
#include <input.h>

typedef unsigned char byte;

#define K_UP       'Q'
#define K_DOWN     'A'
#define K_LEFT     'O'
#define K_RIGHT    'P'
#define K_RESET    'R'

enum
{
	UP,
	RIGHT,
	DOWN,
	LEFT,
	RESET
};


char *display;
extern int d_file @16396;

byte undrawList[16];

byte snake[16] = {0};
byte snakeHead, snakeTail, snakeFace, snakeLen;

int tileMap[] = {
	0,0,0,0,
	0x8a,0x8a,0x80,0x80,
	0x08,0x08,0x08,0x08,
	0x80,0x80,0x80,0x80,
	0x9b,0x82,0x80,0x80,
	0x80,0x9b,0x80,0x07,
	0x80,0x80,0x9b,0x07,
	0x9b,0x80,0x84,0x80,
	0x81,0x82,0x84,0x07
};

byte map[16*12];
byte gameMap[16*12] = {
	1,0,0,0,8,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,8,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


void putBlock(int tile, int cellId) {
	int cellX = cellId & 15;
	int cellY = cellId / 16;
	char* cell = display + (cellX * 2) + (cellY * 66);
	int ci = tile * 4;

	cell[0] = tileMap[ci];
	++ci;
	cell[1] = tileMap[ci];
	++ci;
	cell[33] = tileMap[ci];
	++ci;
	cell[34] = tileMap[ci];
}

void renderMap() {
	for (int i = 0; i < 16*12; ++i) {
		putBlock(map[i], i);
	}
}


void renderSnake() {
	int i = 0;
	while (undrawList[i] != 0xff) {
		putBlock(0, undrawList[i]);
		++i;
	}

	i = snakeHead;
	putBlock(4 + snakeFace, snake[i]);
	i = (i - 1) & 15;

	int n = snakeLen - 1;
	int m = 0;
	while (n) {
		putBlock(2 + (m & 1), snake[i]);
		i = (i - 1) & 15;
		--n;
		++m;
	}
}

int tryMove(int newDirn) {
	int dirs[] = {
		-16,1,16,-1
	};

	int newMapPos = snake[snakeHead] + dirs[newDirn];
	if (map[newMapPos] != 0 && map[newMapPos] != 8)
		return 0;

	for (int i = snakeHead, j = 0; j < snakeLen; ++j) {
		if (snake[i] == newMapPos)
			return 0;
		i = (i + 1) & 15;
	}

	snakeHead = (snakeHead + 1) & 15;
	snake[snakeHead] = newMapPos;

	if (map[newMapPos] == 0) {
		// update tail if not moving on to fruit
		undrawList[0] = snake[snakeTail];
		undrawList[1] = 0xff;
		snakeTail = (snakeTail + 1) & 15;
	} else {
		++snakeLen;
		map[newMapPos] = 0;
	}

	snakeFace = newDirn;

	return 1;
}

int checkFall() {
	// if none of the segments are supported then move them all down
	for (int i = 0, n = snakeTail; i < snakeLen; ++i) {
		if (map[snake[n] + 16] != 0)
			return 0;
		n = (n + 1) & 15;
	}

	for (int i = 0, n = snakeTail; i < snakeLen; ++i) {
		undrawList[i] = snake[n];
		undrawList[i+1] = 0xff;
		snake[n] = snake[n] + 16;
		n = (n + 1) & 15;
	}

	return 1;
}


int reset() {
	snake[0] = 6+16*6;
	snake[1] = 7+16*6;
	snake[2] = 8+16*6;
	snakeHead=2;
	snakeTail=0;
	snakeLen = 3;
	snakeFace = 1; // U = 0, R, D, L
	memcpy(map, gameMap, 16 * 12);
	renderMap();
}

int getMove()
{
	int k = in_Inkey();
	while (k != 0)
	{
		k = in_Inkey();
	}

	while(1)
	{
		k = in_Inkey();

		if (k == K_UP) return tryMove(UP);
		if (k == K_DOWN) return tryMove(DOWN);
		if (k == K_LEFT) return tryMove(LEFT);
		if (k == K_RIGHT) return tryMove(RIGHT);
		if (k == K_RESET) return renderMap();
	}
}

void main()
{
	display = d_file+1;

	reset();
	undrawList[0] = 0xff;
	while (1) {
		renderSnake();
		if (checkFall()) {
			continue;
		}
		getMove();
	}
}
