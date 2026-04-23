#include "geto.h"

#include <stdio.h>

int main (int argc, char **argv) {
	struct GetoFlag flags[] = {
		{
			.longname    = "verbose",
			.description = "enable verbose output",
			.shortname   = 'v',
			.opts        = GETO_ARG_IS_NONEXISTENT
		},
		{
			.longname    = "output",
			.description = "output file path",
			.shortname   = 'o',
			.opts        = GETO_ARG_IS_MANDATORY | GETO_ARG_TYPE_TEXT
		},
		{
			.longname    = "retries",
			.description = "number of retry attempts",
			.shortname   = 'r',
			.opts        = GETO_ARG_IS_MANDATORY | GETO_ARG_TYPE_UI32
		},
		{
			.longname    = "timeout",
			.description = "connection timeout in seconds",
			.shortname   = 't',
			.opts        = GETO_ARG_IS_MANDATORY | GETO_ARG_TYPE_SI32
		},
		{
			.longname    = "scale",
			.description = "scaling factor",
			.shortname   = 's',
			.opts        = GETO_ARG_IS_MANDATORY | GETO_ARG_TYPE_DOUB
		},
		{
			.longname    = "help",
			.description = "display usage information",
			.shortname   = 'h',
			.opts        = GETO_ARG_IS_NONEXISTENT
		},
		{
			.longname    = "hidden",
			.description = "does not display information",
			.shortname   = 'h',
			.opts        = GETO_ARG_IS_NONEXISTENT
		}
	};

	struct GetoParsed p;
	geto_parse(argc, argv, 7, flags, &p);

	if (p.error != GETO_ERROR_NONE) {
		printf("fault: %d\n", p.error);
		return 0;
	}

	for (unsigned short i = 0; i < 7; i++) {
		if (flags[i].seen) {
			printf("parsed ok: %c (%d)\n", flags[i].shortname, flags[i].argset);
		}
	}

	for (unsigned short i = 0; i < p.nopositional; i++) {
		printf("pos #%d: %s\n", i, p.positionalArgs[i]);
	}

	return 0;
}
