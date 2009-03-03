/*****************************************************************************
 *
 *    Copyright (C) 2009 Codefidence Ltd www.codefidence.com
 *
 *    This file is a part of Distore
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

#ifndef DISTORE_CALLBACKS_H
#define DISTORE_CALLBACKS_H

void evReadAnnounce(int fd, short event, void *arg);

void evSendAnnounce(int fd, short event, void *arg);

void evRunUpdateIfNeeded(int fd, short event, void *arg);

#endif
