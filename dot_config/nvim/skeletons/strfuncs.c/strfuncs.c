#include <string.h>
#include <ctype.h>

static inline int is_delim(char c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == ',' ||
	       c == ';';
}


// c strings

static inline char toggle_case(char val)
{
	return val ^ 32;
}
static inline char to_upper(char val)
{
	return val & ~32;
}
static inline char to_lower(char val)
{
	return val | 32;
}

static inline char is_alpha(char val)
{
	return ((val | 0x20) - 'a') < 26;
}


static inline int is_alphanum(char c)
{
	return ((c | 32) >= 'a' && (c | 32) <= 'z') || ((c >= '0' && c <= '9'));
}



void ltrim(char **p)
{
	char *s = *p;

	if (!strlen(s))
		return;

	while (isspace((int)*s))
		s++;

	*p = s;
}

void rtrim(char *p)
{
	char *start = p, *s;

	if (!strlen(p))
		return;

	s = strchr(p, ';');
	if (s)
		*s = '\0';
	s = strchr(p, '#');
	if (s)
		*s = '\0';
	if (s)
		p = s;

	s = p + strlen(p);
	while ((isspace((int)*s) || iscntrl((int)*s)) && (s > start))
		s--;

	*(s + 1) = '\0';
}



