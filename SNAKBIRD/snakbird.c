// ZX81 SNAKEBIRD-ALIKE
//
// Styled as SNAKE81RD

#include <stdio.h>
#include <string.h>
#include <input.h>

typedef unsigned char byte;

void reset(int level);

char *_display;
extern int D_FILE @16396;

#define K_UP       'Q'
#define K_DOWN     'A'
#define K_LEFT     'O'
#define K_RIGHT    'P'
#define K_RESET    'R'
#define K_SWAP     ' '

enum
{
	UP,
	RIGHT,
	DOWN,
	LEFT,
	RESET
};


typedef struct {
	byte segs[16];
	byte id, head, tail, face, length, isDead;
} snake_t;

snake_t _snake1;
snake_t _snake2;

typedef void(*UDFN)(snake_t*);

int _level;
int _levelCount;
byte _fruitCount, _doorIsOpen;


enum {
	T_EMPTY,

	T_SPIKES,

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
	T_BODDEAD1,
	T_BODDEAD2,

	T_LAST
};


int _tileMap[] = {
	// unsolids
	0,0,0,0,				// air
	0x3d,0x3d,0x3d,0x3d,	// spikes

	// solids
	0x8a,0x8a,0x80,0x80,	// ground
	0x81,0x82,0x84,0x07,	// '_fruitCount'
	0x87,0x04,0x02,0x01,	// door, closed
	0x07,0x84,0x82,0x81,	// door, open

	// snakes
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

	0x17,0x17,0x17,0x17,	// bod dead 1
	0x1b,0x1b,0x1b,0x1b,	// bod dead 2
};

// snake 1
#define A 0x40
#define B 0x41
#define C 0x42
#define D 0x43
#define E 0x44

// snake 2
#define F 0x80
#define G 0x81
#define H 0x82
#define I 0x83
#define J 0x84

#define O T_FRUIT1
#define M T_GROUND1
#define X T_DOORCLOSED
#define V T_SPIKES

const int MAPSIZE = 16*12;

byte _map[16*12];
byte _gameMap[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,O,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,F,G,H,0,O,0,0,0,0,0,0,0,
	0,0,V,V,M,M,0,M,0,M,0,0,0,0,0,0,
	0,0,V,V,V,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,C,B,A,0,0,0,X,0,0,0,
	0,0,0,0,0,0,M,M,M,M,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,X,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,M,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,O,0,0,M,O,M,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,M,A,B,0,0,0,0,0,0,0,0,
	0,0,0,0,0,M,M,M,M,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,X,0,0,0,0,0,0,0,
	0,0,M,M,M,M,0,0,0,0,0,0,0,0,0,0,
	0,0,M,0,O,M,0,0,0,0,0,0,0,0,0,0,
	0,0,M,0,0,M,0,0,C,0,0,0,0,0,0,0,
	0,0,M,0,0,0,0,0,B,A,0,O,0,0,0,0,
	0,0,M,M,M,M,M,M,M,M,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	0xf0,0xff
};

const int _dirs[] = {
	-16,1,16,-1
};


void putBlock(int cellId, int tile) {
	int cellX = cellId & 15;
	int cellY = cellId / 16;
	char* cell = _display + (cellX * 2) + (cellY * 66);

	int ci = (tile & 0x3f) * 4;

	cell[0] = _tileMap[ci];
	++ci;
	cell[1] = _tileMap[ci];
	++ci;
	cell[33] = _tileMap[ci];
	++ci;
	cell[34] = _tileMap[ci];
}


void renderMap() {
	for (int i = 0; i < MAPSIZE; ++i) {
		putBlock(i, _map[i]);
	}
}


// update _map and track which cells have changed for drawing
byte _updateList[32];
byte _updateListCount;
void updateMap(int index, byte value) {
	_map[index] = value;

	// only render a cell once
	for (int i = 0; i < _updateListCount; ++i) {
		if (_updateList[i] == index)
			return;
	}

	_updateList[_updateListCount] = index;
	++_updateListCount;
}


void renderUpdates() {
	for (int i = 0; i < _updateListCount; ++i) {
		byte cell = _updateList[i];
		putBlock(cell, _map[cell]);
	}
	_updateListCount = 0;
}


// game update function handler
int _udTimer;
UDFN _updateFn;
int doUpdateFn(snake_t* snake) {
	if (_updateFn) {
		_updateFn(snake);
	}
	return _updateFn != NULL;
}


void putSnakeInMap(snake_t* snake) {
	if (snake->length == 0)
		return;

	if (!snake->isDead) {
		updateMap(snake->segs[snake->head], snake->id + snake->face + T_FACEUP);
		for (int i = 1, n = snake->head; i < snake->length; ++i) {
			n = (n - 1) & 15;
			updateMap(snake->segs[n], snake->id + T_SNAKEBOD2 - (i & 1));
		}
	} else {
		int phase = _udTimer / 10;
		switch(phase) {
			case 0:
			case 1:
			case 2:
			case 3:
			{
				int x = snake->isDead ? T_DEADFACEUP : T_FACEUP;
				updateMap(snake->segs[snake->head], snake->face + x);
				for (int i = 1, n = snake->head; i < snake->length; ++i) {
					n = (n - 1) & 15;
					updateMap(snake->segs[n], T_SNAKEBOD2 - (i & 1));
				}
			}
			break;
			case 4:
			{
				for (int i = 0, n = snake->head; i < snake->length; ++i) {
					updateMap(snake->segs[n], T_BODDEAD1);
					n = (n - 1) & 15;
				}
			}
			break;
			case 5:
			{
				for (int i = 0, n = snake->head; i < snake->length; ++i) {
					updateMap(snake->segs[n], T_BODDEAD2);
					n = (n - 1) & 15;
				}
			}
			break;
			default:
			{
				for (int i = 0, n = snake->head; i < snake->length; ++i) {
					updateMap(snake->segs[n], T_EMPTY);
					n = (n - 1) & 15;
				}
			}
			break;
		}
	}
}


void fnExiting(snake_t*);
void fnDeath(snake_t*);
void fnSnakeMove(snake_t*);
void fnNextLevel(snake_t*);

void setUpdateFn(UDFN fn) {
	_udTimer = 0;
	_updateFn = fn;
}


snake_t* otherSnake(snake_t* thisSnake) {
	return thisSnake == &_snake1 ? &_snake2 : &_snake1; 
}


snake_t* _activeSnake;
void swapSnakes() { // hmm is there a bug here waiting to happen? should only call if you *can* swap
	if (_activeSnake == &_snake1 && !_snake2.isDead) {
		_activeSnake = &_snake2;
	} else
		// active snake is snake 2 or it's snake 1 but snake 2 is dead
		_activeSnake = &_snake1;
}


void fnNextLevel(snake_t* snake) {
	++_udTimer;
	if (_udTimer == 100) {
		++_level;
		_level %= _levelCount;
		reset(_level);
	}
}


void fnDeath(snake_t* snake) {
	_snake1.isDead = 1;
	_snake2.isDead = 1;

	++_udTimer;
	if (_udTimer == 100) {
		reset(_level);
	}
}



void countFruit() {
	_fruitCount = 0;
	for (int i = 0; i < MAPSIZE; ++i) {
		if (_map[i] == T_FRUIT1) {
			++_fruitCount;
		}
	}
}


void openDoor() {
	_doorIsOpen = 1;
	for (int i = 0; i < MAPSIZE; ++i) {
		if (_map[i] == T_DOORCLOSED) {
			updateMap(i, T_DOOROPEN);
			return;
		}
	}
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


int mySegment(snake_t* snake, byte cellContent) {
	return (cellContent & 0xc0) == snake->id;
}


int canMove(snake_t* snake, int direction) {
	// if any segment would move onto ground or object then we can't move
	for (int i = 0, n = snake->tail; i < snake->length; ++i) {
		if (_map[_dirs[direction] + snake->segs[n]] > T_EMPTY &&
			_map[_dirs[direction] + snake->segs[n]] < T_SNAKEBOD1)
				return 0;

		n = (n + 1) & 15;
	}
	return 1;
}


int checkFall(snake_t* snake) {
	if (snake->isDead)
		return 0;

	for (int i = 0, n = snake->tail; i < snake->length; ++i) {
		// if any segment would go off screen then die
		if (snake->segs[n] + 16 >= MAPSIZE) {
			setUpdateFn(fnDeath);
			return 1;
		} 

		// if any segment is supported by something solid other than another segment then nothing to do
		byte m = _map[snake->segs[n] + 16];
		if (m > T_SPIKES) {
			if (m < T_SNAKEBOD1 || !mySegment(snake, m))
				return 0;
		}

		n = (n + 1) & 15;
	}

	// we would be falling so check for hazards

	for (int i = 0, n = snake->tail; i < snake->length; ++i) {
		// if any segment would go into spikes then die
		if (_map[snake->segs[n] + 16] == T_SPIKES) {
			setUpdateFn(fnDeath);
			return 1;
		} 

		n = (n + 1) & 15;
	}


	// none of the segments are supported so move them all down
	for (int i = 0, n = snake->tail; i < snake->length; ++i) {
		updateMap(snake->segs[n], T_EMPTY);
		snake->segs[n] = snake->segs[n] + 16;
		n = (n + 1) & 15;
	}

	return 1;
}


void moveSnake(snake_t* snake, int direction) {
	for (int i = 0, n = snake->tail; i < snake->length; ++i) {
		updateMap(snake->segs[n], T_EMPTY);
		snake->segs[n] += _dirs[direction];
		n = (n + 1) & 15;
	}
}


int tryMove(snake_t* snake, int newDirn) {
	int newMapPos = snake->segs[snake->head] + _dirs[newDirn];

	if (!isTraversible(_map[newMapPos])) {
		// it's not traversible, check if it's another snake's seg blocking
		if ((_map[newMapPos] & 0xc0) == 0 || (_map[newMapPos] & 0xc0) == snake->id)
			return 0;

		if (!canMove(otherSnake(snake), newDirn))
			return 0;
		
		moveSnake(otherSnake(snake), newDirn);
	}

	snake->face = newDirn;
	snake->head = (snake->head + 1) & 15;
	snake->segs[snake->head] = newMapPos;

	if (_map[newMapPos] != T_FRUIT1) {
		// update tail if not moving on to fruit
		updateMap(snake->segs[snake->tail], T_EMPTY);
		snake->tail = (snake->tail + 1) & 15;
	}

	if (_map[newMapPos] == T_FRUIT1) {
		// moved on to fruit, so grow
		++snake->length;
		--_fruitCount;
	} else if (_map[newMapPos] == T_DOOROPEN)
		setUpdateFn(fnExiting);

	if (!_doorIsOpen && !_fruitCount) {
		openDoor();
	}

	return 1;
}


void fnSnakeMove(snake_t* snake) {
	if (checkFall(&_snake1) || checkFall(&_snake2))
		return;

	int k = in_Inkey();
	while (k != 0)
	{
		k = in_Inkey();
	}

	while(1)
	{
		k = in_Inkey();

		if (k == K_UP) return tryMove(snake, UP);
		if (k == K_DOWN) return tryMove(snake, DOWN);
		if (k == K_LEFT) return tryMove(snake, LEFT);
		if (k == K_RIGHT) return tryMove(snake, RIGHT);
		if (k == K_RESET) return reset(_level);
		if (k == K_SWAP) return swapSnakes();
	}
}


void fnExiting(snake_t* snake) {
	int exited = 1;

	// collapse all segments onto the head, counting how many segments are in one place
	updateMap(snake->segs[snake->tail], T_EMPTY);
	for (int i = 0; i < snake->length - 1; ++i) {
		if (snake->segs[(snake->tail + i) & 15] == snake->segs[snake->head]) {
			++exited;
		}
		snake->segs[(snake->tail + i) & 15] = snake->segs[(snake->tail + 1 + i) & 15];
	}

	if (exited == snake->length) {
		snake->length = 0;
		snake->isDead = 1;

		if (otherSnake(snake)->isDead) {
			setUpdateFn(fnNextLevel);
			updateMap(snake->segs[snake->head], T_DOORCLOSED);
		} else {
			swapSnakes();
			setUpdateFn(fnSnakeMove);
			updateMap(snake->segs[snake->head], T_DOOROPEN);
		}
	}

	return 1;
}


void initialiseSnake(snake_t* snake) {
	snake->length = 0;

	for (int i = 0; i < MAPSIZE; ++i) {
		if ((_map[i] & 0xc0) == snake->id) {
			snake->segs[(-(_map[i] & 0x3f)) & 15] = i;
			++snake->length;
		}
	}

	snake->head = 0;
	snake->tail = 17 - snake->length;
	snake->face = snake->segs[0] - snake->segs[15];
	if (snake->face == -1) snake->face = LEFT;
	else if (snake->face == 1) snake->face = RIGHT;
	else if (snake->face == -16) snake->face = UP;
	else snake->face = DOWN;

	snake->isDead = snake->length == 0;
}


void reset(int level) {
	_level = level;
	_doorIsOpen = 0;

	_updateListCount = 0;

	_activeSnake = &_snake1;

	setUpdateFn(fnSnakeMove);

	int levelOffset = _level * MAPSIZE;
	memcpy(_map, _gameMap + levelOffset, MAPSIZE);

	countFruit();

	initialiseSnake(&_snake1);
	initialiseSnake(&_snake2);

	putSnakeInMap(&_snake1);
	putSnakeInMap(&_snake2);

	renderMap();
}


void main()
{
	_display = (char*)(D_FILE + 1);

	_snake1.id = 0x40;
	_snake2.id = 0x80;

	for (_levelCount = 0; _gameMap[_levelCount * MAPSIZE] != 0xf0 && _gameMap[_levelCount * MAPSIZE + 1] != 0xff; ++_levelCount) {}

	reset(0);

	while(1) {
		putSnakeInMap(&_snake1);
		putSnakeInMap(&_snake2);

		renderUpdates();

		doUpdateFn(_activeSnake);
	}
}
