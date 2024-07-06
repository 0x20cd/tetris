#include "tetris.h"

static void tetris_countlines(tetris_t *tetris);
static int tetris_set(tetris_t *tetris, int x, int y, int state); // ERRORCODE : tetris==NULL, 0 : out of the field or state isn't valid, 1 : success

void random_init()
{
	srand(time(NULL));
}

tetris_t* tetris_init(int width, int height, unsigned level, unsigned highscore)
{
	if (width < MIN_W || height < MIN_H)
		return NULL;

	tetris_t *tetris = malloc(sizeof(tetris_t));
	if (!tetris)
		return NULL;

	tetris->field = calloc(width*height, sizeof(int));
	if (!tetris->field) {
		free(tetris);
		return NULL;
	}

	tetris->width = width;
	tetris->height = height;
	tetris->compl_count = 0;
	tetris->nextfig = rand()%7+1;
	tetris->lines = 0;
	tetris->level = level;
	tetris->score = 0;
	tetris->add = 0;
	tetris->highscore = highscore;
	tetris->fig.type = 0; // 'tetris_next' has to be called further

	for (int i = 0; i < 7; i++)
		tetris->statistics[i] = 0;

	return tetris;
}

void tetris_free(tetris_t *tetris)
{
	if (!tetris)
		return;
	free(tetris->field);
	free(tetris);
}

int tetris_left(tetris_t *tetris)
{
	if (!tetris)
		return ERRORCODE;
	if (tetris->fig.type==0)
		return 0;

	int i;
	for (i = 0; i < 4; i++)
		if (tetris_get(tetris, tetris->fig.coords[i].x-1, tetris->fig.coords[i].y)) // if there is no empty cell left to current cell 
			return 0;

	for (i = 0; i < 4; i++)
		tetris->fig.coords[i].x--;
	return 1;
}

int tetris_right(tetris_t *tetris)
{
	if (!tetris)
		return ERRORCODE;
	if (tetris->fig.type==0)
		return 0;

	int i;
	for (i = 0; i < 4; i++)
		if (tetris_get(tetris, tetris->fig.coords[i].x+1, tetris->fig.coords[i].y)) // if there is no empty cell right to current cell 
			return 0;

	for (i = 0; i < 4; i++)
		tetris->fig.coords[i].x++;
	return 1;
}

int tetris_rotate(tetris_t *tetris, int is_clockwise)
{
	if (!tetris)
		return ERRORCODE;

	int i, x0, y0;
	figure_t res = tetris->fig;
	x0 = tetris->fig.coords[0].x;
	y0 = tetris->fig.coords[0].y;

	switch(res.type) {
	case FIG_I:
		if (res.coords[1].x == x0)
		{
			res.coords[1] = (coord_t){x0-2, y0};
			res.coords[2] = (coord_t){x0-1, y0};
			res.coords[3] = (coord_t){x0+1, y0};
		}
		else {
			res.coords[1] = (coord_t){x0, y0-2};
			res.coords[2] = (coord_t){x0, y0-1};
			res.coords[3] = (coord_t){x0, y0+1};
		}
		break;
	case FIG_S:
		if (res.coords[1].x == x0)
		{
			res.coords[1] = (coord_t){x0+1, y0};
			res.coords[2] = (coord_t){x0, y0+1};
			res.coords[3] = (coord_t){x0-1, y0+1};
		}
		else {
			res.coords[1] = (coord_t){x0, y0+1};
			res.coords[2] = (coord_t){x0-1, y0};
			res.coords[3] = (coord_t){x0-1, y0-1};
		}
		break;
	case FIG_Z:
		if (res.coords[1].x == x0)
		{
			res.coords[1] = (coord_t){x0-1, y0};
			res.coords[2] = (coord_t){x0, y0+1};
			res.coords[3] = (coord_t){x0+1, y0+1};
		}
		else {
			res.coords[1] = (coord_t){x0, y0-1};
			res.coords[2] = (coord_t){x0-1, y0};
			res.coords[3] = (coord_t){x0-1, y0+1};
		}
		break;
	case FIG_T:
	case FIG_J:
	case FIG_L:
		if (is_clockwise)
			for (i = 1; i < 4; i++) {
				res.coords[i].x = x0 - (tetris->fig.coords[i].y-y0);//x0-y0+tetris->fig.coords[i].y;
				res.coords[i].y = y0 + (tetris->fig.coords[i].x-x0);
			}
		else
			for (i = 1; i < 4; i++) {
				res.coords[i].x = x0 + (tetris->fig.coords[i].y-y0);
				res.coords[i].y = y0 - (tetris->fig.coords[i].x-x0);
			}
		break;
	case FIG_O:
		return 1;
	default:
		return 0;
	}

	for (i = 0; i < 4; i++)
		if (tetris_get(tetris, res.coords[i].x, res.coords[i].y))
			return 0;

	tetris->fig = res;
	return 1;
}

void tetris_restart(tetris_t *tetris, unsigned level)
{
	if (!tetris)
		return;

	int len = tetris->width * tetris->height;
	for (int i = 0; i < len; i++)
		tetris->field[i] = 0;

	tetris->compl_count = 0;
	tetris->nextfig = rand()%7+1;
	tetris->lines = 0;
	tetris->level = level;
	tetris->score = 0;
	tetris->add = 0;
	tetris->fig.type = 0;

	for (int i = 0; i < 7; i++)
		tetris->statistics[i] = 0;
}

int tetris_down(tetris_t *tetris, int is_user)
{
	if (!tetris)
		return ERRORCODE;
	if (tetris->fig.type==0)
		return -1;

	if (!is_user)
		tetris->add = 0;

	int i, j;
	for (i = 0; i < 4; i++)
		if (tetris_get(tetris, tetris->fig.coords[i].x, tetris->fig.coords[i].y+1))
		{
			for (j = 0; j < 4; j++)
				tetris_set(tetris, tetris->fig.coords[j].x, tetris->fig.coords[j].y, tetris->fig.type);

			tetris->score += tetris->add;
			tetris->add = 0;
			tetris->fig.type = 0; // 'tetris_next' has to be called further
			tetris_countlines(tetris);
			return 0;
		}

	if (tetris->score > tetris->highscore)
		tetris->highscore = tetris->score;

	if (is_user)
		tetris->add++;

	for (i = 0; i < 4; i++)
		tetris->fig.coords[i].y++;
	return 1;
}

int tetris_drop(tetris_t *tetris)
{
	if (!tetris)
		return ERRORCODE;
	if (tetris->fig.type==0)
		return 0;

	while (tetris_down(tetris, 1)); // drop is always user's action
	return 1;
}

int tetris_next(tetris_t *tetris)
{
	if (!tetris)
		return ERRORCODE;
	if (tetris->fig.type != 0)
		return -1;

	int x, y, wy, t;
	t = tetris->lines/10;
	if (t > tetris->level)
		tetris->level = t;

	for (wy = y = tetris->height-1; wy >= 0; y--) {
		if (tetris->compl_count)
			if (tetris->completed[tetris->compl_count-1] == y) {
				tetris->compl_count--;
				continue;
			}

		if (y != wy) {
			for (x = 0; x < tetris->width; x++)
				tetris_set(tetris, x, wy, tetris_get(tetris, x, y));
		}
		wy--;
	}

	int x0 = tetris->width/2;
	tetris->fig.type = tetris->nextfig;
	tetris->nextfig = rand()%7+1;

	switch (tetris->fig.type) {
	case FIG_I:
		// [1][2][0][3]
		tetris->fig.coords[0] = (coord_t){x0, 0};
		tetris->fig.coords[1] = (coord_t){x0-2, 0};
		tetris->fig.coords[2] = (coord_t){x0-1, 0};
		tetris->fig.coords[3] = (coord_t){x0+1, 0};
		break;
	case FIG_S:
		//    [0][1]
		// [3][2]
		tetris->fig.coords[0] = (coord_t){x0, 0};
		tetris->fig.coords[1] = (coord_t){x0+1, 0};
		tetris->fig.coords[2] = (coord_t){x0, 1};
		tetris->fig.coords[3] = (coord_t){x0-1, 1};
		break;
	case FIG_Z:
		// [1][0]
		//    [2][3]
		tetris->fig.coords[0] = (coord_t){x0, 0};
		tetris->fig.coords[1] = (coord_t){x0-1, 0};
		tetris->fig.coords[2] = (coord_t){x0, 1};
		tetris->fig.coords[3] = (coord_t){x0+1, 1};
		break;
	case FIG_T:
		//    [3]
		// [1][0][2]
		tetris->fig.coords[0] = (coord_t){x0, 1};
		tetris->fig.coords[1] = (coord_t){x0-1, 1};
		tetris->fig.coords[2] = (coord_t){x0+1, 1};
		tetris->fig.coords[3] = (coord_t){x0, 0};
		break;
	case FIG_J:
		// [3]
		// [2][0][1]
		tetris->fig.coords[0] = (coord_t){x0, 1};
		tetris->fig.coords[1] = (coord_t){x0+1, 1};
		tetris->fig.coords[2] = (coord_t){x0-1, 1};
		tetris->fig.coords[3] = (coord_t){x0-1, 0};
		break;
	case FIG_L:
		//       [3]
		// [1][0][2]
		tetris->fig.coords[0] = (coord_t){x0, 1};
		tetris->fig.coords[1] = (coord_t){x0-1, 1};
		tetris->fig.coords[2] = (coord_t){x0+1, 1};
		tetris->fig.coords[3] = (coord_t){x0+1, 0};
		break;
	case FIG_O:
		// [1][0]
		// [2][3]
		tetris->fig.coords[0] = (coord_t){x0, 0};
		tetris->fig.coords[1] = (coord_t){x0-1, 0};
		tetris->fig.coords[2] = (coord_t){x0-1, 1};
		tetris->fig.coords[3] = (coord_t){x0, 1};
		break;
	default:
		return -1;
	}

	for (int i = 0; i < 4; i++)
		if (tetris_get(tetris, tetris->fig.coords[i].x, tetris->fig.coords[i].y))
			return 0;

	tetris->statistics[tetris->fig.type-1]++;
	return 1;
}

// finds completed lines
// and writes their ids in tetris->completed in ascending order
// and their count in tetris->compl_count
static void tetris_countlines(tetris_t *tetris)
{
	if (!tetris)
		return;

	tetris->compl_count = 0;
	int x, y;
	for (y = 0; y < tetris->height; y++) {
		for (x = 0; x < tetris->width; x++)
			if (!tetris_get(tetris, x, y))
				break;

		// line is complete
		if (x == tetris->width)
			tetris->completed[tetris->compl_count++] = y;
	}

	tetris->lines += tetris->compl_count;
	switch (tetris->compl_count) {
	case 1:
		tetris->score += 40;
		break;
	case 2:
		tetris->score += 100;
		break;
	case 3:
		tetris->score += 600;
		break;
	case 4:
		tetris->score += 2400;
		break;
	default:
		break;
	}

	if (tetris->score > tetris->highscore)
		tetris->highscore = tetris->score;
}

int tetris_get(const tetris_t *tetris, int x, int y)
{
	if (!tetris)
		return ERRORCODE;
	if (x < 0 || x >= tetris->width || y >= tetris->height)
		return -1;
	if (y < 0) // top
		return 0;

	return tetris->field[tetris->width*y+x];
}

static int tetris_set(tetris_t *tetris, int x, int y, int state)
{
	if (!tetris)
		return ERRORCODE;
	if (x < 0 || y < 0 || x >= tetris->width || y >= tetris->height || state < 0 || state > 7)
		return 0;

	tetris->field[tetris->width*y+x] = state;
	return 1;
}