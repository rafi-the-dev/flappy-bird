#include "raylib.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct Pipe {
  float x = 0;
  float pipe_height = 0;
  bool passed = false;
};
std::vector<Pipe> pipes = {};

Texture2D player_tex;
Rectangle player_sourceRec;
bool texture_loaded = false;

struct Player {
  Rectangle position = {50, 225, 64, 64};
  Vector2 origin = {position.width / 2, position.height / 2};
  float rotation = 0;
};

float player_jump_height = 500.0f;
float gravity = 1000.0f;
float velY = 0.0f;
float pipe_speed = 350.0f;
float spawnTimer = 0.0f;
float spawnInterval = 3.0f;
float gap = 250.0f;
Player player;

enum class Game_state { menu, playing, game_over };
struct Game {
  Game_state game_state = Game_state::menu;
  int score = 0;
};

Game game;

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 450, "Raylib - Flappy Bird");
  SetTargetFPS(60);

  player_tex = LoadTexture("resources/bird.png");

  if (player_tex.width > 0 && player_tex.height > 0) {
    SetTextureFilter(player_tex, TEXTURE_FILTER_POINT);
    player_sourceRec = {0.0f, 0.0f, (float)player_tex.width,
                        (float)player_tex.height};
    texture_loaded = true;

    float targetWidth = 64.0f;
    float scale = targetWidth / (float)player_tex.width;

    player.position.width = (float)player_tex.width * scale;
    player.position.height = (float)player_tex.height * scale;
    player.origin = {player.position.width / 2.0f,
                     player.position.height / 2.0f};
  } else {
    player_sourceRec = {0.0f, 0.0f, 64.0f, 64.0f};
    texture_loaded = false;
  }

  while (!WindowShouldClose()) {
    float delta = GetFrameTime();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    // --- Logic & State Engine ---
    if (game.game_state == Game_state::menu) {
      if (IsKeyPressed(KEY_SPACE)) {
        velY = -player_jump_height;
        game.game_state = Game_state::playing;
      }
    } else if (game.game_state == Game_state::playing) {
      for (auto &pipe : pipes) {
        pipe.x -= pipe_speed * delta;
      }

      spawnTimer += delta;
      if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        float randomHeight = (float)GetRandomValue(50, h - 250);
        pipes.push_back({(float)w, randomHeight, false});
      }

      if (IsKeyPressed(KEY_SPACE)) {
        velY = -player_jump_height;
      }
    } else if (game.game_state == Game_state::game_over) {
      if (IsKeyPressed(KEY_R)) {
        pipes.clear();
        player.position.y = h / 2.0f;
        velY = 0.0f;
        game.score = 0;
        spawnTimer = 0.0f;
        game.game_state = Game_state::menu;
      }
    }

    if (game.game_state == Game_state::playing ||
        game.game_state == Game_state::game_over) {
      velY += gravity * delta;
      player.position.y += velY * delta;
      player.rotation = std::clamp(velY / 12.0f, -30.0f, 70.0f);
    }

    // --- Collisions ---
    float playerLeft = player.position.x - player.origin.x;
    float playerRight = player.position.x + player.origin.x;
    float playerTop = player.position.y - player.origin.y;
    float playerBottom = player.position.y + player.origin.y;

    for (Pipe &pipe : pipes) {
      float pipeLeft = pipe.x;
      float pipeRight = pipe.x + 100.0f;
      float topPipeBot = pipe.pipe_height;
      float botPipeTop = pipe.pipe_height + gap;

      if (game.game_state == Game_state::playing) {
        bool hit_top = (playerRight > pipeLeft) && (playerLeft < pipeRight) &&
                       (playerTop < topPipeBot);
        bool hit_bottom = (playerRight > pipeLeft) &&
                          (playerLeft < pipeRight) &&
                          (playerBottom > botPipeTop);
        bool hit_floor = (playerBottom >= h);
        bool hit_ceiling = (playerTop <= 0);

        if (hit_top || hit_bottom || hit_floor || hit_ceiling) {
          game.game_state = Game_state::game_over;
        }

        if (!pipe.passed && playerLeft > pipeRight) {
          game.score++;
          pipe.passed = true;
        }
      }
    }

    if (game.game_state == Game_state::playing) {
      pipes.erase(std::remove_if(pipes.begin(), pipes.end(),
                                 [](const Pipe &p) { return p.x + 100 < 0; }),
                  pipes.end());
    }

    // --- Render System ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (const auto &pipe : pipes) {
      float botPipeTop = pipe.pipe_height + gap;
      DrawRectangle((int)pipe.x, 0, 100, (int)pipe.pipe_height, GREEN);
      DrawRectangle((int)pipe.x, (int)botPipeTop, 100, h - (int)botPipeTop,
                    GREEN);
    }

    if (texture_loaded) {
      DrawTexturePro(player_tex, player_sourceRec, player.position,
                     player.origin, player.rotation, WHITE);
    } else {
      DrawRectanglePro(player.position, player.origin, player.rotation, BLUE);
    }

    std::string score_text = "score: " + std::to_string(game.score);
    DrawText(score_text.c_str(), w / 3, h / 8, 80, BLACK);

    if (game.game_state == Game_state::menu) {
      DrawText("PRESS SPACE TO START", w / 4, h / 2, 24, DARKGRAY);
    } else if (game.game_state == Game_state::game_over) {
      DrawText("game_over", w / 3, h / 2, 50, BLACK);
      DrawText("press R to play again", w / 3, h / 2 + 100, 25, BLACK);
    }

    EndDrawing();
  }

  if (texture_loaded)
    UnloadTexture(player_tex);
  CloseWindow();
  return 0;
}
