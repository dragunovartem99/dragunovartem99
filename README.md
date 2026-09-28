# Artem Dragunov

<img src="demo.gif" alt="A small terminal in tmux: whoami and what I'm exploring right now" />

<details>
<summary>Behind the scenes</summary>
<br>

It's a real program, not a screen recording of me typing:

```sh
gcc main.c -o intro -lncursesw -lm && ./intro
```

With [asciinema](https://asciinema.org) and [agg](https://github.com/asciinema/agg), in Tomorrow Night colors and [VT323](fonts):

```sh
TERM=xterm-256color asciinema rec --cols 67 --rows 17 -c "timeout --foreground 15.40 ./intro" raw.cast
# keep one loop, drop the exit
jq -c 'select(type == "object" or (.[0] < 14.77 and .[1] == "o"))' raw.cast > demo.cast
agg --renderer fontdue --font-dir fonts --font-family VT323 --font-size 40 --line-height 1 --fps-cap 30 --last-frame-duration 0 \
    --theme 1d1f21,c5c8c6,282a2e,cc6666,b5bd68,f0c674,81a2be,b294bb,8abeb7,c5c8c6,969896,cc6666,b5bd68,f0c674,81a2be,b294bb,8abeb7,ffffff \
    demo.cast raw.gif
# CRT look: glow, scanlines, curvature, vignette
ffmpeg -i raw.gif -filter_complex_script crt.ffscript -fps_mode passthrough -loop 0 demo.gif
```

</details>
