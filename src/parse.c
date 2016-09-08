 /** @file
    Game protocol parser.

    @author Krzysztof Sornat <kso@mimuw.edu.pl>
    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-04-26
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#define WRONG_COMMAND_EXIT_CODE 42

#define MAX_COMMAND_LENGTH 100
#define BUFFER_LENGTH (MAX_COMMAND_LENGTH + 1)

#define TO_STRING(X) #X
#define STRING_FMT(LEN) "%" TO_STRING(LEN) "s"

typedef struct def_command {
	char name[16];
	int data[7];
} command;

int parsing_failure_exit(char *allocated_memory1, command *allocated_memory2) {
	free(allocated_memory1);
	free(allocated_memory2);
	exit(WRONG_COMMAND_EXIT_CODE);
}

int buffer_overflow(char *buffer, int buffer_length) {
	return (strlen(buffer) == buffer_length - 1 && buffer[strlen(buffer)-1] != '\n');
}

int count_singular_spaces_in_buffer(char *s) {
	int i;
	int s_length = strlen(s);
	int space_counter = 0;

	if (!isalpha(s[0])) {
		return -1;
	}

	for (i = 1; i < s_length-1; i++) {
		if (isspace(s[i])) {
			if (s[i] != ' ') {
				return -1;
			}
			else {
				if (isdigit(s[i+1])) { // s[i+1] is always accessible
					++space_counter;
				}
				else {
					return -1;
				}
			}
		}
	}

	if (s[s_length-1] != '\n') {
		return -1;
	}

	return space_counter;
}

int first_space(char *s) {
	int i;
	int s_length = strlen(s);
	for (i = 0; i <= s_length; i++) {
		if (s[i] == ' ') {
			return i;
		}
	}

	return -1;
}

int nonumbers_after_command_name(char *input, int first_space_in_buffer) {
	if (first_space_in_buffer == -1) {
		return 0;
	}

	int i;
	int input_length = strlen(input);
	for (i = first_space_in_buffer; i <= input_length-2; i++) {
		if (input[i] != ' ' && !isdigit(input[i])) {
			return 1;
		}
	}

	return 0;
}

command* parse_command() {
    char *input = calloc(BUFFER_LENGTH, sizeof(char));

	if (fgets(input, BUFFER_LENGTH, stdin) == NULL || buffer_overflow(input, BUFFER_LENGTH)) {
		parsing_failure_exit(input, NULL); // error of reading, EOF before \n or buffer overflow
	}

	int data_points = count_singular_spaces_in_buffer(input);
	if (data_points < 0) {
		parsing_failure_exit(input, NULL); // error, whitespace characters different than space (before \n\0) or double spaces
	}

	command *new_command = malloc(sizeof(command));

	// command name parsing
	int first_space_in_buffer = first_space(input);
	if (first_space_in_buffer > 15) {	
		parsing_failure_exit(input, new_command); // error, too long command name
	}

	if (nonumbers_after_command_name(input, first_space_in_buffer)) {
		parsing_failure_exit(input, new_command); // error, no digits after command name
	}

	sscanf(input, "%15s", new_command->name);

    if (strcmp(new_command->name, "END_TURN") == 0) {
		if (data_points != 0) {
			parsing_failure_exit(input, new_command);			
		}
	}
    else {
		int known_command = 0;
		if (strcmp(new_command->name, "INIT") == 0) {
			if (data_points != 7) {
				parsing_failure_exit(input, new_command);
			}

			known_command = 1;
		}
		if (strcmp(new_command->name, "MOVE") == 0)	{
			if (data_points != 4) {
				parsing_failure_exit(input, new_command);
			}

			known_command = 1;
		}
		if (strcmp(new_command->name, "PRODUCE_KNIGHT") == 0) {
			if (data_points != 4) {
				parsing_failure_exit(input, new_command);;	
			}

			known_command = 1;
		}
		if (strcmp(new_command->name, "PRODUCE_PEASANT") == 0) {
			if (data_points != 4) {
				parsing_failure_exit(input, new_command);
			}

			known_command = 1;
		}

		if ( known_command != 1 ) { // error, unknown command
			parsing_failure_exit(input, new_command);
		}

		int i;
		long int data_long;
		char *pointer_end_of_reading = input+first_space_in_buffer+1;
		for (i = 0; i < data_points ; i++) {
			data_long = strtol(pointer_end_of_reading, &pointer_end_of_reading, 10);

			// too big for int or less than 1
			if (data_long > INT_MAX || errno == ERANGE || data_long <= 0) {
				parsing_failure_exit(input, new_command);
			}

			new_command->data[i] = (int) data_long;
		}
	}

	free(input);
	return new_command;
}
