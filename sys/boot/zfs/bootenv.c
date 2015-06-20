#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/queue.h>
#include "bootenv.h"

typedef int (*becmp_t) (boot_env_t *be1, boot_env_t *be2);

int
bootenv_init(boot_conf_t *conf, sort_type_t sort)
{
	TAILQ_INIT(&conf->head);
	conf->sort  = sort;
	conf->count = 0;
	return 0;
}

int
bootenv_new(const char *name, uint64_t objnum, uint64_t timestamp, int active,
		boot_env_t **bepp)
{
	boot_env_t *be;

	be = malloc(sizeof(*be));

	strncpy(be->name, name, sizeof(be->name));
	be->objnum    = objnum;
	be->timestamp = timestamp;
	be->active    = active;
	be->id        = 0;
	*bepp         = be;

	return (0);
}

static int
cmp_objnum(boot_env_t *be1, boot_env_t *be2)
{
	if (be1->objnum < be2->objnum) {
		return (-1);
	} else if (be1->objnum > be2->objnum) {
		return (1);
	}

	return (0);
}

static int
cmp_timestamp(boot_env_t *be1, boot_env_t *be2)
{
	if (be1->timestamp < be2->timestamp) {
		return (-1);
	} else if (be1->timestamp > be2->timestamp) {
		return (1);
	}

	return (0);
}

static int
cmp_name(boot_env_t *be1, boot_env_t *be2)
{
	return (strcmp(be1->name, be2->name));
}

static int
bootenv_insert(boot_conf_t *conf, boot_env_t *be, becmp_t func)
{
	boot_env_t *b;
	boot_env_t *tmp;
	int        rc;
	int        inserted;

	inserted = 0;
	TAILQ_FOREACH_SAFE(b, &conf->head, be_list, tmp) {
		rc = func(b, be);
		if (rc > 0) {
			TAILQ_INSERT_BEFORE(b, be, be_list);
			inserted = 1;
			break;
		}
	}

	if (inserted == 0) {
		TAILQ_INSERT_TAIL(&conf->head, be, be_list);
	}

	return (0);
}

int
bootenv_add(boot_conf_t *conf, boot_env_t *be)
{
	becmp_t cmp_fn;
	int     rc;

	switch (conf->sort) {
	default:
	case SORT_TIMESTAMP:
		cmp_fn = cmp_timestamp;
		break;
	case SORT_OBJNUM:
		cmp_fn = cmp_objnum;
		break;
	case SORT_NAME:
		cmp_fn = cmp_name;
		break;
	}

	rc = bootenv_insert(conf, be, cmp_fn);
	if (rc < 0) {
		return (rc);
	}

	be->id = ++conf->count;
	return (rc);
}

static void
be_print(boot_env_t *be)
{
	char active;

	active = ' ';
	if (be->active) {
		active = '*';
	}
	printf("%s (%"PRIu64") (%"PRIu64") %c\n", be->name, be->objnum, be->timestamp, active);
}

void
bootenv_print(boot_conf_t *conf)
{
	boot_env_t *b;

	TAILQ_FOREACH(b, &conf->head, be_list) {
		be_print(b);
	}
}

static int
bootenv_search(boot_conf_t *conf, boot_env_t *cmp_be, sort_type_t type,
		boot_env_t **bepp)
{
	becmp_t    cmp_fn;
	boot_env_t *b;
	int        rc;

	switch (conf->sort) {
	default:
	case SORT_TIMESTAMP:
		cmp_fn = cmp_timestamp;
		break;
	case SORT_OBJNUM:
		cmp_fn = cmp_objnum;
		break;
	case SORT_NAME:
		cmp_fn = cmp_name;
		break;
	}

	TAILQ_FOREACH(b, &conf->head, be_list) {
		rc = cmp_fn(b, cmp_be);
		if (rc == 0) {
			*bepp = b;
			return (0);
		}
	}

	*bepp = NULL;
	return (-1);
}

int
bootenv_search_objnum(boot_conf_t *conf, uint64_t objnum, boot_env_t **bepp)
{
	boot_env_t t;
	t.objnum = objnum;

	return (bootenv_search(conf, &t, SORT_OBJNUM, bepp));
}

int
bootenv_search_name(boot_conf_t *conf, char *name, boot_env_t *bepp)
{
	boot_env_t t;
	strncpy(t.name, name, sizeof(t.name));

	return (bootenv_search(conf, &t, SORT_NAME, bepp));
}

int
bootenv_search_timestamp(boot_conf_t *conf, uint64_t timestamp, boot_env_t *bepp)
{
	boot_env_t t;
	t.timestamp = timestamp;

	return (bootenv_search(conf, &t, SORT_TIMESTAMP, bepp));
}
