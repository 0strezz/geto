#include "geto.h"
#include <stdio.h>

static void usage () {
	struct GetoUsageContext context = {
		.units = {
			{"[arguments] [file ..]", "edit specified file(s)"},
			{"[arguments] -", "read text from stdin"},
			{"[arguments] -t tag", "edit file where tag is defined"},
			{"[arguments] -q [errorfile]", "edit file with first error"}
		},
		.programName = "vim",
		.programDescription = "Vi IMproved 9.1",
		.writeTo = GETO_STDOUT_FD,
		.unitsDefined = 4,
		.flagsDefined = 2,
		.notes = "this is a test"
	};

	struct GetoFlag flags[] = {
		{
			.longname = "help",
			.description = "display this message",
			.shortname = 'h',
			.options = GETO_FLAG_OPTIONAL
		},
		{
			.longname = "verbose",
			.description = "verbose messages",
			.shortname = 'v',
			.options = GETO_FLAG_OPTIONAL
		}
	};

	geto_print_usage(context, flags);
}

int main () {
	usage();
	return 0;
}
