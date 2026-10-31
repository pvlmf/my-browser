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
#include "ephy-password-record.h"

struct _EphyPasswordRecord {
  GObject parent_instance;

  char *id;
  char *origin;
  char *target_origin;
  char *username;
  char *password;
  char *username_field;
  char *password_field;
  guint64 time_password_changed;
};

G_DEFINE_FINAL_TYPE (EphyPasswordRecord, ephy_password_record, G_TYPE_OBJECT)

typedef enum {
  PROP_ID = 1,                    /* Firefox Sync */
  PROP_ORIGIN,                /* Epiphany && Firefox Sync */
  PROP_TARGET_ORIGIN,         /* Epiphany && Firefox Sync */
  PROP_USERNAME,              /* Epiphany && Firefox Sync */
  PROP_PASSWORD,              /* Epiphany && Firefox Sync */
  PROP_USERNAME_FIELD,        /* Epiphany && Firefox Sync */
  PROP_PASSWORD_FIELD,        /* Epiphany && Firefox Sync */
  PROP_TIME_PASSWORD_CHANGED,
} EphyPasswordRecordProps;

static GParamSpec *obj_properties[PROP_TIME_PASSWORD_CHANGED + 1];

static void
ephy_password_record_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  EphyPasswordRecord *self = EPHY_PASSWORD_RECORD (object);

  switch ((EphyPasswordRecordProps)prop_id) {
    case PROP_ID:
      g_free (self->id);
      self->id = g_value_dup_string (value);
      break;
    case PROP_ORIGIN:
      g_free (self->origin);
      self->origin = g_value_dup_string (value);
      break;
    case PROP_TARGET_ORIGIN:
      g_free (self->target_origin);
      self->target_origin = g_value_dup_string (value);
      break;
    case PROP_USERNAME:
      g_free (self->username);
      self->username = g_value_dup_string (value);
      break;
    case PROP_PASSWORD:
      g_free (self->password);
      self->password = g_value_dup_string (value);
      break;
    case PROP_USERNAME_FIELD:
      g_free (self->username_field);
      self->username_field = g_value_dup_string (value);
      break;
    case PROP_PASSWORD_FIELD:
      g_free (self->password_field);
      self->password_field = g_value_dup_string (value);
      break;
    case PROP_TIME_PASSWORD_CHANGED:
      self->time_password_changed = g_value_get_uint64 (value);
      break;
  }
}

static void
ephy_password_record_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  EphyPasswordRecord *self = EPHY_PASSWORD_RECORD (object);

  switch ((EphyPasswordRecordProps)prop_id) {
    case PROP_ID:
      g_value_set_string (value, self->id);
      break;
    case PROP_ORIGIN:
      g_value_set_string (value, self->origin);
      break;
    case PROP_TARGET_ORIGIN:
      g_value_set_string (value, self->target_origin);
      break;
    case PROP_USERNAME:
      g_value_set_string (value, self->username);
      break;
    case PROP_PASSWORD:
      g_value_set_string (value, self->password);
      break;
    case PROP_USERNAME_FIELD:
      g_value_set_string (value, self->username_field);
      break;
    case PROP_PASSWORD_FIELD:
      g_value_set_string (value, self->password_field);
      break;
    case PROP_TIME_PASSWORD_CHANGED:
      g_value_set_uint64 (value, self->time_password_changed);
      break;
  }
}

static void
ephy_password_record_finalize (GObject *object)
{
  EphyPasswordRecord *self = EPHY_PASSWORD_RECORD (object);

  g_free (self->id);
  g_free (self->origin);
  g_free (self->target_origin);
  g_free (self->username);
  g_free (self->password);
  g_free (self->username_field);
  g_free (self->password_field);

  G_OBJECT_CLASS (ephy_password_record_parent_class)->finalize (object);
}

static void
ephy_password_record_class_init (EphyPasswordRecordClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = ephy_password_record_set_property;
  object_class->get_property = ephy_password_record_get_property;
  object_class->finalize = ephy_password_record_finalize;

  /* The property names must match Firefox password object structure, see
   * https://mozilla-services.readthedocs.io/en/latest/sync/objectformats.html#passwords
   */
  obj_properties[PROP_ID] =
    g_param_spec_string ("id",
                         NULL, NULL,
                         "Default id",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  /* Origin matches hostname field from Firefox.
   * Despite its name, it's actually a security origin (scheme + host + port), so call it appropriately, see
   * https://dxr.mozilla.org/mozilla-central/rev/892c8916ba32b7733e06bfbfdd4083ffae3ca028/toolkit/components/passwordmgr/LoginManagerContent.jsm#922
   * https://dxr.mozilla.org/mozilla-central/rev/892c8916ba32b7733e06bfbfdd4083ffae3ca028/toolkit/components/passwordmgr/LoginManagerContent.jsm#1380
   */
  obj_properties[PROP_ORIGIN] =
    g_param_spec_string ("hostname",
                         NULL, NULL,
                         "Default security origin",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  /* Target origin matches formSubmitURL field from Firefox.
   * Despite its name, it's actually a security origin, so call it appropriately, see
   * https://dxr.mozilla.org/mozilla-central/rev/892c8916ba32b7733e06bfbfdd4083ffae3ca028/toolkit/components/passwordmgr/LoginManagerContent.jsm#928
   */
  obj_properties[PROP_TARGET_ORIGIN] =
    g_param_spec_string ("formSubmitURL",
                         NULL, NULL,
                         "Default target origin",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_USERNAME] =
    g_param_spec_string ("username",
                         NULL, NULL,
                         "Default username",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_PASSWORD] =
    g_param_spec_string ("password",
                         NULL, NULL,
                         "Default password",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_USERNAME_FIELD] =
    g_param_spec_string ("usernameField",
                         NULL, NULL,
                         "Default username field",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_PASSWORD_FIELD] =
    g_param_spec_string ("passwordField",
                         NULL, NULL,
                         "Default password field",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_TIME_PASSWORD_CHANGED] =
    g_param_spec_uint64 ("timePasswordChanged",
                         NULL, NULL,
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_properties), obj_properties);
}

static void
ephy_password_record_init (EphyPasswordRecord *self)
{
}

EphyPasswordRecord *
ephy_password_record_new (const char *id,
                          const char *origin,
                          const char *target_origin,
                          const char *username,
                          const char *password,
                          const char *username_field,
                          const char *password_field,
                          guint64     time_password_changed)
{
  return EPHY_PASSWORD_RECORD (g_object_new (EPHY_TYPE_PASSWORD_RECORD,
                                             "id", id,
                                             "hostname", origin,
                                             "formSubmitURL", target_origin,
                                             "username", username,
                                             "password", password,
                                             "usernameField", username_field,
                                             "passwordField", password_field,
                                             "timePasswordChanged", time_password_changed,
                                             NULL));
}

EphyPasswordRecord *
ephy_password_record_copy (EphyPasswordRecord *self)
{
  EphyPasswordRecord *record = ephy_password_record_new (self->id,
                                                         self->origin,
                                                         self->target_origin,
                                                         self->username,
                                                         self->password,
                                                         self->username_field,
                                                         self->password_field,
                                                         self->time_password_changed);
  return record;
}

const char *
ephy_password_record_get_id (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->id;
}

const char *
ephy_password_record_get_origin (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->origin;
}

const char *
ephy_password_record_get_target_origin (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->target_origin;
}

const char *
ephy_password_record_get_username (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->username;
}

const char *
ephy_password_record_get_password (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->password;
}

void
ephy_password_record_set_password (EphyPasswordRecord *self,
                                   const char         *password)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  g_set_str (&self->password, password);
}

const char *
ephy_password_record_get_username_field (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->username_field;
}

const char *
ephy_password_record_get_password_field (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->password_field;
}

guint64
ephy_password_record_get_time_password_changed (EphyPasswordRecord *self)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  return self->time_password_changed;
}

void
ephy_password_record_set_username (EphyPasswordRecord *self,
                                   const char         *username)
{
  g_assert (EPHY_IS_PASSWORD_RECORD (self));

  g_set_str (&self->username, username);
}
