#include "geto.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define INTERNAL_GROWING_FACTOR 4
#define INTERNAL_TRUE           1
#define INTERNAL_FALSE          0

typedef unsigned char boolean_t;

static enum GetoErrorType flags_are_well_formed (struct GetoFlag*, const unsigned short);
static unsigned short normalize_shortname (const unsigned char);

void geto_print_usage (const struct GetoUsageContext context, struct GetoFlag *flags) {
	if (context.programName == NULL || context.programDescription == NULL || flags == NULL) {
		return;
	}
	if (context.noUnitsDef >= GETO_MAX_USAGE_UNITS) {
		return;
	}

	unsigned short pad1 = 0, pad2 = 0;
	for (unsigned short i = 0; i < context.noUnitsDef; i++) {
		const unsigned short width = (unsigned short) strlen(context.units[i].like);
		pad1 = (pad1 < width) ? width : pad1;
	}

	dprintf(context.writeTo, "\x1b[1m%s\x1b[0m - %s (last compilation on %s at %s)\n\n\x1b[1mUsage\x1b[0m:\n",
		context.programName,
		context.programDescription,
		__DATE__,
		__TIME__
	);

	for (unsigned short i = 0; i < context.noUnitsDef; i++) {
		dprintf(
			context.writeTo,
			"   %s %-*s   %s\n",
			context.programName,
			pad1,
			context.units[i].like,
			context.units[i].description
		);
	}

	for (unsigned short i = 0; i < context.noFlagsDef; i++) {
		const unsigned short width = (unsigned short) strlen(flags[i].longname);
		pad2 = (pad2 < width) ? width : pad2;
	}

	if (pad1 > pad2) { pad2 = pad1 - 3; }
	dprintf(context.writeTo, "\x1b[1mArguments\x1b[0m:\n");

	for (unsigned short i = 0; i < context.noFlagsDef; i++) {
		dprintf(context.writeTo, "   -%c or --%-*s  %s\n", flags[i].shortname, pad2, flags[i].longname, flags[i].description);
	}

	if (context.notes != NULL) {
		dprintf(context.writeTo, "\x1b[1mNotes\x1b[0m:\n   %s\n", context.notes);
	}
}

unsigned short geto_go_for_it (const unsigned int argc, char **argv, struct GetoAns *ans, struct GetoFlag *flags, const unsigned short noFlagsDef) {
	if (flags == NULL) {
		return 0;
	}
	ans->error = flags_are_well_formed(flags, noFlagsDef);
	if (ans->error != GETO_ERROR_DUPLICATED_SHORTNAME) {
		return 0;
	}

	ans->flagsFound = (struct GetoFlag*) calloc(INTERNAL_GROWING_FACTOR, sizeof(*ans->flagsFound));
	assert(ans->flagsFound);

	for (unsigned int i = 1; i < argc; i++) {
		const char *element = argv[i];
		const unsigned short length = (unsigned short) strlen(element);

		/*
		 * end of arguments; whatever comes next are positional arguments
		 */
		if (length == 1 && *element == '-') {
		}
		/*
		 * using shortname flags (-h, -e, -A, etc)
		 */
		else if (length == 2 && *element == '-' && isalpha(element[1])) {
		}
	}

	return 0;
}

static enum GetoErrorType flags_are_well_formed (struct GetoFlag *flags, const unsigned short noFlagsDef) {
	boolean_t shortnameTaken[26 * 2 + 10] = {INTERNAL_FALSE};

	/*
	 * Since no more than 20 flags are defined (or at least it's not likely to happen)
	 * a simple double loop will help to know if there are duplicated longnames
	 */
	for (unsigned short i = 0; i < noFlagsDef; i++) {
		const unsigned short position = normalize_shortname(flags[i].shortname);
		if (shortnameTaken[position] == INTERNAL_TRUE) {
			return GETO_ERROR_DUPLICATED_SHORTNAME;
		}

		shortnameTaken[position] = INTERNAL_TRUE;
		const size_t thislength = strlen(flags[i].longname);

		for (unsigned short j = i + 1; j < noFlagsDef; j++) {
			const size_t thatlength = strlen(flags[i].longname);
			if (thatlength != thislength) {
				continue;
			}

			if (!strncmp(flags[i].longname, flags[j].longname, thatlength)) {
				return GETO_ERROR_DUPLICATED_LONGNAME;
			}
		}
	}

	return GETO_ERROR_NONE;
}

static unsigned short normalize_shortname (const unsigned char shortname) {
	if (islower(shortname)) {
		return shortname - 'a';
	}
	if (isupper(shortname)) {
		return shortname - 'A' + 26;
	}

	return shortname - '0' + 52;
}
