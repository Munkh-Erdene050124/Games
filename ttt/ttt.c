#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <MMsystem.h>

#define COMPUTER 1
#define HUMAN 2
#define COMPUTERMOVE 'O'
#define HUMANMOVE 'X'

// Хамгийн сайн хөдөлгөөнийг хадгалах бүтэц
struct Move {
    int row, col;
};

// Global variables
char player = 'O', opponent = 'X';
int difficulty = 3; // Default to Hard

// hudulguun uldsen eshiig shalgana
bool isMovesLeft(char board[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == '_')
                return true;
    return false;
}

// talbariig uneleh unelgee
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

// AI-iin best move oloh function
struct Move findBestMove(char board[3][3]) {
    int bestVal = -1000;
    struct Move bestMove = {1, 1};
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

// Random hudulguun hiine
struct Move makeRandomMove(char board[3][3]) {
    struct Move move;
    do {
        move.row = rand() % 3;
        move.col = rand() % 3;
    } while (board[move.row][move.col] != '_');
    return move;
}

// Dundaj hudulguun hiine
struct Move makeMediumMove(char board[3][3]) {
    return (rand() % 2 == 0) ? makeRandomMove(board) : findBestMove(board);
}

//her hetsuug n songoh function
void ChooseDifficulty() {
    printf("Her hetsuug n songo: 1. Amarhan  2. dundaj  3. Hetsuu\n");
    while (scanf("%d", &difficulty) != 1 || difficulty < 1 || difficulty > 3) {
        printf("Buruu songolt baina! 1, 2, 3 ali negiig n songo: ");
        while (getchar() != '\n');
    }
}

// talbariig haruulah function
void showBoard(char board[3][3]) {
    system("cls"); //umnuh talbariig ustgana
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

// Ur dung shalgana hojson tsenssen ylsan eshyg
bool checkWinner(char board[3][3]) {
    int score = evaluate(board);
    if (score == 10) {
        printf("HOJIGDCHIHLOO SUGAA!\n");
        PlaySound(TEXT("Lose.wav"), NULL, SND_ASYNC);
        return true;
    } else if (score == -10) {
        printf("YALLAAA!\n");
        PlaySound(TEXT("Victory.wav"), NULL, SND_ASYNC);
        return true;
    } else if (!isMovesLeft(board)) {
        printf("Uuu tentslee!\n");
        PlaySound(TEXT("Lose.wav"), NULL, SND_ASYNC);
        return true;
    }
    return false;
}

// Togloh function
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
            printf("COMPUTER %c, %d tawilaa.\n", COMPUTERMOVE, x * 3 + y + 1);
            showBoard(board);
            if (checkWinner(board)) return;
            whoseTurn = HUMAN;
        } else {
            int move;
            printf("(1-9) too oruulj hudul: ");
            if (scanf("%d", &move) != 1 || move < 1 || move > 9) {
                printf("Buruu too baina 1ees 9iin hoorond too oruul.\n");
                while (getchar() != '\n');
                continue;
            }
            x = (move - 1) / 3;
            y = (move - 1) % 3;
            if (board[x][y] != '_') {
                printf("Buruu nuudel baina. Uur nud deer tavih gej uzne uu.\n");
                continue;
            }
            board[x][y] = HUMANMOVE;
            showBoard(board);
            if (checkWinner(board)) return;
            whoseTurn = COMPUTER;
        }
    }
}

//main menu function 
void mainmenu() {
    int choice;
    while (1) {
        system("cls");
        PlaySound(TEXT("Background.wav"), NULL, SND_ASYNC | SND_LOOP);
        printf("\n\033[1;32m==== TIC-TAC-TOE ====");
        printf("\n1. Togloh (Play)");
        printf("\n2. Zaavar (Instructions)");
        printf("\n3. Garah (Quit)");
        printf("\n\nSongoltoo hiine uu: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Buruu songolt baina! 1, 2, 3 ali negiig songono uu.\n");
            while (getchar() != '\n');
            continue;
        }
        
        if (choice == 1) {
            ChooseDifficulty();
            int firstMove;
            printf("Ehleed nuuhuu? (1 = Tiim, 2 = Ugui): ");
            while (scanf("%d", &firstMove) != 1 || (firstMove != 1 && firstMove != 2)) {
                printf("Buruu songolt baina 1 esvel 2iig songo: ");
                while (getchar() != '\n');
            }
            playTicTacToe(firstMove == 1 ? HUMAN : COMPUTER);
            {
                char c;
                printf("Nuur huudasruu butsahuu esvel garahuu? (y/n): ");
                while (scanf(" %c", &c) != 1 || (c != 'y' && c != 'n')) {
                    printf("Buruu songolt baina! y esvel n songo: ");
                    while (getchar() != '\n');
                }
                if (c == 'n'){
                    printf("\nTogloomnoos garlaa. Bayartai!\033[0m\n");
                    break;
                };
            }
        } 
        else if (choice == 2) {
            system("cls");
            printf("\n==== Zaavar ====");
            printf("\n1. Door haragdah 1-9 hurtel dugaar oruulj toglono:\n");
            printf("\n 1 | 2 | 3 ");
            printf("\n-----------");
            printf("\n 4 | 5 | 6 ");
            printf("\n-----------");
            printf("\n 7 | 8 | 9 \n");
            printf("\n2. Hudulguunuu hiihyn tuld hooson nud songono.\n");
            printf("\nUrd ni ajillaj baisan toglolt baival, shine toglolt exelne.\n");
            printf("\nEnter darj main menu ruu orno uu.\n");
            getchar();
            getchar();
        } 
        else if (choice == 3) {
            printf("\nTogloomnoos garlaa. Bayartai!\033[0m\n");
            exit(0);
        } 
        else {
            printf("Buruu songolt baina! 1, 2, 3 ali negiig songono uu.\n");
        }
    }
}


int main() {
    srand(time(0));
    mainmenu();
    return 0;
}