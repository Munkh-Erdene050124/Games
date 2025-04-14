/*******************************************************************************************
*
*   raylib Snake Game - Final Version
*   Features:
*   - Adjustable snake speed that increases gradually
*   - Grass background in gameplay
*   - Background music with eat/die sound effects
*   - Main menu and how-to-play screen
*   - Restart/Quit options on game over
*
********************************************************************************************/
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Defines and Constants
//----------------------------------------------------------------------------------
#define SNAKE_LENGTH   256
#define SQUARE_SIZE     31
#define MAX_MENU_ITEMS  3

// Game states
typedef enum GameScreen { MENU, PLAY, HOW_TO_PLAY } GameScreen;
typedef enum GameOverChoice { RESTART, QUIT } GameOverChoice;

//----------------------------------------------------------------------------------
// Structures Definition
//----------------------------------------------------------------------------------
typedef struct Snake {
    Vector2 position;
    Vector2 size;
    Vector2 speed;
    Color color;
} Snake;

typedef struct Food {
    Vector2 position;
    Vector2 size;
    bool active;
    Color color;
} Food;

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

// Game state
static GameScreen currentScreen = MENU;
static GameOverChoice gameOverChoice = RESTART;
static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

// Game objects
static Food fruit = { 0 };
static Snake snake[SNAKE_LENGTH] = { 0 };
static Vector2 snakePosition[SNAKE_LENGTH] = { 0 };
static bool allowMove = false;
static Vector2 offset = { 0 };
static int counterTail = 0;

// Menu
static int menuItemSelected = 0;
static const char* menuItems[MAX_MENU_ITEMS] = { "PLAY", "HOW TO PLAY", "EXIT" };

// Audio
static Music backgroundMusic;
static Sound eatSound;
static Sound dieSound;

// Graphics
static Texture2D grassTexture;

// Speed control
static int snakeSpeedDelay = 15;    // Higher = slower movement
static const int MIN_SPEED = 8;     // Minimum speed (higher = slower max speed)
static const int SPEED_CHANGE = 1;  // How much to change speed by

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void DrawMenu(void);
static void DrawHowToPlay(void);
static void UpdateDrawFrame(void);
static void UnloadGame(void);

//------------------------------------------------------------------------------------
// Program Entry Point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialize window
    InitWindow(screenWidth, screenHeight, "Snake Game");
    InitAudioDevice();

    // Load resources
    grassTexture = LoadTexture("resources/snake_grass.jpg");
    backgroundMusic = LoadMusicStream("resources/snake_background.wav");
    eatSound = LoadSound("resources/snake_eat.wav");
    dieSound = LoadSound("resources/snake_die.wav");

    // Start music
    SetMusicVolume(backgroundMusic, 0.5f);
    PlayMusicStream(backgroundMusic);

    InitGame();

    // Main game loop
    SetTargetFPS(60);
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    // Cleanup
    UnloadGame();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

// Initialize game state
void InitGame(void)
{
    framesCounter = 0;
    gameOver = false;
    pause = false;
    counterTail = 1;
    allowMove = false;
    snakeSpeedDelay = 15;  // Reset to initial speed

    offset.x = screenWidth % SQUARE_SIZE;
    offset.y = screenHeight % SQUARE_SIZE;

    // Initialize snake
    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snake[i].position = (Vector2){ offset.x/2, offset.y/2 };
        snake[i].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
        snake[i].speed = (Vector2){ SQUARE_SIZE, 0 };
        snake[i].color = (i == 0) ? DARKGREEN : GREEN;
    }

    // Initialize fruit
    fruit.size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
    fruit.color = RED;
    fruit.active = false;
}

// Draw main menu
void DrawMenu(void)
{
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Title
        DrawText("SNAKE GAME", screenWidth/2 - MeasureText("SNAKE GAME", 50)/2, 80, 50, DARKGREEN);
        
        // Menu items
        for (int i = 0; i < MAX_MENU_ITEMS; i++)
        {
            Color color = (i == menuItemSelected) ? DARKGREEN : LIGHTGRAY;
            DrawText(menuItems[i],
                    screenWidth/2 - MeasureText(menuItems[i], 40)/2,
                    200 + i * 70,
                    40,
                    color);
        }
        
        // Controls hint
        DrawText("Use ARROW KEYS to navigate, ENTER to select", 
                screenWidth/2 - MeasureText("Use ARROW KEYS to navigate, ENTER to select", 20)/2,
                screenHeight - 50, 20, GRAY);
    EndDrawing();
}

// Draw instructions screen
void DrawHowToPlay(void)
{
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Title
        DrawText("HOW TO PLAY", screenWidth/2 - MeasureText("HOW TO PLAY", 40)/2, 40, 40, DARKGREEN);
        
        // Controls
        DrawText("CONTROLS:", 40, 100, 30, DARKGRAY);
        DrawText("- Arrow keys to move", 60, 140, 25, GRAY);
        DrawText("- P to pause", 60, 170, 25, GRAY);
        DrawText("- C to return to menu", 60, 200, 25, GRAY);
        
        // Gameplay
        DrawText("GAMEPLAY:", 40, 250, 30, DARKGRAY);
        DrawText("- Eat red fruits to grow", 60, 290, 25, GRAY);
        DrawText("- Avoid walls and yourself", 60, 320, 25, GRAY);
        DrawText("- Longer snake = higher score and higher risk of death", 60, 350, 25, GRAY);
        
        // Return hint
        DrawText("Press C to return to menu", 
                screenWidth/2 - MeasureText("Press C to return to menu", 20)/2,
                screenHeight - 50, 20, GRAY);
    EndDrawing();
}

// Draw gameplay screen
void DrawGame(void)
{
    BeginDrawing();
        // Draw grass background
        DrawTextureEx(grassTexture, (Vector2){0,0}, 0, 
                     (float)screenWidth/grassTexture.width, WHITE);
        
        // Draw grid (semi-transparent)
        for (int i = 0; i < screenWidth/SQUARE_SIZE + 1; i++)
        {
            DrawLineV((Vector2){SQUARE_SIZE*i + offset.x/2, offset.y/2}, 
                     (Vector2){SQUARE_SIZE*i + offset.x/2, screenHeight - offset.y/2}, 
                     (Color){0, 100, 0, 50});
        }
        for (int i = 0; i < screenHeight/SQUARE_SIZE + 1; i++)
        {
            DrawLineV((Vector2){offset.x/2, SQUARE_SIZE*i + offset.y/2}, 
                     (Vector2){screenWidth - offset.x/2, SQUARE_SIZE*i + offset.y/2}, 
                     (Color){0, 100, 0, 50});
        }
        
        // Draw snake
        for (int i = 0; i < counterTail; i++)
            DrawRectangleV(snake[i].position, snake[i].size, snake[i].color);
        
        // Draw fruit
        if (fruit.active)
            DrawRectangleV(fruit.position, fruit.size, fruit.color);
        
        // Draw score
        DrawText(TextFormat("SCORE: %04d", counterTail - 1), 20, 20, 20, WHITE);
        
        // Pause screen
        if (pause)  
        {
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
            DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, 
                    screenHeight/2 - 40, 40, WHITE);
        }
        
        // Game over screen
        if (gameOver)
        {
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200});
            
            //score text
            DrawText(TextFormat("SCORE: %04d", counterTail - 1), 330, 100, 20, WHITE);
            
            // Game over text
            DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 40)/2, 
                    screenHeight/2 - 80, 40, RED);
            
            // Options
            DrawText("RESTART", screenWidth/2 - MeasureText("RESTART", 30)/2, 
                    screenHeight/2, 30, (gameOverChoice == RESTART) ? YELLOW : WHITE);
            DrawText("QUIT", screenWidth/2 - MeasureText("QUIT", 30)/2, 
                    screenHeight/2 + 40, 30, (gameOverChoice == QUIT) ? YELLOW : WHITE);
            
            // Controls
            DrawText("Use ARROW KEYS to choose, ENTER to confirm", 
                    screenWidth/2 - MeasureText("Use ARROW KEYS to choose, ENTER to confirm", 20)/2,
                    screenHeight - 50, 20, LIGHTGRAY);
        }
    EndDrawing();
}

// Update game logic
void UpdateGame(void)
{
    // Update music
    UpdateMusicStream(backgroundMusic);

    // Menu navigation
    if (currentScreen == MENU)
    {
        if (IsKeyPressed(KEY_DOWN)) menuItemSelected = (menuItemSelected + 1) % MAX_MENU_ITEMS;
        if (IsKeyPressed(KEY_UP)) menuItemSelected = (menuItemSelected - 1 + MAX_MENU_ITEMS) % MAX_MENU_ITEMS;
        
        if (IsKeyPressed(KEY_ENTER))
        {
            switch(menuItemSelected)
            {
                case 0: currentScreen = PLAY; InitGame(); break;
                case 1: currentScreen = HOW_TO_PLAY; break;
                case 2: CloseWindow(); break;
            }
        }
    }
    // Instructions screen
    else if (currentScreen == HOW_TO_PLAY)
    {
        if (IsKeyPressed(KEY_C)) currentScreen = MENU;
    }
    // Gameplay
    else if (currentScreen == PLAY)
    {
        if (!gameOver && !pause)
        {
            // Movement controls
            if (IsKeyPressed(KEY_RIGHT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ SQUARE_SIZE , 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ -SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_UP) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ 0, -SQUARE_SIZE };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ 0, SQUARE_SIZE };
                allowMove = false;
            }

            // Snake movement
            for (int i = 0; i < counterTail; i++) snakePosition[i] = snake[i].position;
            
            if ((framesCounter % snakeSpeedDelay) == 0)
            {
                for (int i = 0; i < counterTail; i++)
                {
                    if (i == 0)
                    {
                        snake[0].position.x += snake[0].speed.x;
                        snake[0].position.y += snake[0].speed.y;
                        allowMove = true;
                    }
                    else snake[i].position = snakePosition[i-1];
                }
            }

            // Wall collision
            if ((snake[0].position.x < offset.x/2) || 
               (snake[0].position.x + SQUARE_SIZE > screenWidth - offset.x/2) ||
               (snake[0].position.y < offset.y/2) || 
               (snake[0].position.y + SQUARE_SIZE > screenHeight - offset.y/2))
            {
                gameOver = true;
                PlaySound(dieSound);
            }

            // Self collision
            for (int i = 1; i < counterTail; i++)
            {
                if ((snake[0].position.x == snake[i].position.x) && 
                    (snake[0].position.y == snake[i].position.y))
                {
                    gameOver = true;
                    PlaySound(dieSound);
                }
            }

            // Fruit spawning
            if (!fruit.active)
            {
                fruit.active = true;
                fruit.position = (Vector2){ 
                    GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, 
                    GetRandomValue(0, (screenHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 
                };

                // Ensure fruit doesn't spawn on snake
                for (int i = 0; i < counterTail; i++)
                {
                    while ((fruit.position.x == snake[i].position.x) && 
                           (fruit.position.y == snake[i].position.y))
                    {
                        fruit.position = (Vector2){ 
                            GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, 
                            GetRandomValue(0, (screenHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 
                        };
                        i = 0;
                    }
                }
            }

            // Fruit collision
            if ((snake[0].position.x < (fruit.position.x + fruit.size.x)) &&
               (snake[0].position.x + snake[0].size.x > fruit.position.x) &&
               (snake[0].position.y < (fruit.position.y + fruit.size.y)) &&
               (snake[0].position.y + snake[0].size.y > fruit.position.y))
            {
                PlaySound(eatSound);
                snake[counterTail].position = snakePosition[counterTail - 1];
                if (counterTail < SNAKE_LENGTH) counterTail++;
                fruit.active = false;
                
                // Gradual speed increase every 3 fruits
                if (counterTail % 3 == 0 && snakeSpeedDelay > MIN_SPEED) {
                    snakeSpeedDelay -= SPEED_CHANGE;
                }
            }

            framesCounter++;
        }

        // Pause toggle
        if (IsKeyPressed(KEY_P)) pause = !pause;

        // Game over handling
        if (gameOver)
        {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))
                gameOverChoice = !gameOverChoice;
            
            if (IsKeyPressed(KEY_ENTER))
            {
                if (gameOverChoice == RESTART) 
                {
                    InitGame();
                    gameOver = false;
                }
                else 
                {
                    currentScreen = MENU;
                    gameOver = false;
                }
            }
        }
        // Return to menu
        else if (IsKeyPressed(KEY_ESCAPE)) 
        {
            currentScreen = MENU;
        }
    }
}

// Main update/draw frame
void UpdateDrawFrame(void)
{
    UpdateGame();
    
    if (currentScreen == MENU)
        DrawMenu();
    else if (currentScreen == HOW_TO_PLAY)
        DrawHowToPlay();
    else if (currentScreen == PLAY)
        DrawGame();
}

// Cleanup resources
void UnloadGame(void)
{
    UnloadTexture(grassTexture);
    UnloadMusicStream(backgroundMusic);
    UnloadSound(eatSound);
    UnloadSound(dieSound);
}