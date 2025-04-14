#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

//----------------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------------
#define DECK_SIZE 52
#define MAX_HAND 12
#define CARD_WIDTH 107
#define CARD_HEIGHT 150
#define MAX_MENU_ITEMS 4

// Betting defaults
#define INITIAL_BALANCE 10000
#define DEFAULT_BET 5000
#define MIN_BET 100

//----------------------------------------------------------------------------------
// Game states
//----------------------------------------------------------------------------------
typedef enum GameScreen { MENU, PLAY, SETTINGS, HOW_TO_PLAY } GameScreen;

//----------------------------------------------------------------------------------
// Structures
//----------------------------------------------------------------------------------
typedef struct Card {
    char suit;    // 'H', 'D', 'C', 'S' huzurnii ungu H = bund, D = durvuljin,C = tsetseg, S = gil
    char rank;    // '2'-'9', 'T', 'J', 'Q', 'K', 'A'
    bool revealed;
} Card;

typedef struct BlackjackGame {
    Card deck[DECK_SIZE];
    int deckIndex;
    Card playerHand[MAX_HAND];
    int playerCount;
    Card dealerHand[MAX_HAND];
    int dealerCount;
    int playerValue;
    int dealerValue;
    bool gameOver;     // round duusval true
    bool playerBust;
    bool dealerBust;
} BlackjackGame;

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
static const int screenWidth = 1000;
static const int screenHeight = 800;
static GameScreen currentScreen = MENU;
static int menuItemSelected = 0;
static const char* menuItems[MAX_MENU_ITEMS] = { "PLAY", "HOW TO PLAY", "SETTINGS", "EXIT" };

static BlackjackGame game;

// Resource textures and sounds
static Texture2D cardBackTexture;
static Music backgroundMusic;
static Sound cardSound;
static Sound winSound;
static Sound loseSound;

// Settings-iin huwisagch
static bool isFullscreen = false;
static bool musicOn = true;
static float soundVolume = 0.5f; // Range: 0.0f - 1.0f

// Bet-nii huwisagch (round hoorond hadgalagdaad ywna)
static int playerBalance = INITIAL_BALANCE;
static int currentBet = DEFAULT_BET;
static bool betPlaced = false;      // false = bet tawij baigaa; true = round ehelsen

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------
void InitGame(void);
void ResetRound(void);
void UpdateGame(void);
void DrawGame(void);
void DrawMenu(void);
void DrawHowToPlay(void);
void DrawSettings(void);
void UpdateSettings(void);
void DrawBlackjackGame(void);
void UpdateBlackjackGame(void);
void DrawBettingScreen(void);
void UpdateBettingScreen(void);
void ShuffleDeck(Card deck[]);
int CalculateHandValue(Card hand[], int count);
void DealCard(Card hand[], int *count, Card deck[], int *deckIndex, bool revealed);
void CheckWinCondition(void);
void CheckLoseCondition(void);

//------------------------------------------------------------------------------------
// Main Entry Point
//------------------------------------------------------------------------------------
int main(void)
{
    // delgets gargana Fullscreen bolgoj bas bolno
    InitWindow(screenWidth, screenHeight, "Blackjack Game");
    InitAudioDevice();

    // resource-uud
    cardBackTexture = LoadTexture("resources/blackjack_card_back.png");
    backgroundMusic = LoadMusicStream("resources/blackjack_background.wav");
    cardSound = LoadSound("resources/blackjack_card.wav");
    winSound = LoadSound("resources/blackjack_win.wav");
    loseSound = LoadSound("resources/blackjack_lose.wav");

    SetMusicVolume(backgroundMusic, soundVolume);
    PlayMusicStream(backgroundMusic);

    // togloomni turul
    betPlaced = false; //bet tawiagu baihad 
    playerBalance = INITIAL_BALANCE; //dansand 10000 ehlene
    currentBet = DEFAULT_BET; //default betnii hemjee 5000

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        // Duu
        UpdateMusicStream(backgroundMusic);

        // delgetsuud hoorond solih
        switch(currentScreen) {
            case MENU:
                // Menu nav
                if (IsKeyPressed(KEY_DOWN)) menuItemSelected = (menuItemSelected + 1) % MAX_MENU_ITEMS;
                if (IsKeyPressed(KEY_UP)) menuItemSelected = (menuItemSelected - 1 + MAX_MENU_ITEMS) % MAX_MENU_ITEMS;
                if (IsKeyPressed(KEY_ENTER))
                {
                    switch(menuItemSelected)
                    {
                        case 0: // PLAY
                            currentScreen = PLAY;
                            betPlaced = false; // betnii delgets toglohoos umnu haruulna
                            break;
                        case 1: // HOW TO PLAY
                            currentScreen = HOW_TO_PLAY;
                            break;
                        case 2: // SETTINGS
                            currentScreen = SETTINGS;
                            break;
                        case 3: // EXIT
                            CloseWindow();
                            break;
                    }
                }
                break;
            case HOW_TO_PLAY:
                if (IsKeyPressed(KEY_C)) currentScreen = MENU; //c darj main menu ruu orno
                break;
            case SETTINGS:
                UpdateSettings();
                break;
            case PLAY:
                if (!betPlaced)
                    UpdateBettingScreen();
                else
                    UpdateBlackjackGame();
                break;
            default:
                break;
        }

        BeginDrawing();
            ClearBackground(DARKGREEN);
            switch(currentScreen) {
                case MENU: DrawMenu(); break;
                case HOW_TO_PLAY: DrawHowToPlay(); break;
                case SETTINGS: DrawSettings(); break;
                case PLAY: 
                    // bet tawigdaagui bol betnii delgets haruulna
                    if (!betPlaced) DrawBettingScreen();
                    else DrawBlackjackGame();
                    break;
            }
        EndDrawing();
    }

    // Cleanup
    UnloadTexture(cardBackTexture);
    UnloadMusicStream(backgroundMusic);
    UnloadSound(cardSound);
    UnloadSound(winSound);
    UnloadSound(loseSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

//------------------------------------------------------------------------------------
// Initialize (or reinitialize) a round/game state
//------------------------------------------------------------------------------------
void InitGame(void)
{
    // bet tawigdsanii daraa shine round deer 
    char suits[] = {'H', 'D', 'C', 'S'};
    char ranks[] = {'2','3','4','5','6','7','8','9','T','J','Q','K','A'};
    
    for (int i = 0; i < DECK_SIZE; i++)
    {
        game.deck[i].suit = suits[i / 13];
        game.deck[i].rank = ranks[i % 13];
        game.deck[i].revealed = true;
    }
    ShuffleDeck(game.deck);
    game.deckIndex = 0;

    // round shineer ehleheer shineclegdene
    game.playerCount = 0;
    game.dealerCount = 0;
    game.gameOver = false;
    game.playerBust = false;
    game.dealerBust = false;

    // Initial deal
    DealCard(game.playerHand, &game.playerCount, game.deck, &game.deckIndex, true);
    DealCard(game.dealerHand, &game.dealerCount, game.deck, &game.deckIndex, false); // haragdahgui huzur
    DealCard(game.playerHand, &game.playerCount, game.deck, &game.deckIndex, true);
    DealCard(game.dealerHand, &game.dealerCount, game.deck, &game.deckIndex, true);
}

// neg round duusah uyd duudagdana
void ResetRound(void)
{
    // ur dungees hamaarch uldegdel mungu uurclugdunu.
    game.playerValue = CalculateHandValue(game.playerHand, game.playerCount);
    game.dealerValue = CalculateHandValue(game.dealerHand, game.dealerCount);
        
    if (game.playerBust) {
        // toglogch hojigdwol
        playerBalance -= currentBet;
        PlaySound(loseSound);
    } else if (game.dealerBust || game.playerValue > game.dealerValue) {
        playerBalance = playerBalance + currentBet * 2;
        PlaySound(winSound);
    } else if (game.dealerValue > game.playerValue) {
        playerBalance -= currentBet;
        PlaySound(loseSound);
    }
    
    betPlaced = false;
    
    //hojson esvel hojigdsoniig shalgana.
    CheckWinCondition();
    CheckLoseCondition();
}

//------------------------------------------------------------------------------------
// Update settings screen
//------------------------------------------------------------------------------------
void UpdateSettings(void)
{
    // F darj full screen bolgono
    if (IsKeyPressed(KEY_F)) {
        ToggleFullscreen();
        isFullscreen = !isFullscreen;
    }
    // M darj duug neej haana
    if (IsKeyPressed(KEY_M)) {
        if (musicOn) {
            StopMusicStream(backgroundMusic);
            musicOn = false;
        } else {
            PlayMusicStream(backgroundMusic);
            musicOn = true;
        }
    }
    // sumaar duug nemj hasna
    if (IsKeyDown(KEY_RIGHT)) {
        soundVolume += 0.01f;
        if (soundVolume > 1.0f) soundVolume = 1.0f;
        SetMusicVolume(backgroundMusic, soundVolume);
    }
    if (IsKeyDown(KEY_LEFT)) {
        soundVolume -= 0.01f;
        if (soundVolume < 0.0f) soundVolume = 0.0f;
        SetMusicVolume(backgroundMusic, soundVolume);
    }
    if (IsKeyPressed(KEY_C)) {
        currentScreen = MENU;
    }
}

//------------------------------------------------------------------------------------
// Draw settings screen
//------------------------------------------------------------------------------------
void DrawSettings(void)
{
    DrawText("SETTINGS", screenWidth/2 - MeasureText("SETTINGS", 50)/2, 50, 50, GOLD);
    
    // Fullscreen daragdsan eseh
    DrawText(TextFormat("Fullscreen: %s (Press F)", isFullscreen ? "ON" : "OFF"), 50, 150, 30, WHITE);
    
    // Duug neej haasah eseh
    DrawText(TextFormat("Music: %s (Press M)", musicOn ? "ON" : "OFF"), 50, 200, 30, WHITE);
    
    // Sound volume
    DrawText(TextFormat("Volume: %.2f (<-/-> to adjust)", soundVolume), 50, 250, 30, WHITE);
    
    DrawText("Press C to return to MENU", screenWidth/2 - MeasureText("Press C to return to MENU", 20)/2, screenHeight - 50, 20, WHITE);
}

//------------------------------------------------------------------------------------
// Draw main menu screen
//------------------------------------------------------------------------------------
void DrawMenu(void)
{
    DrawText("BLACKJACK", screenWidth/2 - MeasureText("BLACKJACK", 50)/2, 100, 50, GOLD);
    
    for (int i = 0; i < MAX_MENU_ITEMS; i++)
    {
        Color color = (i == menuItemSelected) ? GOLD : WHITE;
        DrawText(menuItems[i],
            screenWidth/2 - MeasureText(menuItems[i], 40)/2,
            200 + i * 70,
            40,
            color);
    }
    
    DrawText("Use ARROW KEYS to navigate, ENTER to select",
             screenWidth/2 - MeasureText("Use ARROW KEYS to navigate, ENTER to select", 20)/2,
             screenHeight - 50, 20, WHITE);
}

//------------------------------------------------------------------------------------
// Draw How To Play screen
//------------------------------------------------------------------------------------
void DrawHowToPlay(void)
{
    DrawText("HOW TO PLAY BLACKJACK", screenWidth/2 - MeasureText("HOW TO PLAY BLACKJACK", 40)/2, 40, 40, GOLD);
    
    DrawText("OBJECTIVE:", 40, 100, 30, WHITE);
    DrawText("- Get as close to 21 as possible without going over", 60, 140, 25, LIGHTGRAY);
    DrawText("- Beat the dealer's hand to win", 60, 170, 25, LIGHTGRAY);
    
    DrawText("CARD VALUES:", 40, 220, 30, WHITE);
    DrawText("- Number cards = face value", 60, 260, 25, LIGHTGRAY);
    DrawText("- Face cards (J, Q, K) = 10", 60, 290, 25, LIGHTGRAY);
    DrawText("- Ace = 1 or 11", 60, 320, 25, LIGHTGRAY);
    
    DrawText("GAMEPLAY:", 40, 370, 30, WHITE);
    DrawText("- [H] Hit: Take another card (or click on deck)", 60, 410, 25, LIGHTGRAY);
    DrawText("- [S] Stand: End your turn", 60, 440, 25, LIGHTGRAY);
    
    DrawText("Press C to return to MENU", 
             screenWidth/2 - MeasureText("Press C to return to MENU", 20)/2,
             screenHeight - 50, 20, WHITE);
}

//------------------------------------------------------------------------------------
// Draw betting screen (in PLAY state before the round starts)
//------------------------------------------------------------------------------------
void DrawBettingScreen(void)
{
    DrawText(TextFormat("Balance: %d$", playerBalance), 50, 50, 30, WHITE);
    
    DrawText(TextFormat("Place Your Bet: %d$ (<-/-> to adjust, ENTER to confirm)", currentBet),
             screenWidth/2 - MeasureText(TextFormat("Place Your Bet: %d$ (←/→ to adjust, ENTER to confirm)", currentBet), 30)/2,
             screenHeight/2, 30, YELLOW);
    
    DrawText("Press A to go ALL IN", 
         screenWidth/2 - MeasureText("Press A to go ALL IN", 30)/2, 
         screenHeight/2 + 80, 30, YELLOW);
    
    DrawText("Press C to return to MENU", screenWidth/2 - MeasureText("Press C to return to MENU", 20)/2, screenHeight - 50, 20, WHITE);
}

//------------------------------------------------------------------------------------
// Update betting screen logic
//------------------------------------------------------------------------------------
void UpdateBettingScreen(void)
{
    // betee sumaar ihesgej bagasgana
    if (IsKeyPressed(KEY_RIGHT)) {
        currentBet += 100;
        if (currentBet > playerBalance) currentBet = playerBalance;
    }
    if (IsKeyPressed(KEY_LEFT)) {
        currentBet -= 100;
        if (currentBet < MIN_BET) currentBet = MIN_BET;
    }
    //all In
    if (IsKeyPressed(KEY_A)) {
        currentBet = playerBalance;
    }
    
    // Enter darwal bet tawigdana
    if (IsKeyPressed(KEY_ENTER)) {
        if (currentBet <= playerBalance) {
            betPlaced = true;
            InitGame();
        }
    }

    // C darwal menu ruu butsna
    if (IsKeyPressed(KEY_C)) {
        currentScreen = MENU;
    }
}

//------------------------------------------------------------------------------------
// Draw Blackjack game screen (when round is active)
//------------------------------------------------------------------------------------
void DrawBlackjackGame(void)
{
    // dealeriin huzur
    DrawText("DEALER'S HAND:", 20, 20, 20, WHITE);
    for (int i = 0; i < game.dealerCount; i++) {
        if (game.dealerHand[i].revealed) {
            DrawRectangle(20 + i*(CARD_WIDTH+10), 50, CARD_WIDTH, CARD_HEIGHT, WHITE);
            DrawText(TextFormat("%c", game.dealerHand[i].rank), 40 + i*(CARD_WIDTH+10), 70, 40, BLACK);
        } else {
            DrawTextureEx(cardBackTexture, (Vector2){20 + i*(CARD_WIDTH+10), 50}, 0, 1.0f, WHITE);
        }
    }
   
    // toglogchiin huzur
    DrawText("YOUR HAND:", 20, 300, 20, WHITE);
    for (int i = 0; i < game.playerCount; i++) {
        DrawRectangle(20 + i*(CARD_WIDTH+10), 330, CARD_WIDTH, CARD_HEIGHT, WHITE);
        DrawText(TextFormat("%c", game.playerHand[i].rank), 40 + i*(CARD_WIDTH+10), 350, 40, BLACK);
    }
    
    // uldsen huzriig hajuu tald n haruulna
    DrawText(TextFormat("Deck: %d", DECK_SIZE - game.deckIndex), screenWidth - 150, 50, 20, WHITE);
    DrawTextureEx(cardBackTexture, (Vector2){screenWidth - 150, 80}, 0, 1.0f, WHITE);
    
    // uy duusval ur dung haruulna
    if (game.gameOver)
    {
        game.playerValue = CalculateHandValue(game.playerHand, game.playerCount);
        game.dealerValue = CalculateHandValue(game.dealerHand, game.dealerCount);
        DrawText(TextFormat("Dealer: %d | Player: %d", game.dealerValue, game.playerValue), 
                 20, screenHeight - 150, 30, WHITE);
        if (game.playerBust)
            DrawText("BUST! YOU LOSE!", 20, screenHeight - 110, 30, RED);
        else if (game.dealerBust)
            DrawText("DEALER BUSTS! YOU WIN!", 20, screenHeight - 110, 30, GREEN);
        else if (game.playerValue > game.dealerValue)
            DrawText("YOU WIN!", 20, screenHeight - 110, 30, GREEN);
        else if (game.dealerValue > game.playerValue)
            DrawText("YOU LOSE!", 20, screenHeight - 110, 30, RED);
        else
            DrawText("PUSH!", 20, screenHeight - 110, 30, YELLOW);
        
        // shineclegdsen dans haruulna
        DrawText(TextFormat("Balance: %d$", playerBalance), 20, screenHeight - 70, 30, WHITE);
        DrawText("Press SPACE to start next round", screenWidth/2 - 150, screenHeight - 30, 25, WHITE);
    }
    else
    {
        DrawText(TextFormat("Current Value: %d", CalculateHandValue(game.playerHand, game.playerCount)),
                 20, screenHeight - 60, 30, WHITE);
        DrawText("[H] Hit  [S] Stand  (or click on the deck to Hit)", 
                 20, screenHeight - 30, 25, WHITE);
    }
}

//------------------------------------------------------------------------------------
// Update Blackjack game logic (during a round)
//------------------------------------------------------------------------------------
void UpdateBlackjackGame(void)
{
    // uy duusval player ahij ehlehiig huleene
    if (game.gameOver) {
        if (IsKeyPressed(KEY_SPACE)) {
            ResetRound();
        }
        if (IsKeyPressed(KEY_C)) {
            currentScreen = MENU;
        }
        return;
    }

    // huzrun deer darj bas nemj awj bolno
    Rectangle deckRect = { screenWidth - 150, 80, cardBackTexture.width, cardBackTexture.height };
    Vector2 mousePoint = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePoint, deckRect)) {
        DealCard(game.playerHand, &game.playerCount, game.deck, &game.deckIndex, true);
        PlaySound(cardSound);
    }

    // Keyboard input
    if (IsKeyPressed(KEY_H)) {
        DealCard(game.playerHand, &game.playerCount, game.deck, &game.deckIndex, true);
        PlaySound(cardSound);
    }

    if (IsKeyPressed(KEY_S)) {
        // dealeriin nuuts huzriig harna
        game.dealerHand[0].revealed = true;

        // dealer 17 hurtel huzur nemj awna
        while (CalculateHandValue(game.dealerHand, game.dealerCount) < 17) {
            DealCard(game.dealerHand, &game.dealerCount, game.deck, &game.deckIndex, true);
            PlaySound(cardSound);
        }

        // Evaluate hands
        game.playerValue = CalculateHandValue(game.playerHand, game.playerCount);
        game.dealerValue = CalculateHandValue(game.dealerHand, game.dealerCount);
        game.playerBust = (game.playerValue > 21);
        game.dealerBust = (game.dealerValue > 21);
        game.gameOver = true;

        if (game.playerBust || (game.dealerValue > game.playerValue && !game.dealerBust)) {
            PlaySound(loseSound);
        } else if (game.dealerBust || game.playerValue > game.dealerValue) {
            PlaySound(winSound);
        }
    }

    // huzur nemj awsan uyd hojigdson eshiig shalgana
    game.playerValue = CalculateHandValue(game.playerHand, game.playerCount);
    if (game.playerValue > 21) {
        game.playerBust = true;
        game.gameOver = true;
        PlaySound(loseSound);
    }
}

//------------------------------------------------------------------------------------
// Calculate hand value (including proper Ace handling)
//------------------------------------------------------------------------------------
int CalculateHandValue(Card hand[], int count)
{
    int value = 0;
    int aces = 0;
    
    for (int i = 0; i < count; i++) {
        if (!hand[i].revealed) continue;
        
        if (hand[i].rank == 'A')
            aces++;
        else if (hand[i].rank == 'T' || hand[i].rank == 'J' ||
                 hand[i].rank == 'Q' || hand[i].rank == 'K')
            value += 10;
        else
            value += hand[i].rank - '0';
    }
    
    // 21ees baga baiwal 1eer tootsono ugui bol 1eer tootsono
    value += aces;
    if (aces > 0 && value + 10 <= 21)
        value += 10;
    
    return value;
}

//------------------------------------------------------------------------------------
// Deal a card to a hand
//------------------------------------------------------------------------------------
void DealCard(Card hand[], int *count, Card deck[], int *deckIndex, bool revealed)
{
    if(*deckIndex < DECK_SIZE) {
        hand[*count] = deck[*deckIndex];
        hand[*count].revealed = revealed;
        (*count)++;
        (*deckIndex)++;
    }
}

//------------------------------------------------------------------------------------
// Shuffle the deck using Fisher-Yates algorithm
//------------------------------------------------------------------------------------
void ShuffleDeck(Card deck[])
{
    // time ashiglan random too zohiono
    srand(time(NULL));
    
    // Fisher-Yates shuffle algorithm
    for (int i = DECK_SIZE - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        // Swap cards
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

//Hojih
void CheckWinCondition(void)
{
    if (playerBalance >= 100000)
    {
        currentScreen = MENU;
        playerBalance = INITIAL_BALANCE;
        currentBet = DEFAULT_BET;
        betPlaced = false;
        DrawText("YOU WON THE GAME! CONGRATULATIONS!", screenWidth / 2 - 300, screenHeight / 2, 30, GREEN);
        PlaySound(winSound);
    }
}

//Hojigdoh
void CheckLoseCondition(void)
{
    if (playerBalance <= 0)
    {
        currentScreen = MENU;
        playerBalance = INITIAL_BALANCE;
        currentBet = DEFAULT_BET;
        betPlaced = false;
    }
}