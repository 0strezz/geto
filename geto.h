#ifndef GETO_H
#define GETO_H

#define GETO_ARG_IS_NONEXISTENT 0b00000000
#define GETO_ARG_IS_OPTIONAL    0b00000001
#define GETO_ARG_IS_MANDATORY   0b00000010

#define GETO_ARG_TYPE_TEXT      0b00000100
#define GETO_ARG_TYPE_DOUB      0b00001000
#define GETO_ARG_TYPE_UI64      0b00010000
#define GETO_ARG_TYPE_UI32      0b00100000
#define GETO_ARG_TYPE_SI64      0b01000000
#define GETO_ARG_TYPE_SI32      0b10000000

#define GETO_IS_PROGRAMMER_FAULT(ec) (((ec) >= 1) && ((ec) <= 4))

typedef unsigned char getopts_t;
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
