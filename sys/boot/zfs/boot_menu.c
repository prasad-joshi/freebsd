#include <btxv86.h>
#include <sys/_null.h>
#include "bootenv.h"
#include "boot_menu.h"

static void
move(be_console_t *cons, int row, int col)
{
	v86.ctl  = 0;
	v86.addr = 0x10;
	v86.eax  = 0x0200;
	v86.ebx  = 0x0;
	v86.edx  = ((0x00ff & row) << 8) + (0x00ff & col);
	v86int();

	cons->cur_row = row;
	cons->cur_col = col;
}

static void
erase(be_console_t *cons)
{
	int row;
	int col;
	int i;

	row = cons->cur_row;
	col = cons_total_cols(cons);
	move(cons, row, cons->scol);

	for (i = 0; i < col; i++) {
		putchar(' ');
	}

	cons->cur_col = cons->scol;
}

static void
put_char(be_console_t *cons, char ch)
{
	move(cons, cons->cur_row, cons->cur_col);
	cons->cur_col++;

	/* emit character */
	v86.addr = 0x10;
	v86.eax  = 0xe00 | (ch & 0xff);
	v86.ebx  = 0x7;
	v86int();
}

static void
print_msg(be_console_t *cons, const char *msg, int msglen)
{
	int nchars;
	int i;

	nchars  = cons_total_cols(cons);
	nchars -= (cons->cur_col - cons->scol);
	for (i = 0; i < nchars && i < msglen; i++) {
		put_char(cons, msg[i]);
	}
}

static void
draw_line(be_console_t *cons, int row, int col, int nchars)
{
	int i;

	move(cons, row, col);
	for (i = 0; i < nchars; i++) {
		print_msg(cons, "=", strlen("="));
	}
}

static void
draw_container(be_console_t *cons, int row)
{
	move(cons, row, cons->scol);
	print_msg(cons, "|", strlen("|"));

	move(cons, cons->cur_row, cons->scol + cons_total_cols(cons) - 1);
	print_msg(cons, "|", strlen("|"));
}

static void
draw_box(be_console_t *cons)
{
	int sr;
	int sc;
	int er;

	cons->cur_row = cons->srow;
	cons->cur_col = cons->scol;

	sr = cons->srow;
	sc = cons->scol;
	er = sr + cons_total_rows(cons);

	while (sr <= er) {
		if (sr == cons->srow || sr == er) {
			draw_line(cons, sr, sc, cons_total_cols(cons));
		} else {
			draw_container(cons, sr);
		}

		sr++;
	}

	cons->cur_row = cons->srow;
	cons->cur_col = cons->scol;
}

static key_event_t
get_key_event(be_console_t *cons, int timeout)
{
	int rc;

	if (cons->timeout >= 0) {
		rc = keyhit(timeout);

		if (rc == 0) {
			/* timed out */
			return TIMEOUT;
		}
	}

	/* read key scancode */
	rc = xgetc(0);
	switch (rc) {
	case KEY_ENTER:
		rc = ENTER;
		break;
	case KEY_K:
		rc = SCROLL_UP;
		break;
	case KEY_J:
		rc = SCROLL_DOWN;
		break;
	case KEY_S:
		rc = SORT_KEY;
		break;
	case KEY_O:
		rc = SORT_ORDER;
		break;
	case KEY_G:
		rc = SCROLL_HOME;
		break;
	case KEY_CAP_G:
		rc = SCROLL_END;
		break;
	case KEY_H:
	case KEY_QUST:
		rc = HELP;
		break;
	}
	return rc;
}

static void
print_menu_item(be_console_t *cons, const char *msg, int msglen, int select)
{
	const char *s  = " > ";
	const char *ns = "   ";
	const char *p  = "|";

	print_msg(cons, p, strlen(p));
	if (select) {
		print_msg(cons, s, strlen(s));
	} else {
		print_msg(cons, ns, strlen(ns));
	}

	print_msg(cons, msg, msglen);

	move(cons, cons->cur_row, cons->scol + cons_total_cols(cons) - 1);
	print_msg(cons, p, strlen(p));
}

static int
be_menu_display(be_console_t *cons, boot_conf_t *conf, int skip, int cur_active)
{
	int        i;
	boot_env_t *b;
	char       str[128];
	int        select;
	int        d;        /* number of BEs displayed */

	cons->cur_row = cons->srow + 1;
	cons->cur_col = cons->scol;
	i = 0;
	d = 0;
	BOOTENV_FOREACH(conf, b) {
		if (i < skip) {
			i++;
			continue;
		}

		if (d > cons_dispay_rows(cons)) {
			break;
		}

		memset(str, 0, sizeof(str));
		bootenv_string(b, str, cons_dispay_cols(cons));
		str[sizeof(str) - 1] = 0;

		select = (i == cur_active);

		move(cons, cons->cur_row, cons->cur_col);
		erase(cons);

		print_menu_item(cons, str, strlen(str), select);

		cons->cur_row++;
		cons->cur_col = cons->scol;
		i++;
		d++;
	}

	cons->cur_row = cons->srow;
	cons->cur_col = cons->scol;
	return (0);
}

struct help_msg {
	const char *msg;
	int        is_help;
};

static void
be_menu_display_help(be_console_t *cons, boot_conf_t *conf, int help, int timeout)
{
	struct help_msg help_msg[] = {
		{"Default BE marked with '*' at the end", 0},
		{"Press 'h' or '?' to display help message", 0},
		{"Press 'Enter'    to select BE for booting", 1},
		{"Press 'o'        to change order of sorting", 1},
		{"Press 's'        to change sort key", 1},
		{"Press 'j'        to scroll down", 1},
		{"Press 'k'        to scroll up", 1},
		{"Press 'G'        to scroll to end", 1},
		{"Press 'g'        to scroll to start", 1},
	};

	int             r;
	int             c;
	char            msg[128];
	char            d[3];
	uint32_t        sz;
	uint32_t        s;
	struct help_msg *h;
	int             i;
	int             n;

	cons->cur_row = cons->srow;
	cons->cur_col = cons->scol;
	r = cons->nrows + cons->srow + 2;
	c = cons->scol;

	/* print sort key and order information */
	memset(msg, 0, sizeof(msg));
	sz = s = sizeof(msg) - 1;
	strncpy(msg, "BEs sorted using ", sz);

	sz = s - strlen(msg);
	switch (conf->key) {
	case SORT_OBJNUM:
		strncat(msg, "'object number' ", sz);
		break;
	case SORT_TIMESTAMP:
		strncat(msg, "'creation timestamp' ", sz);
		break;
	case SORT_NAME:
		strncat(msg, "'BE name' ", sz);
		break;
	}

	sz = s - strlen(msg);
	switch (conf->order) {
	case SORT_ASCENDING:
		strncat(msg, "in ascending order", sz);
		break;
	case SORT_DESCENDING:
		strncat(msg, "in descending order", sz);
		break;
	}
	msg[s] = 0;

	move(cons, r, c);
	erase(cons);
	print_msg(cons, msg, strlen(msg));

	r++;
	r++;
	move(cons, r, c);
	erase(cons);
	if (cons->timeout >= 0) {
		memset(msg, 0, sizeof(msg));
		strncpy(msg, "Booting default BE in ", sizeof(msg));
		sz = sizeof(msg) - strlen(msg);

		d[0] = '0' + timeout;
		d[1] = 0;
		strncat(msg, d, sz);
		sz--;

		strncat(msg, " seconds", sz);
		print_msg(cons, msg, strlen(msg));
	}

	/* print help message if required */
	r++;
	r++;

	n = sizeof(help_msg) / sizeof(help_msg[0]);
	for (i = 0; i < n; i++) {
		h = help_msg + i;
		move(cons, r + i, c);
		erase(cons);
		if (help == 0 && h->is_help == 1) {
			continue;
		}

		print_msg(cons, h->msg, strlen(h->msg));
	}

	cons->cur_row = cons->srow;
	cons->cur_col = cons->scol;
}

int
be_menu_select(be_console_t *cons, boot_conf_t *conf, boot_env_t **bepp,
	int *sort_order, int *sort_key)
{
	int        rc;
	boot_env_t *b;
	int        cur_active;
	int        def_be;
	int        skip;
	int        be_count;
	int        be_selected;
	int        help;
	int        timeout;

	/* NOTE: conf->count does not start with 0 */
	be_count   = conf->count;
	cur_active = 0;
	BOOTENV_FOREACH(conf, b) {
		if (b->active == 0) {
			cur_active++;
			continue;
		}
		break;
	}

	def_be  = cur_active;
	help    = 0;
	timeout = cons->timeout;
	while (1) {
		skip = 0;

		cons->cur_row = cons->srow;
		cons->cur_col = cons->scol;

		draw_box(cons);

		be_menu_display_help(cons, conf, help, timeout);
		help = 0;

		if (cur_active > cons_dispay_rows(cons)) {
			skip = cur_active - cons_dispay_rows(cons);
		}

		rc = be_menu_display(cons, conf, skip, cur_active);
		if (rc < 0) {
			return (rc);
		}

		rc = get_key_event(cons, 1);
		switch (rc) {
		case ENTER:
			goto enter;
		case TIMEOUT:
			timeout--;
			if (timeout == 0) {
				cur_active = def_be;
				goto enter;
			}
			break;
		case SORT_ORDER:
			cons->timeout = timeout = -1;
			*bepp         = NULL;
			*sort_order   = 1;
			goto out;
		case SORT_KEY:
			cons->timeout = timeout = -1;
			*bepp         = NULL;
			*sort_key     = 1;
			goto out;
		case SCROLL_UP:
			cons->timeout = timeout = -1;
			if (cur_active) {
				cur_active--;
			}
			break;
		case SCROLL_DOWN:
			cons->timeout = timeout = -1;
			if (cur_active < be_count - 1) {
				cur_active++;
			}
			break;
		case SCROLL_PAGE_UP:
			cons->timeout = timeout = -1;
			if (cur_active > cons_dispay_rows(cons)) {
				cur_active -= cons_dispay_rows(cons);
			} else {
				cur_active = 0;
			}
			break;
		case SCROLL_PAGE_DOWN:
			cons->timeout = timeout = -1;
			if (cur_active + cons_dispay_rows(cons) < be_count - 1) {
				cur_active += cons_dispay_rows(cons);
			} else {
				cur_active = be_count - 1;
			}
			break;
		case SCROLL_HOME:
			cons->timeout = timeout = -1;
			cur_active    = 0;
			break;
		case SCROLL_END:
			cons->timeout = timeout = -1;
			cur_active    = be_count - 1;
			break;
		case HELP:
			cons->timeout = timeout = -1;
			help          = 1;
			break;
		}
	}

enter:
	be_selected = cur_active;
	cur_active  = 0;
	b           = NULL;
	BOOTENV_FOREACH(conf, b) {
		if (cur_active == be_selected) {
			break;
		}
		cur_active++;
	}

	*bepp = b;

out:
	return (0);
}

int
be_console_init(be_console_t *console, int srow, int scol, int nrows,
		int ncols, int timeout)
{
	console->srow    = srow;
	console->scol    = scol;
	console->nrows   = nrows;
	console->ncols   = ncols;
	console->timeout = timeout;

	return (0);
}
