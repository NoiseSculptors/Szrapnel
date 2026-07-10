
#include "delay.h"
#include "rng.h"
#include "io.h"
#include "memory.h"
#include <stdint.h>

#define YELLOW   rgb565(255,255,0)
#define BLUE     rgb565(0,0,127)

// Size of one "cell" on the screen (in pixels)
#define CELL_SIZE   8
// Top margin (for score text)
#define TOP_MARGIN  8

// How many cells we have horizontally and vertically for the game area
#define GRID_W    (WIDTH / CELL_SIZE)
#define GRID_H    ((HEIGHT - TOP_MARGIN) / CELL_SIZE)

#define MAX_SNAKE_LENGTH 64

// Game timing:
// - main loop runs every GAME_TICK_MS
// - snake moves every GAME_TICKS_PER_STEP ticks
#define GAME_TICK_MS        10   // ms between loop iterations
#define GAME_TICKS_PER_STEP 12   // 10 * 12 = 120 ms per snake move

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3
} Direction;

static Point     snake[MAX_SNAKE_LENGTH];
static int       snake_length;
static Direction dir;
static Point     apple;
static int       game_over;
static int       score;

static void place_apple(void)
{
    while (1) {
        int x = (int)(rng_rnd() % GRID_W);
        int y = (int)(rng_rnd() % GRID_H);

        int on_snake = 0;
        for (int i = 0; i < snake_length; ++i) {
            if (snake[i].x == x && snake[i].y == y) {
                on_snake = 1;
                break;
            }
        }
        if (!on_snake) {
            apple.x = x;
            apple.y = y;
            break;
        }
    }
}

static void game_init(void)
{
    // Start with a 3-block snake in the middle of the grid
    snake_length = 3;
    snake[0].x = GRID_W / 2;
    snake[0].y = GRID_H / 2;
    snake[1].x = snake[0].x - 1;
    snake[1].y = snake[0].y;
    snake[2].x = snake[1].x - 1;
    snake[2].y = snake[1].y;

    dir       = DIR_RIGHT;
    score     = 0;
    game_over = 0;

    place_apple();
}

static void handle_input(void)
{
    static int prev_btn0 = 0;
    static int prev_btn1 = 0;

    int btn0 = button(28); // LEFT
    int btn1 = button(29); // RIGHT

    // NOTE: if buttons are active-low, just invert:
    // int pressed0 = !btn0; int pressed1 = !btn1;
    // For now we assume "1 = pressed"
    int pressed0 = btn0;
    int pressed1 = btn1;

    // On rising edge: just pressed now (was 0, now 1)
    if (pressed0 && !prev_btn0) {
        // turn left: dir - 1 (mod 4)
        dir = (Direction)((dir + 3) & 3);
    }
    if (pressed1 && !prev_btn1) {
        // turn right: dir + 1 (mod 4)
        dir = (Direction)((dir + 1) & 3);
    }

    prev_btn0 = pressed0;
    prev_btn1 = pressed1;
}

static void game_step(void)
{
    if (game_over) {
        return;
    }

    // current head
    Point head = snake[0];

    // move head one cell in current direction
    if (dir == DIR_UP) {
        head.y -= 1;
    } else if (dir == DIR_DOWN) {
        head.y += 1;
    } else if (dir == DIR_LEFT) {
        head.x -= 1;
    } else if (dir == DIR_RIGHT) {
        head.x += 1;
    }

    // wrap at edges
    if (head.x < 0)         head.x = GRID_W - 1;
    if (head.x >= GRID_W)   head.x = 0;
    if (head.y < 0)         head.y = GRID_H - 1;
    if (head.y >= GRID_H)   head.y = 0;

    // collision with our own body?
    for (int i = 0; i < snake_length; ++i) {
        if (snake[i].x == head.x && snake[i].y == head.y) {
            game_over = 1;

            // flash LEDs when game over 
            for(int i=0;i<30;i++){
                led_on(i);
                delay_ms(3);
            }

            led_off();
            return;
        }
    }

    // move body: from tail to head
    for (int i = snake_length; i > 0; --i) {
        snake[i] = snake[i - 1];
    }
    snake[0] = head;

    // apple eaten?
    if (head.x == apple.x && head.y == apple.y) {
        if (snake_length < MAX_SNAKE_LENGTH - 1) {
            snake_length++;
        }
        score++;

        // flash a LED when eating
        for(int i=0;i<6;i++){
            led_on(i);
            delay_ms(3);
        }

        led_off();

        place_apple();
    }
}

static void game_update(void)
{
    static int tick_counter = 0;

    handle_input(); // turn left/right as soon as button is pressed

    if (game_over) {
        if(button(28)|button(29)){
            game_over=1;
            game_init();    
        }
        return;
    }

    if (++tick_counter >= GAME_TICKS_PER_STEP) {
        tick_counter = 0;
        game_step(); // move snake one cell
    }
}

static void draw_cell(int gx, int gy, int filled)
{
    int x1 = gx * CELL_SIZE;
    int y1 = TOP_MARGIN + gy * CELL_SIZE; // game starts below top margin
    int x2 = x1 + CELL_SIZE - 1;
    int y2 = y1 + CELL_SIZE - 1;

    if (filled) {
        lcd_box_filled(x1, y1, x2, y2, rgb565(255,0,0));
        lcd_box(x1, y1, x2, y2, YELLOW);
    } else {
        lcd_box(x1, y1, x2, y2, YELLOW);
    }
}

static void draw_score(void)
{
    char buf[16];
    int s = score;
    char tmp[10];
    int len = 0;

    if (s == 0) {
        tmp[len++] = '0';
    } else {
        while (s > 0 && len < 10) {
            tmp[len++] = (char)('0' + (s % 10));
            s /= 10;
        }
    }
    // reverse into buf
    for (int i = 0; i < len; ++i) {
        buf[i] = tmp[len - 1 - i];
    }
    buf[len] = '\0';

    lcd_printf(0, 0, 1, 1, YELLOW, BLUE, "Score: %s", buf);
}

static void game_draw(void)
{
    // draw snake
    for (int i = 0; i < snake_length; ++i) {
        draw_cell(snake[i].x, snake[i].y, 1);
    }

    // draw apple (as hollow cell)
    draw_cell(apple.x, apple.y, 0);

    // draw score on top line
    draw_score();

    if (game_over) {
        lcd_printf(44, 36, 1, 1, YELLOW, BLUE, "GAME OVER");
    }
}

int main(void){

    io_init();
    game_init();

    for (;;){

        lcd_clear(BLUE);
        game_update();
        game_draw();
        lcd_flush_fb();

        delay_ms(GAME_TICK_MS);
    }

    return 0;
}

