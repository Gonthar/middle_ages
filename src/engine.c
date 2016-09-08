 /** @file
    Game engine.

    @author Krzysztof Sornat <kso@mimuw.edu.pl>
    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-04-26

    @edited by Maciej Gontar <mg277344@mimuw.edu.pl>
    @date 2016-08-26
 */

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "engine.h"
#include "print.h"

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

typedef struct def_unit unit;

struct def_unit {
    char type;        // K,R,C - king, knight, peasant of first player; k,r or c - king, knight, peasant of second player
    int x;            // x coordinate of the unit
    int y;            // y coordinate of the unit
    int empty_rounds; // -1 means a move done in a current round; when =2 then peasant can produce new unit
    bool ai_move;    // ai has already chosen what to do with the unit in this turn
    unit *next;
};

typedef struct def_board {
    unit *head;					  // list of units
    int size;				      // size of a board
    int number_of_rounds_left;    // number of rounds to finish the game
    int turn;                     // in {1,2} as first or second player
    int this_player;              // in {1,2} as which player we are playing as
    bool built_peasant;           // 1 peasant has been built by ai
} board;

static board* game; // global variable (common for all functions in engine.c)

void start_game() {
    game = NULL;
}

static int game_is_not_initialized() {
    return (game == NULL);
}

/**
 * Frees memory.
 */
void end_game() {
    if (game_is_not_initialized()) {
        return;
    }

    unit *unit_iterator = game->head;

    if (unit_iterator == NULL) {
        free(game);
        game = NULL;
        return;
    }

    unit *unit_temp;
    while (unit_iterator->next != NULL) {
        unit_temp = unit_iterator;
        unit_iterator = unit_iterator->next;
        free(unit_temp);
    }

    free(unit_iterator);
    free(game);
    game = NULL;
}

static int wrong_command_exit() {
    end_game();
    return RESULT_WRONG_COMMAND;
}

static board *new_board(int n, int k, int p) {
    board *new_board = malloc(sizeof(board));
    new_board->head = NULL;
    new_board->size = n;
    new_board->number_of_rounds_left = k;
    new_board->this_player = p;
    new_board->turn = 1;
    new_board->built_peasant = false;

    return new_board;
}

static unit* find_unit(int x1, int y1) {
    unit *unit_iterator = game->head;
    while (unit_iterator != NULL) {
        if (unit_iterator->x == x1 && unit_iterator->y == y1) {
            return unit_iterator;
        }

        unit_iterator = unit_iterator->next;
    }

    return NULL;
}

/**
 * Inserts new unit to the beginning of the list.
 */
static int insert_unit(char unit_type, int x, int y) {
    if (MAX(x, y) > game->size || MIN(x, y) < 1) {
        return wrong_command_exit(); // error, position (x,y) is out of a board
    } else if (find_unit( x, y ) != NULL) {
        return wrong_command_exit(); // error, position (x,y) is occupied
    }

    unit *new_unit = malloc(sizeof(unit));
    new_unit->type = unit_type;
    new_unit->x = x;
    new_unit->y = y;
    new_unit->empty_rounds = 0;
    new_unit->ai_move = false;
    new_unit->next = game->head;
    game->head = new_unit;

    return 0;
}

/**
 * Distance in an infinity norm between (x1,y1) and (x2,y2).
 */
static int distance(int x1, int y1, int x2, int y2) {
    return MAX(abs(x1-x2), abs(y1-y2));
}

/**
 * Evaluates which one player is an owner of unit u.
 */
static int player(unit* u) {
    char t = u->type;
    if (t == 'K' || t == 'R' || t == 'C') {
        return 1;
    } else {
        return 2;
    }
}

/**
 * Locates closest enemy unit.
 */
static unit* find_closest_enemy_unit(int x1, int y1) {
    unit *unit_iterator = game->head;
    unit *closest_enemy;
    int dist = 0;
    int min_dist = INT_MAX;
    while (unit_iterator != NULL) {
        if (player(unit_iterator) != game->this_player) {
            dist = distance(x1, y1, unit_iterator->x, unit_iterator->y);
            if ( dist < min_dist) {
                min_dist = dist;
                closest_enemy = unit_iterator;
            }
        }
         unit_iterator = unit_iterator->next;
    }
    return closest_enemy;
}

/**
 * Finds and returns next unit that wasn't considered by AI this turn.
 */
static unit* find_next_free_unit() {
    unit *unit_iterator = game->head;
    while (unit_iterator != NULL) {
        if (unit_iterator->ai_move == false && player(unit_iterator) == game->this_player) {
            return unit_iterator;
        }
        unit_iterator = unit_iterator->next;
    }
    return NULL;
}

/**
 * Clears ai choices for this turn.
 */
void clear_ai_move() {
    unit *unit_iterator = game->head;
    while (unit_iterator != NULL) {
        unit_iterator->ai_move = false;
        unit_iterator = unit_iterator->next;
    }
}

 /**
 * Deleting unit u from the list of units.
 */
static void kill(unit* u) {
    unit *unit_iterator = game->head;

    if (unit_iterator->x == u->x &&
        unit_iterator->y == u->y &&
        unit_iterator->type == u->type) {

        game->head = (game->head)->next;
        free(u);
        u = NULL;
    } else {
        while ((unit_iterator->next)->x != u->x ||
               (unit_iterator->next)->y != u->y ||
               (unit_iterator->next)->type != u->type) {
            unit_iterator = unit_iterator->next;
        }

        unit_iterator->next = (unit_iterator->next)->next;
        free(u);
        u = NULL;
    }
}

/**
 * Fight between unit1 and unit2.
 * @return the same kind of output as `move`.
 */
static int fight(unit* unit1, unit* unit2) {
    // change uppercase letters into lowercase
    char simple_type1 = unit1->type + 32 * (unit1->type <= 90);
    char simple_type2 = unit2->type + 32 * (unit2->type <= 90);

    if (simple_type1 == simple_type2) {
        kill(unit1);
        kill(unit2);

        if (simple_type1 == 'k') {		// both kings die
            return RESULT_DRAW;
        } else {
            return RESULT_ONGOING;
        }
    } else if (simple_type1 == 'c') {
        kill(unit1);

        return RESULT_ONGOING;
    } else if (simple_type2 == 'c') {
        kill(unit2);

        return RESULT_ONGOING;
    } else if (simple_type1 == 'r' && simple_type2 == 'k') {
        int result;

        if (unit2->type == 'k') {
            result = game->this_player == 1 ? RESULT_WIN : RESULT_LOSE;
        } else {
            assert(unit2->type == 'K');
            result = game->this_player == 2 ? RESULT_WIN : RESULT_LOSE;
        }

        kill(unit2);

        return result;
    } else if (simple_type1 == 'k' && simple_type2 == 'r') {
        int result;

        if (unit1->type == 'k') {
            result = game->this_player == 1 ? RESULT_WIN : RESULT_LOSE;
        } else {
            assert(unit1->type == 'K');
            result = game->this_player == 2 ? RESULT_WIN : RESULT_LOSE;
        }

        kill(unit1);

        return result;
    }
}

static void increase_empty_rounds() {
    unit *unit_iterator = game->head;
    while (unit_iterator != NULL) {
        (unit_iterator->empty_rounds)++;
        unit_iterator = unit_iterator->next;
    }
}

/**
 * Returns `RESULT_WRONG_COMMAND` in a case of error.
 * Returns `RESULT_ONGOING` otherwise.
 */
int init(int n, int k, int player, int x1, int y1, int x2, int y2) {
    if (player < 1  ||
        player > 2  ||
             n <= 8 ||
             k < 1) {
        return wrong_command_exit(); // error, wrong player number, too small board size or non-positive number of rounds
    }

    if (x1 < 1 || y1 < 1 || x2 < 1 || y2 < 1 ||
        x1 > n || y1 > n || x2 > n || y2 > n) {
        return wrong_command_exit(); // error, (x1,y1) or (x2,y2) out of board
    }

    if (MAX(x1, x2) > n-3 || distance(x1, y1, x2, y2) < 8) {
        return wrong_command_exit(); // error, the units are out of a board or distance between kings is too small
    }

    if (game == NULL) {
        int execution_code = 0; // check if any insert_unit failed

        game = new_board(n, k, player);
        execution_code += insert_unit('K', x1  , y1);
        execution_code += insert_unit('C', x1+1, y1);
        execution_code += insert_unit('R', x1+2, y1);
        execution_code += insert_unit('R', x1+3, y1);
        execution_code += insert_unit('k', x2  , y2);
        execution_code += insert_unit('c', x2+1, y2);
        execution_code += insert_unit('r', x2+2, y2);
        execution_code += insert_unit('r', x2+3, y2);

        if (execution_code != 0) {
            return wrong_command_exit(); // error, some insert_unit failed
        } else {
            return RESULT_ONGOING;
        }
    } else {
        return wrong_command_exit(); // error, we got two INITs
    }
}

int ai_turn() {
    return game_is_not_initialized() || game->turn != game->this_player ? 0 : 1;
}

/**
 * Makes the move and returns state of the game afterwards.
 */
int move(int x1, int y1, int x2, int y2) {
    if (game_is_not_initialized()) {
        return wrong_command_exit(); // error, move before INIT
    } else if (distance( x1, y1, x2, y2 ) > 1) {
        return wrong_command_exit(); // error, move to non-adjacent position
    } else if (MAX(MAX(x1, y1), MAX(x2, y2)) > game->size ||
        MIN(MIN(x1, y1), MIN(x2, y2)) < 1) {
        return wrong_command_exit(); // error, move out of a board
    }

    unit* moved_unit = find_unit(x1, y1);
    if (moved_unit == NULL) {
        return wrong_command_exit(); // error, lack of unit at (x1,x2)
    }

    if (moved_unit->empty_rounds == -1) {
        return wrong_command_exit(); // error, this unit was already moved
    }
    if (player(moved_unit) != game->turn) {
        return wrong_command_exit(); // error, unit does not belong to the current player
    }

    unit* destination_unit = find_unit(x2, y2);

    // only change a position of an unit
    if (destination_unit == NULL) {
        moved_unit->x = x2;
        moved_unit->y = y2;
        moved_unit->empty_rounds = -1;

        return RESULT_ONGOING;
    } else {
        if (player(moved_unit) == player(destination_unit)) {
            return wrong_command_exit(); // error, try to move into position occupied by his own unit
        }
        else {
            moved_unit->x = x2;
            moved_unit->y = y2;
            moved_unit->empty_rounds = -1;
            return fight(moved_unit, destination_unit);
        }
    }
}

static int is_not_peasant(unit *pawn) {
    if (pawn->type != 'c' && pawn->type != 'C') {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Real implementation of `produce_*` functions. Returns the same what they do.
 */
static int produce_unit(int x1, int y1, int x2, int y2, char type) {
    if (game_is_not_initialized()) {
        return wrong_command_exit(); // error, action before INIT
    } else if (distance( x1, y1, x2, y2 ) > 1) {
        return wrong_command_exit(); // error, action at non-adjacent position
    } else if (MAX(MAX(x1, y1), MAX(x2, y2)) > game->size ||
        MIN(MIN(x1, y1), MIN(x2, y2)) < 1) {
        return wrong_command_exit(); // error, move out of a board
    }

    unit* peasant_produces = find_unit(x1, y1);
    if (peasant_produces == NULL) {
        return wrong_command_exit(); // error, lack of unit at (x1,x2)
    } else if (player(peasant_produces) != game->turn ||
        is_not_peasant(peasant_produces)) {
        return wrong_command_exit(); // error, an unit does not belong to the current player or it is not a peasant
    } else if (peasant_produces->empty_rounds < 2) {
        return wrong_command_exit(); // error, a peasant did not wait at least 2 rounds
    }

    unit* new_unit_destination = find_unit(x2, y2);
    if (new_unit_destination != NULL) {
        return wrong_command_exit(); // error, try to move into position occupied by his own unit
    } else if ( insert_unit(type, x2, y2) != 0 ) {
        return wrong_command_exit(); // error during inserting unit
    }

    peasant_produces->empty_rounds = -1;

    return RESULT_ONGOING;
}

/**
 * AI function, determines if move in a specified direction is allowed
 */
int check_if_move_legal(int x1, int y1, enum MoveDirection direction) {
    int x2 = x1;
    int y2 = y1;
    switch (direction) {
        case NW :
            x2--;
            y2--;
            break;
        case W :
            x2--;
            break;
        case SW :
            x2--;
            y2++;
            break;
        case N :
            y2--;
            break;
        case S :
            y2++;
            break;
        case NE:
            x2++;
            y2--;
            break;
        case E :
            x2++;
            break;
        case SE :
            x2++;
            y2++;
            break;
        case STAY :
            return 1;
        default :
            assert(false);
    }
    unit* new_unit_destination = find_unit(x2, y2); // checks for other units from the same player
    if (new_unit_destination != NULL && player(new_unit_destination) == game->this_player) {
        return 0;
    }                                               // checks for board borders
    if (MAX(x2, y2) > game->size ||
        MIN(x2, y2) < 1) {
        return 0;
    }
    return 1;
}

/**
 * Produces a knight and returns state of the game.
 */
int produce_knight(int x1, int y1, int x2, int y2) {
    if (game_is_not_initialized()) {
        return wrong_command_exit(); // error, move before INIT
    }

    return produce_unit(x1, y1, x2, y2, game->turn == 1 ? 'R' : 'r'); // inserts r or R (if player is 2 or 1 respectively)
}

/**
 * Produces a peasant and returns state of the game.
 */
int produce_peasant(int x1, int y1, int x2, int y2) {
    if (game_is_not_initialized()) {
        return wrong_command_exit(); // error, move before INIT
    }

    return produce_unit(x1, y1, x2, y2, game->turn == 1 ? 'C' : 'c'); // inserts c or C (if player is 2 or 1 respectively)
}

/**
 * Returns 1 when the game finished (reaching rounds limit).
 * Returns 42 when game was not initialized.
 * Returns 0 otherwise.
 */
int end_turn() {
    if (game_is_not_initialized()) {
        return wrong_command_exit(); // error, move before INIT
    }

    if (game->turn == 1) {
        game->turn = 2;
    } else {
        --(game->number_of_rounds_left);

        if (game->number_of_rounds_left == 0) { // it was the last one round
            return RESULT_DRAW;
        }

        game->turn = 1;
        increase_empty_rounds(); // updates empty rounds for each unit and prepares units for new turn
    }

    return RESULT_ONGOING;
}

/**
 * Checks if the desired move is possible, if not, suggests 2 alternatives
 */
enum MoveDirection correct_best_move_towards(int x, int y, enum MoveDirection direction) {
    if (check_if_move_legal(x, y, direction)) {
        return direction;
    } else if (check_if_move_legal(x, y, (direction+1)%8)) { // clockwise
        return (direction + 1) % 8;
    } else if (check_if_move_legal(x, y, (direction-1)%8)) { // counterclockwise
        return (direction-1)%8;
    } else {
        return STAY;
    }
}

enum MoveDirection find_best_move_towards(unit* ally, unit* enemy) {
    enum MoveDirection direction;
    int xdiff = ally->x - enemy->x;
    int ydiff = ally->y - enemy->y;
    if (xdiff > 0) {
        if (ydiff > 0) {
            direction = NW;
        } else if (ydiff == 0) {
            direction = W;
        } else {
            direction = SW;
        }
    } else if (xdiff == 0) {
        if (ydiff > 0) {
            direction = N;
        } else if (ydiff == 0) {
            direction = STAY;
        } else {
            direction = S;
        }
    } else {
        if (ydiff > 0) {
            direction = NE;
        } else if (ydiff == 0) {
            direction = E;
        } else {
            direction = SE;
        }
    }
    return correct_best_move_towards(ally->x, ally->y, direction);
}

int move_king_ai(unit* king) {
    king->ai_move = true;
    return RESULT_ONGOING;
}

int move_peasant_ai(unit* peasant) {
    peasant->ai_move = true;
    int x = peasant->x;
    int y = peasant->y;

    if (peasant->empty_rounds == 2) {
        unit* enemy = find_closest_enemy_unit(x, y);
        enum MoveDirection direction = find_best_move_towards(peasant, enemy);
        switch (direction) {
            case NW :
                x--;
                y--;
                break;
            case W :
                x--;
                break;
            case SW :
                x--;
                y++;
                break;
            case N :
                y--;
                break;
            case S :
                y++;
                break;
            case NE:
                x++;
                y--;
                break;
            case E :
                x++;
                break;
            case SE :
                x++;
                y++;
                break;
            case STAY :
                return RESULT_ONGOING;
            default :
                assert(false);
        }
        if (game->built_peasant == false) {
            game->built_peasant = true;
            print_produce_peasant_command(peasant->x, peasant->y, x, y);
            return produce_peasant(peasant->x, peasant->y,x, y);
        } else {
            print_produce_knight_command(peasant->x, peasant->y, x, y);
            return produce_knight(peasant->x, peasant->y, x, y);
        }
    } else {
        return RESULT_ONGOING;
    }
}

int move_knight_ai(unit* knight) {
    unit* enemy = find_closest_enemy_unit(knight->x, knight->y);
    enum MoveDirection direction = find_best_move_towards(knight, enemy);
    int x = knight->x;
    int y = knight->y;

    knight->ai_move = true;
    switch (direction) {
        case NW :
            x--;
            y--;
            break;
        case W :
            x--;
            break;
        case SW :
            x--;
            y++;
            break;
        case N :
            y--;
            break;
        case S :
            y++;
            break;
        case NE:
            x++;
            y--;
            break;
        case E :
            x++;
            break;
        case SE :
            x++;
            y++;
            break;
        case STAY :
            return RESULT_ONGOING;
        default :
            assert(false);
    }
    print_move_command(knight->x, knight->y, x, y);
    return move(knight->x, knight->y, x, y);
}

int move_unit_ai(unit* pawn) {
    switch(pawn->type){
        case 'c':
        case 'C':
            return move_peasant_ai(pawn);
            break;
        case 'k':
        case 'K':
            return move_king_ai(pawn);
            break;
        case 'r':
        case 'R':
            return move_knight_ai(pawn);
            break;
        default:
            assert(false);
    }
}

/**
 * Have AI compute and print its move.
 */
int ai_make_move() {
    int exit_code = RESULT_ONGOING;
    unit *next_unit;
    clear_ai_move();

    while (exit_code == RESULT_ONGOING) {
        next_unit = find_next_free_unit();
        if (next_unit == NULL) {
            print_end_turn_command();
            exit_code = end_turn();
            // assert(exit_code != RESULT_WRONG_COMMAND);
            return exit_code;
        } else {
            exit_code = move_unit_ai(next_unit);
        }
    }
    assert(exit_code != RESULT_WRONG_COMMAND);

    return exit_code;
};
