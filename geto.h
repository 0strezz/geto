#ifndef GETO_H
#define GETO_H

#define GETO_ARG_IS_NONEXISTENT 0x0
#define GETO_ARG_IS_OPTIONAL    0x1
#define GETO_ARG_IS_MANDATORY   0x2

#define GETO_ARG_TYPE_TEXT      0x4
#define GETO_ARG_TYPE_DOUB      0x8
#define GETO_ARG_TYPE_UI64      0x10
#define GETO_ARG_TYPE_UI32      0x20
#define GETO_ARG_TYPE_SI64      0x40
#define GETO_ARG_TYPE_SI32      0x80

/*
 * define bellow:
 *
 * GETO_NUM_FLAGS: Natural number which specifies the amount of flags the program
 * have
 *
 * GETO_NUM_USAGE_UNITS: Natural number which specifies the amount of ways the program
 * can be run correctly
 *
 */

typedef unsigned short getopts_t;
typedef unsigned char geto_flgseen_t;
typedef unsigned char geto_argset_t;

enum GetoError {
	GETO_ERROR_NONE            = 0,
	GETO_ERROR_DUP_SHORTNAME   = 1,
	GETO_ERROR_DUP_LONGNAME    = 2,
	GETO_ERROR_BAD_LONGNAME    = 3,
	GETO_ERROR_BAD_SHORTNAME   = 4,
	GETO_ERROR_UNKNOWN_SHORT   = 5,
	GETO_ERROR_UNNECESSARY_ARG = 6,
	GETO_ERROR_MISSING_ARG     = 7,
	GETO_ERROR_UNKNOWN_LONG    = 8
};

struct GetoFlag {
	union {
		char *astext;
		double asdouble;
		unsigned long asuint64;
		unsigned int asuint32;
		signed long asint64;
		signed int asint32;
	} argument;
	const char *longname;
	const char *description;
	const char shortname;
	const getopts_t opts;
	/*
	 * These two fields should not be initializated, the parser
	 * will set them to 0 anyway.
	 */
	geto_flgseen_t seen;
	geto_argset_t argset;
};

struct GetoParsed {
	char *lastArgvalueSeen;
	char **positionalArgs;
	unsigned short lastArgc;
	unsigned short nopositional;
	enum GetoError error;
};

void geto_parse (const unsigned int, char**, const unsigned short, struct GetoFlag*, struct GetoParsed*);

#endif
