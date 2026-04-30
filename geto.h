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

#include <stdint.h>

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
#define GETO_NUM_FLAGS       0
#define GETO_NUM_USAGE_UNITS 0

typedef uint8_t getopts_t;
typedef uint8_t geto_flgseen_t;
typedef uint8_t geto_argset_t;

enum GetoError {
	GETO_ERROR_NONE            = 0,
	GETO_ERROR_DUP_SHORTNAME   = 1,
	GETO_ERROR_DUP_LONGNAME    = 2,
	GETO_ERROR_BAD_LONGNAME    = 3,
	GETO_ERROR_BAD_SHORTNAME   = 4,
	GETO_ERROR_BAD_ARG_TYPE    = 5,
	GETO_ERROR_BAD_NULL_FLAGS  = 6,
	GETO_ERROR_UNKNOWN_SHORT   = 7,
	GETO_ERROR_UNNECESSARY_ARG = 8,
	GETO_ERROR_MISSING_ARG     = 9,
	GETO_ERROR_UNKNOWN_LONG    = 10
};

struct GetoFlag {
	union {
		char *astext;
		double asdouble;
		uint64_t asuint64;
		uint32_t asuint32;
		int64_t asint64;
		int32_t asint32;
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
	struct GetoFlag *lastFlagSeen;
	char *lastArgvalueSeen;
	char **positionalArgs;
	uint16_t nopositional;
	enum GetoError error;
};

struct GetoUsage {
	struct {
		const char *how;
		const char *why;
	} units[GETO_NUM_USAGE_UNITS];
	const char *programName;
	const char *programDesc;
	const char *notes;
};

struct GetoParsed geto_parse (const uint32_t, char**, struct GetoFlag*);
void geto_usage (const uint16_t, const struct GetoUsage*, const struct GetoFlag*);
void geto_error (const uint16_t, const char*, const struct GetoParsed);
void geto_free_posargs (struct GetoParsed*);

#endif
