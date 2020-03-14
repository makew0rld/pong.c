/* pong.c

Author:     makeworld
Date:       2019-01
License:    GPLv3 (https://www.gnu.org/licenses/gpl-3.0.en.html)

TODO:
- Set DELAY to be based on terminal width
- Test bouncing on the edge of the paddles
- Add a splash screen on start
- Optimize ball speed and gameplay in general
- Upload to Github
- Add flash?
- Add delay after hitting walls
*/

#include <curses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#define DELAY 20000
#define PADDLE_STEP 5  // Amount a paddle is moved by when a key is pressed
#define WIN_SCORE 2    // The score needed by either side to win a game and exit
#define BALL_STEP 1    // The amount the ball moves by each cycle, in one axis

// Vars used by random_ball function
int ball_x = 0, ball_y = 0;
int ball_step_x = 0, ball_step_y = 0;  // The change to ball_x and ball_y each loop
int max_field_x = 0, max_field_y = 0;
int l_paddle_x = 0, r_paddle_x = 0;
int paddle_w = 3;

// Used by min_sec_time
int min_sec_time_r[2];

void mvwprint_num(WINDOW *win, int y, int x, int n) {

    // Max of 10 right now, otherwise just display the integer as is
    if (n > 10) {
        mvwprintw(win, y, x, "%i", n);
        return;
    }

    switch (n) {
        case 0:
            mvwprintw(win, y,   x, "  ___"  );
            mvwprintw(win, y+1, x, " / _ \\");
            mvwprintw(win, y+2, x, "| | | |");
            mvwprintw(win, y+3, x, "| |_| |");
            mvwprintw(win, y+4, x, " \\___/");
            break;
        case 1:
            mvwprintw(win, y,   x, " _" );
            mvwprintw(win, y+1, x, "/ |");
            mvwprintw(win, y+2, x, "| |");
            mvwprintw(win, y+3, x, "| |");
            mvwprintw(win, y+4, x, "|_|");
            break;
        case 2:
            mvwprintw(win, y,   x, " ____"  );
            mvwprintw(win, y+1, x, "|___ \\");
            mvwprintw(win, y+2, x, "  __) |");
            mvwprintw(win, y+3, x, " / __/" );
            mvwprintw(win, y+4, x, "|_____|");
            break;
        case 3:
            mvwprintw(win, y,   x, " _____" );
            mvwprintw(win, y+1, x, "|___ /" );
            mvwprintw(win, y+2, x, "  |_ \\");
            mvwprintw(win, y+3, x, " ___) |");
            mvwprintw(win, y+4, x, "|____/" );
            break;
        case 4:
            mvwprintw(win, y,   x, " _  _"   );
            mvwprintw(win, y+1, x, "| || |"  );
            mvwprintw(win, y+2, x, "| || |_" );
            mvwprintw(win, y+2, x, "|__   _|");
            mvwprintw(win, y+3, x, "   |_|"  );
            break;
        case 5:
            mvwprintw(win, y,   x, " ____"  );
            mvwprintw(win, y+1, x, "| ___|" );
            mvwprintw(win, y+2, x, "|___ \\");
            mvwprintw(win, y+3, x, " ___) |");
            mvwprintw(win, y+4, x, "|____/" );
            break;
        case 6:
            mvwprintw(win, y,   x, "  __"   );
            mvwprintw(win, y+1, x, " / /_"  );
            mvwprintw(win, y+2, x, "| '_ \\");
            mvwprintw(win, y+3, x, "| (_) |");
            mvwprintw(win, y+4, x, " \\___/");
            break;
        case 7:
            mvwprintw(win, y,   x, " _____" );
            mvwprintw(win, y+1, x, "|___  |");
            mvwprintw(win, y+2, x, "   / /" );
            mvwprintw(win, y+3, x, "  / /"  );
            mvwprintw(win, y+4, x, " /_/ "  );
            break;
        case 8:
            mvwprintw(win, y,   x,  " ___" );
            mvwprintw(win, y+1, x, " ( _ )");
            mvwprintw(win, y+2, x, " / _ \\");
            mvwprintw(win, y+3, x, "| (_) |");
            mvwprintw(win, y+4, x, " \\___/");
            break;
        case 9:
            mvwprintw(win, y,   x, "  ___"   );
            mvwprintw(win, y+1, x, " / _ \\" );
            mvwprintw(win, y+2, x, "| (_) |" );
            mvwprintw(win, y+3, x, " \\__, |");
            mvwprintw(win, y+4, x, "   /_/"  );
            break;
        case 10:
            mvwprintw(win, y,   x, " _  ___"  );
            mvwprintw(win, y+1, x, "/ |/ _ \\");
            mvwprintw(win, y+2, x, "| | | | |");
            mvwprintw(win, y+3, x, "| | |_| |");
            mvwprintw(win, y+4, x, "|_|\\___/");
            break;
    }
}


int ball_random() {
    // Set ball to a random place and direction

    // num = rand() % (max_number + 1 - minimum_number) + minimum_number
    ball_y = rand() % ((max_field_y - 2) + 1 - 1) + 1;
    // ball_x is somewhere between the paddles, at least 10 units away from each one
    ball_x = rand() % ((r_paddle_x - 10) + 1 - (l_paddle_x + paddle_w + 10)) + (l_paddle_x + paddle_w + 10);

    // Going up or down is random
    if (rand() % 2 == 0) {
        ball_step_y = BALL_STEP;
    } else {
        ball_step_y = -BALL_STEP;
    }
    // Ball tries to go to the centre (x-axis wise) when starting
    if (ball_x < (max_field_x/2)) {
        ball_step_x = BALL_STEP;
    } else {
        ball_step_x = -BALL_STEP;
    }
}


int * min_sec_time(int secs) {
    min_sec_time_r[0] = secs/60;
    min_sec_time_r[1] = secs - ((secs/60) * 60);
    return min_sec_time_r;
}


int main() {
    // Window dimensions
    int max_y = 0, max_x = 0;  // Max terminal size 
    //max_field_y = 0, max_field_x = 0;  // Calculated after
    int scores_h = 8;
    // Ball stats
    //ball_x = 0, ball_y = 0;  // Start pos, set randomly later
    int ball_next_x = 0, ball_next_y = 0;  // Where the ball will be next loop
    // ball_step_x and ball_step_y init'd outside main
    // Misc
    char* debug;
    int key;  // The key pressed, reassigned each loop
    int l_points = 0, r_points = 0;  // Points for each side
    int *current_time;  // for use with min_secs_time()

    // Init
    setlocale(LC_CTYPE, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);  // getch is non-blocking
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, max_y, max_x);

    // Variables dependent on above inits
    int l_paddle_y = 20;
    int r_paddle_y = 20;
    // l_paddle_x and r_paddle_x initialized outside of main
    // paddle_w init outside of main
    
    // Display a message if the window is too small, then quit
    if (max_x < 130 || max_y < 40) {
        endwin();
        printf("Your terminal window is too small to play this game.\n");
        printf("It is only %i by %i\n", max_y, max_x);
        printf("Increase it to at least 130x40.\n");
        exit(1);
    }

    // Set terminal size dependent vars now
    WINDOW *field = newwin(max_y - scores_h, max_x, scores_h, 0);
    getmaxyx(field, max_field_y, max_field_x);
    l_paddle_x = max_x / 10;
    r_paddle_x = max_x - (max_x / 10) - paddle_w;
    WINDOW *scores = newwin(scores_h, max_x, 0, 0);
    int paddle_h = max_field_y / 5;
    WINDOW *l_paddle = newwin(paddle_h, paddle_w, l_paddle_y, l_paddle_x);
    WINDOW *r_paddle = newwin(paddle_h, paddle_w, r_paddle_y, r_paddle_x);

    // Set the loop delay (and therefore ball speed) based on the width of the terminal - TODO
    //const int DELAY = (-(50*max_x)) + 35000;

    // Color pairs
    init_pair(1, COLOR_BLACK, COLOR_MAGENTA);  // l_paddle
    init_pair(2, COLOR_BLACK, COLOR_BLUE);     // r_paddle
    init_pair(3, COLOR_GREEN, COLOR_BLACK);    // ball and win text
    init_pair(4, COLOR_RED,   COLOR_BLACK);    // Error - used mainly at the end just in case

    srand(time(NULL));
    ball_random();

    int start_time = time(NULL);

    while (1) {        
        // Check if a side has won
        if (l_points >= WIN_SCORE || r_points >= WIN_SCORE) {
            break;
        }


        werase(scores);
        werase(field);

        box(scores, 0, 0);
        box(field, 0, 0);

        // All drawing here
        // Color
        wbkgd(l_paddle, COLOR_PAIR(1));
        wbkgd(r_paddle, COLOR_PAIR(2));
        // Labels - scores and run time
        mvwprint_num(scores, 1, 7, l_points);
        mvwprint_num(scores, 1, max_x - 10, r_points);
        // Print run time in minutes and seconds
        current_time = min_sec_time(time(NULL) - start_time);
        mvwprintw(scores, (scores_h/2)-1, (max_x/2) - 5, "%02i:%02i", *(current_time + 0), *(current_time + 1));
        // Ball
        wattron(field, COLOR_PAIR(3));
        mvwprintw(field, ball_y, ball_x, "●");
        wattroff(field, COLOR_PAIR(3));

        // Refresh all the windows in one go - this prevents flickering!
        wnoutrefresh(scores);  // Copy all the windows to the virtual screen first
        wnoutrefresh(field);
        wnoutrefresh(l_paddle);
        wnoutrefresh(r_paddle);
        doupdate();  // Draw all of them to the real screen at once

        usleep(DELAY);


        // All logic goes below

        // Moving the paddles with keys
        key = getch();
        // WS for left paddle
        if (key == 'w') {
            if (l_paddle_y - PADDLE_STEP < scores_h + 1) {  // If moving it like normal would collide...
                // Move it to the edge
                mvwin(l_paddle, scores_h + 1, l_paddle_x);
                l_paddle_y = scores_h;
            } else {
                mvwin(l_paddle, l_paddle_y - PADDLE_STEP, l_paddle_x);
                l_paddle_y -= PADDLE_STEP;
            }
        } else if (key == 's') {
            if (l_paddle_y + PADDLE_STEP + paddle_h > max_y - 1) {
                mvwin(l_paddle, max_y - 1 - paddle_h, l_paddle_x);
                l_paddle_y = max_y - paddle_h;
            } else {
                mvwin(l_paddle, l_paddle_y + PADDLE_STEP, l_paddle_x);
                l_paddle_y += PADDLE_STEP;
            }
        // Arrow keys for right paddle
        } else if (key == KEY_UP) {
            if (r_paddle_y - PADDLE_STEP < scores_h + 1) {
                mvwin(r_paddle, scores_h + 1, r_paddle_x);
                r_paddle_y = scores_h;
            } else {
                mvwin(r_paddle, r_paddle_y - PADDLE_STEP, r_paddle_x);
                r_paddle_y -= PADDLE_STEP;
            }
        } else if (key == KEY_DOWN) {
            if (r_paddle_y + paddle_h + PADDLE_STEP > max_y - 1) {
                mvwin(r_paddle, max_y - 1 - paddle_h, r_paddle_x);
                r_paddle_y = max_y - paddle_h;
            } else {
                mvwin(r_paddle, r_paddle_y + PADDLE_STEP, r_paddle_x);
                r_paddle_y += PADDLE_STEP;
            }
        } else if (key == ' ') {
            ball_random();
            continue;
        }


        // Ball bouncing
        ball_next_x = ball_x + ball_step_x;
        ball_next_y = ball_y + ball_step_y;
        // Check field borders first
        if (ball_next_x < 1) {
            // Ball hit the left side
            beep();
            r_points += 1;
            ball_random();
            continue;  // Don't advance the ball or check paddle collisions
        }
        else if (ball_next_x >= max_field_x - 1) {
            // Ball hit right side
            beep();
            l_points += 1;
            ball_random();
            continue;
        } else if (ball_next_y >= max_field_y - 1 || ball_next_y < 1) {
            ball_step_y *= -1;
        
        // Now check for bouncing off the paddles
        // Left paddle check
        } else if (ball_next_x < l_paddle_x + paddle_w                  // Going to go past/into the paddle
                    && ball_next_x > l_paddle_x                         // But not behind the paddle already
                    && ball_next_y + scores_h >= l_paddle_y             // Below the top of the paddle - scores_h is added bc ball_y is within the field only
                    && ball_next_y + scores_h <= l_paddle_y + paddle_h  // Above the bottom of the paddle
                    ) {
            ball_step_x *= -1;
        // Right paddle check
        } else if (ball_next_x > r_paddle_x - 1  // The -1 is necessary for a proper bounce
                    && ball_next_x < r_paddle_x + paddle_w
                    && ball_next_y + scores_h >= r_paddle_y
                    && ball_next_y + scores_h <= r_paddle_y + paddle_h
                    ) {
            ball_step_x *= -1;
        }

        ball_x += ball_step_x;
        ball_y += ball_step_y;

    }
    
    clear();

    char buff[127];

    // Open ASCII art files for winning
    if (l_points == WIN_SCORE) {
        attron(COLOR_PAIR(3));
        //mvprintw(max_y/2, (max_x/2) - 7, "Left side won!");
        FILE *fp = fopen("left_won.txt", "r");
        int i = 3;  // Half the height of the ASCII art
        while (fgets(buff, 127, (FILE*)fp) != NULL) {
            mvprintw(max_y/2 - i, max_x/2 - 31, buff);  // 31 is half the width
            i--;
        }
        fclose(fp);
        attroff(COLOR_PAIR(3));
    } else if (r_points == WIN_SCORE) {
        attron(COLOR_PAIR(3));
        //mvprintw(max_y/2, (max_x/2) - 7, "Right side won!");
        FILE *fp = fopen("right_won.txt", "r");
        int i = 3;  // Half the height of the ASCII art
        while (fgets(buff, 127, (FILE*)fp) != NULL) {
            mvprintw(max_y/2 - i, max_x/2 - 34, buff);  // 34 is half the width
            i--;
        }
        fclose(fp);
        attroff(COLOR_PAIR(3));
    } else {
        attron(COLOR_PAIR(4));
        mvprintw(max_y/2, (max_x/2) - 15, "Something unexpected happened.");
        attroff(COLOR_PAIR(4));
    }

    mvprintw(max_y - 1, max_x/2 - 11, "Press any key to quit.");
    refresh();

    // Wait, then wait for a keypress, then quit
    sleep(2);
    nodelay(stdscr, FALSE);
    getch();
    endwin();
    return 0;
}
