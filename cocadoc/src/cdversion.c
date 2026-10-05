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

/**
 * @file cdversion.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include "cdversion.h"


static const semver VERSION = {
	.major = 0,
	.minor = 2,
	.patch = 0,
	.pre_rel = NULL,
	.build = NULL
};


const semver *cocadoc_version()
{
	return &VERSION;
}


const char *cocadoc_version_str()
{
	static char str[64] = "";
	if (str[0] == '\0') {
		semver_to_str(&VERSION, str);
	}
	return str;
}
