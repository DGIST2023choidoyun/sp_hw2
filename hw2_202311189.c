#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#define TRUE                    1
#define FALSE                   0
#define MAP_ROW                 8
#define MAP_COL                 8
#define MAX_STR                 256
#define SRC                     0
#define DST                     1
#define PASS                    1
#define CLONE                   2
#define JUMP                    3
#define VALID                   1
#define INVALID                 0

#define RED                     'R'
#define BLUE                    'B'
#define EMPTY                   '.'
#define WALL                    '#'

#define PARSING(x, mode)        [(x)[mode * 2] - 1][(x)[mode * 2 + 1] - 1]
#define POS(x, y)               [x - 1][y - 1]
#define IN_RANGE(x, min, max)   ((x) >= min && (x) <= max)
#define MATRIX_MAP              [MAP_ROW][MAP_COL]

typedef char Validity;
typedef char Movement;
typedef char bool;
typedef char TILE;

// utils
Validity scanFreeStrs(size_t len, char** buffer, size_t bufSize);
void getUnitCnt(const TILE map MATRIX_MAP, int* red, int* blue);
bool isFullMap(const TILE map MATRIX_MAP);
bool isValidInt(const char* str, int* out);

// IO
Validity scanMap(TILE pmap MATRIX_MAP);
void printMap(TILE pmap MATRIX_MAP);

Validity scanMoveNumber(int* pnum);
Validity scanMove(int move[4]);

// process
Movement validation(TILE map MATRIX_MAP, const int move[4], const TILE cur);
bool doMove(TILE map MATRIX_MAP, const int move[4], const Movement movement, const TILE cur);

Validity process(TILE map MATRIX_MAP, const int maxTurn) {
    int turn;
    int cnt[2];
    getUnitCnt(map, &cnt[0], &cnt[1]); // init unit cnt
    Movement prevMovement = INVALID;

    for (turn = 1; turn <= maxTurn; turn++) {
        int move[4] = { -1, -1, -1, -1 }; // invalid initial movement
        Movement movement;
        const TILE cur = turn % 2 == 1 ? RED : BLUE;

        if (!scanMove(move)) {
            printf("Invalid input at turn %d\n", turn);
            return INVALID;
        } else if (!(movement = validation(map, move, cur)) || !doMove(map, move, movement, cur)) {
            printf("Invalid move at turn %d\n", turn);
            return INVALID;
        } else  {
            getUnitCnt(map, &cnt[0], &cnt[1]);
            if (cnt[0] == 0 || cnt[1] == 0 || isFullMap(map) || prevMovement == PASS && movement == PASS) {
                break;
            }
        }

        prevMovement = movement;
    }

    printMap(map);

    if (cnt[0] > cnt[1]) {
        printf("Red\n");
    } else if (cnt[0] < cnt[1]) {
        printf("Blue\n");
    } else {
        printf("Draw\n");
    }

    return VALID;
}

int main() {
    TILE map MATRIX_MAP;
    int maxTurn;

    if (!scanMap(map)) {
        printf("Board input error\n");
        return 1; // case 1: Board input error
    } else if (!scanMoveNumber(&maxTurn)) {
        printf("Invalid input at turn 0\n");
        return 2; // case 2: Move number input error
    } else if (!process(map, maxTurn)) {
        return 3; // case 3: Invalid input
    }

    return 0; // valid termination
}

Movement validation(TILE map MATRIX_MAP, const int move[4], const TILE cur) {
    const int gap[2] = { move[2] - move[0], move[3] - move[1] };
    int i;
    
    if (!move[0] && !move[1] && !move[2] && !move[3]) {
        return PASS;
    }
    for (i = 0; i < 4; i++) { // out of map
        if (move[i] > (i % 2 == 0 ? MAP_ROW : MAP_COL) || move[i] <= 0) {
            return INVALID;
        }
    }

    if (map PARSING(move, SRC) != cur || map PARSING(move, DST) != EMPTY) {
        return INVALID;
    }else if (IN_RANGE(gap[0], -1, 1) && IN_RANGE(gap[1], -1, 1)) {
        return CLONE;
    } else if (IN_RANGE(gap[0], -2, 2) && IN_RANGE(gap[1], -2, 2) && (gap[0] == 0 || gap[1] == 0 || gap[1] == gap[0] || gap[1] == -gap[0])) {
       return JUMP;
    }

    return INVALID;
}

bool doMove(TILE map MATRIX_MAP, const int move[4], const Movement movement, const TILE cur) {
    const int gap[2] = { move[2] - move[0], move[3] - move[1] };

    switch (movement) {
        case PASS:
            {
                int i, j;
                for (i = 0; i < MAP_ROW; i++) {
                    for (j = 0; j < MAP_COL; j++) {
                        if (map[i][j] == cur) {
                            int x, y;
                            for (y = -2; y <= 2; y++) {
                                for (x = -2; x <= 2; x++) {
                                    if (!(x == 0 || y == 0 || x == y || x == -y) || !IN_RANGE(i + y, 0, MAP_ROW - 1) || !IN_RANGE(j + x, 0, MAP_COL - 1))
                                        continue;
                                    else if (map[i + y][j + x] == EMPTY)
                                        return FALSE;
                                }
                            }
                        }
                    }
                }
            }
        break;
        case CLONE:
        case JUMP:
            {
                int i, j;

                if (movement == JUMP)
                    map PARSING(move, SRC) = EMPTY;
                map PARSING(move, DST) = cur;

                for (i = -1; i <= 1; i++) {
                    for (j = -1; j <= 1; j++) {
                        const size_t aroundRow = move[2] + i;
                        const size_t aroundCol = move[3] + j;
                        TILE* sur;

                        if (!IN_RANGE(aroundRow, 1, MAP_ROW) || !IN_RANGE(aroundCol, 1, MAP_COL)) { // map boundary
                            continue;
                        } else if (*(sur = &map POS(aroundRow, aroundCol)) != EMPTY && *sur != WALL && *sur != cur ) { // check surround
                            *sur = cur;
                        }
                    }
                }
            }
        break;
    }

    return TRUE;
}

Validity scanMap(TILE pmap MATRIX_MAP) {
    int i;
    int j;

    for (i = 0; i < MAP_ROW; i++) {
        char line[8];
        for (j = 0; j < MAP_COL + 1; j++) { // 9 characters including \n
            char temp = getchar();

            switch (temp) {
                case RED:
                case BLUE:
                case EMPTY:
                case WALL:
                    if (j >= MAP_COL)
                        return INVALID;
                break;
                case '\n':
                    if (j < MAP_COL)
                        return INVALID;
                break;
                default:
                    return INVALID;
            }

            if (j < MAP_COL)
                line[j] = temp;
        }
        memcpy(pmap[i], line, MAP_COL);
    }

    return VALID;
}

void printMap(TILE pmap MATRIX_MAP) {
    int i;
    int j;

    for (i = 0; i < MAP_ROW; i++) {
        for (j = 0; j < MAP_COL; j++) {
            putchar(pmap[i][j]);
        }
        printf("\n");
    }
}

bool isValidInt(const char* str, int* out){
    char *ep;
    long value;
    
    errno = 0;

    value = strtol(str, &ep, 10);

    if (*ep != '\0' || errno == ERANGE || value < INT_MIN || value > INT_MAX) {
        errno = 0;
        return FALSE;
    }

    *out = (int)value;

    return TRUE;
}

Validity scanMoveNumber(int* pnum) {
    char* tempFormat;
    char temp[MAX_STR];
    Validity valid = VALID;

    if (!scanFreeStrs(1, &tempFormat, MAX_STR) || sscanf(tempFormat, "%s", temp) != 1) {
        valid = INVALID;
    } else if (!isValidInt(temp, pnum) || *pnum < 0) {
        valid = INVALID;
    }

    free(tempFormat);

    return valid;
}

Validity scanMove(int move[4]) {
    char* tempFormat;
    char temp[4][MAX_STR];
    Validity valid = VALID;

    if (!scanFreeStrs(4, &tempFormat, MAX_STR) || sscanf(tempFormat, "%s %s %s %s", &temp[0], &temp[1], &temp[2], &temp[3]) != 4) {
        valid = INVALID;
    } else { // negative check
        int i;
        for (i = 0; i < 4; i++) {
            int res;

            if (!isValidInt(temp[i], &res) || res < 0) {
                valid = INVALID;
                continue;
            }

            move[i] = res;
        }
    }

    free(tempFormat);

    return valid;
}

Validity scanFreeStrs(size_t len, char** buffer, size_t bufSize) {
    char* tempFormat = (char*)malloc(bufSize);
    int pivot = 0;
    bool context = 0; // 0: whitespace, 1: readable
    short contentLen = 0;
    bool EOI = 0;

    while (!EOI) {
        char tempChar;
        scanf("%c", &tempChar);
        switch (tempChar) {
            case ' ':
            case '\t':
                if (context) { // save one blank b/w numbers
                    context = 0;
                    break;
                }
                context = 0;
            continue;
            case '\n':
            EOI = 1;
            continue;
            default:
                 if (!context) {
                    contentLen++;
                    if (contentLen > len) {
                        return INVALID;
                    }
                }
                context = 1;
        }

        tempFormat[pivot] = tempChar;
        pivot++;
        if (pivot == MAX_STR - 2)
            break;
    }
    tempFormat[pivot] = '\0';

    *buffer = tempFormat;

    return VALID;
}

void getUnitCnt(const TILE map MATRIX_MAP, int* pred, int* pblue) {
    int i, j;
    int red = 0, blue = 0;

    for (i = 0; i < MAP_ROW; i++) {
        for (j = 0; j < MAP_COL; j++) {
            if (map[i][j] == RED)
                red++;
            else if (map[i][j] == BLUE)
                blue++;
        }
    }

    *pred = red;
    *pblue = blue;
}

bool isFullMap(const TILE map MATRIX_MAP) {
    int i, j;

    for (i = 0; i < MAP_ROW; i++)
        for (j = 0; j < MAP_COL; j++)
            if (map[i][j] == EMPTY)
                return FALSE;

    return TRUE;
}