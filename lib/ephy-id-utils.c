#include "config.h"

#include "ephy-id-utils.h"

#include <errno.h>
#include "glib.h"

#if defined(__linux__)
#include <sys/random.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#include <uninstd.h>
#endif

static void
generate_random_bytes (gsize   num_bytes,
                       guint8  *out)
{
  gssize ret;

  g_assert (num_bytes > 0);
  g_assert (out);

#ifdef __OpenBSD__
  if (getentropy (out, num_bytes) == -1)
    g_error ("Failed to get entropy: %s", g_strerror (errno));
#else
  do {
    ret = getrandom (out, num_bytes, 0);
  } while (ret < (gssize)num_bytes && errno == EINTR);
#endif
}

char *
ephy_id_utils_get_random_id (void)
{
  guint8 bytes[9];
  char *base64;
  char *id;

  /* 9 bytes produce exactly 12 Base64 characters. */
  generate_random_bytes (sizeof bytes, bytes);
  base64 = g_base64_encode (bytes, sizeof bytes);

  for (gsize i = 0; base64[i]; i++) {
    if (base64[i] == '+')
      base64[i] = '-';
    else if (base64[i] == '/')
      base64[i] = '_';
  }

  id = g_strdup (base64);
  g_free (base64);

  return id;
}
