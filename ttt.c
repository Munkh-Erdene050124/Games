#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COMPUTER 1
#define HUMAN 2
#define SIDE 3
#define COMPUTERMOVE 'O'
#define HUMANMOVE 'X'

// Structure to store the best move
struct Move {
    int row, col;
};

// Global variables
char player = 'O', opponent = 'X';
int difficulty = 3; // Default to Hard

// Function to check if moves are left on the board
bool isMovesLeft(char board[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == '_')
                return true;
    return false;
}

// Function to evaluate the board
int evaluate(char b[3][3]) {
    for (int row = 0; row < 3; row++) {
        if (b[row][0] == b[row][1] && b[row][1] == b[row][2]) {
            if (b[row][0] == player) return +10;
            else if (b[row][0] == opponent) return -10;
        }
    }
    for (int col = 0; col < 3; col++) {
        if (b[0][col] == b[1][col] && b[1][col] == b[2][col]) {
            if (b[0][col] == player) return +10;
            else if (b[0][col] == opponent) return -10;
        }
    }
    if (b[0][0] == b[1][1] && b[1][1] == b[2][2]) {
        if (b[0][0] == player) return +10;
        else if (b[0][0] == opponent) return -10;
    }
    if (b[0][2] == b[1][1] && b[1][1] == b[2][0]) {
        if (b[0][2] == player) return +10;
        else if (b[0][2] == opponent) return -10;
    }
    return 0;
}

// Optimized Minimax with Alpha-Beta Pruning
int minimax(char board[3][3], int depth, bool isMax, int alpha, int beta) {
    int score = evaluate(board);
    if (score == 10 || score == -10) return score;
    if (!isMovesLeft(board)) return 0;

    if (isMax) {
        int best = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = player;
                    best = (best > minimax(board, depth + 1, false, alpha, beta)) ? best : minimax(board, depth + 1, false, alpha, beta);
                    board[i][j] = '_';
                    alpha = (alpha > best) ? alpha : best;
                    if (beta <= alpha) break;
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = opponent;
                    best = (best < minimax(board, depth + 1, true, alpha, beta)) ? best : minimax(board, depth + 1, true, alpha, beta);
                    board[i][j] = '_';
                    beta = (beta < best) ? beta : best;
                    if (beta <= alpha) break;
                }
            }
        }
        return best;
    }
}

// Function to find the best move for AI
struct Move findBestMove(char board[3][3]) {
    int bestVal = -1000;
    struct Move bestMove = {-1, -1};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == '_') {
                board[i][j] = player;
                int moveVal = minimax(board, 0, false, -1000, 1000);
                board[i][j] = '_';
                if (moveVal > bestVal) {
                    bestMove.row = i;
                    bestMove.col = j;
                    bestVal = moveVal;
                }
            }
        }
    }
    return bestMove;
}

// Function to make a random move
struct Move makeRandomMove(char board[3][3]) {
    struct Move move;
    do {
        move.row = rand() % 3;
        move.col = rand() % 3;
    } while (board[move.row][move.col] != '_');
    return move;
}

// Function to make a medium difficulty move
struct Move makeMediumMove(char board[3][3]) {
    return (rand() % 2 == 0) ? makeRandomMove(board) : findBestMove(board);
}

void ChooseDifficulty() {
    printf("Choose difficulty: 1. Easy  2. Medium  3. Hard\n");
    while (scanf("%d", &difficulty) != 1 || difficulty < 1 || difficulty > 3) {
        printf("Invalid choice! Enter 1, 2, or 3: ");
        while (getchar() != '\n');
    }
}

// Function to display the board
void showBoard(char board[3][3]) {
    printf("\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
        }
        if (i < 2) printf("\n-----------\n");
    }
    printf("\n\n");
}

// Function to check for a winner or draw
bool checkWinner(char board[3][3]) {
    int score = evaluate(board);
    if (score == 10) {
        printf("COMPUTER wins!\n");
        return true;
    } else if (score == -10) {
        printf("You win!\n");
        return true;
    } else if (!isMovesLeft(board)) {
        printf("It's a draw!\n");
        return true;
    }
    return false;
}

// Function to play the game
void playTicTacToe(int whoseTurn) {
    char board[3][3] = {{'_', '_', '_'}, {'_', '_', '_'}, {'_', '_', '_'}};
    showBoard(board);
    while (true) {
        int x, y;
        if (whoseTurn == COMPUTER) {
            struct Move move = (difficulty == 1) ? makeRandomMove(board) : (difficulty == 2) ? makeMediumMove(board) : findBestMove(board);
            x = move.row;
            y = move.col;
            board[x][y] = COMPUTERMOVE;
            printf("COMPUTER placed %c at %d\n", COMPUTERMOVE, x * 3 + y + 1);
            showBoard(board);
            if (checkWinner(board)) return;
            whoseTurn = HUMAN;
        } else {
            int move;
            printf("Enter your move (1-9): ");
            if (scanf("%d", &move) != 1 || move < 1 || move > 9) {
                printf("Invalid input! Enter a number between 1 and 9.\n");
                while (getchar() != '\n');
                continue;
            }
            x = (move - 1) / 3;
            y = (move - 1) % 3;
            if (board[x][y] != '_') {
                printf("Invalid move! Try again.\n");
                continue;
            }
            board[x][y] = HUMANMOVE;
            showBoard(board);
            if (checkWinner(board)) return;
            whoseTurn = COMPUTER;
        }
    }
}

int main() {
    srand(time(0));
    ChooseDifficulty();
    int choice;
    printf("Go first? (1 = YES, 2 = NO): ");
    while (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        printf("Invalid choice! Enter 1 or 2: ");
        while (getchar() != '\n');
    }
    playTicTacToe(choice == 1 ? HUMAN : COMPUTER);
    return 0;
}