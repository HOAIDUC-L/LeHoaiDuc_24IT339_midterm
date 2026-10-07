#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "options.h"

/*
 * Traverses command-line operands or current working directory.
 * - If no operands: lists current directory "."
 * - If operands given: sorts non-directory operands and displays them first,
 *   then sorts and displays directory operands.
 * - Handles recursion (-R) if specified.
 * - Prints appropriate directory headers when multiple directories or recursion occurs.
 * - Reports filesystem errors to stderr and updates return status.
 * Returns 0 if all operations succeeded, >0 if any error occurred.
 */
int traverse_operands(int argc, char *argv[], int optind_val, const Options *opts);

#endif /* TRAVERSE_H */
