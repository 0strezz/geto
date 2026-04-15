#include "geto.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ARGUMENT_NEED_MASK 0b00000011
#define ARGUMENT_TYPE_MASK 0b11111100

#define FLAG_WAS_SEEN   1
#define FLAG_WASNT_SEEN 0

#define ARG_WAS_SET     1
#define ARG_WASNT_SET   0

#define POSITIONAL_ARGS_GROW_FACTOR 8

/*
 * defines a simple mapper in which the parser can access flags
 * more quickly
 */
struct Map {
	struct GetoFlag *shortnames[26 * 2 + 10];
	struct GetoFlag *flagsorted;
	unsigned short noflags;
};

static enum GetoError check_integrity (struct GetoFlag*, const unsigned short);
static unsigned short normalize_alias (const char);

static void mapflags (struct Map*);
static int cmpflg (const void*, const void*);

static enum GetoError parse_shortname (const char*, const size_t, struct Map*, struct GetoFlag**);
static enum GetoError assign_argument (const char*, struct GetoFlag*);

static enum GetoError check_flags_its_arg (struct GetoFlag*);
static enum GetoError parse_longname (const char*, const size_t, struct Map*, struct GetoFlag**);

void geto_parse (const unsigned int argc, char **argv, const unsigned short noflags, struct GetoFlag *flags, struct GetoParsed *parsed) {
	if (!noflags || !flags || argc == 1 || !argv || !parsed) {
		return;
	}

	memset(parsed, 0, sizeof(*parsed));
	parsed->error = check_integrity(flags, noflags);

	if (parsed->error != GETO_ERROR_NONE) {
		return;
	}

	struct Map map = { .flagsorted = flags, .noflags = noflags };
	mapflags(&map);

	struct GetoFlag *lastseen = NULL;
	unsigned short ffound = 0;
	unsigned int positionalArgsCap = 0;

	for (unsigned short i = 1; i < argc && !parsed->error; i++) {
		const char *argval = argv[i];
		const size_t argvalen = strlen(argval);

		parsed->lastArgvalueSeen = (char*) argval;
		parsed->lastArgc = i;

		if (positionalArgsCap != 0) {
			if (positionalArgsCap == parsed->nopositional) {
				positionalArgsCap += POSITIONAL_ARGS_GROW_FACTOR;
				parsed->positionalArgs = (char**) reallocarray(parsed->positionalArgs, positionalArgsCap, sizeof(*parsed->positionalArgs));
				assert(parsed->positionalArgs);
			}

			parsed->positionalArgs[parsed->nopositional++] = (char*) argval;
			continue;
		}
		if (argvalen >= 2 && *argval == '-' && isalnum(argval[1])) {
			parsed->error = parse_shortname(argval, argvalen, &map, &lastseen);
		}
		else if (argvalen >= 3 && *argval == '-' && argval[1] == '-' && isalnum(argval[2])) {
			parsed->error = parse_longname(argval, argvalen, &map, &lastseen);
		}
		else if (argvalen == 2 && *argval == '-' && argval[1] == '-') {
			parsed->nopositional = 0;
			parsed->positionalArgs = (char**) calloc(POSITIONAL_ARGS_GROW_FACTOR, sizeof(*parsed->positionalArgs));
			positionalArgsCap = POSITIONAL_ARGS_GROW_FACTOR;
			assert(parsed->positionalArgs);
		}
		else {
			parsed->error = assign_argument(argval, lastseen);
		}
	}

	if (parsed->error != GETO_ERROR_NONE) {
		return;
	}
	parsed->error = check_flags_its_arg(lastseen);
}

static enum GetoError check_integrity (struct GetoFlag *flags, const unsigned short noflags) {
	unsigned char aliaseen[26 * 2 + 10] = {0};

	for (unsigned short i = 0; i < noflags; i++) {
		const char shortname = flags[i].shortname;

		if (!isalnum(shortname)) {
			return GETO_ERROR_BAD_SHORTNAME;
		}

		const char *longname = flags[i].longname;
		if (!longname) {
			return GETO_ERROR_BAD_LONGNAME;
		}

		flags[i].seen = FLAG_WASNT_SEEN;
		flags[i].argset = ARG_WASNT_SET;
	}

	for (unsigned short i = 0; i < noflags; i++) {
		const unsigned short pos = normalize_alias(flags[i].shortname);
		if (aliaseen[pos]) {
			return GETO_ERROR_DUP_SHORTNAME;
		}

		const char *thisLongname = flags[i].longname;
		const size_t thisLength = strlen(thisLongname);

		for (unsigned short j = i + 1; j < noflags; j++) {
			const char *thatLongname = flags[j].longname;
			const size_t thatLength = strlen(thatLongname);

			if (thisLength != thatLength) {
				continue;
			}
			if (!strncmp(thisLongname, thatLongname, thatLength)) {
				return GETO_ERROR_DUP_LONGNAME;
			}
		}
		aliaseen[pos] = 1;
	}

	return GETO_ERROR_NONE;
}

static unsigned short normalize_alias (const char alias) {
	if (islower(alias)) {
		return alias - 'a';
	}
	if (isupper(alias)) {
		return alias - 'A' + 26;
	}
	return alias - '0' + 52;
}

static void mapflags (struct Map *map) {
	qsort(map->flagsorted, map->noflags, sizeof(*map->flagsorted), cmpflg);

	for (unsigned short i = 0; i < map->noflags; i++) {
		const unsigned short pos = normalize_alias(map->flagsorted[i].shortname);
		map->shortnames[pos] = &map->flagsorted[i];
	}
}

static int cmpflg (const void *f1, const void *f2) {
	struct GetoFlag *_f1 = (struct GetoFlag*) f1;
	struct GetoFlag *_f2 = (struct GetoFlag*) f2;

	return strcmp(_f1->longname, _f2->longname);
}

static enum GetoError parse_shortname (const char *argval, const size_t argvalen, struct Map *map, struct GetoFlag **lastseen) {
	if (check_flags_its_arg(*lastseen)) {
		return GETO_ERROR_MISSING_ARG;
	}

	for (size_t i = 1; i < argvalen; i++) {
		const char name = argval[i];
		const unsigned short pos = normalize_alias(name);

		if (map->shortnames[pos] == 0) {
			return GETO_ERROR_UNKNOWN_SHORT;
		}

		map->shortnames[pos]->seen = FLAG_WAS_SEEN;
		*lastseen = map->shortnames[pos];

		if ((((*lastseen)->opts & ARGUMENT_NEED_MASK) == GETO_ARG_IS_MANDATORY) && ((i + 1) != argvalen)) {
			return GETO_ERROR_MISSING_ARG;
		}
	}

	return GETO_ERROR_NONE;
}

static enum GetoError assign_argument (const char *argvalue, struct GetoFlag *owner) {
	if (!owner) {
		return GETO_ERROR_UNNECESSARY_ARG;
	}

	const getopts_t opts = owner->opts;

	if ((opts & ARGUMENT_NEED_MASK) == GETO_ARG_IS_NONEXISTENT) {
		return GETO_ERROR_UNNECESSARY_ARG;
	}

	switch (opts & ARGUMENT_TYPE_MASK) {
		case GETO_ARG_TYPE_TEXT: {
			owner->argument.astext = (char*) argvalue;
			break;
		}
		case GETO_ARG_TYPE_DOUB: {
			owner->argument.asdouble = strtod(argvalue, NULL);
			break;
		}
		case GETO_ARG_TYPE_UI64: {
			owner->argument.asuint64 = strtoul(argvalue, NULL, 10);
			break;
		}
		case GETO_ARG_TYPE_UI32: {
			owner->argument.asuint32 = (unsigned int) strtoul(argvalue, NULL, 10);
			break;
		}
		case GETO_ARG_TYPE_SI64: {
			owner->argument.asint64 = strtol(argvalue, NULL, 10);
			break;
		}
		case GETO_ARG_TYPE_SI32: {
			owner->argument.asint32 = (unsigned int) strtol(argvalue, NULL, 10);
			break;
		}
	}
	owner->argset = ARG_WAS_SET;
	return GETO_ERROR_NONE;
}

static enum GetoError check_flags_its_arg (struct GetoFlag *flag) {
	if (flag && (flag->opts & ARGUMENT_NEED_MASK) == GETO_ARG_IS_MANDATORY) {
		return (flag->argset) ? GETO_ERROR_NONE : GETO_ERROR_MISSING_ARG;
	}
	return GETO_ERROR_NONE;
}

static enum GetoError parse_longname (const char *argval, const size_t argvalen, struct Map *map, struct GetoFlag **owner) {
	const unsigned short assumptionPos = normalize_alias(argval[2]);
	const char *ogname = argval + 2;
	const size_t cmpbytes = argvalen - 2;

	*owner = NULL;
	if (!strncmp(ogname, map->shortnames[assumptionPos]->longname, cmpbytes)) {
		*owner = map->shortnames[assumptionPos];
		goto ownerset;
	}

	for (unsigned short i = 0; i < map->noflags && *owner == NULL; i++) {
		const char *flagname = map->flagsorted[i].longname;
		const size_t length = strlen(flagname);

		if (length != cmpbytes) {
			continue;
		}
		if (!strncmp(flagname, ogname, cmpbytes)) {
			*owner = &map->flagsorted[i];
		}
	}

	if (*owner == NULL) {
		return GETO_ERROR_UNKNOWN_LONG;
	}

ownerset:
	(*owner)->seen = FLAG_WAS_SEEN;
	return GETO_ERROR_NONE;
}
