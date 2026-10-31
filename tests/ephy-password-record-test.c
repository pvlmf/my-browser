#include "config.h"

#include "ephy-password-record.h"

#include <glib-object.h>
#include <glib.h>

static void
test_password_record_properties (void)
{
  static const char *const properties[] = {
    "id",
    "origin",
    "target-origin",
    "username",
    "password",
    "username-field",
    "password-field",
    "time-password-changed"
  };
  static const char *const removed_properties[] = {
    "hostname",
    "formSubmitURL",
    "usernameField",
    "passwordField",
    "timePasswordChanged",
    "timeCreated"
  };

  EphyPasswordRecord *record;
  EphyPasswordRecord *copy;
  GObjectClass *klass;
  g_autofree char *origin = NULL;
  g_autofree char *target_origin = NULL;
  g_autofree char *username_field = NULL;
  g_autofree char *password_field = NULL;
  guint64 time_password_changed = 0;

  record = ephy_password_record_new ("test-id",
                                     "https://example.com",
                                     "https://example.com/login",
                                     "test-user",
                                     "test-password",
                                     "username",
                                     "password",
                                     1234567890);

  klass = G_OBJECT_GET_CLASS (record);

  for (guint i = 0; i < G_N_ELEMENTS (properties); i++)
    g_assert_nonnull (g_object_class_find_property (klass, properties[i]));
  for (guint i = 0; i < G_N_ELEMENTS (removed_properties); i++)
    g_assert_null (g_object_class_find_property (klass, removed_properties[i]));

  g_object_get (record,
                "origin", &origin,
                "target-origin", &target_origin,
                "username-field", &username_field,
                "password-field", &password_field,
                "time-password-changed", &time_password_changed,
                NULL);

  g_assert_cmpstr (origin, ==, "https://example.com");
  g_assert_cmpstr (target_origin, ==, "https://example.com/login");
  g_assert_cmpstr (username_field, ==, "username");
  g_assert_cmpstr (password_field, ==, "password");
  g_assert_true (time_password_changed == 1234567890);

  copy = ephy_password_record_copy (record);

  g_assert_cmpstr (ephy_password_record_get_id (copy), ==, "test-id");
  g_assert_cmpstr (ephy_password_record_get_password (copy), ==, "test-password");
  g_assert_cmpstr (ephy_password_record_get_target_origin (copy), ==, "https://example.com/login");
  g_assert_true (ephy_password_record_get_time_password_changed (copy) == 1234567890);

  g_object_unref (copy);
  g_object_unref (record);
}

int
main (int   argc,
      char  *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/passwords/record/properties",
                  test_password_record_properties);

  return g_test_run ();
}
