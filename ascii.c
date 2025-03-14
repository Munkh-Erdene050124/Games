#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h> // For _kbhit() and _getch() on Windows
#include <locale.h> // For Unicode character support
#include <wchar.h> // For Unicode character support

#define WIDTH 20
#define HEIGHT 10

// Characters for different map elements
unsigned char light_shade = 176;  // ░ wall texture
unsigned char medium_shade = 177; // ▒ wall texture
unsigned char dark_shade = 178;   // ▓ wall texture
unsigned char full_block = 219;   // █ wall texture
unsigned char space = 32;         // place where player will move
wchar_t player = L'\u25C9';       // Player character

// ANSI color codes
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

void generateDungeon(unsigned char map[HEIGHT][WIDTH]) {
    // Fill map with walls
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map[y][x] = (rand() % 4 == 0) ? full_block : ((rand() % 3 == 0) ? dark_shade : ((rand() % 2 == 0) ? medium_shade : light_shade));
        }
    }

    // Create paths where the player can move
    for (int y = 1; y < HEIGHT - 1; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            if (rand() % 3 != 0) { // 2/3 chance to be an open space
                map[y][x] = space;
            }
        }
    }
}

void printDungeon(unsigned char map[HEIGHT][WIDTH], int playerX, int playerY) {
    system("cls"); // Clear the console
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == playerX && y == playerY) {
                printf(CYAN "%c" RESET, player);
            } else if (map[y][x] == full_block) {
                printf(RED "%c" RESET, map[y][x]);
            } else if (map[y][x] == dark_shade) {
                printf(YELLOW "%c" RESET, map[y][x]);
            } else if (map[y][x] == medium_shade) {
                printf(GREEN "%c" RESET, map[y][x]);
            } else if (map[y][x] == light_shade) {
                printf(BLUE "%c" RESET, map[y][x]);
            } else {
                printf("%c", map[y][x]); // Default for space
            }
        }
        printf("\n");
    }
}

int main() {
    setlocale(LC_ALL, ""); // Enable wide character support
    unsigned char dungeon[HEIGHT][WIDTH];
    srand(time(NULL)); // Seed random number generator
    generateDungeon(dungeon);

    // Set player start position
    int playerX = 1, playerY = 1;
    dungeon[playerY][playerX] = space; // Ensure player starts on an empty tile

    char input;
    printDungeon(dungeon, playerX, playerY); // Initial draw
    while (1) {
        if (_kbhit()) {
            input = _getch();
            int newX = playerX, newY = playerY;
            if (input == 'w' && playerY > 0) newY--;
            if (input == 's' && playerY < HEIGHT - 1) newY++;
            if (input == 'a' && playerX > 0) newX--;
            if (input == 'd' && playerX < WIDTH - 1) newX++;
            
            // Check if new position is walkable
            if (dungeon[newY][newX] == space) {
                playerX = newX;
                playerY = newY;
                printDungeon(dungeon, playerX, playerY); // Refresh the screen after movement
            }
        }
    }
    return 0;
}