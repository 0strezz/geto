#include "geto.h"

#if GETO_NUM_FLAGS == 0
	#error "GETO: please define the number of flags the program has (GETO_NUM_FLAGS)."
#endif

#if GETO_NUM_USAGE_UNITS == 0
	#error "GETO: please define the number of usage units the program has (GETO_NUM_USAGE_UNITS)."
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ARGUMENT_NEED_MASK 0x3
#define ARGUMENT_TYPE_MASK 0xFC

#define FLAG_WAS_SEEN      0x1
#define FLAG_WASNT_SEEN    0x0

#define ARG_WAS_SET        0x1
#define ARG_WASNT_SET      0x0

#define POSITIONAL_ARGS_GROW_FACTOR 0x8

#define IS_PROGRAMMER_FAULT(ec) (((ec) >= 1) && ((ec) <= 6))

/*
 * defines a simple mapper in which the parser can access flags
 * more quickly
 */
struct Map {
	struct GetoFlag *shortnames[26 * 2 + 10];
	struct GetoFlag *flagsorted;
};

static void abort_programmer_fault (const enum GetoError);

static enum GetoError check_integrity (struct GetoFlag*);
static uint16_t normalize_alias (const char);

static void mapflags (struct Map*);
static int cmpflg (const void*, const void*);

static enum GetoError parse_shortname (const char*, const size_t, struct Map*, struct GetoFlag**);
static enum GetoError assign_argument (const char*, struct GetoFlag*);

static enum GetoError check_flags_its_arg (struct GetoFlag*);
static enum GetoError parse_longname (const char*, const size_t, struct Map*, struct GetoFlag**);

static struct GetoFlag *get_longname_flag (const char*, const size_t, struct Map *);

struct GetoParsed geto_parse (const uint32_t argc, char **argv, struct GetoFlag *flags) {
	struct GetoParsed parsed;
	memset(&parsed, 0, sizeof(parsed));

	parsed.error = check_integrity(flags);
	if ((argc == 1) || (argv == NULL)) {
		return parsed;
	}
	if (IS_PROGRAMMER_FAULT(parsed.error)) {
		abort_programmer_fault(parsed.error);
	}

	/*
	 * qsort is used in the 'mapflags' function, which means the original 'flags'
	 * array is going to be sorted and flagsorted is just a reference to 'flags'
	 */
	struct Map map = { .flagsorted = flags };
	mapflags(&map);

	uint32_t positionalArgsCap = 0;
	for (uint16_t i = 1; (i < argc) && !parsed.error; i++) {
		const char *argval = argv[i];
		const size_t argvalen = strlen(argval);

		parsed.lastArgvalueSeen = (char*) argval;
		if (positionalArgsCap != 0) {
			if (positionalArgsCap == parsed.nopositional) {
				positionalArgsCap += POSITIONAL_ARGS_GROW_FACTOR;
				parsed.positionalArgs = (char**) reallocarray(parsed.positionalArgs, positionalArgsCap, sizeof(*parsed.positionalArgs));
				assert(parsed.positionalArgs);
			}

			parsed.positionalArgs[parsed.nopositional++] = (char*) argval;
			continue;
		}
		if (argvalen >= 2 && *argval == '-' && isalnum(argval[1])) {
			parsed.error = parse_shortname(argval, argvalen, &map, &parsed.lastFlagSeen);
		}
		else if (argvalen >= 3 && *argval == '-' && argval[1] == '-' && isalnum(argval[2])) {
			parsed.error = parse_longname(argval + 2, argvalen - 2, &map, &parsed.lastFlagSeen);
		}
		else if (argvalen == 2 && *argval == '-' && argval[1] == '-') {
			parsed.nopositional = 0;
			parsed.positionalArgs = (char**) calloc(POSITIONAL_ARGS_GROW_FACTOR, sizeof(*parsed.positionalArgs));
			positionalArgsCap = POSITIONAL_ARGS_GROW_FACTOR;
			assert(parsed.positionalArgs);
		}
		else {
			parsed.error = assign_argument(argval, parsed.lastFlagSeen);
		}
	}

	if (parsed.error != GETO_ERROR_NONE) {
		return parsed;
	}

	parsed.error = check_flags_its_arg(parsed.lastFlagSeen);
	return parsed;
}

void geto_usage (const uint16_t fd, const struct GetoUsage *u, const struct GetoFlag *flags) {
	if (!u) {
		return;
	}
	dprintf(fd, "%s - %s (%s %s)\n\nUsage:\n", u->programName, u->programDesc, __DATE__, __TIME__);

	uint16_t lngstHow = 0, lngstflag = 0;
	for (uint16_t i = 0; i < GETO_NUM_USAGE_UNITS; i++) {
		const uint16_t howlen = (uint16_t) strlen(u->units[i].how);
		if (lngstHow < howlen) {
			lngstHow = howlen;
		}
	}
	for (uint16_t i = 0; i < GETO_NUM_FLAGS; i++) {
		const uint16_t flglen = ((uint16_t) strlen(flags[i].longname)) + 8;
		if (lngstflag < flglen) {
			lngstflag = flglen;
		}
	}

	const uint16_t finalpadd = (lngstHow > lngstflag) ? lngstHow : lngstflag - 3;
	for (uint16_t i = 0; i < GETO_NUM_USAGE_UNITS; i++) {
		dprintf(fd, "\t%s %-*s   %s\n", u->programName, finalpadd, u->units[i].how, u->units[i].why);
	}
	dprintf(fd, "Arguments:\n");
	for (uint16_t i = 0; i < GETO_NUM_FLAGS; i++) {
		dprintf(fd, "\t-%c or --%-*s   %s\n", flags[i].shortname, finalpadd - 3, flags[i].longname, flags[i].description);
	}

	if (u->notes) {
		dprintf(fd, "Notes:\n\t%s\n", u->notes);
	}
}

void geto_error (const uint16_t fd, const char *pn, const struct GetoParsed gp) {
	switch (gp.error) {
		case GETO_ERROR_UNKNOWN_SHORT:
		case GETO_ERROR_UNKNOWN_LONG: {
			dprintf(
				fd,
				"%s:usage:error: unknown flag encountered.\n"
				"\t`%s` is not recognized as a program option.\n"
				"\tPlease check usage.\n",
				pn,
				gp.lastArgvalueSeen
			);
			break;
		}
		case GETO_ERROR_UNNECESSARY_ARG: {
			dprintf(
				fd,
				"%s:usage:error: unnecesary argument provided.\n"
				"\t`%s` (%c) flag does take any argument, yet `%s` was given.\n"
				"\tPlease check usage.\n",
				pn,
				gp.lastFlagSeen->longname,
				gp.lastFlagSeen->shortname,
				gp.lastArgvalueSeen
			);
			break;
		}
		case GETO_ERROR_MISSING_ARG: {
			dprintf(
				fd,
				"%s:usage:error: missing argument.\n"
				"\t`%s` (%c) flag takes an argument, yet none was given.\n"
				"\tPlease check usage.\n",
				pn,
				gp.lastFlagSeen->longname,
				gp.lastFlagSeen->shortname
			);
			break;
		}
		default: { break; }
	}
}

void geto_free_posargs (struct GetoParsed *gp) {
	if (!gp || gp->positionalArgs == 0) {
		return;
	}
	free(gp->positionalArgs);
	gp->nopositional = 0;
}

static void abort_programmer_fault (const enum GetoError error) {
	static const char *const errors[] = {
		"duplicated shortname",
		"duplicated longname",
		"invalid longname",
		"invalid shortname",
		"flag argument's type not specified",
		"null array of flags given"
	};

	fprintf(stderr, "\x1b[5m\x1b[1mGeto\x1b[0m: (programmer fault): %s\nAborting now!\n", errors[error - 1]);
	exit(EXIT_FAILURE);
}

static enum GetoError check_integrity (struct GetoFlag *flags) {
	if (!flags) {
		return GETO_ERROR_BAD_NULL_FLAGS;
	}

	uint8_t aliaseen[26 * 2 + 10] = {0};
	for (uint16_t i = 0; i < GETO_NUM_FLAGS; i++) {
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

	for (uint16_t i = 0; i < GETO_NUM_FLAGS; i++) {
		const uint16_t pos = normalize_alias(flags[i].shortname);
		if (aliaseen[pos]) {
			return GETO_ERROR_DUP_SHORTNAME;
		}

		const getopts_t opts = flags[i].opts;
		if (((opts & ARGUMENT_NEED_MASK) != GETO_ARG_IS_NONEXISTENT) && !(opts & ARGUMENT_TYPE_MASK)) {
			return GETO_ERROR_BAD_ARG_TYPE;
		}

		const char *thisLongname = flags[i].longname;
		const size_t thisLength = strlen(thisLongname);

		for (uint16_t j = i + 1; j < GETO_NUM_FLAGS; j++) {
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

static uint16_t normalize_alias (const char alias) {
	if (islower(alias)) {
		return alias - 'a';
	}
	if (isupper(alias)) {
		return alias - 'A' + 26;
	}

	/*
	 * if this function is called, it is because the program know
	 * 'alias' satisfaces 'isalnum'
	 */
	return alias - '0' + 52;
}

static void mapflags (struct Map *map) {
	qsort(map->flagsorted, GETO_NUM_FLAGS, sizeof(*map->flagsorted), cmpflg);

	for (uint16_t i = 0; i < GETO_NUM_FLAGS; i++) {
		const uint16_t pos = normalize_alias(map->flagsorted[i].shortname);
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
		const uint16_t pos = normalize_alias(name);

		if (map->shortnames[pos] == 0) {
			return GETO_ERROR_UNKNOWN_SHORT;
		}

		map->shortnames[pos]->seen = FLAG_WAS_SEEN;
		map->shortnames[pos]->argset = ARG_WASNT_SET;
		*lastseen = map->shortnames[pos];

		if ((((*lastseen)->opts & ARGUMENT_NEED_MASK) == GETO_ARG_IS_MANDATORY) && ((i + 1) != argvalen)) {
			return GETO_ERROR_MISSING_ARG;
		}
	}

	return GETO_ERROR_NONE;
}

static enum GetoError assign_argument (const char *argvalue, struct GetoFlag *lastseen) {
	if (!lastseen) {
		return GETO_ERROR_UNNECESSARY_ARG;
	}

	const getopts_t opts = lastseen->opts;

	if ((opts & ARGUMENT_NEED_MASK) == GETO_ARG_IS_NONEXISTENT) {
		return GETO_ERROR_UNNECESSARY_ARG;
	}

	switch (opts & ARGUMENT_TYPE_MASK) {
		case GETO_ARG_TYPE_TEXT: {
			lastseen->argument.astext = (char*) argvalue;
			break;
		}
		case GETO_ARG_TYPE_DOUB: {
			lastseen->argument.asdouble = strtod(argvalue, NULL);
			break;
		}
		case GETO_ARG_TYPE_UI64: {
			lastseen->argument.asuint64 = strtoul(argvalue, NULL, 10);
			break;
		}
		case GETO_ARG_TYPE_UI32: {
			lastseen->argument.asuint32 = (uint32_t) strtoul(argvalue, NULL, 10);
			break;
		}
		case GETO_ARG_TYPE_SI64: {
			lastseen->argument.asint64 = strtol(argvalue, NULL, 10);
			break;
		}
		case GETO_ARG_TYPE_SI32: {
			lastseen->argument.asint32 = (int32_t) strtol(argvalue, NULL, 10);
			break;
		}
	}
	lastseen->argset = ARG_WAS_SET;
	return GETO_ERROR_NONE;
}

static enum GetoError check_flags_its_arg (struct GetoFlag *flag) {
	if (flag && (flag->opts & ARGUMENT_NEED_MASK) == GETO_ARG_IS_MANDATORY) {
		return (flag->argset) ? GETO_ERROR_NONE : GETO_ERROR_MISSING_ARG;
	}
	return GETO_ERROR_NONE;
}

static enum GetoError parse_longname (const char *argval, const size_t argvalen, struct Map *map, struct GetoFlag **lastseen) {
	if (check_flags_its_arg(*lastseen)) {
		return GETO_ERROR_MISSING_ARG;
	}

	const char *adjArgument = strchr(argval, '=');

	if (adjArgument != NULL) {
		*lastseen = get_longname_flag(argval, adjArgument - argval, map);
	} else {
		*lastseen = get_longname_flag(argval, argvalen, map);
	}

	if (*lastseen == NULL) {
		return GETO_ERROR_UNKNOWN_LONG;
	}

	(*lastseen)->seen = FLAG_WAS_SEEN;
	(*lastseen)->argset = ARG_WASNT_SET;
	if (adjArgument != NULL) {
		return assign_argument(adjArgument + 1, *lastseen);
	}
	return GETO_ERROR_NONE;
}

static struct GetoFlag *get_longname_flag (const char *name, const size_t length, struct Map *map) {
	const uint16_t assumptionPos = normalize_alias(*name);

	/*
	 * attempts to optimize the search by picking the first character
	 * of the longname and see if the shortname matches with the longname
	 * version, for example: --version => -v
	 *                         ~          /
	 *                          `---------'
	 */
	if (map->shortnames[assumptionPos] != NULL) {
		struct GetoFlag *asmpflag = map->shortnames[assumptionPos];
		const size_t asmplen = strlen(asmpflag->longname);

		if (length == asmplen && !strncmp(name, asmpflag->longname, length)) {
			return asmpflag;
		}
	}

	for (uint16_t i = 0; i < GETO_NUM_FLAGS; i++) {
		const char *flagsname = map->flagsorted[i].longname;

		if (length != strlen(flagsname)) {
			continue;
		}
		if (!strncmp(name, flagsname, length)) {
			return &map->flagsorted[i];
		}
	}

	return NULL;
}
