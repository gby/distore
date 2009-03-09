/*****************************************************************************
 *
 *    Copyright (C) 2009 Codefidence Ltd www.codefidence.com
 *
 *    This file is a part of Distore.
 *
 *    Distore is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Distore is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Distore.  If not, see <http://www.gnu.org/licenses/>.
 * 
 ****************************************************************************/

#ifndef DISTORE_UPDATE_H
#define DISTORE_UPDATE_H

#include <ght_hash_table.h>

void runUpdateIfNeeded();

/* Tries to read current versions according to info in our config file.
 * If particular version can not be determined - its set to 0
 * Hash keys are versions' ids and hash values are respective version numbers.
 */
ght_hash_table_t * getCurrentVersions();

void runUpdateIfNeeded();
void doBootUpdate();
void doRulesUpdate();

#endif
