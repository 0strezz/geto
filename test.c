#include "geto.h"

int main (int argc, char **argv) {
	struct GetoFlag flags[] = {
		{
			.longname = "nirvana",
			.description = "",
			.shortname = 'n',
			.opts = GETO_ARG_IS_OPTIONAL
		},
		{
			.longname = "aaaaaaaaaaaaaaaaa",
			.description = "",
			.shortname = 'p',
			.opts = GETO_ARG_IS_NONEXISTENT
		},
	};

	struct GetoParsed p;
	geto_parse(argc, argv, 2, flags, &p);
	return 0;
}
