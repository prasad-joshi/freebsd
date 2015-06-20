#ifndef __BOOTENV_H__
#define __BOOTENV_H__

#include <stdint.h>
#include <sys/queue.h>

typedef enum {
	SORT_OBJNUM,
	SORT_TIMESTAMP,
	SORT_NAME,
} sort_type_t;

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
	sort_type_t            sort;
	uint32_t               count;
} boot_conf_t;

int bootenv_init(boot_conf_t *conf, sort_type_t sort);
int bootenv_new(const char *name, uint64_t objnum, uint64_t timestamp,
	int active, boot_env_t **be);
int bootenv_add(boot_conf_t *conf, boot_env_t *be);
void bootenv_print(boot_conf_t *conf);
int bootenv_search_objnum(boot_conf_t *conf, uint64_t objnum, boot_env_t **bepp);
int bootenv_search_name(boot_conf_t *conf, char *name, boot_env_t *bepp);
int bootenv_search_timestamp(boot_conf_t *conf, uint64_t timestamp, boot_env_t *bepp);
#endif
