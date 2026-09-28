// gcc main.c -o intro -lncursesw -lm && ./intro

#include <locale.h>
#include <math.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void quit(int sig) {
	(void)sig;
	endwin();
	exit(0);
}

enum {
	TITLE = 1,
	RED,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE,
	GREY
};

#define CHAR 0.07 // seconds per typed key
#define IDLE 0.6  // cursor blinks this long before typing starts

// scenes measure their own length with live off, then draw with it on
static int live;
static double now;

// scenes sit this many rows lower, clear of the title bar
static int top;

static void put(int y, int x, const char *s, attr_t attr) {
	if (!live || y < 0 || x < 0 || y + top >= LINES || x >= COLS)
		return;
	attrset(attr);
	mvaddstr(y + top, x, s);
}

static void putn(int y, int x, const char *s, int n, attr_t attr) {
	if (!live || y < 0 || x < 0 || y + top >= LINES || x >= COLS)
		return;
	attrset(attr);
	mvaddnstr(y + top, x, s, n);
}

static int clampi(double v, int lo, int hi) {
	return v < lo ? lo : v > hi ? hi : v;
}

static void cursor(int y, int x, int solid) {
	if (solid || fmod(now, 1.0) < 0.5)
		put(y, x, " ", COLOR_PAIR(WHITE) | A_REVERSE);
}

// the prompt shows at t0 and the command is typed after a pause;
// returns the moment Enter is hit
static double command(int y, const char *cmd, double t0, double pause) {
	if (now < t0)
		return t0 + pause + strlen(cmd) * CHAR + 0.3;
	int len = strlen(cmd), n = clampi((now - t0 - pause) / CHAR, 0, len);
	double enter = t0 + pause + len * CHAR + 0.3;
	put(y, 1, "~", COLOR_PAIR(BLUE) | A_BOLD);
	put(y, 3, "$", COLOR_PAIR(WHITE));
	putn(y, 5, cmd, n, COLOR_PAIR(WHITE));
	if (now < enter)
		cursor(y, 5 + n, n > 0 && n < len);
	return enter;
}

// one scene: a command, its output, a moment to read it, then `clear`
static double finish(int y, double done, double hold) {
	return command(y, "clear", done, hold);
}

static const char *ART[] = {
	" ______________ ",	   "||            ||",	  "||            ||",
	"||            ||",	   "||            ||",	  "||____________||",
	"|______________|",	   " \\\\############\\\\",  "  \\\\############\\\\",
	"   \\      ____    \\", "    \\_____\\___\\____\\",
};
#define ART_H 11

static const char *INFO[][2] = {
	{ "Role", "Frontend → Full-stack" },
	{ "Origin", "Self-taught" },
	{ "Uptime", "27 years" },
	{ "Packages", "TypeScript, Vue, Nuxt, Vite (npm)" },
	{ "Shell", "bash 5.2.37" },
	{ "WM", "i3" },
	{ "Tooling", "nvim, tmux, xterm, claude" },
	{ "Languages", "en, ru" },
};

static double whoami(void) {
	double out = command(1, "whoami", 0, IDLE);
	for (int i = 0; i < ART_H; i++) {
		if (now < out + i * 0.06)
			break;
		put(3 + i, 2, ART[i], COLOR_PAIR(RED) | A_BOLD);
		int y = 3 + i, x = 26;
		if (i == 0) {
			put(y, x, "dragunovartem99", COLOR_PAIR(RED) | A_BOLD);
			put(y, x + 15, "@", COLOR_PAIR(WHITE));
			put(y, x + 16, "debian", COLOR_PAIR(RED) | A_BOLD);
		} else if (i == 1)
			put(y, x, "----------------------", COLOR_PAIR(WHITE));
		else if (i < 10) {
			put(y, x, INFO[i - 2][0], COLOR_PAIR(RED) | A_BOLD);
			put(y, x + strlen(INFO[i - 2][0]), ": ", COLOR_PAIR(WHITE));
			put(y, x + strlen(INFO[i - 2][0]) + 2, INFO[i - 2][1],
				COLOR_PAIR(WHITE));
		}
	}
	return finish(3 + ART_H + 1, out + ART_H * 0.06, 4.5);
}

static const struct {
	const char *topic;
	double progress;
} TOPICS[] = {
	{ "Computer science fundamentals", 0.40 },
	{ "Big picture of the Web architecture", 0.65 },
	{ "Object-oriented programming", 0.35 },
	{ "Automation, better DX on Linux", 0.85 },
	{ "Agentic coding systems", 0.50 },
};
#define TOPICS_N 5
#define BAR 20

static const int GRADIENT[] = { GREEN, CYAN, BLUE };

static double exploring(void) {
	double out = command(1, "./exploring --now", 0, IDLE);
	if (now >= out)
		put(3, 1, "Exploring right now", COLOR_PAIR(GREEN) | A_BOLD);
	for (int i = 0; i < TOPICS_N; i++) {
		double s = now - out - 0.3 - i * 0.2;
		if (s < 0)
			break;
		double p = TOPICS[i].progress * (1 - pow(1 - fmin(s / 1.4, 1), 3));
		int y = 5 + i, fill = lround(p * BAR);
		char pct[8];
		snprintf(pct, sizeof pct, "%3d%%", (int)lround(p * 100));
		char num[4] = { '1' + i, '.' };
		put(y, 1, num, COLOR_PAIR(GREY));
		put(y, 4, TOPICS[i].topic, COLOR_PAIR(WHITE));
		for (int b = 0; b < BAR; b++)
			put(y, 41 + b, "━",
				b < fill ? COLOR_PAIR(GRADIENT[b * 3 / BAR]) | A_BOLD
						 : COLOR_PAIR(GREY) | A_DIM);
		put(y, 42 + BAR, pct, COLOR_PAIR(WHITE) | A_BOLD);
	}
	double done = out + 0.3 + (TOPICS_N - 1) * 0.2 + 1.4;
	return finish(5 + TOPICS_N + 1, done, 2.5);
}

// two pixels per cell, drawn with half blocks
static const char *KNIGHT[] = {
	"....#.#.....", "...#####....", "..#######...", ".##.######..",
	"##########..", "###...#####.", ".....######.", "....#######.",
	"..##########", ".###########",
};

static int knight_px(int r, int c, int blink) {
	if (r < 0 || r > 9)
		return 0;
	return KNIGHT[r][c] == '#' || (blink && r == 3 && c == 3);
}

// the knight hops a pixel, as if it were being moved, and blinks now and then
static void draw_knight(int y, int x, double t) {
	put(y + 5, x, "▔▔▔▔▔▔▔▔▔▔▔▔", COLOR_PAIR(GREY));
	int lift = fmod(t, 1.6) > 0.4 && fmod(t, 1.6) < 0.65,
		blink = fmod(t, 2.2) > 2.05;
	for (int r = -1; r < 5; r++)
		for (int c = 0; c < 12; c++) {
			int top = knight_px(2 * r + lift, c, blink),
				bot = knight_px(2 * r + 1 + lift, c, blink);
			if (top || bot)
				put(y + r, x + c, top && bot ? "█" : top ? "▀" : "▄",
					COLOR_PAIR(BLUE));
		}
}

static const char *BROCCOLI[] = {
	"  .o@@o.  ", " o@@@@@@o ", "  '@@@@'  ", "   \\|/    ", "    |     ",
};

// grows from the ground up
static void draw_broccoli(int y, int x, double t) {
	put(y + 5, x, "▔▔▔▔▔▔▔▔▔▔", COLOR_PAIR(YELLOW));
	int rows = clampi(t / 0.45, 0, 5);
	for (int r = 5 - rows; r < 5; r++)
		put(y + r, x, BROCCOLI[r],
			COLOR_PAIR(GREEN) | (r < 3 ? A_BOLD : 0));
}

// strummed strings ring out; notes drift up and fade
static void draw_guitar(int y, int x, double t) {
	static const double STRUM[] = { 0.3, 1.3, 2.3 };
	double ring = 0;
	for (int i = 0; i < 3; i++)
		if (t >= STRUM[i])
			ring = exp(-(t - STRUM[i]) * 2.2);
		else
			break;
	for (int s = 0; s < 4; s++) {
		put(y + 2 + s, x, "┃", COLOR_PAIR(WHITE) | A_BOLD);
		for (int k = 1; k < 17; k++) {
			double v = ring * sin(k * 0.9 + now * 40 + s * 1.7) *
					   sin(k * M_PI / 17);
			put(y + 2 + s, x + k, k % 5 == 0 ? "┼" : fabs(v) > 0.25 ? "~" : "─",
				COLOR_PAIR(k % 5 == 0 ? GREY : WHITE) |
					(fabs(v) > 0.25 ? A_BOLD : 0));
		}
	}
	for (int i = 0; i < 3; i++) {
		double age = t - STRUM[i];
		if (age < 0 || age > 1.2)
			continue;
		put(y + 1 - (int)(age / 0.5), x + 4 + 5 * i, i % 2 ? "\U000F075A" : "\uF001",
			COLOR_PAIR(i % 2 ? MAGENTA : YELLOW) | (age < 0.6 ? A_BOLD : A_DIM));
	}
}

static double hobbies(void) {
	double out = command(1, "./hobbies", 0, IDLE), t = now - out;
	if (t >= 0) {
		draw_knight(4, 3, t);
		draw_broccoli(4, 31, t);
		draw_guitar(3, 54, t);
		put(10, 3, "#chess-lover", COLOR_PAIR(BLUE) | A_BOLD);
		put(10, 30, "#vegetarian", COLOR_PAIR(GREEN) | A_BOLD);
		put(10, 54, "#hobby-guitarist", COLOR_PAIR(MAGENTA) | A_BOLD);
	}
	return finish(12, out + 3.8, 2.5);
}

static double (*const SCENES[])(void) = { whoami, exploring, hobbies };
#define SCENES_N 3

// i3's title bar over the window
static void draw_title(void) {
	for (int x = 0; x < COLS; x++)
		put(0, x, " ", COLOR_PAIR(TITLE));
	const char *title = "dragunovartem99@debian: ~";
	put(0, (COLS - (int)strlen(title)) / 2, title, COLOR_PAIR(TITLE));
}

static void init_colors(void) {
	start_color();
	use_default_colors();
	int rich = COLORS >= 256;
	init_pair(TITLE, COLOR_BLACK, COLOR_BLUE);
	init_pair(RED, COLOR_RED, -1);
	init_pair(GREEN, COLOR_GREEN, -1);
	init_pair(YELLOW, COLOR_YELLOW, -1);
	init_pair(BLUE, COLOR_BLUE, -1);
	init_pair(MAGENTA, COLOR_MAGENTA, -1);
	init_pair(CYAN, COLOR_CYAN, -1);
	init_pair(WHITE, COLOR_WHITE, -1);
	init_pair(GREY, rich ? 8 : COLOR_WHITE, -1);
}

int main(void) {
	setlocale(LC_ALL, "");
	signal(SIGINT, quit);
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(20);
	init_colors();

	double lens[SCENES_N], loop = 0;
	now = -1;
	for (int i = 0; i < SCENES_N; i++)
		loop += lens[i] = SCENES[i]();

	struct timespec start, ts;
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int ch; (ch = getch()) == ERR || ch == KEY_RESIZE;) {
		clock_gettime(CLOCK_MONOTONIC, &ts);
		double t = fmod(ts.tv_sec - start.tv_sec +
							(ts.tv_nsec - start.tv_nsec) / 1e9,
						loop);

		erase();
		live = 1;
		top = 0;
		draw_title();
		top = 1;
		for (int i = 0; i < SCENES_N; t -= lens[i++])
			if (t < lens[i]) {
				now = t;
				SCENES[i]();
				break;
			}
		refresh();
	}
	endwin();
}
