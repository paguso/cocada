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

#ifndef CDVERSION_H
#define CDVERSION_H

#include "semver.h"

/**
 * @file cdversion.h
 * @author Paulo Fonseca
 * @brief The version of cocadoc.
 */


/**
 * @brief Returns the version of cocadoc.
 * @return The version (static).
 */
const semver *cocadoc_version();


/**
 * @brief Returns the version of cocadoc as a string, e.g. "0.1.0".
 * @return The version (static string).
 */
const char *cocadoc_version_str();

#endif
