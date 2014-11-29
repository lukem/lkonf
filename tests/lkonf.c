#include <lkonf.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Return non-zero if the strings are equal.
 */
int
streq(const char * lhs, const char * rhs)
{
	assert(lhs);
	assert(rhs);
	return (0 == strcmp(lhs, rhs));
}

/**
 * Execute cmd (as "file" or "string") with arg in lkonf_context.
 */
int
command(lkonf_context * lkonf, const int limit, const char * cmd, const char * arg)
{
	lkonf_error lerr = lkonf_set_instruction_limit(lkonf, limit);
	if (LK_OK != lerr) {
		fprintf(stderr, "Can't set instruction limit %d: %s\n",
			limit,
			lkonf_get_error_string(lkonf));
		return EXIT_FAILURE;
	}

	if (0) {
	} else if (streq("file", cmd)) {
		lerr = lkonf_load_file(lkonf, arg);
	} else if (streq("string", cmd)) {
		lerr = lkonf_load_string(lkonf, arg);
	} else {
		abort();
	}

	if (LK_OK != lerr) {
		printf("Error %d: %s\n",
			lerr, /* TODO err_to_str(lr), */
			lkonf_get_error_string(lkonf));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Print a usage.
 */
int
usage(const char * progname)
{
	fprintf(stderr, "Usage: %s [-l <limit>] <cmd> <argument>\n",
		progname);
	fprintf(stderr, "    -l <limit>  Instruction limit; 0 for none. [0]\n");
	fprintf(stderr, "    <cmd>       'file' or 'string'.\n");
	fprintf(stderr, "    <argument>  If <cmd> is file, a pathname.\n");
	fprintf(stderr, "                If <cmd> is string, a Lua expression.\n");
	return EXIT_FAILURE;
}

/**
 * Main entry.
 */
int
main(int argc, char * argv[])
{
		/* determine progname */
	char * progname = strrchr(argv[0], '/');
	if (progname) {
		++progname;
	} else {
		progname = argv[0];
	}

	int limit = 0;
	int ch;
	while (-1 != (ch = getopt(argc, argv, "l:"))) {
		switch (ch) {
			case 'l':
				limit = atoi(optarg);
				if (limit < 0) {
					fprintf(stderr,
						"%s: Invalid limit '%s'\n",
						progname, optarg);
					return EXIT_FAILURE;
				}
				break;
			default:
				return usage(progname);
		}
	}

	argc -= optind;
	argv += optind;
	if (2 != argc) {
		return usage(progname);
	}

	const char * cmd = argv[0];
	const char * arg = argv[1];

	if (! streq("file", cmd) && !streq("string", cmd)) {
		fprintf(stderr, "Unknown <cmd> '%s'\n", cmd);
		return usage(progname);
	}

	lkonf_context * lkonf = lkonf_construct();
	if (! lkonf) {
		fprintf(stderr, "%s: Can't construct lkonf_context\n", progname);
		return EXIT_FAILURE;
	}

	const int rv = command(lkonf, limit, cmd, arg);

	lkonf_destruct(lkonf);

	return rv;
}
