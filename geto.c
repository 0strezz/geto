#include "geto.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct Map {
	struct GetoFlag *aliases[26 * 2 + 10];
	struct GetoFlag *flags;
	unsigned short noflags;
};

static enum GetoError check_integrity (struct GetoFlag*, const unsigned short);
static unsigned short normalize_alias (const char);

static void mapflags (struct Map*);
static int cmpflg (const void*, const void*);

unsigned short geto_parse (const unsigned int argc, char **argv, const unsigned short noflags, struct GetoFlag *flags, struct GetoParsed *parsed) {
	if (!noflags || !flags || argc == 1 || !argv || !parsed) {
		return 0;
	}

	memset(parsed, 0, sizeof(*parsed));
	parsed->error = check_integrity(flags, noflags);

	if (parsed->error) {
		return 0;
	}

	struct Map map = {
		.flags = flags,
		.noflags = noflags
	};
	mapflags(&map);

	for (unsigned short i = 1; i < argc; i++) {
	}
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
	qsort(map->flags, map->noflags, sizeof(*map->flags), cmpflg);

	for (unsigned short i = 0; i < map->noflags; i++) {
		const unsigned short pos = normalize_alias(map->flags[i].shortname);
		map->aliases[pos] = &map->flags[i];
	}
}

static int cmpflg (const void *f1, const void *f2) {
	struct GetoFlag *_f1 = (struct GetoFlag*) f1;
	struct GetoFlag *_f2 = (struct GetoFlag*) f2;

	return strcmp(_f1->longname, _f2->longname);
}
