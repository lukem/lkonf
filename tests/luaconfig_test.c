#include <luaconfig.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TestMask {
	TEST_CONSTRUCT	= 1,
};

int
test_construct(void)
{
	printf("luaconfig_construct()\n");

	return EXIT_SUCCESS;
}


void
do_test(
	const enum TestMask	iTests,
	const enum TestMask	iFlag, 
	int 			(*iFunc)(void),
	int *			ioResult)
{
	int res = 0;

	if (! (iTests & iFlag)) {
		return;
	}

	res = iFunc();
	if (res > *ioResult) {
		*ioResult = res;
	}

}

int
main(int argc, char * argv[])
{
	enum TestMask	tests = 0;
	int		i = 0;
	int		result = EXIT_SUCCESS;
	char *		progname = 0;

		/* determine progname */
	progname = strrchr(argv[0], '/');
	if (progname) {
		++progname;
	} else {
		progname = argv[0];
	}

		/* no arguments: test everything */
	if (argc == 1) {
		tests = ~0;
	}

		/* only test specific items */
	for (i = 1; i < argc; ++i) {
		if (0 == strcmp("construct", argv[i])) {
			tests |= TEST_CONSTRUCT;
		} else {
			fprintf(stderr, "%s: unknown argument '%s'\n",
				progname, argv[i]);
			return EXIT_FAILURE;
		}
	}

	assert(tests && "no tests selected");

		/* perform each test */
	do_test(tests, TEST_CONSTRUCT, test_construct, &result);

	return result;
}
