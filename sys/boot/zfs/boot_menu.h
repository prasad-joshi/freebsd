#ifndef __BOOT_MENU_H__
#define __BOOT_MENU_H__

#include "bootenv.h"

typedef struct be_console {
	int srow;
	int scol;

	int nrows;
	int ncols;

	int cur_row;
	int cur_col;

	int timeout;
} be_console_t;

static inline int
cons_total_rows(be_console_t *cons)
{
	return (cons->nrows);
}

static inline int
cons_dispay_rows(be_console_t *cons)
{
	/* leave two rows for box lines */
	return (cons->nrows - 2);
}

static inline int
cons_total_cols(be_console_t *cons)
{
	return (cons->ncols);
}

static inline int
cons_dispay_cols(be_console_t *cons)
{
	/* leave two cols for box lines */
	return (cons->ncols - 2);
}

typedef enum {
	ENTER,

	SCROLL_UP,
	SCROLL_DOWN,

	SCROLL_PAGE_UP,
	SCROLL_PAGE_DOWN,

	SCROLL_HOME,
	SCROLL_END,

	SORT_ORDER,
	SORT_KEY,

	HELP,

	TIMEOUT,
} key_event_t;

#define KEY_ENTER      13
#define KEY_ESC        27

/* Arrow key scan code has ESC, 91, key, enter */
#define KEY_K          107
#define KEY_J          106

/* home and end key scan code contains ESC, 79, key, and enter */
#define KEY_G          103
#define KEY_CAP_G      71

#define KEY_O          111 /* 'o' to reverse sort order */
#define KEY_S          115 /* 's' to change sort key    */

#define KEY_H          104 /* 'h' key ascii code         */
#define KEY_QUST       63  /* '?' key ascii code         */

int be_menu_select(be_console_t *console, boot_conf_t *conf, boot_env_t **bep,
		int *sort_order, int *sort_key);
int be_console_init(be_console_t *console, int srow, int scol, int nrows,
		int ncols, int timeout);

#endif
