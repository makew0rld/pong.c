# pong.c

This is a pong style game written in C, using ncurses. It was made for a school project.

This is designed to work under Linux systems only. Compile it with:
```bash
gcc pong.c -o pong -lcurses
```

## Issues
The speed of the ball does not scale with the window size. You will have to change the line that reads
```C
#define DELAY 20000
```
manually to change the ball speed.

Licensed under the MIT License.
