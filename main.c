#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "cp_functions.h"

#define WindowTitle "Breakout 62"
#define WindowWidth 800
#define WindowHeight 700

Sound hit_paddle_sound, hit_brick_sound;
Sound hit_top_sound, end_sound;
Texture paddle_texture, ball_texture;
Texture brick_texture, background_texture;
Font big_font, small_font;

// Structure for storing info for objects, i.e. Paddle, Brick, Ball.
typedef struct
{
   float pos_x, pos_y;
   float vel_x, vel_y;
   float width, height;
   int active;
} Object;

// Collision Detection between objects a and b
int collide(Object a, Object b)
{
   if (a.pos_x + a.width < b.pos_x || b.pos_x + b.width < a.pos_x ||
       a.pos_y + a.height < b.pos_y || b.pos_y + b.height < a.pos_y)
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

   return hit_paddle_sound && hit_brick_sound && hit_top_sound && end_sound &&
      paddle_texture && ball_texture && brick_texture && background_texture &&
      big_font && small_font;
}

// Special Objects
Texture ghost_texture;
Object ghost = {0, WindowHeight / 2, 0, 0, 50, 50, True};
int ready_to_swap=1;
pthread_t ghost_thread;

void *ghostBot(void *ptr)
{  double timestep=0;
   float vel_x=5;

   while (True) {
      if (!ready_to_swap) {
         usleep(1000);
         continue;
      }
      if (ghost.pos_x < 0) {
         ghost.pos_x = 0;
         vel_x *= -1.;
      }
      else if (ghost.pos_x > WindowWidth - ghost.width) {
         ghost.pos_x = WindowWidth - ghost.width;
         vel_x *= -1.;
      }
      ghost.pos_x += vel_x;
      ghost.pos_y = WindowHeight/2 + 50*sin(8*M_PI*timestep/(WindowWidth - ghost.width));
      timestep += fabs(vel_x);
      ready_to_swap = 0;
   }
}

int main(int argc, char *args[])
{
   enum {
      BALL_VEL_Y = -5,
      PADDLE_VEL_X = 7
   };
   int running, n_bricks = 15, n_hits = 0, score = 0;
   char msg[80];
   Object bricks[n_bricks];
   Object ball = {WindowWidth / 2 - 12, 350, 0, BALL_VEL_Y, 24, 24, True};
   Object paddle = {WindowWidth / 2 - 62, WindowHeight - 50, 0, 0, 124, 18, True};
   Event event;

   if (!cpInit(WindowTitle, WindowWidth, WindowHeight)) {
      fprintf(stderr, "Window initialization failed!\n");
      exit(1);
   }

   if (!game_init()) {
      fprintf(stderr, "Game initialization failed!\n");
      exit(1);
   }

   for (int n = 0, x = -10, y = 80; n < n_bricks; n++) {
      bricks[n].pos_x = x;
      bricks[n].pos_y = y;
      bricks[n].width = 55;
      bricks[n].height = 18;
      bricks[n].active = True;
      x += bricks[n].width;
   }

   ghost_texture = cpLoadTexture("ghost.png");
   pthread_create(&ghost_thread, NULL, ghostBot, (void *)NULL);

   running = True;
   while (running) {
      cpClearScreen();
      cpDrawTexture(255, 255, 255,
                    0, 0, WindowWidth, WindowHeight, background_texture);
      cpDrawTexture(255, 255, 255,
                    paddle.pos_x, paddle.pos_y, paddle.width, paddle.height, paddle_texture);
      cpDrawTexture(255, 255, 255,
                    ball.pos_x, ball.pos_y, ball.width, ball.height, ball_texture);
      cpDrawTexture(255, 255, 255,
                    ghost.pos_x, ghost.pos_y, ghost.width, ghost.height, ghost_texture);
      ready_to_swap = 1;
      for (int n = 0; n < n_bricks; n++) {
         if (bricks[n].active)
            cpDrawTexture(255, 255, 255,
                          bricks[n].pos_x, bricks[n].pos_y, bricks[n].width, bricks[n].height,
                          brick_texture);
      }
      sprintf(msg, "คะแนน %d", score);
      cpDrawText(255, 255, 255, 3, 3, msg, small_font, 0);
      
      if (ball.pos_y + ball.width > WindowHeight || n_hits == n_bricks) {
         cpPlaySound(end_sound);
         cpDrawText(255, 255, 0, 400, 350, "จบเกมจบกัน", big_font, 1);
         cpSwapBuffers();
         while (True) {
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
         else if (event.type == KEYUP) {
            if (event.key.keysym.sym == K_LEFT)
               paddle.vel_x = 0;
            if (event.key.keysym.sym == K_RIGHT)
               paddle.vel_x = 0;
         }
      }
      paddle.pos_x += paddle.vel_x;

      if (paddle.pos_x < 0)
         paddle.pos_x = 0;
      if (paddle.pos_x + paddle.width > WindowWidth)
         paddle.pos_x = WindowWidth - paddle.width;

      ball.pos_x += ball.vel_x;
      ball.pos_y += ball.vel_y;

      if (ball.pos_x < 0 || ball.pos_x + ball.width > WindowWidth)
         ball.vel_x = -ball.vel_x;

      if (ball.pos_y < 0) {
         cpPlaySound(hit_top_sound);
         ball.vel_y = -ball.vel_y;
      }

      for (int n = 0; n < n_bricks; n++) {
         if (bricks[n].active && collide(ball, bricks[n])) {
            cpPlaySound(hit_brick_sound);
            ball.vel_y = -ball.vel_y;
            bricks[n].active = False;
            n_hits++;
            score += 10;
            break;
         }
      }

      if (collide(ball, paddle)) {
         cpPlaySound(hit_paddle_sound);
         ball.vel_y = -ball.vel_y;
      }

      cpDelay(10);
   }
   cpCleanUp();
   return 0;
}