/**
 * @file main.c
 * @brief Very brief example of lib's usage.
 * @warning Library is not complete yet and API will be changed.
 * @athor Slavomir Vlcek
 * @copyright none
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>		/* getopt() */

#include "../lib/rapm.h"


typedef enum {

	NONE = 0,
	CHAT = (1 << 0),
	JOINERS = (1 << 1),
	LENGTH = (1 << 2),
	MAP_PATH = (1 << 3),
	NO_OF_MAP_POSITIONS = (1 << 4),
	BUILD = (1 << 5),
	PATCH = (1 << 6),
	RELEASE = (1 << 7)

} req_t;


void usage(char *prog_name)
{
	printf("This demoprogram inspects WarCraft III replays.\n");
	printf("usage: %s [HELP/OPTIONS] FILE\n", prog_name);
	printf("HELP: -h\n");
	printf("OPTIONS:\n");
	printf("-a	all stuff\n"
	       "-b	build version\n"
	       "-c	chat messages\n"
	       "-j	every joiner info\n"
	       "-l	replay length\n"
	       "-m	map location path\n"
	       "-p	patch version\n"
	       "-r	release version\n");
}


/**
 * @brief Get statically allocated string that hold printable mmt.
 */
char *get_mmt_str(mmt_t *mmt)
{
#define STR_LEN 20

	static char str[STR_LEN];

	snprintf(str, STR_LEN, "%.2u:%.2u:%.2u", mmt->hour, mmt->min, mmt->sec);
	return str;

#undef STR_LEN
}


int demo_lib(req_t req, char *file_path)
{
	int ret = 0;
	apm_wc3_attr_t attr;

	apm_wc3_attr_init(&attr);
	apm_wc3_attr_setfilepath(&attr, file_path);

	/*
	 * notice that by using all 3 enum values, we always process all info
	 * out of replay, but this is just demonstration program
	 */
	apm_wc3_attr_settask(&attr,
			     APM_TASK_BASIC | APM_TASK_ADDTL | APM_TASK_APM);

	apm_t *apm = apm_wc3_init();
	if (NULL == apm) {
		ret = 1;
		goto cleanup;
	}

	ret = apm_wc3_operate(apm, &attr);
	if (0 != ret) {
		goto cleanup;
	}


	char rls[][30] = { "Reign Of Chaos", "Frozen Throne"};

	if (req & RELEASE) {
		printf("+release: %s\n", rls[ apm_wc3_getrlsver(apm) ]);
	}

	if (req & PATCH) {
		printf("+patch_version: %u\n", apm_wc3_getpatchver(apm));
	}

	if (req & BUILD) {
		printf("+build: %u\n", apm_wc3_getbuild(apm));
	}

	if (req & LENGTH) {
		printf("+replay_length (ms): %u\n", apm_wc3_getreplen(apm));
	}

	if (req & MAP_PATH) {
		printf("+map_path: %s\n", apm_wc3_getmappath(apm) );
	}

	if (req & NO_OF_MAP_POSITIONS) {
		printf("+map_positions: %u\n", apm_wc3_getmapposcnt(apm) );
	}


	if (req & CHAT) {
		unsigned cnt = apm_wc3_getmsgcnt(apm);
		msgbox_t *box;

		printf("---->CHAT MESSAGES\n");

		for (size_t i = 0;  i < cnt;  ++i) {

			box = apm_wc3_getmsg(apm, i);
			printf("msg #: %u, from joiner #: %u, at %s\n %s\n",
			       box->no, box->joiner_id, get_mmt_str(&box->mmt),
			       box->msg);
			printf("----------------------------\n");
		}
	}

	if (req & JOINERS) {
		printf("---->JOINERS\n");

		joiner_t *j;
		unsigned cnt;

		cnt = apm_wc3_getjoinercnt(apm);

		for (size_t i = 0;  i < cnt;  ++i) {

			j = apm_wc3_getjoiner(apm, i);
			printf("joiner: %s (id: %u, team: %u)\n",
			       j->name, j->id, j->team_no);
			printf("host: %u, saver: %u, HP: %u, apm %lu,"\
			       " leave_time : %s\n",
			       (unsigned)j->host, (unsigned)j->saver,
			       j->hp_handicap, j->apm,
			       get_mmt_str(&j->leave_mmt));
			printf("----------------------------\n");
		}
	}
cleanup:
	apm_wc3_deinit(apm);
	apm_wc3_attr_deinit(&attr);

	return ret;
}


/**
 * @brief Process program arguments.
 */
int proc_args(int argc, char **argv, req_t *req, char **path)
{
	int ret;
	unsigned help = 0;

	while (-1 != (ret = getopt(argc, argv, "abchjlmpr")))
		switch (ret)
		{

		case 'a':
			*req = CHAT | JOINERS | LENGTH | MAP_PATH\
				| NO_OF_MAP_POSITIONS | BUILD | PATCH | RELEASE;
			break;
		case 'b':
			*req |= BUILD;
			break;
		case 'c':
			*req |= CHAT;
			break;
		case 'h':
			help = 1;
			break;
		case 'j':
			*req |= JOINERS;
			break;
		case 'l':
			*req |= LENGTH;
			break;
		case 'm':
			*req |= MAP_PATH;
			break;
		case 'n':
			*req |= NO_OF_MAP_POSITIONS;
			break;
		case 'p':
			*req |= PATCH;
			break;
		case 'r':
			*req |= RELEASE;
			break;

		case '?':
			if (isprint(optopt)) {
				fprintf(stderr,
					"Unrecognized opt (%c).\n",
					optopt);
			} else {
				fprintf(stderr,
					"Unrecognized unprintable opt (%c).\n",
					optopt);
			}
			return 1;
		default:
			abort();
		}

	/* the only argument was '-h' for help */
	if (help) {
		if (2 == argc) {
			return -1;
		} else {
			return 1;
		}
	}

	if (argc != optind + 1) {
		fprintf(stderr, "Replay file path missing.\n");
		return 1;
	} else {
		*path = argv[optind];
	}

	return 0;
}


int main(int argc, char **argv)
{
	int ret;
	req_t req;
	char *file_path;

	ret = proc_args(argc, argv, &req, &file_path);
	if (-1 == ret) {
		usage(argv[0]);
		return EXIT_SUCCESS;
	} else if (1 == ret) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	ret = demo_lib(req, file_path);
	if (0 != ret) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
