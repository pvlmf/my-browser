/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 *  Copyright © 2018 Purism SPC
 *  Copyright © 2018 Adrien Plazas <kekun.plazas@laposte.net>
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

#include "ephy-action-bar-end.h"

#include "ephy-bookmark-properties.h"
#include "ephy-bookmarks-dialog.h"
#include "ephy-desktop-utils.h"
#include "ephy-downloads-paintable.h"
#include "ephy-downloads-popover.h"
#include "ephy-embed-container.h"
#include "ephy-shell.h"
#include "ephy-window.h"

#define NEEDS_ATTENTION_ANIMATION_TIMEOUT 2000 /*ms */

struct _EphyActionBarEnd {
  GtkBox parent_instance;

  GtkWidget *bookmarks_button;
  GtkWidget *downloads_revealer;
  GtkWidget *downloads_button;
  GtkWidget *downloads_popover;
  GtkWidget *downloads_icon;
  GtkWidget *overview_button;

  GdkPaintable *downloads_paintable;

  guint downloads_button_attention_timeout_id;
};

G_DEFINE_FINAL_TYPE (EphyActionBarEnd, ephy_action_bar_end, GTK_TYPE_BOX)

static void
add_attention_timeout_cb (EphyActionBarEnd *self)
{
  gtk_widget_remove_css_class (self->downloads_icon, "accent");
  self->downloads_button_attention_timeout_id = 0;
}

static void
add_attention (EphyActionBarEnd *self)
{
  g_clear_handle_id (&self->downloads_button_attention_timeout_id, g_source_remove);

  gtk_widget_add_css_class (self->downloads_icon, "accent");
  self->downloads_button_attention_timeout_id = g_timeout_add_once (NEEDS_ATTENTION_ANIMATION_TIMEOUT,
                                                                    (GSourceOnceFunc)add_attention_timeout_cb,
                                                                    self);
}

static void
download_added_cb (EphyDownloadsManager *manager,
                   EphyDownload         *download,
                   EphyActionBarEnd     *action_bar_end)
{
  if (!action_bar_end->downloads_popover) {
    action_bar_end->downloads_popover = ephy_downloads_popover_new ();
    gtk_menu_button_set_popover (GTK_MENU_BUTTON (action_bar_end->downloads_button),
                                 action_bar_end->downloads_popover);
  }

  add_attention (action_bar_end);
  gtk_revealer_set_reveal_child (GTK_REVEALER (action_bar_end->downloads_revealer), TRUE);
}

static void
download_completed_cb (EphyDownloadsManager *manager,
                       EphyDownload         *download,
                       EphyActionBarEnd     *action_bar_end)
{
  ephy_downloads_paintable_animate_done (EPHY_DOWNLOADS_PAINTABLE (action_bar_end->downloads_paintable));
}

static void
download_removed_cb (EphyDownloadsManager *manager,
                     EphyDownload         *download,
                     EphyActionBarEnd     *action_bar_end)
{
  if (!ephy_downloads_manager_get_downloads (manager))
    gtk_revealer_set_reveal_child (GTK_REVEALER (action_bar_end->downloads_revealer), FALSE);
}

static void
downloads_estimated_progress_cb (EphyDownloadsManager *manager,
                                 EphyActionBarEnd     *action_bar_end)
{
  gdouble fraction = ephy_downloads_manager_get_estimated_progress (manager);

  g_object_set (action_bar_end->downloads_paintable, "progress", fraction, NULL);
}

static void
show_downloads_cb (EphyDownloadsManager *manager,
                   EphyActionBarEnd     *action_bar_end)
{
  if (gtk_widget_get_mapped (GTK_WIDGET (action_bar_end)))
    gtk_menu_button_popup (GTK_MENU_BUTTON (action_bar_end->downloads_button));
}

static void
on_bookmarks_button (GtkButton *button,
                     gpointer   user_data)
{
  GtkWindow *parent_window = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (button)));
  ephy_window_toggle_bookmarks (EPHY_WINDOW (parent_window));
}

static void
ephy_action_bar_end_dispose (GObject *object)
{
  EphyActionBarEnd *self = EPHY_ACTION_BAR_END (object);

  g_clear_handle_id (&self->downloads_button_attention_timeout_id, g_source_remove);

  G_OBJECT_CLASS (ephy_action_bar_end_parent_class)->dispose (object);
}

static void
ephy_action_bar_end_class_init (EphyActionBarEndClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = ephy_action_bar_end_dispose;

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/epiphany/gtk/action-bar-end.ui");

  gtk_widget_class_bind_template_child (widget_class,
                                        EphyActionBarEnd,
                                        bookmarks_button);
  gtk_widget_class_bind_template_child (widget_class,
                                        EphyActionBarEnd,
                                        downloads_revealer);
  gtk_widget_class_bind_template_child (widget_class,
                                        EphyActionBarEnd,
                                        downloads_button);
  gtk_widget_class_bind_template_child (widget_class,
                                        EphyActionBarEnd,
                                        downloads_icon);
  gtk_widget_class_bind_template_child (widget_class,
                                        EphyActionBarEnd,
                                        overview_button);
  gtk_widget_class_bind_template_callback (widget_class,
                                           on_bookmarks_button);
}

static void
ephy_action_bar_end_init (EphyActionBarEnd *action_bar_end)
{
  GObject *object = G_OBJECT (action_bar_end);
  EphyDownloadsManager *downloads_manager;
  EphyEmbedShell *embed_shell;

  gtk_widget_init_template (GTK_WIDGET (action_bar_end));

  /* Downloads */
  embed_shell = ephy_embed_shell_get_default ();
  downloads_manager = ephy_embed_shell_get_downloads_manager (embed_shell);

  gtk_revealer_set_reveal_child (GTK_REVEALER (action_bar_end->downloads_revealer),
                                 !!ephy_downloads_manager_get_downloads (downloads_manager));

  if (ephy_downloads_manager_get_downloads (downloads_manager)) {
    action_bar_end->downloads_popover = ephy_downloads_popover_new ();
    gtk_menu_button_set_popover (GTK_MENU_BUTTON (action_bar_end->downloads_button), action_bar_end->downloads_popover);
  }

  action_bar_end->downloads_paintable = ephy_downloads_paintable_new (action_bar_end->downloads_icon);
  gtk_image_set_from_paintable (GTK_IMAGE (action_bar_end->downloads_icon),
                                action_bar_end->downloads_paintable);

  if (is_desktop_pantheon ()) {
    gtk_button_set_icon_name (GTK_BUTTON (action_bar_end->bookmarks_button),
                              "user-bookmarks");
    gtk_button_set_icon_name (GTK_BUTTON (action_bar_end->overview_button),
                              "view-grid");
  }

  gtk_widget_set_visible (action_bar_end->overview_button,
                          ephy_embed_shell_get_mode (embed_shell) != EPHY_EMBED_SHELL_MODE_APPLICATION);

  g_signal_connect_object (downloads_manager, "download-added",
                           G_CALLBACK (download_added_cb),
                           object, G_CONNECT_DEFAULT);
  g_signal_connect_object (downloads_manager, "download-completed",
                           G_CALLBACK (download_completed_cb),
                           object, G_CONNECT_DEFAULT);
  g_signal_connect_object (downloads_manager, "download-removed",
                           G_CALLBACK (download_removed_cb),
                           object, G_CONNECT_DEFAULT);
  g_signal_connect_object (downloads_manager, "estimated-progress-changed",
                           G_CALLBACK (downloads_estimated_progress_cb),
                           object, G_CONNECT_DEFAULT);
  g_signal_connect_object (downloads_manager, "show-downloads",
                           G_CALLBACK (show_downloads_cb),
                           object, G_CONNECT_DEFAULT);
}

EphyActionBarEnd *
ephy_action_bar_end_new (void)
{
  return g_object_new (EPHY_TYPE_ACTION_BAR_END,
                       NULL);
}

void
ephy_action_bar_end_set_show_bookmarks_button (EphyActionBarEnd *action_bar_end,
                                               gboolean          show)
{
  gtk_widget_set_visible (action_bar_end->bookmarks_button, show);
}

void
ephy_action_bar_end_show_downloads (EphyActionBarEnd *action_bar_end)
{
  if (gtk_widget_get_visible (action_bar_end->downloads_button))
    gtk_menu_button_popup (GTK_MENU_BUTTON (action_bar_end->downloads_button));
}

GtkWidget *
ephy_action_bar_end_get_downloads_revealer (EphyActionBarEnd *action_bar_end)
{
  return action_bar_end->downloads_revealer;
}
