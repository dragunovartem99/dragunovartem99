// gcc main.c -o intro -lncursesw -lm && ./intro

#include <locale.h>
#include <math.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void quit(int sig) {
	(void)sig;
	endwin();
	exit(0);
}

enum {
	STATUS = 1,
	WHITE,
	GREY,
	TRACK
};

#define CHAR 0.07 // seconds per typed key
#define IDLE 0.6  // cursor blinks this long before typing starts

// scenes measure their own length with live off, then draw with it on
static int live;
static double now;

static void put(int y, int x, const char *s, attr_t attr) {
	if (!live || y < 0 || x < 0 || y >= LINES || x >= COLS)
		return;
	attrset(attr);
	mvaddstr(y, x, s);
}

static void putn(int y, int x, const char *s, int n, attr_t attr) {
	if (!live || y < 0 || x < 0 || y >= LINES || x >= COLS)
		return;
	attrset(attr);
	mvaddnstr(y, x, s, n);
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
	put(y, 1, "~", COLOR_PAIR(WHITE) | A_BOLD);
	put(y, 3, "$", COLOR_PAIR(WHITE));
	putn(y, 5, cmd, n, COLOR_PAIR(WHITE));
	if (now < enter)
		cursor(y, 5 + n, n > 0 && n < len);
	return enter;
}

// one scene: a command, its output, a moment to read it, then a fresh
// prompt until tmux switches to the next window
static double finish(int y, double done, double hold) {
	return command(y, "", done, hold);
}

static const char *ART[] = {
	" ______________ ",	   "||            ||",	  "||            ||",
	"||            ||",	   "||            ||",	  "||____________||",
	"|______________|",	   " \\\\############\\\\",  "  \\\\############\\\\",
	"   \\      ____    \\", "    \\_____\\___\\____\\",
};
#define ART_H 11

static const char *INFO[][2] = {
	{ "Role", "Frontend => Full-stack" },
	{ "Origin", "Self-taught" },
	{ "Uptime", "27 years" },
	{ "Packages", "TypeScript, Vue, Nuxt, Vite (npm)" },
	{ "Shell", "bash 5.2.37" },
	{ "WM", "i3" },
	{ "Tooling", "nvim, tmux, xterm, claude" },
	{ "Languages", "en, ru" },
	{ "Hobbies", "chess, guitar, Chinese tea" },
};
#define INFO_N (int)(sizeof INFO / sizeof *INFO)

static double whoami(void) {
	double out = command(0, "whoami", 0, IDLE);
	for (int i = 0; i < ART_H; i++) {
		if (now < out + i * 0.06)
			break;
		put(2 + i, 1, ART[i], COLOR_PAIR(GREY));
		int y = 2 + i, x = 23;
		if (i == 0) {
			put(y, x, "dragunovartem99", COLOR_PAIR(WHITE) | A_BOLD);
			put(y, x + 15, "@", COLOR_PAIR(WHITE));
			put(y, x + 16, "debian", COLOR_PAIR(WHITE) | A_BOLD);
		} else if (i == 1)
			put(y, x, "----------------------", COLOR_PAIR(WHITE));
		else if (i - 2 < INFO_N) {
			put(y, x, INFO[i - 2][0], COLOR_PAIR(WHITE) | A_BOLD);
			put(y, x + strlen(INFO[i - 2][0]), ": ", COLOR_PAIR(WHITE));
			put(y, x + strlen(INFO[i - 2][0]) + 2, INFO[i - 2][1],
				COLOR_PAIR(WHITE));
		}
	}
	return finish(2 + ART_H + 1, out + ART_H * 0.06, 10);
}

static const struct {
	const char *topic;
	double progress;
} TOPICS[] = {
	{ "Computer science fundamentals", 0.40 },
	{ "Big picture of the Web architecture", 0.65 },
	{ "Object-oriented programming", 0.35 },
	{ "Automation, better DX on Linux", 0.85 },
	{ "CI/CD: reusable GitHub Actions", 0.60 },
	{ "Agentic coding systems", 0.50 },
	{ "Backend: Node.js, SQLite, DuckDB", 0.20 },
	{ "Web performance: Core Web Vitals", 0.45 },
	{ "Web security: XSS, CWE", 0.30 },
};
#define TOPICS_N 9
#define BAR 20

static void heading(int y, const char *title) {
	put(y, 1, title, COLOR_PAIR(WHITE) | A_BOLD);
}

static double exploring(void) {
	double out = command(0, "./exploring --now", 0, IDLE);
	if (now >= out)
		heading(2, "Exploring right now");
	for (int i = 0; i < TOPICS_N; i++) {
		double s = now - out - 0.3 - i * 0.2;
		if (s < 0)
			break;
		double p = TOPICS[i].progress * (1 - pow(1 - fmin(s / 1.4, 1), 3));
		int y = 4 + i, fill = lround(p * BAR);
		char pct[8];
		snprintf(pct, sizeof pct, "%3d%%", (int)lround(p * 100));
		char num[4] = { '1' + i, '.' };
		put(y, 1, num, COLOR_PAIR(GREY));
		put(y, 4, TOPICS[i].topic, COLOR_PAIR(WHITE));
		// the percentage ends one column short of the right edge, like the
		// left margin, and the bar sits just before it
		for (int b = 0; b < BAR; b++)
			put(y, COLS - 6 - BAR + b, "━",
				b < fill ? COLOR_PAIR(WHITE) : COLOR_PAIR(TRACK));
		put(y, COLS - 5, pct, COLOR_PAIR(WHITE) | A_BOLD);
	}
	double done = out + 0.3 + (TOPICS_N - 1) * 0.2 + 1.4;
	return finish(4 + TOPICS_N + 1, done, 10);
}

static double (*const SCENES[])(void) = { whoami, exploring };
static const char *WINDOWS[] = { "whoami", "exploring" };
#define SCENES_N 2

// tmux's status line, one window per scene; the date is my birthday
static void draw_status(int active) {
	int y = LINES - 1, x = 0;
	char buf[32];
	for (int c = 0; c < COLS; c++)
		put(y, c, " ", COLOR_PAIR(STATUS));
	put(y, x, "[intro] ", COLOR_PAIR(STATUS));
	x += 8;
	for (int i = 0; i < SCENES_N; i++) {
		int last = (active + SCENES_N - 1) % SCENES_N;
		snprintf(buf, sizeof buf, "%d:%s%c ", i, WINDOWS[i],
				 i == active ? '*' : i == last ? '-' : ' ');
		put(y, x, buf, COLOR_PAIR(STATUS));
		x += strlen(buf);
	}
	const char *right = "\"debian\" 12-Apr-99";
	put(y, COLS - (int)strlen(right), right, COLOR_PAIR(STATUS));
}

static void init_colors(void) {
	start_color();
	use_default_colors();
	int rich = COLORS >= 256;
	init_pair(STATUS, COLOR_WHITE, COLOR_BLACK);
	init_pair(WHITE, COLOR_WHITE, -1);
	init_pair(GREY, rich ? 8 : COLOR_WHITE, -1);
	init_pair(TRACK, rich ? 237 : COLOR_BLACK, -1);
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
		for (int i = 0; i < SCENES_N; t -= lens[i++])
			if (t < lens[i]) {
				now = t;
				SCENES[i]();
				draw_status(i);
				break;
			}
		refresh();
	}
	endwin();
}
