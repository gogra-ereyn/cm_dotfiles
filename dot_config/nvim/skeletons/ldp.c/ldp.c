#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define LDAP_DEPRECATED 1
#include <ldap.h>

#define MAX_USERS 4096
#define MAX_GROUPS 8192
#define MAX_NAME_LEN 64
#define FILTER_BUF 512
#define PW_BUFSZ 32
#define BITSET_WORDS ((MAX_USERS + 63) / 64)

typedef struct {
	char name[MAX_NAME_LEN];
} user_t;

typedef struct {
	char name[MAX_NAME_LEN];
	uint64_t members[BITSET_WORDS];
} group_t;

typedef struct {
	const char *name; /* points into user_t.name or group_t.name */
	uint16_t idx;
} name_index_t;

typedef struct {
	user_t users[MAX_USERS];
	group_t groups[MAX_GROUPS];
	name_index_t user_index[MAX_USERS];
	name_index_t group_index[MAX_GROUPS];
	int user_count;
	int group_count;
} ldap_cache_t;

static int cmp_name_index(const void *a, const void *b)
{
	return strcmp(((const name_index_t *)a)->name, ((const name_index_t *)b)->name);
}

static inline void cache_group_add_member(ldap_cache_t *cache, int gi, int ui)
{
	cache->groups[gi].members[ui / 64] |= (uint64_t)1 << (ui % 64);
}

static inline int cache_user_in_group(const ldap_cache_t *cache, int gi, int ui)
{
	return (cache->groups[gi].members[ui / 64] >> (ui % 64)) & 1;
}

static int cache_insert_user(ldap_cache_t *cache, const char *name)
{
	int idx;

	if (cache->user_count >= MAX_USERS)
		return -1;

	idx = cache->user_count++;
	strncpy(cache->users[idx].name, name, MAX_NAME_LEN - 1);
	cache->users[idx].name[MAX_NAME_LEN - 1] = '\0';
	cache->user_index[idx].name = cache->users[idx].name;
	cache->user_index[idx].idx = (uint16_t)idx;

	return idx;
}

static int cache_find_or_insert_group(ldap_cache_t *cache, const char *name)
{
	int i;
	int idx;

	/* linear scan during population — index not yet sorted */
	for (i = 0; i < cache->group_count; i++) {
		if (strcmp(cache->groups[i].name, name) == 0)
			return i;
	}

	if (cache->group_count >= MAX_GROUPS)
		return -1;

	idx = cache->group_count++;
	strncpy(cache->groups[idx].name, name, MAX_NAME_LEN - 1);
	cache->groups[idx].name[MAX_NAME_LEN - 1] = '\0';
	cache->group_index[idx].name = cache->groups[idx].name;
	cache->group_index[idx].idx = (uint16_t)idx;

	return idx;
}

static int cache_lookup_group(const ldap_cache_t *cache, const char *name)
{
	name_index_t key;
	name_index_t *found;

	key.name = name;
	found = bsearch(&key, cache->group_index, (size_t)cache->group_count, sizeof(name_index_t),
			cmp_name_index);
	return found ? (int)found->idx : -1;
}

static int cache_lookup_user(const ldap_cache_t *cache, const char *name)
{
	name_index_t key;
	name_index_t *found;

	key.name = name;
	found = bsearch(&key, cache->user_index, (size_t)cache->user_count, sizeof(name_index_t),
			cmp_name_index);
	return found ? (int)found->idx : -1;
}

static void cache_list_group_members(const ldap_cache_t *cache, int gi)
{
	int word, bit, ui;
	uint64_t w;

	for (word = 0; word < BITSET_WORDS; word++) {
		w = cache->groups[gi].members[word];
		while (w) {
			bit = __builtin_ctzll(w);
			ui = word * 64 + bit;
			printf("%s\n", cache->users[ui].name);
			w &= w - 1;
		}
	}
}

static int read_users_stdin(char usernames[][MAX_NAME_LEN], int max_users)
{
	char c;
	int count = 0;
	int len = 0;
	ssize_t n;

	while ((n = read(STDIN_FILENO, &c, 1)) == 1) {
		if (c == '\n' || c == '\r') {
			if (len > 0) {
				usernames[count][len] = '\0';
				if (++count >= max_users) {
					fprintf(stderr, "input truncated at %d users\n", max_users);
					break;
				}
				len = 0;
			}
		} else if (len < MAX_NAME_LEN - 1) {
			usernames[count][len++] = c;
		}
	}

	/* flush final line if no trailing newline */
	if (len > 0 && count < max_users) {
		usernames[count][len] = '\0';
		count++;
	}

	return count;
}

static int cache_load_users_stdin(ldap_cache_t *cache)
{
	static char usernames[MAX_USERS][MAX_NAME_LEN];
	int count;
	int i;

	count = read_users_stdin(usernames, MAX_USERS);

	for (i = 0; i < count; i++) {
		if (cache_insert_user(cache, usernames[i]) < 0) {
			fprintf(stderr, "user limit reached at %d\n", cache->user_count);
			break;
		}
	}

	fprintf(stderr, "loaded %d users from stdin\n", cache->user_count);
	return 0;
}

static int read_pw_file(const char *path, char *buf, size_t bufsz)
{
	int fd;
	ssize_t n;
	size_t len;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror(path);
		return -1;
	}

	n = read(fd, buf, bufsz - 1);
	close(fd);

	if (n <= 0)
		return -1;

	buf[n] = '\0';

	len = (size_t)n;
	if (len > 0 && buf[len - 1] == '\n')
		buf[len - 1] = '\0';

	return 0;
}

static const char *extract_cn(const char *dn, char *buf, size_t bufsz)
{
	const char *p;
	const char *end;
	size_t len;

	if (strncasecmp(dn, "CN=", 3) != 0)
		return 0;

	p = dn + 3;
	end = strchr(p, ',');
	len = end ? (size_t)(end - p) : strlen(p);

	if (len == 0 || len >= bufsz)
		return 0;

	memcpy(buf, p, len);
	buf[len] = '\0';
	return buf;
}

static int cache_populate_user(ldap_cache_t *cache, LDAP *ld, const char *base, int user_idx)
{
	char filter[FILTER_BUF];
	char cn_buf[MAX_NAME_LEN];
	char *attrs[] = { "memberOf", 0 };
	LDAPMessage *result = 0;
	LDAPMessage *entry = 0;
	BerElement *ber = 0;
	char *attr = 0;
	char **vals = 0;
	int rc;
	int i;
	int gi;

	snprintf(filter, sizeof(filter), "(&(objectClass=person)(sAMAccountName=%s))",
		 cache->users[user_idx].name);

	rc = ldap_search_ext_s(ld, base, LDAP_SCOPE_SUBTREE, filter, attrs, 0, 0, 0, 0,
			       LDAP_NO_LIMIT, &result);
	if (rc != LDAP_SUCCESS) {
		fprintf(stderr, "user %s: %s\n", cache->users[user_idx].name, ldap_err2string(rc));
		return -1;
	}

	entry = ldap_first_entry(ld, result);
	if (!entry) {
		/* not found in LDAP — inactive/deleted, skip silently */
		ldap_msgfree(result);
		return 0;
	}

	for (attr = ldap_first_attribute(ld, entry, &ber); attr;
	     attr = ldap_next_attribute(ld, entry, ber)) {
		if (strcmp(attr, "memberOf") != 0) {
			ldap_memfree(attr);
			continue;
		}

		vals = (char **)ldap_get_values(ld, entry, attr);
		if (vals) {
			for (i = 0; vals[i]; i++) {
				if (!extract_cn(vals[i], cn_buf, sizeof(cn_buf)))
					continue;
				gi = cache_find_or_insert_group(cache, cn_buf);
				if (gi >= 0)
					cache_group_add_member(cache, gi, user_idx);
			}
			ldap_value_free(vals);
		}
		ldap_memfree(attr);
	}

	if (ber)
		ber_free(ber, 0);
	ldap_msgfree(result);
	return 0;
}

static int cache_populate(ldap_cache_t *cache, LDAP *ld, const char *base)
{
	int i;
	int failed = 0;

	for (i = 0; i < cache->user_count; i++) {
		if (cache_populate_user(cache, ld, base, i) != 0)
			failed++;

		if ((i + 1) % 100 == 0)
			fprintf(stderr, "progress: %d/%d users, %d groups so far\n", i + 1,
				cache->user_count, cache->group_count);
	}

	/* sort both indexes once, after all data is known */
	qsort(cache->user_index, (size_t)cache->user_count, sizeof(name_index_t), cmp_name_index);
	qsort(cache->group_index, (size_t)cache->group_count, sizeof(name_index_t), cmp_name_index);

	fprintf(stderr, "cache built: %d users, %d groups, %d failed\n", cache->user_count,
		cache->group_count, failed);
	return 0;
}

static LDAP *cuteldap_connect(const char *uri, const char *bind_dn, const char *pw)
{
	LDAP *ld;
	struct berval cred;
	int rc;
	int version = LDAP_VERSION3;

	rc = ldap_initialize(&ld, uri);
	if (rc != LDAP_SUCCESS) {
		fprintf(stderr, "ldap_initialize(%s): %s\n", uri, ldap_err2string(rc));
		return 0;
	}

	ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
	ldap_set_option(ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);

	cred.bv_val = (char *)pw;
	cred.bv_len = strlen(pw);

	rc = ldap_sasl_bind_s(ld, bind_dn, LDAP_SASL_SIMPLE, &cred, 0, 0, 0);
	if (rc != LDAP_SUCCESS) {
		fprintf(stderr, "ldap_sasl_bind_s: %s\n", ldap_err2string(rc));
		ldap_unbind_ext_s(ld, 0, 0);
		return 0;
	}

	fprintf(stderr, "ldap connected: %s\n", uri);
	return ld;
}

int main(void)
{
	static ldap_cache_t cache;

	char pw[PW_BUFSZ];
	LDAP *ld;
	int gi;

	const char *uri = getenv("LDAP_URI");
	const char *base = getenv("LDAP_BASE");
	const char *bind_dn = getenv("LDAP_BIND_DN");
	const char *pw_file = getenv("LDAP_PW_FILE");

	if (!uri || !base || !bind_dn || !pw_file) {
		fprintf(stderr, "required env: LDAP_URI, LDAP_BASE, "
				"LDAP_BIND_DN, LDAP_PW_FILE\n");
		return 1;
	}

	if (read_pw_file(pw_file, pw, sizeof(pw)) != 0)
		return 1;

	memset(&cache, 0, sizeof(cache));

	cache_load_users_stdin(&cache);

	if (cache.user_count == 0) {
		fprintf(stderr, "no users loaded, exiting\n");
		return 1;
	}

	ld = cuteldap_connect(uri, bind_dn, pw);
	memset(pw, 0, sizeof(pw));
	if (!ld)
		return 1;

	cache_populate(&cache, ld, base);

	ldap_unbind_ext_s(ld, 0, 0);

	{
		int i;
		for (i = 0; i < cache.group_count; i++) {
			printf("group: %s\n", cache.groups[i].name);
			cache_list_group_members(&cache, i);
			printf("\n");
		}
	}

	gi = cache_lookup_group(&cache, "some-group");
	if (gi >= 0) {
		printf("members of some-group:\n");
		cache_list_group_members(&cache, gi);
	}

	return 0;
}
