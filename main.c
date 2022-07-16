#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

#define ROWS 3
#define COLS 3

typedef enum {
    circle,
    cross,
    empty,
} State;

typedef enum {
    up,
    down,
    left,
    right,
} Direction;

typedef struct {
    State state;
    char symbol;
} Cell;

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    Cell field[ROWS][COLS];
    Position cursor;
    int round;
    bool running;
    bool single_player;
} Game;

Game     create_game(bool single_player);
void     create_field(Game *game);
void     display_game(Game game);
void     redisplay_game(Game *game);
void     move_cursor(Game *game, Direction dir);
bool     select_cursor_cell(Game *game);
void     select_circle_cell(Game *game);
int      max(int a, int b);
int      min(int a, int b);
int      evaluate(Game game);
int      minmax(Game game, int depth, int is_circle);
Position find_best_move(Game game);
void     play_again_prompt(Game *game);
void     reset_game(Game *game);
bool     check_on_cursor(Game game, int row, int col);
bool     check_if_circle_turn(int round);
bool     check_win(Game game);
bool     check_draw(Game game);

Game create_game(bool single_player) {
    Game game;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            game.field[row][col].state = empty;
        }
    }
    create_field(&game);
    game.cursor.col = 1;
    game.cursor.row = 1;
    game.round = 0;
    game.running = true;
    game.single_player = single_player;

    return game;
}

void create_field(Game *game) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            switch (game->field[row][col].state) {
                case empty:
                game->field[row][col].symbol = ' ';
                break;

                case cross:
                game->field[row][col].symbol = 'X';
                break;

                case circle:
                game->field[row][col].symbol = 'O';
                break;
            }
        }
    }
}

void display_game(Game game) {
    for (int col = 0; col < COLS; col++) {
        for (int row = 0; row < ROWS; row++) {
            bool is_at_cursor = check_on_cursor(game, row, col);
            if (is_at_cursor && game.round % 2 == 0) printf("[ ");
            else if (is_at_cursor && game.round % 2 != 0) printf("( ");
            else printf("| ");
            printf("%c", game.field[row][col].symbol);
            if (is_at_cursor && game.round % 2 == 0) printf(" ]");
            else if (is_at_cursor && game.round % 2 != 0) printf(" )");
            else printf(" |");
        }
        printf("\n");
    }
}

void redisplay_game(Game *game) {
    create_field(game);
    printf("\x1b[3A"); // \[A is the escape code to move the cursor up
    display_game(*game);
}

void move_cursor(Game *game, Direction direction) {
    switch (direction) {
        case up:
        if (game->cursor.col > 0) game->cursor.col--;
        break;

        case down:
        if (game->cursor.col < COLS - 1) game->cursor.col++;
        break;

        case left:
        if (game->cursor.row > 0) game->cursor.row--;
        break;

        case right:
        if (game->cursor.row < ROWS - 1) game->cursor.row++;
        break;
    }
}

bool select_cursor_cell(Game *game) {
    Cell *selected_cell = &game->field[game->cursor.row][game->cursor.col];
    if (selected_cell->state == empty && !check_if_circle_turn(game->round) % 2 != 0 && !game->single_player) {
        selected_cell->state = circle;
        game->round++;
        return true;
    }
    if (selected_cell->state == empty && check_if_circle_turn(game->round)) {
        selected_cell->state = cross;
        game->round++;
        return true;
    }
    return false;
}

void select_circle_cell(Game *game) {
    Position best_move = find_best_move(*game);
    game->field[best_move.row][best_move.col].state = circle;
    game->round++;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

int evaluate(Game game) {
    if (check_win(game) && check_if_circle_turn(game.round)) return 10;
    else if (check_win(game) && !check_if_circle_turn(game.round)) return -10;
    return 0;
}

int minmax(Game game, int depth, int is_circle) {
    int score = evaluate(game);

    if (score == 10) return score;
    if (score == -10) return score;
    if (check_draw(game)) return 0;

    if (is_circle) {
        int best = -1000;

        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                if (game.field[row][col].state == empty) {
                    game.field[row][col].state = circle;
                    game.round++;
                    best = max(best, minmax(game, depth+1, !is_circle));
                    game.field[row][col].state = empty;
                    game.round--;
                }
            }
        }
        return best;
    }
    else {
        int best = 1000;

        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                if (game.field[row][col].state == empty) {
                    game.field[row][col].state = cross;
                    game.round++;
                    best = min(best, minmax(game, depth+1, !is_circle));
                    game.field[row][col].state = empty;
                    game.round--;
                }
            }
        }
        return best;
    }
}

Position find_best_move(Game game) {
    int best = -1000;
    Position best_move;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (game.field[row][col].state == empty) {
                game.field[row][col].state = circle;
                game.round++;
                int move_score = minmax(game, 0, false);
                game.field[row][col].state = empty;
                game.round--;

                if (move_score > best) {
                    best_move.row = row;
                    best_move.col = col;
                    best = move_score;
                }
            }
        }
    }
    return best_move;
}

void play_again_prompt(Game *game) {
    char *winner;

    if (!check_draw(*game)) {
        if (check_if_circle_turn(game->round)) winner = "circle";
        else winner = "cross";
    }
    else winner = "nobody";

    printf("%s won! Press 'r' to play again, and 'q' to quit\n", winner);
    char key = getc(stdin);
    while (true) {
        if (key == 'q') {
            game->running = false;
            break;
        }
        else if (key == 'r') {
            printf("\x1b[A\x1b[K"); // \[K is the escape code to erase to end of line
            reset_game(game);
            break;
        }
        else key = getc(stdin);
    }
}

void reset_game(Game *game) {
    *game = create_game(game->single_player);
    redisplay_game(game);
}

bool check_on_cursor(Game game, int row, int col) {
    if (game.cursor.row == row && game.cursor.col == col) return true;
    return false;
}

bool check_if_circle_turn(int round) {
    if (round % 2 == 0) return true;
    return false;
}

bool check_win(Game game) {
    if ((((game.field[0][0].state == cross && game.field[0][1].state == cross && game.field[0][2].state == cross) || (game.field[0][0].state == circle && game.field[0][1].state == circle && game.field[0][2].state == circle)) && (game.field[0][0].state != empty && game.field[0][1].state != empty && game.field[0][2].state != empty)) ||
        (((game.field[1][0].state == cross && game.field[1][1].state == cross && game.field[1][2].state == cross) || (game.field[1][0].state == circle && game.field[1][1].state == circle && game.field[1][2].state == circle)) && (game.field[1][0].state != empty && game.field[1][1].state != empty && game.field[1][2].state != empty)) ||
        (((game.field[2][0].state == cross && game.field[2][1].state == cross && game.field[2][2].state == cross) || (game.field[2][0].state == circle && game.field[2][1].state == circle && game.field[2][2].state == circle)) && (game.field[2][0].state != empty && game.field[2][1].state != empty && game.field[2][2].state != empty)) ||
        (((game.field[0][0].state == cross && game.field[1][0].state == cross && game.field[2][0].state == cross) || (game.field[0][0].state == circle && game.field[1][0].state == circle && game.field[2][0].state == circle)) && (game.field[0][0].state != empty && game.field[1][0].state != empty && game.field[2][0].state != empty)) ||
        (((game.field[0][1].state == cross && game.field[1][1].state == cross && game.field[2][1].state == cross) || (game.field[0][1].state == circle && game.field[1][1].state == circle && game.field[2][1].state == circle)) && (game.field[0][1].state != empty && game.field[1][1].state != empty && game.field[2][1].state != empty)) ||
        (((game.field[0][2].state == cross && game.field[1][2].state == cross && game.field[2][2].state == cross) || (game.field[0][2].state == circle && game.field[1][2].state == circle && game.field[2][2].state == circle)) && (game.field[0][2].state != empty && game.field[1][2].state != empty && game.field[2][2].state != empty)) ||
        (((game.field[0][0].state == cross && game.field[1][1].state == cross && game.field[2][2].state == cross) || (game.field[0][0].state == circle && game.field[1][1].state == circle && game.field[2][2].state == circle)) && (game.field[0][0].state != empty && game.field[1][1].state != empty && game.field[2][2].state != empty)) ||
        (((game.field[2][0].state == cross && game.field[1][1].state == cross && game.field[0][2].state == cross) || (game.field[2][0].state == circle && game.field[1][1].state == circle && game.field[0][2].state == circle)) && (game.field[2][0].state != empty && game.field[1][1].state != empty && game.field[0][2].state != empty))) return true;

    return false;
}

bool check_draw(Game game) {
    if (game.round == 9 && !check_win(game)) return true;
    return false;
}

int main(int argc, char *argv[]) {
    bool single_player;
    if (argc > 1) {
        int opt;
        while ((opt = getopt(argc, argv, "lh")) != 1) {
            if (opt == 'l') {
                single_player = false;
                break;
            }
            switch (opt) {
                case 'h':
                printf("%s to play alone\n%s -l to play with a friend (local)\n", argv[0], argv[0]);
                exit(0);

                default:
                fprintf(stderr, "Usage: %s -l or %s\n", argv[0], argv[0]);
                exit(1);
            }
        }
    }
    else single_player = true;

    Game game = create_game(single_player);
    display_game(game);
    struct termios old, new;
    tcgetattr(0, &old);

    printf("\x1b[?25l"); // to hide the cursor
    new = old;
    new.c_lflag = ~(ICANON | ECHO);
    tcsetattr(0, TCSAFLUSH, &new);

    while (game.running) {
        char key = getc(stdin);
        switch (key) {
            case 'q':
            game.running = false;
            break;

            case 'r':
            reset_game(&game);
            break;

            case 'w':
            move_cursor(&game, up);
            break;

            case 'a':
            move_cursor(&game, left);
            break;

            case 's':
            move_cursor(&game, down);
            break;

            case 'd':
            move_cursor(&game, right);
            break;

            case ' ':
            if (select_cursor_cell(&game) && !check_win(game) && !check_draw(game) && game.single_player) select_circle_cell(&game);
            break;
        }

        redisplay_game(&game);
        if (check_win(game) || check_draw(game)) play_again_prompt(&game);
    }

    tcsetattr(0, TCSANOW, &old);
    printf("\x1b[?25h"); // To show the cursor again

    return 0;
}
