# Usage: make (build), make run (preview), make demo (update GIF).
# Each rule reads: output: files it depends on
# Make runs the indented commands when a dependency is newer than the output.

CC = gcc
CFLAGS ?= -O2 -Wall -Wextra
LDLIBS = -lncursesw -lm
AGG ?= agg

# Remove incomplete output files if a command fails.
.DELETE_ON_ERROR:
.PHONY: demo run

# main.c -> intro: compile the terminal animation into an executable.
intro: main.c Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) main.c -o intro $(LDLIBS)

run: intro
	./intro

demo: demo.gif

# intro -> raw.cast: record terminal output and timestamps as text.
# Record slightly past one loop so the next step can trim it precisely.
# $$ passes a dollar sign to the shell instead of expanding it in Make.
raw.cast: intro Makefile
	@duration=$$(./intro --duration); \
	stop=$$(awk -v duration="$$duration" 'BEGIN { print duration + 0.6 }'); \
	TERM=xterm-256color asciinema rec --overwrite --cols 67 --rows 17 \
		-c "timeout --foreground $$stop ./intro" raw.cast

# raw.cast -> demo.cast: keep one loop, excluding terminal exit cleanup.
demo.cast: raw.cast intro Makefile
	jq -c --argjson duration "$$(./intro --duration)" \
		'select(type == "object" or (.[0] < $$duration and .[1] == "o"))' raw.cast > demo.cast

# demo.cast -> raw.gif: render the recording using the bundled font.
raw.gif: demo.cast Makefile $(wildcard fonts/*)
	$(AGG) --renderer fontdue --font-dir fonts --font-family VT323 --font-size 40 \
		--line-height 1 --fps-cap 30 --last-frame-duration 0 \
		--theme 111111,c5c8c6,282a2e,cc6666,b5bd68,f0c674,81a2be,b294bb,8abeb7,c5c8c6,969896,cc6666,b5bd68,f0c674,81a2be,b294bb,8abeb7,ffffff \
		demo.cast raw.gif

# raw.gif -> demo.gif: add the CRT effect for the README image.
demo.gif: raw.gif crt.ffscript Makefile
	ffmpeg -y -i raw.gif -filter_complex_script crt.ffscript -fps_mode passthrough -loop 0 demo.gif
