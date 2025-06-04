#include "raylib.h"
#include <stdlib.h>
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Game states
typedef enum { MENU, PLAY, SETTINGS, HOW_TO_PLAY } GameScreen;

// Enemy waves
typedef enum { FIRST = 0, SECOND, THIRD } EnemyWave;

// Constants
#define NUM_SHOOTS 50
#define NUM_MAX_ENEMIES 50
#define FIRST_WAVE 10
#define MAX_MENU_ITEMS 4

// Game Structure
typedef struct Player { Rectangle rec; Vector2 speed; Color color; } Player;
typedef struct Enemy  { Rectangle rec; Vector2 speed; bool active; Color color; } Enemy;
typedef struct Shoot  { Rectangle rec; Vector2 speed; bool active; Color color; } Shoot;

// Global Variables
static const int screenWidth = 800;
static const int screenHeight = 600;
static GameScreen currentScreen = MENU;
static int menuItemSelected = 0;
static const char* menuItems[MAX_MENU_ITEMS] = { "PLAY", "HOW TO PLAY", "SETTINGS", "EXIT" };
static int scorePerKill = 100;
static Texture2D playerTexture;
static Texture2D enemyTexture;
static float playerScale = 0.175f;  
static float enemyScale  = 0.175f;  

static bool gameOver = false;
static int score = 0;
static int activeEnemies = FIRST_WAVE;
static EnemyWave wave = FIRST;

static Player player = { 0 };
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Shoot shoot[NUM_SHOOTS]   = { 0 };

// Audio
static Music bgMusic;
static Sound shootSound;
static Sound explosionSound;
static bool musicOn = true;
static float musicVolume = 0.5f;
static bool isFullscreen = false;

void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UnloadGame(void);

void DrawMainMenu() {
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("SPACE INVADERS", screenWidth/2 - MeasureText("SPACE INVADERS", 50)/2, 50, 50, GREEN);
        for (int i = 0; i < MAX_MENU_ITEMS; i++) {
            Color color = (i == menuItemSelected) ? GREEN : WHITE;
            DrawText(menuItems[i],
                     screenWidth/2 - MeasureText(menuItems[i], 40)/2,
                     200 + i * 70, 40, color);
        }
        DrawText("Use ARROW KEYS to navigate, ENTER to select",
                 screenWidth/2 - MeasureText("Use ARROW KEYS to navigate, ENTER to select", 20)/2,
                 screenHeight - 50, 20, WHITE);
    EndDrawing();
}

void DrawSettings() {
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("SETTINGS", screenWidth/2 - MeasureText("SETTINGS", 50)/2, 50, 50, GREEN);
        DrawText(TextFormat("Fullscreen: %s (Press F)", isFullscreen ? "ON" : "OFF"), 50, 150, 30, WHITE);
        DrawText(TextFormat("Music: %s (Press M)", musicOn ? "ON" : "OFF"),         50, 200, 30, WHITE);
        DrawText(TextFormat("Volume: %.0f%% (LEFT/RIGHT)", musicVolume * 100),       50, 250, 30, WHITE);
        DrawText("Press C to return to MENU",
                 screenWidth/2 - MeasureText("Press C to return to MENU", 20)/2,
                 screenHeight - 50, 20, WHITE);
    EndDrawing();
}

void DrawHowToPlay() {
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("HOW TO PLAY", screenWidth/2 - MeasureText("HOW TO PLAY", 50)/2, 50, 50, GREEN);
        DrawText("MOVEMENT:", 50, 120, 30, WHITE);
        DrawText("Use ARROW KEYS to move", 70, 160, 25, GREEN);
        DrawText("SHOOTING:", 50, 200, 30, WHITE);
        DrawText("Press SPACE to fire", 70, 240, 25, GREEN);
        DrawText("OBJECTIVE:", 50, 280, 30, WHITE);
        DrawText("Destroy all alien waves!", 70, 320, 25, GREEN);
        DrawText("Press C to return to MENU",
                 screenWidth/2 - MeasureText("Press C to return to MENU", 20)/2,
                 screenHeight - 50, 20, WHITE);
    EndDrawing();
}

void InitGame(void) {
    score = 0;
    gameOver = false;
    wave = FIRST;
    activeEnemies = FIRST_WAVE;
    scorePerKill = 100;

    // Load textures
    playerTexture = LoadTexture("resources/space_player.png");
    enemyTexture  = LoadTexture("resources/space_enemy.png");

    // Initialize player rectangle
    player.rec.width  = playerTexture.width  * playerScale;
    player.rec.height = playerTexture.height * playerScale;
    player.rec.x = 20;
    player.rec.y = 50;
    player.speed.x = 5;
    player.speed.y = 5;
    player.color = GREEN;

    // Initialize enemies with scaled size
    for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
        enemy[i].rec.width  = enemyTexture.width  * enemyScale;
        enemy[i].rec.height = enemyTexture.height * enemyScale;
        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
        enemy[i].speed.x = 5;
        enemy[i].speed.y = 5;
        enemy[i].active  = true;
        enemy[i].color   = RED;
    }

    // Initialize shoots
    for (int i = 0; i < NUM_SHOOTS; i++) {
        shoot[i].active = false;
        shoot[i].color  = WHITE;
    }
}

void UpdateGame(void) {
    if (!gameOver) {
        // Player movement
        if (IsKeyDown(KEY_RIGHT)) player.rec.x += player.speed.x;
        if (IsKeyDown(KEY_LEFT))  player.rec.x -= player.speed.x;
        if (IsKeyDown(KEY_UP))    player.rec.y -= player.speed.y;
        if (IsKeyDown(KEY_DOWN))  player.rec.y += player.speed.y;

        // Enemy-player collision
        for (int i = 0; i < activeEnemies; i++) {
            if (enemy[i].active && CheckCollisionRecs(player.rec, enemy[i].rec)) {
                gameOver = true;
                PlaySound(explosionSound);
            }
        }

        // Enemy movement & wrap
        for (int i = 0; i < activeEnemies; i++) {
            if (enemy[i].active) {
                enemy[i].rec.x -= enemy[i].speed.x;
                if (enemy[i].rec.x < 0) {
                    enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
                }
            }
        }

        // Keep player on-screen
        if (player.rec.x < 0) player.rec.x = 0;
        if (player.rec.x > screenWidth - player.rec.width)  player.rec.x = screenWidth - player.rec.width;
        if (player.rec.y < 0) player.rec.y = 0;
        if (player.rec.y > screenHeight - player.rec.height) player.rec.y = screenHeight - player.rec.height;

        // Shooting
        if (IsKeyPressed(KEY_SPACE)) {
            for (int i = 0; i < NUM_SHOOTS; i++) {
                if (!shoot[i].active) {
                    shoot[i].rec.width  = 10;
                    shoot[i].rec.height = 5;
                    shoot[i].active    = true;
                    shoot[i].rec.x     = player.rec.x + player.rec.width;
                    shoot[i].rec.y     = player.rec.y + player.rec.height/4;
                    PlaySound(shootSound);
                    break;
                }
            }
        }

        // Update shoots & collisions
        for (int i = 0; i < NUM_SHOOTS; i++) {
            if (shoot[i].active) {
                shoot[i].rec.x += 15;
                if (shoot[i].rec.x > screenWidth) shoot[i].active = false;
                for (int j = 0; j < activeEnemies; j++) {
                    if (enemy[j].active && CheckCollisionRecs(shoot[i].rec, enemy[j].rec)) {
                        shoot[i].active = false;
                        enemy[j].active = false;
                        PlaySound(explosionSound);
                        score += scorePerKill;
                        break;
                    }
                }
            }
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) InitGame();
    }

    // Wave progression
    bool allDead = true;
    for (int i = 0; i < activeEnemies; i++) { if (enemy[i].active) { allDead = false; break; } }
    if (allDead) {
        int nextCount = activeEnemies * 2;
        if (nextCount > NUM_MAX_ENEMIES) nextCount = NUM_MAX_ENEMIES;
        if (activeEnemies != nextCount) {
            activeEnemies = nextCount;
            scorePerKill  *= 2;
        }
        for (int i = 0; i < activeEnemies; i++) {
            enemy[i].rec.x     = GetRandomValue(screenWidth, screenWidth + 1000);
            enemy[i].rec.y     = GetRandomValue(0, screenHeight - enemy[i].rec.height);
            enemy[i].active    = true;
        }
    }
}

void DrawGame(void) {
    BeginDrawing();
        ClearBackground(BLACK);
        if (!gameOver) {
            // Draw player scaled
            DrawTexturePro(
                playerTexture,
                (Rectangle){ 0, 0, (float)playerTexture.width,  (float)playerTexture.height },
                (Rectangle){ player.rec.x, player.rec.y, player.rec.width, player.rec.height },
                (Vector2){ 0, 0 }, 0.0f, WHITE
            );
            // Draw enemies scaled
            for (int i = 0; i < activeEnemies; i++) {
                if (enemy[i].active) {
                    DrawTexturePro(
                        enemyTexture,
                        (Rectangle){ 0, 0, (float)enemyTexture.width,  (float)enemyTexture.height },
                        (Rectangle){ enemy[i].rec.x, enemy[i].rec.y, enemy[i].rec.width, enemy[i].rec.height },
                        (Vector2){ 0, 0 }, 0.0f, WHITE
                    );
                }
            }
            // Draw shoots
            for (int i = 0; i < NUM_SHOOTS; i++) {
                if (shoot[i].active) DrawRectangleRec(shoot[i].rec, shoot[i].color);
            }
            DrawText(TextFormat("SCORE: %04d", score), 20, 20, 30, GREEN);
        } else {
            DrawText("GAME OVER!", screenWidth/2 - 130, screenHeight/2 - 50, 40, RED);
            DrawText("PRESS ENTER TO RESTART", screenWidth/2 - 150, screenHeight/2 + 10, 20, WHITE);
        }
    EndDrawing();
}

void UnloadGame(void) {
    UnloadMusicStream(bgMusic);
    UnloadSound(shootSound);
    UnloadSound(explosionSound);
    UnloadTexture(playerTexture);
    UnloadTexture(enemyTexture);
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Space Invaders");
    InitAudioDevice();
    bgMusic         = LoadMusicStream("resources/space_music.wav");
    shootSound      = LoadSound("resources/space_shoot.wav");
    explosionSound  = LoadSound("resources/space_explosion.wav");
    SetMusicVolume(bgMusic, musicVolume);
    PlayMusicStream(bgMusic);
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);
        switch (currentScreen) {
            case MENU:
                if (IsKeyPressed(KEY_DOWN)) menuItemSelected = (menuItemSelected + 1) % MAX_MENU_ITEMS;
                if (IsKeyPressed(KEY_UP))   menuItemSelected = (menuItemSelected - 1 + MAX_MENU_ITEMS) % MAX_MENU_ITEMS;
                if (IsKeyPressed(KEY_ENTER)) {
                    switch (menuItemSelected) {
                        case 0: currentScreen = PLAY;     InitGame(); break;
                        case 1: currentScreen = HOW_TO_PLAY;           break;
                        case 2: currentScreen = SETTINGS;             break;
                        case 3: CloseWindow();                        break;
                    }
                }
                DrawMainMenu(); break;
            case SETTINGS:
                if (IsKeyPressed(KEY_F)) ToggleFullscreen();
                if (IsKeyPressed(KEY_M)) {
                    musicOn = !musicOn;
                    if (musicOn) PlayMusicStream(bgMusic); else StopMusicStream(bgMusic);
                }
                if (IsKeyDown(KEY_LEFT))  musicVolume = fmaxf(0.0f, musicVolume - 0.01f);
                if (IsKeyDown(KEY_RIGHT)) musicVolume = fminf(1.0f, musicVolume + 0.01f);
                SetMusicVolume(bgMusic, musicVolume);
                if (IsKeyPressed(KEY_C)) currentScreen = MENU;
                DrawSettings(); break;
            case HOW_TO_PLAY:
                if (IsKeyPressed(KEY_C)) currentScreen = MENU;
                DrawHowToPlay(); break;
            case PLAY:
                UpdateGame(); DrawGame(); break;
        }
    }
    UnloadGame(); CloseAudioDevice(); CloseWindow();
    return 0;
}
