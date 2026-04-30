/*
 * This program tests how effectively geto parses and handles flags
 * and its possible arguments and errors.
 */
#include "geto.h"

#include <stdio.h>
#include <assert.h>

int main (int argc, char **argv) {
	struct GetoFlag flags[] = {
		{
			.longname = "song",
			.shortname = 's',
			.description = "song to be played",
			.opts = GETO_ARG_IS_MANDATORY | GETO_ARG_TYPE_TEXT
		},
		{
			.longname = "time",
			.shortname = 't',
			.description = "time to start playing at (seconds)",
			.opts = GETO_ARG_IS_OPTIONAL | GETO_ARG_TYPE_SI32
		},
		{
			.longname = "repeat",
			.shortname = 'r',
			.description = "repeat song",
			.opts = GETO_ARG_IS_NONEXISTENT
		}
	};

	struct GetoParsed gp = geto_parse(argc, argv, flags);
	if (gp.error != GETO_ERROR_NONE) {
		geto_error("sm", fileno(stderr), gp);
		return 0;
	}

	printf("flags\n=====\n");
	for (unsigned short i = 0; i < GETO_NUM_FLAGS; i++) {
		printf("(%c):%s:\n\tseen: (%d) - argset: (%d)\n", flags[i].shortname, flags[i].longname, flags[i].seen, flags[i].argset);
	}

	printf("\npargs\n=====\n");
	for (unsigned short i = 0; i < gp.nopositional; i++) {
		printf("\t#%d: %s\n", i, gp.positionalArgs[i]);
	}

	printf("\nusage\n=====\n");
	struct GetoUsage usage = {
		.units = {
			{
				.how = "",
				.why = "runs the program all options by default"
			},
			{
				.how = "[-s song]",
				.why = "specifies a song to be played"
			}
		},
		.programName = "sm",
		.programDesc = "something to test",
		.notes = "this program is meant to test geto"
	};
	geto_usage(fileno(stdout), &usage, flags);
	return 0;
}

