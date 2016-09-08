 /** @file
    Interface of game protocol parser.

    @author Krzysztof Sornat <kso@mimuw.edu.pl>
    @author Jan Wroblewski <xi@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2016-04-26
 */

#ifndef PARSE_H
#define PARSE_H

/** Reads a command.
  returns 1 if the command is "END_TURN" and 0 otherwise.
  */
typedef struct def_command {
	char name[16];
	int data[7];
} command;

command* parse_command();

#endif /* PARSE_H */
