#include "cgit.h"
#include "ui-calendar.h"
#include "html.h"
#include "ui-shared.h"

#define ROUND_UP(X,M)  (((X+M-1)/M)*M)

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

	// htmlf ("members = %d<br>\n", members);
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

	int section = (members - skip) / 4;

	quartiles[0] = copy[0];
	quartiles[1] = copy[skip+(1*section)];
	quartiles[2] = copy[skip+(2*section)];
	quartiles[3] = copy[skip+(3*section)];
	quartiles[4] = copy[members-1];
}

void calc_streaks (int array[], int members, int *we, int* wd, int *cur)
{
	// need to know: day of week, month and year
	*we  = 0;
	*wd  = 0;
	*cur = 0;
}

int *get_commit_counts (time_t time_start, time_t time_finish)
{
	// --since=<date>, --after=<date>
	// Show commits more recent than a specific date.

	// --until=<date>, --before=<date>
	// Show commits older than a specific date.

	int days = ((time_finish - time_start) / 86400);
	if ((days < 1) || (days > 1000)) {
		return NULL;
	}

	int *array = xcalloc (days, sizeof(int));
	if (!array)
		return 0;

	struct rev_info rev;
	struct commit *commit;
	const char *argv[] = {NULL, ctx.qry.head, NULL};
	int argc = 3;
	struct commitinfo *info;

	char window[64];
	snprintf (window, sizeof(window), "--after='%ld seconds' --before='%ld seconds'", time_start, time_finish);
	argv[2] = xstrdup(window);

	init_revisions(&rev, NULL);
	rev.abbrev = DEFAULT_ABBREV;
	rev.commit_format = CMIT_FMT_DEFAULT;
	rev.max_parents = 1;
	rev.verbose_header = 1;
	rev.show_root_diff = 0;

	setup_revisions(argc, argv, &rev, NULL);
	prepare_revision_walk(&rev);

	while ((commit = get_revision(&rev)) != NULL) {
		info = cgit_parse_commit(commit);
		int day = ((info->committer_date - time_start) / 86400);
		if ((day >= 0) && (day < days)) {
			array[day]++;
		}

		free_commit_buffer(commit);
		free_commit_list(commit->parents);
		commit->parents = NULL;
		cgit_free_commitinfo(info);
	}

	return array;
}

#if 0
void cgit_show_calendar(void)
{
	//html("<pre>");

	// html("1054: Total commits\n");
	// html("  58: Days ( Feb 25 2014 - Apr 23 2014 ) - Longest streak excluding weekends\n");
	// html("  58: Days ( Feb 25 2014 - Apr 23 2014 ) - Longest streak including weekends\n");
	// html("   0: Days - Current streak\n");

	// html("<br><br>");

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
	time(&now);		// need to round this down to midnight to stop commits changing day dependent on current time of day
	now = (now/86400) * 86400;

	time_t start_date = now - (86400 * 53 * 7);

	fprintf (stderr, "start_date = %ld\n", start_date);

	int cc[54*7];
	memset (cc, 0, sizeof (cc));
	int total = 0;
	int over = 0;
	int commits = 0;
	while ((commit = get_revision(&rev)) != NULL) {
		// fprintf (stderr, "sha: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", commit->object.sha1[0], commit->object.sha1[1], commit->object.sha1[2], commit->object.sha1[3], commit->object.sha1[4], commit->object.sha1[5], commit->object.sha1[6], commit->object.sha1[7], commit->object.sha1[8], commit->object.sha1[9], commit->object.sha1[10], commit->object.sha1[11], commit->object.sha1[12], commit->object.sha1[13], commit->object.sha1[14], commit->object.sha1[15], commit->object.sha1[16], commit->object.sha1[17], commit->object.sha1[18], commit->object.sha1[19]);
		info = cgit_parse_commit(commit);
		// fprintf (stderr, "a date: %ld\n", info->author_date);
		// fprintf (stderr, "c date: %ld\n", info->committer_date);
		// fprintf (stderr, "subject: %s\n", info->subject);
		// fprintf (stderr, "msg: %s\n", info->msg);
		int day = (info->author_date - start_date) / 86400;
		// int day = (info->committer_date - start_date) / 86400;
		if ((day < 0) || (day >= (54*7))) {
			// fprintf (stderr, "day = %d\n", day);
			// fprintf (stderr, "date -d @%ld\n", info->author_date);
			over++;
			continue;
		}

		cc[day]++;
		commits++;
		total++;
		free_commit_buffer(commit);
		free_commit_list(commit->parents);
		commit->parents = NULL;
		cgit_free_commitinfo(info);
	}
	fprintf (stderr, "over = %d\n", over);
	fprintf (stderr, "commits = %d\n", commits);

	int q[4];
	calc_quartiles (cc, sizeof(cc)/sizeof(int), q);

	// int OFF = 7;
	// htmlf("q: %d, %d, %d<br>", q[0], q[1], q[2]);

	// int k;
	// for (k = 0; k < 371; k++) {
	// 	//htmlf ("day %d, count %d<br>", k, cc[k]);
	// 	fprintf (stderr, "day %d, count %d\n", k, cc[k]);
	// }

	const char *square = "&#9632";
	const char *space  = "&nbsp;";

	// html("<span>Nov</span><span>Dec</span><span>Jan</span><span>Feb</span><span>Mar</span><span>Apr</span><span>May</span><span>Jun</span><span>Jul</span><span>Aug</span><span>Sep</span><span>Oct</span><span>Nov</span>");
	// html("<br>");
	// html("<div class=\"calendar\">");
	// int dow, woy;	// day-of-week, week-of-year
	// for (dow = 0; dow < 7; dow++) {
	// 	for (woy = 0; woy < 54; woy++) {
	// 		int index = (54*7)-((7*woy)+dow);
	// 		int count = cc[index];
	// 		if ((index < OFF) || (index > (53*7+OFF)))
	// 			htmlf("<span class='c5'>%s", square);
	// 		else if (count > q[2])
	// 			htmlf("<span class='c4'>%s", square);
	// 		else if (count > q[1])
	// 			htmlf("<span class='c3'>%s", square);
	// 		else if (count > q[0])
	// 			htmlf("<span class='c2'>%s", square);
	// 		else if (count > 0)
	// 			htmlf("<span class='c1'>%s", square);
	// 		else
	// 			htmlf("<span class='c0'>%s", square);
	// 		html("</span>");
	// 	}
	// 	html("<br>\n");
	// }
	// html("</div>");

	// htmlf("now = %ld<br>", now);

	//html("</pre>");
	// html("</div>");

	static const char *months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	// static const char *days[] = {
	// 	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	// };

	struct tm this_month;
	gmtime_r(&now, &this_month);

	struct tm work_month;
	struct tm next_month;

	int month_start = this_month.tm_mon;

	// fprintf (stderr, "tm_sec   = %d\n", this_month.tm_sec);
	// fprintf (stderr, "tm_min   = %d\n", this_month.tm_min);
	// fprintf (stderr, "tm_hour  = %d\n", this_montthis_monthtm_hour);
	// fprintf (stderr, "tm_mday  = %d\n", this_month.tm_mday);
	// fprintf (stderr, "tm_mon   = %d\n", this_montthis_monthtm_mon);
	// fprintf (stderr, "tm_year  = %d\n", this_month.tm_year);
	// fprintf (stderr, "tm_wday  = %d\n", this_month.tm_wday);
	// fprintf (stderr, "tm_yday  = %d\n", this_month.tm_yday);
	// fprintf (stderr, "tm_isdst = %d\n", this_month.tm_isdst);

	next_month = this_month;
	next_month.tm_mday = 1;
	next_month.tm_year--;

	time_t next_time = timegm (&next_month);
	gmtime_r (&next_time, &next_month);

	html("<div class=\"calendar\">");

	html("<div class=\"month\">\n");
	html("<h1>&nbsp;</h1><span>S</span><br><span>M</span><br><span>T</span><br><span>W</span><br><span>T</span><br><span>F</span><br><span>S</span>\n");
	html("</div>\n");

	int total_offset = 0;

	int i;
	time_t work_time;
	for (i = 0; i < 13; i++) {
		html("<div class=\"month\">\n");
		htmlf("<h1>%s</h1>\n", months[(month_start+i)%12]);

		work_month = next_month;
		work_time  = next_time;
		next_month.tm_mon++;
		if (next_month.tm_mon > 11) {
			next_month.tm_year++;
			next_month.tm_mon = 0;
		}

		next_time = timegm (&next_month);
		gmtime_r (&next_time, &next_month);

		int first_day = work_month.tm_wday;
		int month_length = (next_time-work_time)/86400;

		if ((work_month.tm_mon == this_month.tm_mon) && (work_month.tm_year == this_month.tm_year)) {
			month_length = this_month.tm_mday;
		}

		int start = -first_day;
		int end   = month_length - start;

		int old_end = end;
		end = (((end + 6) / 7) * 7);

		int cols = end / 7;

		if (end == old_end)
			cols++;

		int row;
		for (row = 0; row < 7; row++) {
			int col;
			for (col = 0; col < cols; col++) {
				int j = 1+start + (col*7) + row;
				// htmlf ("%d, ", j);
				int index = j+total_offset - 2;
				int count = cc[index];
				int class;
				const char *shape = square;
				// if ((index > 362) || (j < 1) || (j > month_length)) {
				if ((j < 1) || (j > month_length)) {
					class = 0;
					shape = space;
				} else if (count > q[2]) {
					class = 4;
				} else if (count > q[1]) {
					class = 3;
				} else if (count > q[0]) {
					class = 2;
				} else if (count > 0) {
					class = 1;
				} else {
					class = 0;
				}

				// char title[128];
				// snprintf (title, sizeof (title), "%d %s %d\n%d commit%s", j, months[(month_start+i)%12], 1900+work_month.tm_year, count, (count!=1) ? "s" : "");
				// char *sha = "5092128072e6dd21f8d9f25ceed2187167c1992a";

				// htmlf ("<span class='%s'>", class);
				// cgit_commit_link("X", title, NULL, ctx.qry.head, sha, ctx.qry.path, 0);
				// html ("</span>");

				htmlf("<span title=\"%d %s %d\n%d commit%s\" class='b%d c%d'>%s</span>", j, months[(month_start+i)%12], 1900+work_month.tm_year, count, (count!=1) ? "s" : "", i%2, class, shape);
				//html("</a>");
			}
			html("<br>");
		}
		// htmlf ("off = %d", total_offset);
		total_offset += month_length;


		// int j;
		// for (j = start; j < end; j++) {
		// 	if ((j < 0) || (j >= month_length))
		// 		htmlf("<span class='c0'>%s</span>", square);
		// 	else
		// 		htmlf("<span class='c4'>%s</span>", square);
		// }

		// htmlf ("%s<br>", days[first_day]);
		// htmlf ("start %d<br>", start);
		// htmlf ("month %d<br>", month_length);
		// htmlf ("end %d<br>", end);
		// htmlf ("cols %d<br>", cols);
		// html  ("<br>");
		// htmlf ("range %d<br>", end-start+1);

		html("</div>\n");

	}

	html("<div class=\"key\">\n");
	html("Commits:<br>");
	htmlf("<span class='c0'>%s</span> 0<br>",       square);
	htmlf("<span class='c1'>%s</span> %d - %d<br>", square,      1, q[0]);
	htmlf("<span class='c2'>%s</span> %d - %d<br>", square, q[0]+1, q[1]);
	htmlf("<span class='c3'>%s</span> %d - %d<br>", square, q[1]+1, q[2]);
	htmlf("<span class='c4'>%s</span> %d - %d",     square, q[2]+1, q[3]);
	html("</div>\n");

	htmlf("<br>%d: Total commits<br><br>", total);

	// cgit_commit_link("label", "tooltip", "class", "branch", "5092128072e6dd21f8d9f25ceed2187167c1992a", ctx.qry.vpath, 0);

	html("</div>");
}

#endif

void cgit_show_calendar(void)
{
	static const char *months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	time_t time_start  = 0;
	time_t time_finish = 0;

	time(&time_finish);

	struct tm this_month;
	gmtime_r(&time_finish, &this_month);
	this_month.tm_year--;

	time_start = timegm (&this_month);
	this_month.tm_year++;

	time_start  = ROUND_UP(time_start,  86400);
	time_finish = ROUND_UP(time_finish, 86400);
	int days = ((time_finish - time_start) / 86400);

	int *array = get_commit_counts(time_start, time_finish);
	if (!array)
		return;

	// htmlf ("%ld, %ld (%ld)<br>\n", time_start, time_finish, (time_finish - time_start) / 86400);

	int q[5];
	calc_quartiles (array, days, q);

	// htmlf("q: %d, %d, %d, %d, %d<br>", q[0], q[1], q[2], q[3], q[4]);

	const char *square = "&#9632";
	const char *space  = "&nbsp;";

	time_t next_time = time_start;
	struct tm next_month;
	gmtime_r(&next_time, &next_month);

	// next_month.tm_mday = 1;

	struct tm work_month;
	int start_month = next_month.tm_mon;

	html("<div class=\"month\">\n");
	html("<h1>&nbsp;</h1><span>S</span><br><span>M</span><br><span>T</span><br><span>W</span><br><span>T</span><br><span>F</span><br><span>S</span>\n");
	html("</div>\n");

	// int i;
	// for (i = 0; i < days; i++) {
	int moy;
	time_t work_time;
	int total_offset = 0;
	for (moy = 0; moy < 13; moy++) {

		htmlf("<div class='month'>\n");
		//htmlf("<h1>%s<br>%d</h1>", months[(start_month+moy)%12], next_month.tm_year+1900);
		htmlf("<h1>%s</h1>", months[(start_month+moy)%12]);

		work_month = next_month;
		work_time  = next_time;
		next_month.tm_mon++;
		next_month.tm_mday = 1;
		if (next_month.tm_mon > 11) {
			next_month.tm_year++;
			next_month.tm_mon = 0;
		}

		next_time = timegm (&next_month);
		gmtime_r (&next_time, &next_month);

		int first_day = work_month.tm_wday;
		int month_length = (next_time-work_time)/86400;

		// htmlf ("%d-%02d-%02d (%d)<br>\n", 1900+work_month.tm_year, work_month.tm_mon+1, work_month.tm_mday, month_length);

		// htmlf ("mday = %d\n", work_month.tm_mday);

		// int dow = next_month.tm_wday;
		// htmlf ("wday = %d<br>\n", work_month.tm_wday);

		// int cols = month_length;
		// htmlf("c:%d:%d:%d<br>\n", cols, cols/7,cols%7);
		// cols = (cols+6) / 7;
		// htmlf("c:%d<br>\n", cols);

		if ((work_month.tm_mon == this_month.tm_mon) && (work_month.tm_year == this_month.tm_year)) {
			month_length = this_month.tm_mday;
		}

		int start = -first_day;
		int end   = month_length - start;

		int old_end = end;
		end = (((end + 6) / 7) * 7);

		int cols = end / 7;

		if (end == old_end)
			cols++;

		int row;
		for (row = 0; row < 7; row++) {
			int col;
			for (col = 0; col < cols; col++) {

				int class;
				const char *shape = square;
				int j = 1+start + (col*7) + row;
				// htmlf ("%d, ", j);
				int index = j+total_offset - 2;
				int count = array[index];

				if ((j < 0) || (j > month_length) || ((j+total_offset) > days)) {
					shape = space;
					class = 0;
				} else if (count > q[3]) {
					class = 4;
				} else if (count > q[2]) {
					class = 3;
				} else if (count > q[1]) {
					class = 2;
				} else if (count > 0) {
					class = 1;
				} else {
					class = 0;
				}

				htmlf("<span title=\"%d %s %d\n%d commit%s\" class='b%d c%d'>%s</span>\n", j+work_month.tm_mday-1, months[(start_month+moy)%12], 1900+work_month.tm_year, count, (count!=1) ? "s" : "", moy%2, class, shape);

			}
			html("<br>\n");
		}

		total_offset += month_length;

		html("</div>\n");
	}

	free(array);
}

