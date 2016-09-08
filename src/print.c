 /** @file
    Interface of game protocol printer.

    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-05-16

    @edited by Maciej Gontar <mg277344@mimuw.edu.pl>
    @date 2016-08-26
 */

#include <stdio.h>
#include "print.h"

void print_end_turn_command() {
	printf("END_TURN\n");
	fflush(stdout);
}

void print_move_command(int x1, int y1, int x2, int y2) {
	printf("MOVE %d %d %d %d\n", x1, y1, x2, y2);
 	fflush(stdout);
}

void print_produce_peasant_command(int x1, int y1, int x2, int y2) {
	printf("PRODUCE_PEASANT %d %d %d %d\n", x1, y1, x2, y2);
	fflush(stdout);
}

void print_produce_knight_command(int x1, int y1, int x2, int y2) {
	printf("PRODUCE_KNIGHT %d %d %d %d\n", x1, y1, x2, y2);
	fflush(stdout);
}
