
#include "delay.h"
#include "rng.h"
#include "io.h"
#include "memory.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define YELLOW   rgb565(255,255,0)
#define BLUE     rgb565(0,0,127)
#define SAMPLE_RATE    384000
#define BUFFER_MS      25

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


static float    sfx_phase;
static uint32_t sfx_sample;
static uint32_t sfx_note;
static int      sfx_active;
uint32_t        game_delay;

static float sfx_freq[3];

void randomize_sfx(void)
{
    static const float semitone_ratio[12] = {
        1.000000000f, 1.059463094f, 1.122462048f,
        1.189207115f, 1.259921050f, 1.334839854f,
        1.414213562f, 1.498307077f, 1.587401052f,
        1.681792831f, 1.781797436f, 1.887748625f
    };

    const uint32_t base_fq = 523.251130f;
    const uint32_t note0 = (rng_rnd() % 12u);
    const uint32_t note1 = (rng_rnd() % 12u);
    const uint32_t note2 = (rng_rnd() % 12u);

    sfx_freq[0] = base_fq * semitone_ratio[note0 % 12u] * (float)(1u << (note0 / 12u));
    sfx_freq[1] = base_fq * semitone_ratio[note1 % 12u] * (float)(1u << (note1 / 12u));
    sfx_freq[2] = base_fq * semitone_ratio[note2 % 12u] * (float)(1u << (note2 / 12u));
}

static const uint32_t sfx_length[] = {
    (SAMPLE_RATE * 50u) / 1000u,
    (SAMPLE_RATE * 50u) / 1000u,
    (SAMPLE_RATE * 100u) / 1000u
};

static inline void sfx_trigger(void)
{
    sfx_phase  = 0.0f;
    sfx_sample = 0u;
    sfx_note   = 0u;
    sfx_active = 1;
}

void audio_feed(int32_t *restrict snd_buf, uint32_t samples)
{
    static const float two_pi = 6.2831853071795864769f;

    for (uint32_t i = 0u; i < samples; i+=2) {
        float sample = 0.0f;

        if (sfx_active) {
            const uint32_t note_len = sfx_length[sfx_note];
            const float progress = (float)sfx_sample / (float)note_len;
            const float envelope = 0.24f * (1.0f - progress);
            const float phase_incr = sfx_freq[sfx_note] / (float)SAMPLE_RATE;

            sample = sinf(sfx_phase * two_pi) * envelope;

            sfx_phase += phase_incr;
            if (sfx_phase >= 1.0f) {
                sfx_phase -= 1.0f;
            }

            ++sfx_sample;
            if (sfx_sample >= note_len) {
                sfx_sample = 0u;
                ++sfx_note;

                if (sfx_note >= (uint32_t)(sizeof(sfx_freq) /
                                           sizeof(sfx_freq[0]))) {
                    sfx_active = 0;
                }
            }
        }

        snd_buf[i+0] = snd_buf[i+1] = (int32_t)(sample * 2147483647.0f);
    }
}

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
    game_delay = 80000;
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
    int btn_up    = button(22);
    int btn_left  = button(27);
    int btn_down  = button(28);
    int btn_right = button(29);

    static Direction prev_dir;

    if(btn_up && (prev_dir != DIR_DOWN))
        dir = DIR_UP;
    if(btn_down && (prev_dir != DIR_UP))
        dir = DIR_DOWN;
    if(btn_left && (prev_dir != DIR_RIGHT))
        dir = DIR_LEFT;
    if(btn_right && (prev_dir != DIR_LEFT))
        dir = DIR_RIGHT;

    prev_dir = dir;
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
        randomize_sfx();
        sfx_trigger();
        game_delay -= 2000;

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
        if(button(28)|button(29)|button(27)|button(22)){
            game_over=0;
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
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    game_init();

    uint32_t game_ticks = 0;

    for (;;){

        audio_service();

        if(game_ticks++ == game_delay){
            lcd_clear(BLUE);
            game_update();
            game_draw();
            lcd_flush_fb();
            game_ticks = 0;
        }
    }

    return 0;
}

