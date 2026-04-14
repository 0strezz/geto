#ifndef GETO_H
#define GETO_H

#define GETO_FLAG_MANDATORY 0b10000000
#define GETO_FLAG_OPTIONAL  0b00000000

#define GETO_ARG_MANDATIORY 0b01100000
#define GETO_ARG_OPTIONAL   0b01000000
#define GETO_ARG_NONEXISTEN 0b00100000

#define GETO_ARG_IS_TEXT    0b00000111
#define GETO_ARG_IS_UINT64  0b00000110
#define GETO_ARG_IS_DOUBLE  0b00000101
#define GETO_ARG_IS_INT64   0b00000100
#define GETO_ARG_IS_INT32   0b00000011
#define GETO_ARG_IS_UINT32  0b00000010

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
	const unsigned short noUnitsDef;
	const unsigned short noFlagsDef;
};

enum GetoErrorType {
	GETO_ERROR_NONE                 = 0,
	GETO_ERROR_DUPLICATED_SHORTNAME = 1,
	GETO_ERROR_DUPLICATED_LONGNAME  = 2
};


struct GetoAns {
	struct GetoFlag *flagsFound;
	struct GetoFlag *lastFlagSeen;
	enum GetoErrorType error;
};

void geto_print_usage (const struct GetoUsageContext, struct GetoFlag*);
unsigned short geto_go_for_it (const unsigned int, char**, struct GetoAns*, struct GetoFlag*, const unsigned short);

#endif
