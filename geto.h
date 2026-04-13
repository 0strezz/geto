#ifndef GETO_H
#define GETO_H

#define GETO_FLAG_MANDATORY  0b10000000
#define GETO_FLAG_OPTIONAL   0b00000000

#define GETO_ARG_MANDATIORY  0b01100000
#define GETO_ARG_OPTIONAL    0b01000000
#define GETO_ARG_NONEXISTENT 0b00100000

#define GETO_ARG_IS_TEXT     0b00000111
#define GETO_ARG_IS_UINT64   0b00000110
#define GETO_ARG_IS_DOUBLE   0b00000101
#define GETO_ARG_IS_INT64    0b00000100
#define GETO_ARG_IS_INT32    0b00000011
#define GETO_ARG_IS_UINT32   0b00000010

/*
 * Represents the number of ways a program can be executed
 * (see GetoUsageContext)
 *
 * by default it is five, but it can be changed to fit programmer
 * necessities
 */
#define GETO_MAX_USAGE_UNITS 5

#define GETO_STDOUT_FD 1
#define GETO_STDERR_FD 2

#include <stdio.h>
#include <stdlib.h>

extern size_t strlen(const char *s);

typedef unsigned char getopts_t;

/*
 * Defines a container in which flags can be defined by the programmer
 * $ program -v --message "something"
 *           /     `      `~~~~~~~~~ argumemt (text)
 *       shortname  `- longname 
 *
 * description: tiny description of what the flag is used to (in case
 * usage is displayed)
 *
 * options: specfies the following via bitwise
 *
 * x-----:
 *   1: flag is mandatory
 *   0: flag is optional
 *
 * -xx---:
 *   11: takes an argument
 *   10: flag is optional
 *   01: does not take argument
 *
 * ---xxx:
 *   111: argument is text
 *   110: argument is unsigned integer 64
 *   101: argument is double
 *   100: argument is integer 64
 *   011: argument is integer 32
 *   010: argument is unsigned integer 32
 */
struct GetoFlag {
	union {
		char *asText;
		unsigned long  asUint64;
		double asDouble;
		long  asInt64;
		int  asInt32;
		unsigned int  asUint32;
	} argument;
	const char *longname;
	const char *description;
	const char shortname;
	const getopts_t options;
};

struct GetoUsageContext {
	/*
	 * A usage unit is a way in which the caller program can be used or executed by the user.
	 * For example, these are some ways in which vim can be executed properly.
	 *
	 * ~~~~~> displayed by geto (default)
	 * Usage: vim [arguments] [file ..]       edit specified file(s)
	 *    or: vim [arguments] -               read text from stdin
	 *    or: vim [arguments] -t tag          edit file where tag is defined
	 *    or: vim [arguments] -q [errorfile]  edit file with first error
	 *    ~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 *    `        `                          `
	 *    |         `---> like                 `---> description
	 *    v
	 * displayed by geto (default)
	 */
	struct {
		const char *like;
		const char *description;
	} units [GETO_MAX_USAGE_UNITS];
	const char *programName;
	const char *programDescription;
	const char *notes;
	const unsigned int writeTo;
	const unsigned short unitsDefined;
	const unsigned short flagsDefined;
};

void geto_print_usage (const struct GetoUsageContext context, struct GetoFlag *flags) {
	if (context.programName == NULL || context.programDescription == NULL || flags == NULL) {
		return;
	}
	if (context.unitsDefined >= GETO_MAX_USAGE_UNITS) {
		return;
	}

	unsigned short pad1 = 0, pad2 = 0;
	for (unsigned short i = 0; i < context.unitsDefined; i++) {
		const unsigned short width = (unsigned short) strlen(context.units[i].like);
		pad1 = (pad1 < width) ? width : pad1;
	}

	dprintf(context.writeTo, "\x1b[1m%s\x1b[0m - %s (last compilation on %s at %s)\n\n\x1b[1mUsage\x1b[0m:\n",
		context.programName,
		context.programDescription,
		__DATE__,
		__TIME__
	);

	for (unsigned short i = 0; i < context.unitsDefined; i++) {
		dprintf(
			context.writeTo,
			"   %s %-*s   %s\n",
			context.programName,
			pad1,
			context.units[i].like,
			context.units[i].description
		);
	}

	for (unsigned short i = 0; i < context.flagsDefined; i++) {
		const unsigned short width = (unsigned short) strlen(flags[i].longname);
		pad2 = (pad2 < width) ? width : pad2;
	}

	if (pad1 > pad2) { pad2 = pad1 - 3; }
	dprintf(context.writeTo, "\x1b[1mArguments\x1b[0m:\n");

	for (unsigned short i = 0; i < context.flagsDefined; i++) {
		dprintf(context.writeTo, "   -%c or --%-*s  %s\n", flags[i].shortname, pad2, flags[i].longname, flags[i].description);
	}

	if (context.notes != NULL) {
		dprintf(context.writeTo, "\x1b[1mNotes\x1b[0m:\n   %s\n", context.notes);
	}
}

#endif
