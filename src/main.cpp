#include "raylib.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#pragma region imgui
#include "imgui.h"
#include "imguiThemes.h"
#include "rlImGui.h"
#pragma endregion
struct Pipe {
  float x = 0;
  float pipe_height = 0;
  bool passed = false;
};
std::vector<Pipe> pipes = {};

struct Player {
  Rectangle position = {50, 0, 100, 100};
  Vector2 origin = {position.width / 2, position.height / 2};
  float rotation = 0;
};
float player_jump_height = 500.0;
float gravity = 1000.0;
float velY = 0.0;
float pipe_speed = 350.0;
float spawnTimer = 0;
float spawnInterval = 3.0; // seconds between pipes
float gap = 250.0;
Player player;
enum class Game_state { menu, playing, game_over };
struct Game {
  Game_state game_state = Game_state::playing;
  int score = 0;
};

Game game;

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 450, "raylib [core] example - basic window");
#pragma region imgui
  rlImGuiSetup(true);
  // you can use whatever imgui theme you like!
  // ImGui::StyleColorsDark();
  // imguiThemes::yellow();
  // imguiThemes::gray();
  imguiThemes::green();
  // imguiThemes::red();
  // imguiThemes::embraceTheDarkness();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;           // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  io.FontGlobalScale = 2;
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    // style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.5f;
    // style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
  }
#pragma endregion
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    float delta = GetFrameTime();
    int w = GetScreenWidth();
    int h = GetScreenHeight();
#pragma region imgui
    rlImGuiBegin();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    ImGui::PopStyleColor(2);
#pragma endregion
    ImGui::Begin("player");
    ImGui::SliderFloat("gravity", &gravity, 1.0, 2000.0);
    ImGui::SliderFloat("jump height", &player_jump_height, 1.0, 2000.0);
    ImGui::End();
    DrawRectanglePro(player.position, player.origin, player.rotation, GREEN);

    for (Pipe &pipe : pipes) {
      if (game.game_state == Game_state::playing) {
        pipe.x -= pipe_speed * delta;
      }

      float pipeLeft = pipe.x;
      float pipeRight = pipe.x + 100.0f;
      float topPipeBot = pipe.pipe_height;
      float botPipeTop = pipe.pipe_height + gap;

      DrawRectangle(pipe.x, 0, 100, pipe.pipe_height, GREEN);
      DrawRectangle(pipe.x, botPipeTop, 100, h - botPipeTop, GREEN);

      if (game.game_state == Game_state::playing) {

        float playerLeft = player.position.x - player.origin.x;
        float playerRight = player.position.x + player.origin.x;
        float playerTop = player.position.y - player.origin.y;
        float playerBottom = player.position.y + player.origin.y;

        bool hit_top =
            (playerRight > pipeLeft) && (playerLeft < pipeRight) &&
            (playerTop < topPipeBot); // Player top is above pipe bottom

        bool hit_bottom =
            (playerRight > pipeLeft) && (playerLeft < pipeRight) &&
            (playerBottom > botPipeTop); // Player bottom is below pipe top

        bool hit_floor = (playerBottom >= h);
        bool hit_ciling = (playerBottom <= h);

        if (hit_top || hit_bottom || hit_floor || hit_ciling) {
          std::cout << "game over!\n";
          game.game_state = Game_state::game_over;
        }

        if (!pipe.passed && playerLeft > pipeRight) {
          game.score++;
          pipe.passed = true;
        }
      }
    }

    std::string score_text = "score: " + std::to_string((game.score));
    DrawText(score_text.c_str(), w / 3, h / 8, 80, BLACK);

    if (game.game_state == Game_state::game_over) {
      DrawText("game_over", w / 3, h / 2, 50, BLACK);
    }
    // remove pipes that scrolled off the left edge
    pipes.erase(std::remove_if(pipes.begin(), pipes.end(),
                               [](const Pipe &p) { return p.x + 100 < 0; }),
                pipes.end());
    spawnTimer += delta;
    if (spawnTimer >= spawnInterval) {
      spawnTimer = 0;
      float randomHeight = GetRandomValue(50, h - 250); // leave room for gap
      pipes.push_back({(float)w, randomHeight});        // spawn at right edge
    }
    if (game.game_state == Game_state::playing) {

      if (IsKeyPressed(KEY_SPACE)) {
        velY = -player_jump_height; // set velocity, negative = up
      }
    }
    if (game.game_state == Game_state::game_over ||
        game.game_state == Game_state::playing) {
      velY += gravity * delta; // gravity, every frame
      player.position.y += velY * delta;
      player.rotation = std::clamp(velY / 12.f, -30.0f, 70.0f);
    }
#pragma region imgui
    rlImGuiEnd();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }
#pragma endregion
    EndDrawing();
  }
#pragma region imgui
  rlImGuiShutdown();
#pragma endregion
  CloseWindow();
  return 0;
}
