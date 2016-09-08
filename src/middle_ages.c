 /** @file
    Implementation of The Middle Ages game.
	Indywidualny Projekt Programistyczny 2015/2016, MIM UW

    @author Krzysztof Sornat <kso@mimuw.edu.pl>
    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-04-26

    @edited by Maciej Gontar <mg277344@mimuw.edu.pl>
    @date 2016-08-26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "engine.h"

int main() {
	start_game();

	int exit_code = RESULT_ONGOING;
	command *new_command = NULL;
    while (exit_code == RESULT_ONGOING) {
		free(new_command);
		new_command = parse_command();

		if (strcmp(new_command->name, "INIT") == 0) {
			exit_code = init(new_command->data[0],
							 new_command->data[1],
							 new_command->data[2],
							 new_command->data[3],
							 new_command->data[4],
							 new_command->data[5],
							 new_command->data[6]);

			if (exit_code == RESULT_ONGOING && ai_turn()) {
				exit_code = ai_make_move();
			}
		} else if (strcmp(new_command->name, "PRODUCE_KNIGHT") == 0) {
			exit_code = produce_knight(new_command->data[0],
									   new_command->data[1],
									   new_command->data[2],
									   new_command->data[3]);
		} else if (strcmp(new_command->name, "PRODUCE_PEASANT") == 0) {
			exit_code = produce_peasant(new_command->data[0],
										new_command->data[1],
										new_command->data[2],
										new_command->data[3]);
		} else if (strcmp(new_command->name, "MOVE") == 0) {
			exit_code = move(new_command->data[0],
							 new_command->data[1],
							 new_command->data[2],
							 new_command->data[3]);
		} else if (strcmp(new_command->name, "END_TURN") == 0) {
			exit_code = end_turn();

			if (exit_code == RESULT_ONGOING && ai_turn()) {
				exit_code = ai_make_move();
			}
		}
    }

	free(new_command);
	end_game();

    return exit_code;
}
