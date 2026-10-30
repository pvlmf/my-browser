/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 *  Copyright © 2017 Gabriel Ivascu <gabrielivascu@gnome.org>
 *
 *  This file is part of Epiphany.
 *
 *  Epiphany is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Epiphany is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Epiphany.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

char     *ephy_sync_utils_encode_hex                    (const guint8 *data,
                                                         gsize         data_len);
guint8   *ephy_sync_utils_decode_hex                    (const char   *hex);

char     *ephy_sync_utils_base64_urlsafe_encode         (const guint8 *data,
                                                         gsize         data_len,
                                                         gboolean      should_strip);
guint8   *ephy_sync_utils_base64_urlsafe_decode         (const char *text,
                                                         gsize      *out_len,
                                                         gboolean    should_fill);

void      ephy_sync_utils_generate_random_bytes         (void   *random_ctx,
                                                         gsize   num_bytes,
                                                         guint8 *out);
char     *ephy_sync_utils_get_random_sync_id            (void);

gboolean  ephy_sync_utils_bookmarks_sync_is_enabled     (void);
void      ephy_sync_utils_set_bookmarks_sync_time       (gint64 time);
gint64    ephy_sync_utils_get_bookmarks_sync_time       (void);
void      ephy_sync_utils_set_bookmarks_sync_is_initial (gboolean is_initial);
gboolean  ephy_sync_utils_get_bookmarks_sync_is_initial (void);

gboolean  ephy_sync_utils_passwords_sync_is_enabled     (void);
void      ephy_sync_utils_set_passwords_sync_time       (gint64 time);
gint64    ephy_sync_utils_get_passwords_sync_time       (void);
void      ephy_sync_utils_set_passwords_sync_is_initial (gboolean is_initial);
gboolean  ephy_sync_utils_get_passwords_sync_is_initial (void);

G_END_DECLS
