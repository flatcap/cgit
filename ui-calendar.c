#include "cgit.h"
#include "ui-calendar.h"
#include "html.h"
#include "ui-shared.h"

// static int cmp_repos(const void *a, const void *b)
int comp_int (int *a, int *b)
{
	if (*a == *b)
		return 0;
	if (*a < *b)
		return -1;
	return 1;
}

void calc_quartiles (int array[], int members, int *quartiles)
{
	size_t mem_size = members * sizeof (int);

	int *copy = malloc (mem_size);
	memcpy (copy, array, mem_size);

	qsort (copy, members, sizeof(int), (__compar_fn_t) &comp_int);

	// int q;
	// for (q = 0; q < members; q++) {
	// 	htmlf ("%d, ", copy[q]);
	// }

	int i;
	int skip = 0;
	for (i = 0; i < members; i++) {
		if (copy[i] > 0) {
			skip = i;
			break;
		}
	}

	// htmlf ("skip = %d<br>", skip);

	if (skip == 0) {		// no data
		quartiles[0] = 1;
		quartiles[1] = 1;
		quartiles[2] = 1;
	} else {
		int section = (members - skip) / 4;

		quartiles[0] = copy[skip+(1*section)];
		quartiles[1] = copy[skip+(2*section)];
		quartiles[2] = copy[skip+(3*section)];
	}
}

void cgit_show_calendar(void)
{
	//html("<pre>");

	// html("1054: Total commits\n");
	// html("  58: Days ( Feb 25 2014 - Apr 23 2014 ) - Longest streak excluding weekends\n");
	// html("  58: Days ( Feb 25 2014 - Apr 23 2014 ) - Longest streak including weekends\n");
	// html("   0: Days - Current streak\n");

	html("<br><br>");

	struct rev_info rev;
	struct commit *commit;
	const char *argv[] = {NULL, ctx.qry.head, NULL, NULL, NULL, NULL};
	int argc = 3;
	struct commitinfo *info;

	argv[2] = xstrdup("--since='53 weeks'");

	// htmlf ("query = %s<br>", argv[0]);
	// htmlf ("query = %s<br>", argv[1]);
	// htmlf ("query = %s<br>", argv[2]);
	// htmlf ("query = %s<br>", argv[3]);
	// htmlf ("query = %s<br>", argv[4]);

	init_revisions(&rev, NULL);
	rev.abbrev = DEFAULT_ABBREV;
	rev.commit_format = CMIT_FMT_DEFAULT;
	rev.max_parents = 1;
	rev.verbose_header = 1;
	rev.show_root_diff = 0;
	setup_revisions(argc, argv, &rev, NULL);
	prepare_revision_walk(&rev);
	time_t now = 0;
	time(&now);
	int cc[54*7];
	memset (cc, 0, sizeof (cc));
	int total = 0;
	while ((commit = get_revision(&rev)) != NULL) {
		info = cgit_parse_commit(commit);
		int day = (now - commit->date)/ 86400;
		cc[day+4]++;
		total++;
		free_commit_buffer(commit);
		free_commit_list(commit->parents);
		commit->parents = NULL;
		cgit_free_commitinfo(info);
	}

	int q[3];
	calc_quartiles (cc, sizeof(cc)/sizeof(int), q);

	int OFF = 7;
	// htmlf("q: %d, %d, %d<br>", q[0], q[1], q[2]);

	// int k;
	// for (k = 0; k < 371; k++) {
	// 	htmlf ("day %d, count %d<br>", k, cc[k]);
	// }

	html("<div class=\"calendar\">");
	//html("<div class=\"justify\">Nov Dec Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov</div>");
	int dow, woy;	// day-of-week, week-of-year
	for (dow = 0; dow < 7; dow++) {
		for (woy = 0; woy < 54; woy++) {
			int index = (54*7)-((7*woy)+dow);
			int count = cc[index];
			if ((index < OFF) || (index > (53*7+OFF)))
				html("<span class='c5'>&#9632 ");
			else if (count > q[2])
				html("<span class='c4'>&#9632 ");
			else if (count > q[1])
				html("<span class='c3'>&#9632 ");
			else if (count > q[0])
				html("<span class='c2'>&#9632 ");
			else if (count > 0)
				html("<span class='c1'>&#9632 ");
			else
				html("<span class='c0'>&#9632 ");
			html("</span>");
		}
		html("<br>\n");
	}
	html("</div>");

	htmlf("<br>%d: Total commits", total);

	//html("</pre>");
	// html("</div>");
}

