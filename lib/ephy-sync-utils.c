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

#include "config.h"

#include "ephy-sync-utils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <glib/gi18n.h>

#if defined(__linux__)
#include <sys/random.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#include <unistd.h>
#endif

static const char hex_digits[] = "0123456789abcdef";

char *
ephy_sync_utils_encode_hex (const guint8 *data,
                            gsize         data_len)
{
  char *encoded;

  g_assert (data);

  encoded = g_malloc (data_len * 2 + 1);
  for (gsize i = 0; i < data_len; i++) {
    guint8 byte = data[i];

    encoded[2 * i] = hex_digits[byte >> 4];
    encoded[2 * i + 1] = hex_digits[byte & 0xf];
  }
  encoded[data_len * 2] = 0;

  return encoded;
}

guint8 *
ephy_sync_utils_decode_hex (const char *hex)
{
  guint8 *decoded;

  g_assert (hex);

  decoded = g_malloc (strlen (hex) / 2);
  for (gsize i = 0, j = 0; i < strlen (hex); i += 2, j++)
    sscanf (hex + i, "%2hhx", decoded + j);

  return decoded;
}

static void
base64_to_base64_urlsafe (char *text)
{
  g_assert (text);

  /* / and + are inappropriate for URLs and file systems paths, so they have to
   * be omitted to make the base64 string safe. / is replaced with _ and + is
   * replaced with -.
   */
  g_strcanon (text, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=/", '-');
  g_strcanon (text, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=-", '_');
}

char *
ephy_sync_utils_base64_urlsafe_encode (const guint8 *data,
                                       gsize         data_len,
                                       gboolean      should_strip)
{
  char *base64;
  char *out;
  gsize start = 0;
  gssize end;

  g_assert (data);

  base64 = g_base64_encode (data, data_len);
  end = strlen (base64) - 1;

  /* Strip the data of any leading or trailing '=' characters. */
  if (should_strip) {
    while (start < strlen (base64) && base64[start] == '=')
      start++;

    while (end >= 0 && base64[end] == '=')
      end--;
  }

  out = g_strndup (base64 + start, end - start + 1);
  base64_to_base64_urlsafe (out);

  g_free (base64);

  return out;
}

static void
base64_urlsafe_to_base64 (char *text)
{
  g_assert (text);

  /* Replace '-' with '+' and '_' with '/' */
  g_strcanon (text, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=_", '+');
  g_strcanon (text, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=+", '/');
}

guint8 *
ephy_sync_utils_base64_urlsafe_decode (const char *text,
                                       gsize      *out_len,
                                       gboolean    should_fill)
{
  guint8 *out;
  char *to_decode;
  char *suffix = NULL;

  g_assert (text);
  g_assert (out_len);

  /* Fill the text with trailing '=' characters up to the proper length. */
  if (should_fill)
    suffix = g_strnfill ((4 - strlen (text) % 4) % 4, '=');

  to_decode = g_strconcat (text, suffix, NULL);
  base64_urlsafe_to_base64 (to_decode);
  out = g_base64_decode (to_decode, out_len);

  g_free (suffix);
  g_free (to_decode);

  return out;
}

/*
 * This is mainly required by Nettle's RSA support.
 * From Nettle's documentation: random_ctx and random is a randomness generator.
 * random(random_ctx, length, dst) should generate length random octets and store them at dst.
 * We don't use random_ctx.
 */
void
ephy_sync_utils_generate_random_bytes (void   *random_ctx,
                                       gsize   num_bytes,
                                       guint8 *out)
{
  gssize ret;

  g_assert (num_bytes > 0);
  g_assert (out);

#ifdef __OpenBSD__
  if (getentropy (out, num_bytes) == -1) {
    g_error ("Failed to get entropy: %s", g_strerror (errno));
  }
#else
  do {
    ret = getrandom (out, num_bytes, 0);
  } while (ret < (gssize)num_bytes && errno == EINTR);

  if (ret != (gssize)num_bytes)
    g_error ("Failed to generate randomness: %s", g_strerror (errno));
#endif
}

char *
ephy_sync_utils_get_random_sync_id (void)
{
  char *id;
  char *base64;
  guint8 *bytes;
  gsize bytes_len;

  /* The sync id is a base64-urlsafe string. Base64 uses 4 chars to represent 3 bytes,
   * therefore we need ceil(len * 3 / 4) bytes to cover the requested length. */
  bytes_len = (12 + 3) / 4 * 3;
  bytes = g_malloc (bytes_len);

  ephy_sync_utils_generate_random_bytes (NULL, bytes_len, bytes);
  base64 = ephy_sync_utils_base64_urlsafe_encode (bytes, bytes_len, FALSE);
  id = g_strndup (base64, 12);

  g_free (base64);
  g_free (bytes);

  return id;
}
