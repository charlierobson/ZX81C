#include <stdio.h>
#include <input.h>
#include <stdlib.h>
#include <string.h>

#include <zx81.h>

#pragma string name Z8048
#pragma output nostreams

#define K_UP       'Q'
#define K_DOWN     'A'
#define K_LEFT     'O'
#define K_RIGHT    'P'
#define K_RESET    'R'

enum
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	RESET
};


char *display;
extern int d_file @16396;

int score, hiscore;

int tileAddresses[4*4];

int board[4*4];
int shadowBoard[4*4];

int freeCells[4*4];
int tempRows[8*4];

char scoreBuffer[10] = { 32,32,32,32,32,32,32,32,32,32 };


int getFreeCells();
int getNewTileValue();

void calculateTileAddresses();

void drawGrid();
void drawBoard();

void zxTextOut(int, int, const char*);

void setStepLeft();
void setStepRight();
void setStepUp();
void setStepDown();

void moveOutRows();
void moveInRows();

int updateRow(int* tempRow);

int checkLose();
int checkWin();

int getMove();

void cls();
void transitionEffect();


void main()
{
	int changed, gameOn, won, r, n;

	display = d_file+1;

	calculateTileAddresses();

	// draw screen into an off-screen buffer
	//
	display += 1024;

	cls();
	zxTextOut(22, 1, "-=z8048=-");
	zxTextOut(23, 3,  "hiscore");
	zxTextOut(23, 4,  "      0");
	zxTextOut(24, 6,   "score");
	zxTextOut(24, 7,   "    0");
	zxTextOut(22, 15, "up:     q");
	zxTextOut(22, 16, "down:   a");
	zxTextOut(22, 17, "left:   o");
	zxTextOut(22, 18, "right:  p");
	zxTextOut(22, 19, "reset:  r");
	zxTextOut(3, 23, "sir morris 2015");
	drawGrid();

	// now draw to real actual screen
	//
	display -= 1024;

//                    --------========--------========	
	zxTextOut(0,  4, "           -=z8048=-");
	zxTextOut(0,  8, "         based on 2048");
	zxTextOut(0, 10, "   by gabriele cirulli et al.");
	zxTextOut(0, 20, "     press any key to play");
//                    --------========--------========	

	fgetc_cons();

	transitionEffect();
	
	hiscore = 0;
	
	while (1)
	{
		drawGrid();

		// clear the board, and the shadow board, which is used as a dirty
		// buffer to limit the board drawing to the tiles which have changed
		//
		memset(board, 0, 16 * sizeof(int));
		memset(shadowBoard, 1, 16 * sizeof(int));

		board[rand() & 15] = getNewTileValue();

		gameOn = 1;
		won = 0;

		score = 0;

		// set to a non-zero character so that the second start tile gets
		// placed onto the board at the game start
		//
		changed = 999;

		while (gameOn == 1)
		{
			// runs until no more available moves
			//
			if (changed != 0)
			{
				n = getFreeCells();
				r = rand() % n;
				board[freeCells[r]] = getNewTileValue();

				drawBoard();

				// n (free cell count) will be 1 when there are no more free spaces on the
				// board. It's not game over, however, until there are no more possible moves
				//
				if (n == 1 && checkLose())
				{
					// exit the inner loop and reset the board
					//
					gameOn = 0;
					continue;
				}
			}

			changed = 0;

			r = getMove();
			switch(r)
			{
				// set step xx initialises counters which control how the board is walked.
				//
				case UP:
					setStepUp();
					break;
				case DOWN:
					setStepDown();
					break;
				case LEFT:
					setStepLeft();
					break;
				case RIGHT:
					setStepRight();
					break;

				case RESET:
					gameOn = 0;
					continue;
			}

			// by walking the board and copying out the rows into a temporary buffer
			// we can make the game logic easier.
			//
			moveOutRows();

			while (1)
			{
				// keep a count of the number of tiles which have moved during the play
				//
				int updates = updateRow(&tempRows[0]);
				updates += updateRow(&tempRows[8]);
				updates += updateRow(&tempRows[16]);
				updates += updateRow(&tempRows[24]);

				// if no tiles moved then we exit this inner loop. play is over for now. otherwise we note
				// that something happened during this play and thus we need to add a new tile to the board.
				//
				if (updates == 0)
					break;
				else
					++changed;

				// copy the working rows back into the board so it can be rendered
				//
				moveInRows();

				drawBoard();
			}

			sprintf(scoreBuffer + 5, "%d", score);
			r = strlen(scoreBuffer + 5);
			zxTextOut(24, 7, scoreBuffer + 5 - (5 - r)); 

			if (score > hiscore)
			{
				hiscore = score;

				sprintf(scoreBuffer + 5, "%d", hiscore);
				r = strlen(scoreBuffer + 5);
				zxTextOut(25, 4, scoreBuffer + 5 - (5 - r)); 
			}

			// the won flag gets set when we detect a 2048 tile on the board.
			//
			if (!won)
			{
				won = checkWin();
			}
		}
	}
}


void cls()
{
	int i, j;
	for (i = 0; i < 24; ++i)
	{
		memset(display + 33 * i, 0, 32);
	}
}


void transitionEffect()
{
	int i, j;
	for (i = 0; i < 13; ++i)
	{
		memset(display + 33 * i, 8, 32);
		memset(display + 33 * (24-i), 8, 32);
		for (j = 0; j < 100; ++j){}
	}
	for (i = 12; i >= 0; --i)
	{
		memcpy(display + 33 * i, (display+1024) + 33 * i, 32);
		memcpy(display + 33 * (24-i), (display+1024) + 33 * (24-i), 32);
		for (j = 0; j < 100; ++j){}
	}
}


int getMove()
{
	int k;

	k = in_Inkey();
	while (k != 0)
	{
		k = in_Inkey();
	}

	while(1)
	{
		k = in_Inkey();

		//sprintf(scoreBuffer, "%d   ", k);
		//zxTextOut(25, 10, scoreBuffer); 

		if (k == K_UP) return UP;
		if (k == K_DOWN) return DOWN;
		if (k == K_LEFT) return LEFT;
		if (k == K_RIGHT) return RIGHT;
		if (k == K_RESET) return RESET;
	}
}


void clearNotificationArea()
{
	int i;
	for (i = 6; i < 15; ++i)
	{
		memset(&display[i*33+6], 0, 9);
	}
}


void zxTextOut(int x, int y, const char* text)
{
	char* dm = display + x + (33 * y);
	while (*text)
	{
		*dm = ascii_zx(*text);
		
		++text;
		++dm;
	}
}


int checkLose()
{
	int i, j;
	for (i = 0; i < 4; ++i)
	{
		// walk the board and check to see if there are any adjacent tiles which match
		//
		j = i * 4;

		if (board[j + 0] == board[j + 1]) return 0;
		if (board[j + 1] == board[j + 2]) return 0;
		if (board[j + 2] == board[j + 3]) return 0;

		if (board[i + 0] == board[i + 4]) return 0;
		if (board[i + 4] == board[i + 8]) return 0;
		if (board[i + 8] == board[i + 12]) return 0;
	}

	// no adjacent matches - we have a loser!

	clearNotificationArea();

	zxTextOut(7,8, "GAME");
	zxTextOut(10,9, "OVER");
	
	zxTextOut(8,12, "press");
	zxTextOut(7,13, "any key");

	fgetc_cons();

	return 1;
}


int checkWin()
{
	int i;
	for (i = 0; i < 16; ++i)
	{
		// 2 ^ 11 == 2048
		//
		if (board[i] == 11)
		{
			// we have a winner!

			clearNotificationArea();
			
			zxTextOut(7,8, "YOU WIN");
			zxTextOut(8,12, "press");
			zxTextOut(7,13, "any key");
			
			fgetc_cons();

			// we will continue from here, so re-draw the board after ensuring
			// that the shadow board will force every cell to be redrawn.
			//
			for (i = 0; i < 16; ++i)
			{
				// ensure all cells are non-matching, paying special attention to
				// the 'both cells not blank' optimisation.
				//
				shadowBoard[i] = board[i] == 0 ? 1 : 0;
			}
			drawGrid();
			drawBoard();

			return 1;
		}
	}

	return 0;
}


int getNewTileValue()
{
	return rand() % 10 < 9 ? 1 : 2;
}


int getFreeCells()
{
	int i, n;
	
	for (i = 0, n = 0; i < 16; ++i)
	{
		if (board[i] == 0)
		{
			freeCells[n] = i;
			++n;
		}
	}

	return n;
}


int _boardStart, _boardMod, _stepMod;

// these variables control how the board is walked when rows are moved out into the temporary row buffers
//
void setStepping(int boardStart, int boardMod, int stepMod)
{
	_boardStart = boardStart;
	_boardMod = boardMod;
	_stepMod = stepMod;
}

void setStepLeft() { setStepping(0, 4, 1); }
void setStepRight() { setStepping(3, 4, -1); }
void setStepUp() { setStepping(0, 1, 4); }
void setStepDown() { setStepping(12, 1, -4); }


// move a line out of the board into the temp rows
//
void moveOut(int startIndex, int* tempRowPtr)
{
	int destIdx;
	memset(tempRowPtr, 0, 8 * sizeof(int));
	for (destIdx = 0; destIdx < 4; ++destIdx)
	{
		tempRowPtr[destIdx] = board[startIndex];
		startIndex += _stepMod;
	}
}

// move the entire board into the 4 temporary row buffers
//
void moveOutRows()
{
	int row, boardStart = _boardStart;
	for (row = 0; row < 4; ++row)
	{
		moveOut(boardStart, &tempRows[row * 8]);
		boardStart += _boardMod;
	}
}


// move a temp row back into a line on the board
//
void moveIn(int destIndex, int* source)
{
	int sourceIndex;
	for (sourceIndex = 0; sourceIndex < 4; ++sourceIndex)
	{
		board[destIndex] = source[sourceIndex] & 16383;
		destIndex += _stepMod;
	}
}

// move all 4 temporary row buffers back into the board
//
void moveInRows()
{
	int row, boardStart = _boardStart;
	for (row = 0; row < 4; ++row)
	{
		moveIn(boardStart, &tempRows[row * 8]);
		boardStart += _boardMod;
	}
}

// shonky power function
//
int pow(int base, int exp)
{
    int result = 1;

	if (exp > 11) zxTextOut(25,21, "ERR");
	
    while (exp)
	{
		result *= base;
		--exp;
	}
    return result;
}


// the actual meat of the game. because we use temporary row buffers we have one function
// which handles moves in all directions. the buffers have 8 cells so we don't wast time
// or effort clipping the bounds during copy operations.
//
int updateRow(int* tempRow)
{
	int idx, cellsChanged = 0;

	for (idx = 0; idx < 3; ++idx)
	{
		// tiles which have been merged this play are marked so that they are not considered
		// for furhter merges in subsequent iterations
		//
		if (tempRow[idx] & 16384) continue;

		if (tempRow[idx] == 0)
		{
			// there is an empty cell here so shift the rest of the cells in this row along
			//
			tempRow[idx] = tempRow[idx + 1];
			tempRow[idx + 1] = tempRow[idx + 2];
			tempRow[idx + 2] = tempRow[idx + 3];
			tempRow[idx + 3] = tempRow[idx + 4];

			// if this row has no more occupied cells then we will not process it any further
			//
			return tempRow[idx] + tempRow[idx+1] + tempRow[idx+2] + tempRow[idx+3];
		}
		else if (tempRow[idx] == tempRow[idx + 1])
		{
			// merge the identical adjacent cells
			//
			++tempRow[idx];

			// add the value of the merged cells to the score
			//
			score += pow(2, tempRow[idx]);

			// mark the merged cell so it doesn't get merged again
			//
			tempRow[idx] |= 16384;

			// shift the rest of the cells in this row along
			//
			tempRow[idx + 1] = tempRow[idx + 2];
			tempRow[idx + 2] = tempRow[idx + 3];
			tempRow[idx + 3] = tempRow[idx + 4];

			return 1;
		}
	}

	return 0;
}


void calculateTileAddresses()
{
	int q;

	for (q = 0; q < 16; ++q)
	{
		int x = (q & 3) * 5;
		int y = (q / 4) * (33*5);

		tileAddresses[q] = (int)display + x + y + 34;
	}
}


char sprs[] = 
{
 0,0,0,0,
 8,8,'2'-'0'+28,8,
 8,8,'4'-'0'+28,8,
 8,8,'8'-'0'+28,8,
 8,'1'-'0'+28,'6'-'0'+28,8,
 8,'3'-'0'+28,'2'-'0'+28,8,
 8,'6'-'0'+28,'4'-'0'+28,8,
 '1'-'0'+28,'2'-'0'+28,'8'-'0'+28,8,
 '2'-'0'+28,'5'-'0'+28,'6'-'0'+28,8,
 '5'-'0'+28,'1'-'0'+28,'2'-'0'+28,8,
 '1'-'0'+28,'0'-'0'+28,'2'-'0'+28,'4'-'0'+28,
 '2'-'0'+28,'0'-'0'+28,'4'-'0'+28,'8'-'0'+28,
 '4'-'0'+28,'0'-'0'+28,'9'-'0'+28,'6'-'0'+28,
 '8'-'0'+28,'1'-'0'+28,'9'-'0'+28,'2'-'0'+28,
 '8'-'0'+28,'1'-'0'+28,'9'-'0'+28,'2'-'0'+28,
 '8'-'0'+28,'1'-'0'+28,'9'-'0'+28,'2'-'0'+28,
 '8'-'0'+28,'1'-'0'+28,'9'-'0'+28,'2'-'0'+28
};

#define FILL4(x) *dm = x; ++dm; *dm = x; ++dm; *dm = x; ++dm; *dm = x;

#define CPY4 *dm=*spr ;++dm; ++spr; *dm=*spr; ++dm; ++spr; *dm=*spr; ++dm; ++spr; *dm=*spr;

void putTile(int idx, unsigned short q)
 {
	char* spr;
	char* dm = (char*)tileAddresses[idx];

	if (q & 128)
	{
		q &= 127;
		q <<= 2;
		spr = &sprs[q];

		dm += 66;
		CPY4;
	}
	else if (q != 0)
	{
		q <<= 2;
		spr = &sprs[q];

		FILL4(8);
		dm += 30;
		FILL4(8);
		dm += 30;
		CPY4;
		dm += 30;
		FILL4(8);
	}
	else
	{
		FILL4(0);
		dm += 30;
		FILL4(0);
		dm += 30;
		FILL4(0);
		dm += 30;
		FILL4(0);
	}
}


void drawBoard()
{
	int idx;

	for (idx = 0; idx < 16; ++idx)
	{
		// only draw cells which have changed this turn
		//
		int t = board[idx];
		if (t != shadowBoard[idx])
		{
			// if both board & shadow was occupied last turn then we can get away with only
			// drawing the part of the tile with the number in it.
			//
			if (t != 0 && shadowBoard[idx] != 0)
				t |= 128;

			putTile(idx, board[idx]);
		}
	}

	// copy board -> shadow board
	//
	memcpy(shadowBoard, board, 16 * sizeof(int));
}

	
void drawGrid()
{
	int r;
	for(r = 0; r < 21; ++r)
	{
		display[r] = 128;
		display[r+(33*5)] = 128;
		display[r+(33*10)] = 128;
		display[r+(33*15)] = 128;
		display[r+(33*20)] = 128;
		
		display[r*33] = 128;
		display[r*33+5] = 128;
		display[r*33+10] = 128;
		display[r*33+15] = 128;
		display[r*33+20] = 128;
	}
}
