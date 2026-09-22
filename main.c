//#define WIN32_LEAN_AND_MEAN
#define BLOCKSIZE 34
#define WIDTH 10
#define HEIGHT 20
#define GUILEFT 13*BLOCKSIZE
#define GUIRIGHT 12*BLOCKSIZE
#define GUITOP 3*BLOCKSIZE
#define ANIM_DELAY 90
#define DOWN_DELAY 30
#define WAIT_DELAY 50
#define LR_DELAY 70
#define LR_PREDELAY 210
#define BLINK_DELAY 90
#define BLINK_PRIORITY 0.4
//#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <SFML/Graphics.h>
#include "tetris.h"
#include "texturedata.h"

void display_tetris(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture);
int display_completed(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, sfClock *timer_blink, int progress);
int display_gameover(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture, int progress);
void display_gui(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture);
void display_menu(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture, int selected);
void draw_text(sfRenderWindow *window, const char *str, sfVector2f startpos, sfRectangleShape *rect, const sfTexture *texture, sfColor color);
void load_highscore(unsigned *highscore);
void save_highscore(const tetris_t *tetris);
int fall_delay(unsigned level);

const sfVector2f fig_imgpos[7][4] = {{{0, BLOCKSIZE/2}, {BLOCKSIZE, BLOCKSIZE/2}, {2*BLOCKSIZE, BLOCKSIZE/2}, {3*BLOCKSIZE, BLOCKSIZE/2}},   // I
                                     {{BLOCKSIZE/2, BLOCKSIZE}, {3*BLOCKSIZE/2, BLOCKSIZE}, {3*BLOCKSIZE/2, 0}, {5*BLOCKSIZE/2, 0}},         // S
                                     {{BLOCKSIZE/2, 0}, {3*BLOCKSIZE/2, 0}, {3*BLOCKSIZE/2, BLOCKSIZE}, {5*BLOCKSIZE/2, BLOCKSIZE}},         // Z
                                     {{BLOCKSIZE/2, BLOCKSIZE}, {3*BLOCKSIZE/2, BLOCKSIZE}, {5*BLOCKSIZE/2, BLOCKSIZE}, {3*BLOCKSIZE/2, 0}}, // T
                                     {{BLOCKSIZE/2, 0}, {BLOCKSIZE/2, BLOCKSIZE}, {3*BLOCKSIZE/2, BLOCKSIZE}, {5*BLOCKSIZE/2, BLOCKSIZE}},   // J
                                     {{BLOCKSIZE/2, BLOCKSIZE}, {3*BLOCKSIZE/2, BLOCKSIZE}, {5*BLOCKSIZE/2, BLOCKSIZE}, {5*BLOCKSIZE/2, 0}}, // L
                                     {{BLOCKSIZE, 0}, {2*BLOCKSIZE, 0}, {BLOCKSIZE, BLOCKSIZE}, {2*BLOCKSIZE, BLOCKSIZE}}};                  // O

int main()
{
	random_init();
	tetris_t *tetris;
	sfVideoMode mode = {GUILEFT+WIDTH*BLOCKSIZE+GUIRIGHT, GUITOP+HEIGHT*BLOCKSIZE, 32};
	sfRenderWindow *window;
	sfRectangleShape *rect;
	sfTexture *texture;
	sfEvent event;
	sfClock *timer, *timer_anim, *timer_down, *timer_lr, *timer_blink;
	int is_keydown = 0, is_keyright = 0, is_keyleft = 0, re_prog = 0, compl_prog = 0, restart_game = 0, is_pause = 0, is_menu = 0, lr_delay = LR_PREDELAY, t, lvl_inc;
	unsigned highscore, selected_level, initial_level = 0;

	load_highscore(&highscore);

	tetris = tetris_init(WIDTH, HEIGHT, initial_level, highscore); // ALLOC 1
	if (!tetris) {
		//MessageBoxA(NULL, "Memory allocation failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	window = sfRenderWindow_create(mode, "Tetris", sfClose, NULL); // ALLOC 2
	if (!window) {
		tetris_free(tetris); // FREE 1
		//MessageBoxA(NULL, "Memory allocation failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	sfRenderWindow_setFramerateLimit(window, 120);
	sfRenderWindow_setKeyRepeatEnabled(window, sfFalse);
	//sfRenderWindow_setVerticalSyncEnabled(window, sfTrue);

	rect = sfRectangleShape_create(); // ALLOC 3
	if (!rect) {
		sfRenderWindow_destroy(window); // FREE 2
		tetris_free(tetris); // FREE 1
		//MessageBoxA(NULL, "Memory allocation failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	texture = sfTexture_createFromMemory(texture_png, texture_png_len, NULL); // ALLOC 4
	if (!texture) {
		sfRectangleShape_destroy(rect); // FREE 3
		sfRenderWindow_destroy(window); // FREE 2
		tetris_free(tetris); // FREE 1
		//MessageBoxA(NULL, "Unable to load texture", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	timer = sfClock_create(); // ALLOC 5
	timer_anim = sfClock_create(); // ALLOC 6
	timer_down = sfClock_create(); // ALLOC 7
	timer_lr = sfClock_create(); // ALLOC 8
	timer_blink = sfClock_create(); // ALLOC 9
	if (!timer || !timer_anim || !timer_down || !timer_lr) {
		sfClock_destroy(timer_blink); // FREE 9
		sfClock_destroy(timer_lr); // FREE 8
		sfClock_destroy(timer_down); // FREE 7
		sfClock_destroy(timer_anim); // FREE 6
		sfClock_destroy(timer); // FREE 5
		sfTexture_destroy(texture); // FREE 4
		sfRectangleShape_destroy(rect); // FREE 3
		sfRenderWindow_destroy(window); // FREE 2
		tetris_free(tetris); // FREE 1
		//MessageBoxA(NULL, "Memory allocation failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	while (sfRenderWindow_isOpen(window))
	{
		if (is_pause) {
			sfRenderWindow_clear(window, sfBlack);

			draw_text(window, "PAUSE", (sfVector2f){(mode.width-5*BLOCKSIZE)/2, (mode.height-BLOCKSIZE)/2}, rect, texture, sfRed);

			sfRenderWindow_display(window);
			while (sfRenderWindow_pollEvent(window, &event)) {
				if (event.type == sfEvtClosed)
					sfRenderWindow_close(window);
				else if (event.type == sfEvtKeyReleased && event.key.code == sfKeyEscape) {
					is_pause = 0;
					sfClock_restart(timer);
					sfClock_restart(timer_down);
				}
			}
			continue;
		}
		else if (is_menu) {
			display_menu(window, tetris, rect, texture, selected_level);
			sfRenderWindow_display(window);
			while (sfRenderWindow_pollEvent(window, &event)) {
				if (event.type == sfEvtClosed)
					sfRenderWindow_close(window);
				else if (event.type == sfEvtKeyPressed && (event.key.code == sfKeyD || event.key.code == sfKeyF))
					lvl_inc += 10;
				else if (event.type == sfEvtKeyReleased) {
					switch (event.key.code) {
					case sfKeyUp:
						selected_level %= 5;
						break;
					case sfKeyDown:
						if (selected_level < 5)
							selected_level+=5;
						break;
					case sfKeyLeft:
						if (selected_level > 0)
							selected_level--;
						break;
					case sfKeyRight:
						if (selected_level < 9)
							selected_level++;
						break;
					case sfKeyEnter:
						initial_level = lvl_inc + selected_level;
						re_prog = 0;
						compl_prog = 0;
						restart_game = 0;
						is_keydown = 0;
						save_highscore(tetris);
						tetris_restart(tetris, initial_level);
						//NO BREAK HERE!
					case sfKeyEscape:
						is_menu = 0;
						sfClock_restart(timer);
						sfClock_restart(timer_down);
						break;
					case sfKeyF:
					case sfKeyD:
						lvl_inc -= 10;
						break;
					default:
						break;
					}
				}
			}
			continue;
		}

		while (sfRenderWindow_pollEvent(window, &event))
		{
			if (event.type == sfEvtClosed)
				sfRenderWindow_close(window);
			else if (!restart_game && event.type == sfEvtKeyPressed) {
				switch (event.key.code) {
				case sfKeyLeft:
					is_keyleft = 1;
					if (!is_keydown && !is_keyright)
						if (tetris_left(tetris))
							sfClock_restart(timer_lr);
					break;
				case sfKeyRight:
					is_keyright = 1;
					if (!is_keydown && !is_keyleft)
						if (tetris_right(tetris))
							sfClock_restart(timer_lr);
					break;
				case sfKeyD:
					tetris_rotate(tetris, 0);
					break;
				case sfKeyF:
				case sfKeyUp:
					tetris_rotate(tetris, 1);
					break;
				case sfKeyDown:
					is_keydown = 1;
					break;
				default:
					break;
				}
			}
			else if (event.type == sfEvtKeyReleased) {
				switch (event.key.code) {
				case sfKeySpace:
					if (!restart_game) {
						tetris_drop(tetris);
						sfClock_restart(timer);
					}
					break;
				case sfKeyDown:
					is_keydown = 0;
					break;
				case sfKeyLeft:
					is_keyleft = 0;
					is_keydown = 0;
					lr_delay = LR_PREDELAY;
					break;
				case sfKeyRight:
					is_keyright = 0;
					is_keydown = 0;
					lr_delay = LR_PREDELAY;
					break;
				case sfKeyEscape:
					is_pause = 1;
					break;
				case sfKeyR:
					selected_level = 0;
					lvl_inc = 0;
					is_menu = 1;
					break;
				default:
					break;
				}
			}
		}

		if (is_menu || is_pause) {
			is_keydown = 0;
			is_keyleft = 0;
			is_keyright = 0;
			lr_delay = LR_PREDELAY;
			continue;
		}

		if (!restart_game && is_keydown + is_keyleft + is_keyright == 1) {
			if (is_keydown) {
				if (sfTime_asMilliseconds(sfClock_getElapsedTime(timer_down)) >= DOWN_DELAY) {
					tetris_down(tetris, 1);
					sfClock_restart(timer);
					sfClock_restart(timer_down);
				}
			}
			else if (sfTime_asMilliseconds(sfClock_getElapsedTime(timer_lr)) >= lr_delay) {
				t = 1;
				if (is_keyleft) 
					t = tetris_left(tetris);
				else if (is_keyright)
					t = tetris_right(tetris);

				lr_delay = LR_DELAY;
				if (t)
					sfClock_restart(timer_lr);
			}
		}

		if (!restart_game && sfTime_asMilliseconds(sfClock_getElapsedTime(timer)) >= fall_delay(tetris->level)) {
			tetris_down(tetris, 0);
			sfClock_restart(timer);
		}

		display_gui(window, tetris, rect, texture);
		display_tetris(window, tetris, rect, texture);

		if (restart_game) {
			if (!display_gameover(window, tetris, rect, texture, re_prog))
			{
				re_prog = 0;
				restart_game = 0;
				is_keydown = 0;
				sfClock_restart(timer);
				sfClock_restart(timer_down);
				tetris_restart(tetris, initial_level);
			}
			else if (sfTime_asMilliseconds(sfClock_getElapsedTime(timer_anim)) >= ANIM_DELAY) {
				re_prog++;
				sfClock_restart(timer_anim);
			}
		}
		else if (tetris->fig.type==0) {
			if (!display_completed(window, tetris, rect, timer_blink, compl_prog)) {
				compl_prog = 0;

				if (!tetris_next(tetris)) {
					restart_game = 1;
					save_highscore(tetris);
				}
				sfClock_restart(timer);
			}
			else if (sfTime_asMilliseconds(sfClock_getElapsedTime(timer_anim)) >= (tetris->compl_count>0 ? ANIM_DELAY : WAIT_DELAY)) {
				compl_prog++;
				sfClock_restart(timer_anim);
			}
		}

		sfRenderWindow_display(window);
	}

	save_highscore(tetris);

	sfClock_destroy(timer_blink); // FREE 9
	sfClock_destroy(timer_lr); // FREE 8
	sfClock_destroy(timer_down); // FREE 7
	sfClock_destroy(timer_anim); // FREE 6
	sfClock_destroy(timer); // FREE 5
	sfTexture_destroy(texture); // FREE 4
	sfRectangleShape_destroy(rect); // FREE 3
	sfRenderWindow_destroy(window); // FREE 2
	tetris_free(tetris); // FREE 1
	return 0;
}

void load_highscore(unsigned *highscore) {
	FILE *highscore_file = fopen("highscore", "r");
	if (highscore_file) {
		fscanf(highscore_file, "%u", highscore);
		fclose(highscore_file);
	} else
		*highscore = 0;
}

void save_highscore(const tetris_t *tetris)
{
	FILE *highscore_file = fopen("highscore", "w");
	if (highscore_file) {
		fprintf(highscore_file, "%u", tetris->highscore);
		fclose(highscore_file);
	}
	//else MessageBoxA(NULL, "Unable to open file\nHighscore is not saved!", "Warning", MB_OK | MB_ICONWARNING);
}

void display_tetris(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture)
{
	int t;
	sfRectangleShape_setTexture(rect, NULL, sfFalse);
	sfRectangleShape_setFillColor(rect, sfBlack);
	sfRectangleShape_setSize(rect, (sfVector2f){tetris->width*BLOCKSIZE, tetris->height*BLOCKSIZE});
	sfRectangleShape_setPosition(rect, (sfVector2f){GUILEFT, GUITOP});
	sfRenderWindow_drawRectangleShape(window, rect, NULL);

	sfRectangleShape_setFillColor(rect, sfWhite); // disable coloring
	sfRectangleShape_setTexture(rect, texture, sfFalse);
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});
	for (int y = 0; y < tetris->height; y++) {
		for (int x = 0; x < tetris->width; x++) {
			t = tetris_get(tetris, x, y);
			if (t) {
				sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*((t-1)%3), blsizepx*(tetris->level%10), blsizepx, blsizepx});
				sfRectangleShape_setPosition(rect, (sfVector2f){GUILEFT+x*BLOCKSIZE, GUITOP+y*BLOCKSIZE});
				sfRenderWindow_drawRectangleShape(window, rect, NULL);
			}
		}
	}

	if (tetris->fig.type) {
		sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*((tetris->fig.type-1)%3), blsizepx*(tetris->level%10), blsizepx, blsizepx});

		for (int i = 0; i < 4; i++) {
			if (tetris->fig.coords[i].y < 0)
				continue;
			sfRectangleShape_setPosition(rect, (sfVector2f){GUILEFT+tetris->fig.coords[i].x*BLOCKSIZE, GUITOP+tetris->fig.coords[i].y*BLOCKSIZE});
			sfRenderWindow_drawRectangleShape(window, rect, NULL);
		}
	}
}

int display_completed(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, sfClock *timer_blink, int progress)
{
	int i, j, m, ret = 1;

	m = tetris->width/2+progress;
	if (m > tetris->width) {
		ret = 0;
		m = tetris->width;
	}

	sfRectangleShape_setTexture(rect, NULL, sfFalse); // disable texturing
	sfRectangleShape_setFillColor(rect, sfBlack);
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});

	if (tetris->compl_count == 4) {
		if (sfTime_asMilliseconds(sfClock_getElapsedTime(timer_blink)) < BLINK_PRIORITY*BLINK_DELAY) {
			sfRenderWindow_clear(window, sfWhite);
			return ret;
		}
		if (sfTime_asMilliseconds(sfClock_getElapsedTime(timer_blink)) > BLINK_DELAY)
			sfClock_restart(timer_blink);
	}

	for (i = 0; i < tetris->compl_count; i++) {
		for (j = tetris->width/2; j < m; j++) {
			sfRectangleShape_setPosition(rect, (sfVector2f){GUILEFT+j*BLOCKSIZE, GUITOP+tetris->completed[i]*BLOCKSIZE});
			sfRenderWindow_drawRectangleShape(window, rect, NULL);
			sfRectangleShape_setPosition(rect, (sfVector2f){GUILEFT+(tetris->width-j-1)*BLOCKSIZE, GUITOP+tetris->completed[i]*BLOCKSIZE});
			sfRenderWindow_drawRectangleShape(window, rect, NULL);
		}
	}

	return ret;
}

void display_gui(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture)
{
	sfVector2u size = sfRenderWindow_getSize(window);
	sfVector2f origin, pos;
	char buffer[11];
	int i, j;

	sfRenderWindow_clear(window, sfColor_fromRGB(40, 40, 45));

	snprintf(buffer, 11, "LINES-%04u", tetris->lines);
	draw_text(window, buffer, (sfVector2f){GUILEFT, GUITOP-2*BLOCKSIZE}, rect, texture, sfWhite);

	draw_text(window, "TOP", (sfVector2f){size.x-GUIRIGHT+7*BLOCKSIZE, GUITOP}, rect, texture, sfWhite);
	snprintf(buffer, 11, "%u", tetris->highscore);
	draw_text(window, buffer, (sfVector2f){size.x-GUIRIGHT+BLOCKSIZE*(11-strlen(buffer)), GUITOP+BLOCKSIZE}, rect, texture, sfWhite);

	draw_text(window, "SCORE", (sfVector2f){size.x-GUIRIGHT+5*BLOCKSIZE, GUITOP+3*BLOCKSIZE}, rect, texture, sfWhite);
	snprintf(buffer, 11, "%u", tetris->score);
	draw_text(window, buffer, (sfVector2f){size.x-GUIRIGHT+BLOCKSIZE*(11-strlen(buffer)), GUITOP+4*BLOCKSIZE}, rect, texture, sfWhite);

	draw_text(window, "LEVEL", (sfVector2f){size.x-GUIRIGHT+3*BLOCKSIZE, GUITOP+16*BLOCKSIZE}, rect, texture, sfWhite);
	snprintf(buffer, 11, "%02u", tetris->level);
	draw_text(window, buffer, (sfVector2f){size.x-GUIRIGHT+5*BLOCKSIZE, GUITOP+17*BLOCKSIZE}, rect, texture, sfWhite);

	draw_text(window, "NEXT", (sfVector2f){size.x-GUIRIGHT+4*BLOCKSIZE, GUITOP+6*BLOCKSIZE}, rect, texture, sfWhite);
	sfRectangleShape_setTexture(rect, NULL, sfFalse);
	sfRectangleShape_setFillColor(rect, sfBlack);
	sfRectangleShape_setSize(rect, (sfVector2f){6*BLOCKSIZE, 6*BLOCKSIZE});
	sfRectangleShape_setPosition(rect, (sfVector2f){size.x-GUIRIGHT+3*BLOCKSIZE, GUITOP+8*BLOCKSIZE});
	sfRenderWindow_drawRectangleShape(window, rect, NULL);

	sfRectangleShape_setFillColor(rect, sfWhite);
	sfRectangleShape_setTexture(rect, texture, sfFalse);
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});
	sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*((tetris->nextfig-1)%3), blsizepx*(tetris->level%10), blsizepx, blsizepx});

	origin = (sfVector2f){size.x-GUIRIGHT+4*BLOCKSIZE, GUITOP+10*BLOCKSIZE};
	for (i = 0; i < 4; i++) {
		pos = fig_imgpos[tetris->nextfig-1][i];
		sfRectangleShape_setPosition(rect, (sfVector2f){origin.x+pos.x, origin.y+pos.y});
		sfRenderWindow_drawRectangleShape(window, rect, NULL);
	}

	draw_text(window, "STATISTICS", (sfVector2f){0, GUITOP-BLOCKSIZE}, rect, texture, sfWhite);
	sfRectangleShape_setTexture(rect, NULL, sfFalse);
	sfRectangleShape_setFillColor(rect, sfBlack);
	sfRectangleShape_setSize(rect, (sfVector2f){10*BLOCKSIZE, 20*BLOCKSIZE});
	sfRectangleShape_setPosition(rect, (sfVector2f){0, GUITOP});
	sfRenderWindow_drawRectangleShape(window, rect, NULL);

	sfRectangleShape_setFillColor(rect, sfWhite);
	sfRectangleShape_setTexture(rect, texture, sfFalse);
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});
	for (i = 0; i < 7; i++) {
		sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*(i%3), blsizepx*(tetris->level%10), blsizepx, blsizepx});
		for (j = 0; j < 4; j++) {
			pos = fig_imgpos[i][j];
			sfRectangleShape_setPosition(rect, (sfVector2f){pos.x+BLOCKSIZE, GUITOP+3*i*BLOCKSIZE+pos.y});
			sfRenderWindow_drawRectangleShape(window, rect, NULL);
		}

		snprintf(buffer, 11, "%03u", tetris->statistics[i]);
		draw_text(window, buffer, (sfVector2f){6*BLOCKSIZE, GUITOP+(3*i+1)*BLOCKSIZE-BLOCKSIZE/2}, rect, texture, sfWhite);
	}
}

void draw_text(sfRenderWindow *window, const char *str, sfVector2f startpos, sfRectangleShape *rect, const sfTexture *texture, sfColor color)
{
	sfRectangleShape_setFillColor(rect, color);
	sfRectangleShape_setTexture(rect, texture, sfFalse);
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});

	sfVector2f pos = startpos;

	int i, n;
	for (i = 0; str[i]; i++) {
		if ('A' <= str[i] && str[i] <= 'Z') {
			n = str[i]-'A';
			sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*(4+n%3), blsizepx*(n/3), blsizepx, blsizepx});
		} else if ('a' <= str[i] && str[i] <= 'z') {
			n = str[i]-'a';
			sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*(4+n%3), blsizepx*(n/3), blsizepx, blsizepx});
		} else if ('0' <= str[i] && str[i] <= '9') {
			n = str[i]-'0';
			sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*7, blsizepx*n, blsizepx, blsizepx});
		} else {
			switch (str[i]) {
			case '-':
				sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*6, blsizepx*8, blsizepx, blsizepx});
				break;
			case '.':
				sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*4, blsizepx*9, blsizepx, blsizepx});
				break;
			case '?':
				sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*5, blsizepx*9, blsizepx, blsizepx});
				break;
			case '!':
				sfRectangleShape_setTextureRect(rect, (sfIntRect){blsizepx*6, blsizepx*9, blsizepx, blsizepx});
				break;
			case ' ':
				pos.x += BLOCKSIZE;
				continue;
			case '\n':
				pos.x = startpos.x;
				pos.y += BLOCKSIZE;
				continue;
			default:
				break;
			}
		}

		sfRectangleShape_setPosition(rect, pos);
		sfRenderWindow_drawRectangleShape(window, rect, NULL);
		pos.x += BLOCKSIZE;
	}
}

int display_gameover(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture, int progress)
{
	int ret = (progress <= tetris->height);
	if (!ret)
		progress = tetris->height;

	sfRectangleShape_setFillColor(rect, sfWhite);
	sfRectangleShape_setTexture(rect, texture, sfFalse);
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});
	sfRectangleShape_setTextureRect(rect, (sfIntRect){3*blsizepx, (tetris->level%10)*blsizepx, blsizepx, blsizepx});

	for (int y = 0; y < progress; y++) {
		for (int x = 0; x < tetris->width; x++) {
			sfRectangleShape_setPosition(rect, (sfVector2f){GUILEFT+x*BLOCKSIZE, GUITOP+y*BLOCKSIZE});
			sfRenderWindow_drawRectangleShape(window, rect, NULL);
		}
	}
	return ret;
}

void display_menu(sfRenderWindow *window, const tetris_t *tetris, sfRectangleShape *rect, const sfTexture *texture, int selected)
{
	int x0, y0;
	x0 = (sfRenderWindow_getSize(window).x-12*BLOCKSIZE)/2;
	y0 = (sfRenderWindow_getSize(window).y-BLOCKSIZE)/2;

	sfRenderWindow_clear(window, sfBlack);
	draw_text(window, "SELECT LEVEL", (sfVector2f){x0, y0-2*BLOCKSIZE}, rect, texture, sfWhite);

	sfRectangleShape_setTexture(rect, NULL, sfFalse);
	sfRectangleShape_setFillColor(rect, sfColor_fromRGB(252, 152, 56));
	sfRectangleShape_setSize(rect, (sfVector2f){BLOCKSIZE, BLOCKSIZE});
	sfRectangleShape_setPosition(rect, (sfVector2f){x0+BLOCKSIZE*(2*(selected%5)+1), y0+BLOCKSIZE*(selected/5)});
	sfRenderWindow_drawRectangleShape(window, rect, NULL);

	draw_text(window, "0 1 2 3 4\n5 6 7 8 9", (sfVector2f){x0+BLOCKSIZE, y0}, rect, texture, sfRed);
	draw_text(window, "ESC", (sfVector2f){BLOCKSIZE, BLOCKSIZE}, rect, texture, sfRed);
	draw_text(window, "TO RESUME", (sfVector2f){5*BLOCKSIZE, BLOCKSIZE}, rect, texture, sfWhite);
}

int fall_delay(unsigned level)
{
	int n;
	if (level <= 8)
		n = 48 - 10*(level/2) - 5*(level%2);
	else if (level <= 18)
		n = 8 - (level-1)/3;
	else if (level <= 28)
		n = 2;
	else
		n = 1;

	return 1000*n/60;
}
