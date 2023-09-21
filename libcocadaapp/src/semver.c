/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cstrutil.h"
#include "new.h"
#include "semver.h"

#define is_letter( chr )     ( ( 'A' <= chr && chr <= 'z' ) )

#define is_pos_digit( chr )   ( ( '1' <= chr && chr <= '9' ) )

#define is_digit( chr )      ( ( '0' <= chr && chr <= '9' ) )

#define is_non_digit( chr )   ( (chr == '-') || is_letter(chr) )

#define is_id_char( chr )     ( is_digit(chr) || is_non_digit(chr) )

static  bool is_digits(const char *str, size_t len )
{
	if (len==0) return false;
	for (char *c = (char *)str; c != str + len; c++) {
		if ( !is_digit(*c) ) return false;
	}
	return true;
}

/*
static bool is_id_chars(const char *str, size_t len ) {
    if (len==0) return false;
    for (char *c = str; c != str + len; c++) {
        if ( !is_id_char(*c) ) return false;
    }
    return true;
}
*/

bool is_num_id(const char *str, const size_t len)
{
	if (len == 0) return false;
	if (len == 1 && *str == '0') return true;
	if (len == 1 && is_pos_digit(*str)) return true;
	char *c = (char *)str;
	if ( ! is_pos_digit(*c) ) return false;
	c++;
	return is_digits(c, len - 1);
}

bool is_alphanum_id(const char *str, const size_t len)
{
	if (len == 0) return false;
	bool hasnondigit = false;
	for (char *c = (char *)str; c != str + len; c++) {
		if ( !is_id_char(*c) ) return false;
		hasnondigit = ( hasnondigit || is_non_digit(*c) );
	}
	return hasnondigit;
}

static bool is_pre_rel_id(const char *str, const size_t len)
{
	return is_alphanum_id(str, len) || is_num_id(str, len);
}

static bool is_build_id(const char *str, const size_t len)
{
	return is_alphanum_id(str, len) || is_digits(str, len);
}


semver_res semver_new_from_str(const char *src)
{
	char *start = (char *)src;
	size_t len = strlen(src);
	char *stop;
	int major, minor, patch;
	char *pre_rel = NULL, *build = NULL;
	bool parse_err = false;
	errno = 0;
	major = (int) strtol(start, &stop, 10);
	if (errno || stop == NULL || *stop != '.' || !is_num_id(start, (stop-start))) {
		parse_err = true;
		goto cleanup;
	}
	start = stop + 1;
	errno = 0;
	minor = (int) strtol(start, &stop, 10);
	if (errno || stop == NULL || *stop != '.' || !is_num_id(start, (stop-start))) {
		parse_err = true;
		goto cleanup;
	}
	start = stop + 1;
	patch = (int) strtol(start, &stop, 10);
	if (errno || stop == NULL || !is_num_id(start, (stop-start))) {
		parse_err = true;
		goto cleanup;
	}
	start = stop;
	if (start[0] == '-') { // has pre-release
		start++;
		stop = strpbrk(start, "+");
		stop = (stop != NULL) ? stop : ((char *)src + len); // has no build
		pre_rel = cstr_clone_len(start, (stop - start) / sizeof(char));
		char *dot = strpbrk(start, ".");
		dot = (dot != NULL) ? dot : stop;
		while ( dot <= stop) {
			size_t n = (dot - start) / sizeof(char);
			if (!is_pre_rel_id(start, n)) {
				parse_err = true;
				goto cleanup;
			}
			if (dot == stop) break;
			start = dot + 1;
			dot = strpbrk(start, ".");
			dot = (dot != NULL) ? dot : stop;
		}
		start = stop;
	}
	if (start[0] == '+') { // has build but no pre-release
		start++;
		stop = ((char *)src + len);
		build = cstr_clone_len(start, (stop - start) / sizeof(char));
		char *dot = strpbrk(start, ".");
		dot = (dot != NULL) ? dot : stop;
		while ( dot <= stop) {
			size_t n = (dot - start) / sizeof(char);
			if (!is_build_id(start, n)) {
				parse_err = true;
				goto cleanup;
			}
			if (dot == stop) break;
			start = dot + 1;
			dot = strpbrk(start, ".");
			dot = (dot != NULL) ? dot : stop;
		}
	}
cleanup:
	if ( parse_err ) {
		FREE(build);
		FREE(pre_rel);
		return (semver_res) {
			.ok = false, .val = NULL
		};
	}
	semver *ret = NEW(semver);
	ret->major = major;
	ret->minor = minor;
	ret->patch = patch;
	ret->pre_rel = pre_rel;
	ret->build = build;
	return (semver_res) {
		.ok = true, .val = ret
	};
}


void semver_to_str(const semver *src, char *dest)
{
	sprintf(dest, "%d.%d.%d%s%s%s%s",
	        src->major, src->minor, src->patch,
	        ((src->pre_rel) ? "-" : ""),
	        ((src->pre_rel) ? src->pre_rel : ""),
	        ((src->build) ? "+" : ""),
	        ((src->build) ? src->build : "")
	       );
}


void semver_free(semver *sver)
{
	FREE(sver->build);
	FREE(sver->pre_rel);
	FREE(sver);
}