 /** @file
    Interface of game protocol printer.

    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-05-16
 */

/**
 * Prints the END_TURN command.
 */
void print_end_turn_command();

/**
 * Prints the MOVE command.
 */
 void print_move_command(int x1, int x2, int y1, int y2);

/**
 * Prints the PRODUCE_PEASANT command.
 */
 void print_produce_peasant_command(int x1, int x2, int y1, int y2);

/**
 * Prints the PRODUCE_KNIGHT command.
 */
 void print_produce_knight_command(int x1, int x2, int y1, int y2);

