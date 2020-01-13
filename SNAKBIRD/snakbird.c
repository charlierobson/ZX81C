// ZX81 SNAKEBIRD-ALIKE
//
// Styled as SNAKE81RD

#include <stdio.h>
#include <string.h>
#include <input.h>

typedef unsigned char byte;
typedef void(*UDFN)(void);

char *_display;
extern int D_FILE @16396;

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


typedef struct {
	byte segs[16];
	byte head, tail, face, length, isDead;
} snake_t;

snake_t _snake;


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


#define A 0x80
#define B 0x81
#define C 0x82
#define D 0x83
#define E 0x84

#define F T_FRUIT1
#define M T_GROUND1
#define X T_DOORCLOSED
#define V T_SPIKES

const int MAPSIZE = 16*12;

byte _map[16*12];
byte _gameMap[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,F,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,F,0,0,0,0,0,0,0,
	0,0,0,0,M,M,M,M,0,0,0,0,0,0,0,0,
	0,0,V,V,V,0,0,0,0,0,0,0,0,X,0,0,
	0,0,0,0,0,0,C,B,A,0,0,0,0,0,0,0,
	0,0,0,0,0,0,M,M,M,M,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,X,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,M,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,F,0,0,M,F,M,0,0,0,0,0,0,
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
	0,0,M,0,F,M,0,0,0,0,0,0,0,0,0,0,
	0,0,M,0,0,M,0,0,C,0,0,0,0,0,0,0,
	0,0,M,0,0,0,0,0,B,A,0,F,0,0,0,0,
	0,0,M,M,M,M,M,M,M,M,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	0xf0,0xff
};



void putBlock(int cellId, int tile) {
	int cellX = cellId & 15;
	int cellY = cellId / 16;
	char* cell = _display + (cellX * 2) + (cellY * 66);
	int ci = tile * 4;

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
int doUpdateFn() {
	if (_updateFn) {
		_updateFn();
	}
	return _updateFn != NULL;
}


void putSnakeInMap() {
	if (!_snake.isDead) {
		updateMap(_snake.segs[_snake.head], _snake.face + T_FACEUP);
		for (int i = 1, n = _snake.head; i < _snake.length; ++i) {
			n = (n - 1) & 15;
			updateMap(_snake.segs[n], T_SNAKEBOD2 - (i & 1));
		}
	} else {
		int phase = _udTimer / 10;
		switch(phase) {
			case 0:
			case 1:
			case 2:
			case 3:
			{
				int x = _snake.isDead ? T_DEADFACEUP : T_FACEUP;
				updateMap(_snake.segs[_snake.head], _snake.face + x);
				for (int i = 1, n = _snake.head; i < _snake.length; ++i) {
					n = (n - 1) & 15;
					updateMap(_snake.segs[n], T_SNAKEBOD2 - (i & 1));
				}
			}
			break;
			case 4:
			{
				for (int i = 0, n = _snake.head; i < _snake.length; ++i) {
					updateMap(_snake.segs[n], T_BODDEAD1);
					n = (n - 1) & 15;
				}
			}
			break;
			case 5:
			{
				for (int i = 0, n = _snake.head; i < _snake.length; ++i) {
					updateMap(_snake.segs[n], T_BODDEAD2);
					n = (n - 1) & 15;
				}
			}
			break;
			default:
			{
				for (int i = 0, n = _snake.head; i < _snake.length; ++i) {
					updateMap(_snake.segs[n], T_EMPTY);
					n = (n - 1) & 15;
				}
			}
			break;
		}
	}
}



void setUpdateFn(UDFN fn) {
	_udTimer = 0;
	_updateFn = fn;
}


void nextLevel(void) {
	++_udTimer;
	if (_udTimer == 100) {
		++_level;
		_level %= _levelCount;
		reset(_level);
	}
}


void death(void) {
	_snake.isDead = 1;

	++_udTimer;
	if (_udTimer == 100) {
		reset(_level);
	}
}


void exiting(void) {
	int exited = 1;

	// collapse all segments onto the head, counting how many segments are in one place
	updateMap(_snake.segs[_snake.tail], T_EMPTY);
	for (int i = 0; i < _snake.length - 1; ++i) {
		if (_snake.segs[(_snake.tail + i) & 15] == _snake.segs[_snake.head]) {
			++exited;
		}
		_snake.segs[(_snake.tail + i) & 15] = _snake.segs[(_snake.tail + 1 + i) & 15];
	}

	if (exited == _snake.length) {
		setUpdateFn(nextLevel);
		updateMap(_snake.segs[_snake.head], T_DOORCLOSED);
	}

	return 1;
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


int checkFall() {
	for (int i = 0, n = _snake.tail; i < _snake.length; ++i) {
		// if any segment would go off screen then die
		if (_snake.segs[n] + 16 >= MAPSIZE) {
			setUpdateFn(death);
			return 1;
		} 

		// if any segment is supported by something solid other than another segment then nothing to do
		if (_map[_snake.segs[n] + 16] > T_SPIKES && _map[_snake.segs[n] + 16] < T_SNAKEBOD1)
			return 0;

		n = (n + 1) & 15;
	}

	// we would be falling so check for hazards

	for (int i = 0, n = _snake.tail; i < _snake.length; ++i) {
		// if any segment would go into spikes then die
		if (_map[_snake.segs[n] + 16] == T_SPIKES) {
			setUpdateFn(death);
			return 1;
		} 

		n = (n + 1) & 15;
	}


	// none of the segments are supported so move them all down
	for (int i = 0, n = _snake.tail; i < _snake.length; ++i) {
		updateMap(_snake.segs[n], T_EMPTY);
		_snake.segs[n] = _snake.segs[n] + 16;
		n = (n + 1) & 15;
	}

	return 1;
}


int tryMove(int newDirn) {
	const int _dirs[] = {
		-16,1,16,-1
	};

	int newMapPos = _snake.segs[_snake.head] + _dirs[newDirn];
	if (!isTraversible(_map[newMapPos]))
		return 0;

	_snake.face = newDirn;
	_snake.head = (_snake.head + 1) & 15;
	_snake.segs[_snake.head] = newMapPos;

	if (_map[newMapPos] != T_FRUIT1) {
		// update tail if not moving on to _fruitCount
		updateMap(_snake.segs[_snake.tail], T_EMPTY);
		_snake.tail = (_snake.tail + 1) & 15;
	}

	if (_map[newMapPos] == T_FRUIT1) {
		// moved on to _fruitCount, so grow
		++_snake.length;
		--_fruitCount;
	} else if (_map[newMapPos] == T_DOOROPEN)
		setUpdateFn(exiting);

	if (!_doorIsOpen && !_fruitCount) {
		openDoor();
	}

	return 1;
}


void snakeMove() {
	if (checkFall())
		return;

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
		if (k == K_RESET) return reset(_level);
	}
}


int reset(int level) {
	_level = level;
	_doorIsOpen = 0;

	_updateListCount = 0;

	_snake.isDead = 0;

	setUpdateFn(snakeMove);

	int levelOffset = _level * MAPSIZE;
	memcpy(_map, _gameMap + levelOffset, MAPSIZE);

	_snake.length = 0;
	for (int i = 0; i < MAPSIZE; ++i) {
		if (_map[i] > 0x7f) {
			_snake.segs[(0x80 - _map[i]) & 15] = i;
			++_snake.length;
		}
	}
	_snake.head = 0;
	_snake.tail = 17 - _snake.length;
	_snake.face = _snake.segs[0] - _snake.segs[15];
	if (_snake.face == -1) _snake.face = LEFT;
	else if (_snake.face == 1) _snake.face = RIGHT;
	else if (_snake.face == -16) _snake.face = UP;
	else _snake.face = DOWN;

	countFruit();

	putSnakeInMap();
	renderMap();
}


void main()
{
	_display = D_FILE + 1;

	for (_levelCount = 0; _gameMap[_levelCount * MAPSIZE] != 0xf0 && _gameMap[_levelCount * MAPSIZE + 1] != 0xff; ++_levelCount) {}

	reset(0);

	while(1) {
		putSnakeInMap();
		renderUpdates();
		doUpdateFn();
	}
}
