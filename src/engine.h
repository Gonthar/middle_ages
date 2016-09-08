 /** @file
    Interface of game engine.

    @author Krzysztof Sornat <kso@mimuw.edu.pl>
    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-04-26
 */

#ifndef ENGINE_H
#define ENGINE_H

/**
 * Possible results from most functions in this module describing game state after they are finished.
 */
enum GameResult {
	RESULT_ONGOING = -1,
	RESULT_WIN = 0,
	RESULT_DRAW = 1,
	RESULT_LOSE = 2,
	RESULT_WRONG_COMMAND = 42
};

/**
 * Initializes a game.
 * Returns `RESULT_WRONG_COMMAND` in a case of error.
 * Returns `RESULT_ONGOING`  otherwise.
 */
int init(int n, int k, int p, int x1, int y1, int x2, int y2);

/**
 * Returns whether it's this AI's player's turn.
 * @return 1 when it's this AI's player's turn, 0 otherwise
 */
int ai_turn();

/**
 * Makes a move.
 * @param[in] x1 Column number before a move.
 * @param[in] y1 Row number before a move.
 * @param[in] x2 Column number after a move.
 * @param[in] y2 Row number before a move.
 * @return `RESULT_ONGOING`, `RESULT_WIN`, `RESULT_DRAW`, `RESULT_LOSE`, `RESULT_WRONG_COMMAND` describing state of the game after the move and possibly fight.
 */
int move(int x1, int y1, int x2, int y2);

/**
 * Produces a knight on (x2, y2) by a peasant on (x1, y1).
 * @return `RESULT_ONGOING` or `RESULT_WRONG_COMMAND` describing state of the game after the production.
 */
int produce_knight(int x1, int y1, int x2, int y2);

/**
 * Produces a peasant on (x2, y2) by a peasant on (x1, y1).
 * @return `RESULT_ONGOING` or `RESULT_WRONG_COMMAND` describing state of the game after the production.
 */
int produce_peasant(int x1, int y1, int x2, int y2);

/**
 * Returns state of the game after finishing the current turn. They game may end by reaching rounds limit.
 * @return `RESULT_ONGOING`, `RESULT_DRAW` or `RESULT_WRONG_COMMAND` describing state of the game after the production.
 */
int end_turn();

/**
 * It prints (int stdout) top-left corner of the board of size m x m where m = min(n,10).
 */
void print_topleft();

/**
 * It initialize the game. Needed before first INIT.
 */
void start_game();

/**
 * Freeing a memory. Needed after finishing game.
 */
void end_game();

/**
 * Have AI compute and print its move and return state of the game after it.
 * @return `RESULT_ONGOING`, `RESULT_WIN`, `RESULT_DRAW`, `RESULT_LOSE` describing state of the game after the move and possibly fight.
 */
int ai_make_move();

#endif /* ENGINE_H */
