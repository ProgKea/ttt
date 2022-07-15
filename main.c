#include <stdio.h>
#include <stdbool.h>
#include <termios.h>

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
    Cell field[ROWS][COLS];
    int cursor_row;
    int cursor_col;
    int round;
    bool running;
} Game;

Game create_game();
void create_field(Game *game);
void display_game(Game game);
void redisplay_game(Game *game);
void move_cursor(Game *game, Direction dir);
void select_cursor_cell(Game *game);
void play_again_prompt(Game *game);
void reset_game(Game *game);
bool check_on_cursor(Game game, int row, int col);
bool check_win(Game game);
bool check_draw(Game game);

Game create_game() {
    Game game;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            game.field[row][col].state = empty;
        }
    }
    create_field(&game);
    game.cursor_col = 1;
    game.cursor_row = 1;
    game.round = 0;
    game.running = true;

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
        if (game->cursor_col > 0) game->cursor_col--;
        break;

        case down:
        if (game->cursor_col < COLS - 1) game->cursor_col++;
        break;

        case left:
        if (game->cursor_row > 0) game->cursor_row--;
        break;

        case right:
        if (game->cursor_row < ROWS - 1) game->cursor_row++;
        break;
    }
}

void select_cursor_cell(Game *game) {
    Cell *selected_cell = &game->field[game->cursor_row][game->cursor_col];
    if (selected_cell->state == empty && game->round % 2 != 0) {
        selected_cell->state = circle;
        game->round++;
    }
    if (selected_cell->state == empty && game->round % 2 == 0) {
        selected_cell->state = cross;
        game->round++;
    }
}

void play_again_prompt(Game *game) {
    char *winner;

    if (!check_draw(*game)) {
        if (game->round % 2 == 0) winner = "circle";
        else if (game->round % 2 != 0) winner = "cross";
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
    *game = create_game();
    redisplay_game(game);
}

bool check_on_cursor(Game game, int row, int col) {
    if (game.cursor_row == row && game.cursor_col == col) return true;
    return false;
}

bool check_win(Game game) {
    // i can explain this...
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

int main(void) {
    Game game = create_game();
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
            select_cursor_cell(&game);
            break;
        }

        redisplay_game(&game);
        if (check_win(game) || check_draw(game)) play_again_prompt(&game);
    }

    tcsetattr(0, TCSANOW, &old);
    printf("\x1b[?25h"); // To show the cursor again

    return 0;
}
