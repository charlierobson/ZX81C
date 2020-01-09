// ZX81 SNAKEBIRD-ALIKE
//
// Styled as SNAKE81RD

#include <stdio.h>
#include <string.h>
#include <input.h>

typedef unsigned char byte;
typedef void(*UDFN)(void);

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

byte snake[16] = {0};
byte snakeHead, snakeTail, snakeFace, snakeLen, snakeDead;
byte fruit, doorOpen;
byte levelComplete;

enum {
	T_EMPTY,

	T_GROUND1,

	T_FRUIT1,
	T_DOORCLOSED,
	T_DOOROPEN,

	T_SNAKEBOD1,
	T_SNAKEBOD2,
	T_FACEUP,
	T_FACERIGHT,
	T_FACEDOWN,
	T_FACELEFT,
	T_DEADFACEUP,
	T_DEADFACERIGHT,
	T_DEADFACEDOWN,
	T_DEADFACELEFT,

	T_LAST
};

int tileMap[] = {
	0,0,0,0,

	0x8a,0x8a,0x80,0x80,	// ground

	0x81,0x82,0x84,0x07,	// 'fruit'
	0x87,0x04,0x02,0x01,	// door, closed
	0x07,0x84,0x82,0x81,	// door, open

	0x08,0x08,0x08,0x08,	// snakebod
	0x80,0x80,0x80,0x80,

	0x9b,0x82,0x80,0x80,	// face up
	0x80,0x9b,0x80,0x07,	// face right
	0x80,0x80,0x9b,0x07,	// face down
	0x9b,0x80,0x84,0x80,	// face left

	0xbd,0x82,0x80,0x80,	// face up dead
	0x80,0xbd,0x80,0x07,	// face right dead
	0x80,0x80,0xbd,0x07,	// face down dead
	0xbd,0x80,0x84,0x80,	// face left dead


};

#define F T_FRUIT1
#define D T_DOORCLOSED

byte map[16*12];
byte gameMap[16*12] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,F,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,F,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,D,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

UDFN updateFn;
int doUpdateFn() {
	if (updateFn) {
		updateFn();
	}
	return updateFn != NULL;
}

void putBlock(int cellId, int tile) {
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
		putBlock(i, map[i]);
	}
}

byte updateList[32];
byte updateListCount;

// update map and track which cells have changed for drawing
void updateMap(int index, byte value) {
	map[index] = value;

	// only render a cell once
	for (int i = 0; i < updateListCount; ++i) {
		if (updateList[i] == index)
			return;
	}

	updateList[updateListCount] = index;
	++updateListCount;
}


void render() {
	for (int i = 0; i < updateListCount; ++i) {
		byte cell = updateList[i];
		putBlock(cell, map[cell]);
	}
	updateListCount = 0;
}

void waiter(void) {
	--levelComplete;
	return 1;
}

void putSnakeInMap() {
	int x = snakeDead ? T_DEADFACEUP : T_FACEUP;
	updateMap(snake[snakeHead], snakeFace + x);
	for (int i = 1, n = snakeHead; i < snakeLen; ++i) {
		n = (n - 1) & 15;
		updateMap(snake[n], T_SNAKEBOD2 - (i & 1));
	}
}

void exiting(void) {
	int exited = 1;

	// collapse all segments onto the head, counting how many segments are in one place
	updateMap(snake[snakeTail], T_EMPTY);
	for (int i = 0; i < snakeLen - 1; ++i) {
		snake[(snakeTail + i) & 15] = snake[(snakeTail + 1 + i) & 15];
		if (snake[(snakeTail + i) & 15] == snake[snakeHead]) {
			++exited;
		}
	}

//	putSnakeInMap();

	if (exited == snakeLen) {
		levelComplete = 20;
		updateFn = waiter;
	}

	return 1;
}


byte traversable[3] = {
	T_EMPTY, T_FRUIT1, T_DOOROPEN
};
int isTraversible(byte blockType) {
	for (int i = 0; i < sizeof(traversable); ++i) {
		if (blockType == traversable[i]) return 1;
	}
	return 0;
}


int dirs[] = {
	-16,1,16,-1
};
int tryMove(int newDirn) {
	int newMapPos = snake[snakeHead] + dirs[newDirn];
	if (!isTraversible(map[newMapPos]))
		return 0;

	snakeFace = newDirn;
	snakeHead = (snakeHead + 1) & 15;
	snake[snakeHead] = newMapPos;

	if (map[newMapPos] != T_FRUIT1) {
		// update tail if not moving on to fruit
		updateMap(snake[snakeTail], T_EMPTY);
		snakeTail = (snakeTail + 1) & 15;
	}

	if (map[newMapPos] == T_FRUIT1) {
		// moved on to fruit, so grow
		++snakeLen;
		--fruit;
	} else if (map[newMapPos] == T_DOOROPEN)
		updateFn = exiting;

//	putSnakeInMap();

	return 1;
}

int checkFall() {
	for (int i = 0, n = snakeTail; i < snakeLen; ++i) {
		// if any segment would go off screen then die
		if (snake[n] + 16 >= 16*12) {
			snakeDead = 1;
			levelComplete = 50;
			updateFn = waiter;
			//putSnakeInMap();
			return 1;
		} 

		// if any segment is supported by something other than yourself then nothing to do
		if (gameMap[snake[n] + 16] && gameMap[snake[n] + 16] < T_SNAKEBOD1)
			return 0;

		n = (n + 1) & 15;
	}

	// none of the segments are supported so move them all down
	for (int i = 0, n = snakeTail; i < snakeLen; ++i) {
		updateMap(snake[n], T_EMPTY);
		snake[n] = snake[n] + 16;
		n = (n + 1) & 15;
	}

	//putSnakeInMap();

	return 1;
}

void countFruit() {
	fruit = 0;
	for (int i = 0; i < 16*12; ++i) {
		if (map[i] == T_FRUIT1) {
			++fruit;
		}
	}
}

void openDoor() {
	doorOpen = 1;
	for (int i = 0; i < 16*12; ++i) {
		if (map[i] == T_DOORCLOSED) {
			updateMap(i, T_DOOROPEN);
			return;
		}
	}
}

void debugDisplay() {

}

int reset() {
	snakeTail = 0;
	snake[0] = 6+16*7;
	snake[1] = 7+16*7;
	snake[2] = 8+16*7;
	snakeHead = 2;
	snakeLen = 3;
	snakeFace = 1;
	snakeDead = 0;

	doorOpen = 0;
	levelComplete = 0;

	updateFn = NULL;
	updateListCount = 0;

	memcpy(map, gameMap, 16 * 12);

	renderMap();
	countFruit();
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
		if (k == K_RESET) return reset();
	}
}


void main()
{
	display = d_file + 1;

	reset();
	while (levelComplete != 1) {
		putSnakeInMap();
		render();
		if (doUpdateFn()) {
			continue;
		}
		if (checkFall()) {
			continue;
		}
		getMove();

		if (!doorOpen && !fruit) {
			openDoor();
		}
	}
}
