#include "geto.h"
#include <stdio.h>

int main (int argc, char **argv) {
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
		.noUnitsDef = 4,
		.noFlagsDef = 2,
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

	struct GetoAns ans;
	const unsigned short parsed = geto_go_for_it(argc, argv, &ans, flags, 2);
	switch (ans.error) {
		case GETO_ERROR_DUPLICATED_SHORTNAME: {
			printf("dup shortname\n");
			break;
		}
		case GETO_ERROR_DUPLICATED_LONGNAME: {
			printf("dup longname\n");
			break;
		}
	}
	//geto_print_usage(context, flags);
	return 0;
}
