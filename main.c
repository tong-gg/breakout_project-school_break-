#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cp_functions.h"

#define True  1
#define False 0
#define WindowTitle  "Breakout 61"
#define WindowWidth  800
#define WindowHeight 700

Sound hit_paddle_sound, hit_brick_sound;
Sound hit_top_sound, end_sound;
Texture paddle_texture, ball_texture;
Texture brick_texture, background_texture;
Font big_font, small_font;

// Structure for storing info for objects, i.e. Paddle, Brick, Ball.
typedef struct
{
    float x, y;
    float width, height;
    float vel_x, vel_y;
    int destroyed;
} Object;

// Collision Detection between two objects a and b
int collide(Object a, Object b)
{
    if (a.x + a.width  < b.x || b.x + b.width  < a.x ||
        a.y + a.height < b.y || b.y + b.height < a.y)
        return False;
    else
        return True;
}

// Initial routine to load sounds, textures, and fonts.
int game_init()
{
    hit_paddle_sound = cpLoadSound("hitDown.wav");
    hit_brick_sound = cpLoadSound("hitUp.wav");
    hit_top_sound = cpLoadSound("hitTop.wav");
    end_sound = cpLoadSound("theEnd.wav");

    paddle_texture = cpLoadTexture("paddle.png");
    ball_texture = cpLoadTexture("ball.png");
    brick_texture = cpLoadTexture("brick.png");
    background_texture = cpLoadTexture("background.png");

    big_font = cpLoadFont("THSarabun.ttf", 60);
    small_font = cpLoadFont("THSarabun.ttf", 30);

    if (hit_paddle_sound == NULL || hit_brick_sound == NULL ||
        hit_top_sound == NULL || end_sound == NULL ||
        paddle_texture == NULL || ball_texture == NULL ||
        brick_texture == NULL || background_texture == NULL ||
        big_font == NULL || small_font == NULL)
        return False;
    return True;
}

int main(int argc, char *args[])
{
    enum { BALL_VEL_Y = -5, PADDLE_VEL_X = 7 };
    int running, n_bricks = 15, n_hits = 0, score = 0;
    char msg[80];
    Object bricks[n_bricks];
    Object ball = {WindowWidth/2-12, 350, 24, 24, 0, BALL_VEL_Y, False};
    Object paddle = {WindowWidth/2-62, WindowHeight-50, 124, 18, 0, 0, False};
    Event event;

    if (cpInit(WindowTitle, WindowWidth, WindowHeight) == False) {
        fprintf(stderr, "Window initialization failed!\n");
        exit(1);
    }

    if (game_init() == False) {
        fprintf(stderr, "Game initialization failed!\n");
        exit(1);
    }

    for (int n = 0, x = -10, y = 80; n < n_bricks; n++) {
        bricks[n].width = 55;
        bricks[n].height = 18;
        bricks[n].x = x;
        bricks[n].y = y;
        bricks[n].destroyed = False;
        x += bricks[n].width;
    }

    running = True;
    while (running) {
        cpClearScreen();
        cpDrawTexture(255, 255, 255,
            0, 0, WindowWidth, WindowHeight, background_texture);
        cpDrawTexture(255, 255, 255,
            paddle.x, paddle.y, paddle.width, paddle.height, paddle_texture);
        cpDrawTexture(255, 255, 255,
            ball.x, ball.y, ball.width, ball.height, ball_texture);
        sprintf(msg, "คะแนน %d", score);
        cpDrawText(255, 255, 255, 3, 3, msg, small_font, 0);

        for (int n = 0; n < n_bricks; n++) {
            if (!bricks[n].destroyed)
                cpDrawTexture(255, 255, 255,
                    bricks[n].x, bricks[n].y, bricks[n].width, bricks[n].height,
                    brick_texture);
        }

        if (ball.y + ball.width > WindowHeight || n_hits == n_bricks) {
            cpPlaySound(end_sound);
            cpDrawText(255, 255, 0, 400, 350, "จบเกมจบกัน", big_font, 1);
            cpSwapBuffers();
            while (1) {
                cbEventListener(&event);
                if (event.type == QUIT ||
                    event.type == KEYUP && event.key.keysym.sym == K_ESCAPE) {
                    running = False;
                    break;
                }
            }
        }
        cpSwapBuffers();

        while (cbEventListener(&event)) {
            if (event.type == QUIT ||
                 event.type == KEYUP && event.key.keysym.sym == K_ESCAPE) {
                running = False;
                break;
            }

            if (event.type == KEYDOWN) {
                if (event.key.keysym.sym == K_LEFT)
                    paddle.vel_x = -abs(PADDLE_VEL_X);
                if (event.key.keysym.sym == K_RIGHT)
                    paddle.vel_x = abs(PADDLE_VEL_X);
            }
            else
            if (event.type == KEYUP) {
                if (event.key.keysym.sym == K_LEFT)
                    paddle.vel_x = 0;
                if (event.key.keysym.sym == K_RIGHT)
                    paddle.vel_x = 0;
            }
        }
        paddle.x += paddle.vel_x;

        if (paddle.x < 0)
            paddle.x = 0;
        if (paddle.x + paddle.width > WindowWidth)
            paddle.x = WindowWidth - paddle.width;

        ball.x += ball.vel_x;
        ball.y += ball.vel_y;

        if (ball.x < 0 || ball.x + ball.width > WindowWidth)
            ball.vel_x = -ball.vel_x;

        if (ball.y < 0) {
            cpPlaySound(hit_top_sound);
            ball.vel_y = -ball.vel_y;
        }

        for (int n = 0; n < n_bricks; n++) {
            if (!bricks[n].destroyed &&
                collide(ball, bricks[n]) == True) {
                cpPlaySound(hit_brick_sound);
                ball.vel_y = -ball.vel_y;
                bricks[n].destroyed = True;
                n_hits++;
                score += 10;
                break;
            }
        }

        if (collide(ball, paddle) == True) {
            cpPlaySound(hit_paddle_sound);
            ball.vel_y = -ball.vel_y;
        }

        cpDelay(10);
    }
    cpCleanUp();
    return 0;
}
