#ifndef __BOOTENV_H__
#define __BOOTENV_H__

#include <stdint.h>
#include <sys/queue.h>

#include "libzfs.h"

typedef enum {
	SORT_OBJNUM,
	SORT_TIMESTAMP,
	SORT_NAME,
} sort_key_t;

typedef enum {
	SORT_ASCENDING,
	SORT_DESCENDING,
} sort_order_t;

typedef struct boot_env {
	TAILQ_ENTRY(boot_env) be_list;
	char                  name[ZFS_MAXNAMELEN];
	uint64_t              objnum;
	uint64_t              timestamp;
	uint32_t              id;
	int                   active;
} boot_env_t;

typedef struct boot_conf {
	TAILQ_HEAD(, boot_env) head;
	sort_key_t             key;
	sort_order_t           order;
	uint32_t               count;
	void                   *spa;
	char                   *root;
	uint64_t               be_active;
} boot_conf_t;

int bootenv_init(boot_conf_t *conf, sort_key_t key, sort_order_t order);
int bootenv_new(const char *name, uint64_t objnum, uint64_t timestamp,
		int active, boot_env_t **bepp);
int bootenv_add(boot_conf_t *conf, boot_env_t *be);
void bootenv_print(boot_conf_t *conf);
void bootenv_string(boot_env_t *be, char *str, uint32_t size);

#define BOOTENV_FOREACH(conf, b) \
	TAILQ_FOREACH(b, &conf->head, be_list)

#endif
