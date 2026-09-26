# Artem Dragunov

<img src="demo.gif" alt="A small terminal window: whoami, what I'm exploring right now, my motto, and my hobbies — chess, greens and guitar" />

<details>
<summary>Behind the scenes</summary>

It's a real program, not a screen recording of me typing:

```sh
gcc main.c -o artem -lncursesw -lm && ./artem
```

Recorded with [asciinema](https://asciinema.org) and [agg](https://github.com/asciinema/agg), in Tomorrow Night colors:

```sh
TERM=xterm-256color asciinema rec --cols 80 --rows 19 -c "timeout --foreground 32.3 ./artem" raw.cast
# keep exactly one 31.7s loop, without the exit that would blank the last frame
jq -c 'select(type == "object" or (.[0] < 31.7 and .[1] == "o"))' raw.cast > demo.cast
agg --font-family "JetBrainsMonoNL Nerd Font Mono" --font-size 18 --fps-cap 30 --last-frame-duration 0 \
    --theme 1d1f21,c5c8c6,282a2e,cc6666,b5bd68,f0c674,81a2be,b294bb,8abeb7,c5c8c6,969896,cc6666,b5bd68,f0c674,81a2be,b294bb,8abeb7,ffffff \
    demo.cast demo.gif
```

</details>
