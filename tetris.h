#ifndef TETRIS_H
#define TETRIS_H
#define MIN_W 4
#define MIN_H 8
#define FIG_I 1
#define FIG_S 2
#define FIG_Z 3
#define FIG_T 4
#define FIG_J 5
#define FIG_L 6
#define FIG_O 7
#define ERRORCODE -999 // has to be less then -1
#include <stdlib.h>
#include <time.h>

/*
Axes:
 0--------> (x)
 |
 |
 |
 V
(y)
*/

typedef struct {
	int x,y;
} coord_t;

typedef struct {
	int type;
	coord_t coords[4];
} figure_t;

typedef struct {
	int *field, width, height, compl_count, completed[4], nextfig;
	unsigned lines, level, score, highscore, add, statistics[7];
	figure_t fig;
} tetris_t;

     void random_init();
tetris_t* tetris_init(int width, int height, unsigned level, unsigned highscore);
     void tetris_free(tetris_t *tetris);
     void tetris_restart(tetris_t *tetris, unsigned level);

      // undermentioned functions are all return ERRORCODE, if tetris==NULL
      int tetris_left(tetris_t *tetris);                     //
      int tetris_right(tetris_t *tetris);                    // 0 : unable, 1 : success
      int tetris_rotate(tetris_t *tetris, int is_clockwise); //

      int tetris_down(tetris_t *tetris, int is_user); // -1 : there is no figure, 0 : unable (figure dropped), 1 : success
      int tetris_drop(tetris_t *tetris); // 0 : there is no figure, 1 : success
      int tetris_next(tetris_t *tetris); // -1 : there is already a figure, 0 : gameover, 1 : success

      int tetris_get(const tetris_t *tetris, int x, int y); // -1 : out of the borders (besides the area over the field), 0–7 : state of cell

#endif