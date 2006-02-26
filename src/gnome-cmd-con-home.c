/*
    GNOME Commander - A GNOME based file manager
    Copyright (C) 2001-2006 Marcus Bjurman

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "gnome-cmd-includes.h"
#include "gnome-cmd-con-home.h"
#include "gnome-cmd-plain-path.h"
#include "imageloader.h"


struct _GnomeCmdConHomePrivate
{
};


static GnomeCmdConClass *parent_class = NULL;



static void
home_open (GnomeCmdCon *con)
{
}


static gboolean
home_close (GnomeCmdCon *con)
{
    return FALSE;
}


static void
home_cancel_open (GnomeCmdCon *con)
{
}


static gboolean
home_open_is_needed (GnomeCmdCon *con)
{
    return FALSE;
}


static GnomeVFSURI *
home_create_uri (GnomeCmdCon *con, GnomeCmdPath *path)
{
    GnomeVFSURI *u1, *u2;

    u1 = gnome_vfs_uri_new ("file:");
    u2 = gnome_vfs_uri_append_path (u1, gnome_cmd_path_get_path (path));
    gnome_vfs_uri_unref (u1);

    return u2;
}


static GnomeCmdPath *
home_create_path (GnomeCmdCon *con, const gchar *path_str)
{
    return gnome_cmd_plain_path_new (path_str);
}



/*******************************
 * Gtk class implementation
 *******************************/

static void
destroy (GtkObject *object)
{
    GnomeCmdConHome *con = GNOME_CMD_CON_HOME (object);

    g_free (con->priv);

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


static void
class_init (GnomeCmdConHomeClass *class)
{
    GtkObjectClass *object_class;
    GnomeCmdConClass *con_class;

    object_class = GTK_OBJECT_CLASS (class);
    con_class = GNOME_CMD_CON_CLASS (class);
    parent_class = gtk_type_class (gnome_cmd_con_get_type ());

    object_class->destroy = destroy;

    con_class->open = home_open;
    con_class->close = home_close;
    con_class->cancel_open = home_cancel_open;
    con_class->open_is_needed = home_open_is_needed;
    con_class->create_uri = home_create_uri;
    con_class->create_path = home_create_path;
}


static void
init (GnomeCmdConHome *home_con)
{
    GnomeCmdDir *dir;
    GnomeCmdPath *path;
    GnomeCmdCon *con = GNOME_CMD_CON (home_con);

    home_con->priv = g_new (GnomeCmdConHomePrivate, 1);

    con->state = CON_STATE_OPEN;
    con->alias = g_strdup (_("Home"));
    con->open_msg = g_strdup ("This should not be visible anywhere");
    con->should_remember_dir = FALSE;
    con->needs_open_visprog = FALSE;
    con->needs_list_visprog = FALSE;
    con->can_show_free_space = TRUE;
    con->is_local = TRUE;
    con->is_closeable = FALSE;
    con->go_text = g_strdup (_("Go to: Home"));
    con->go_pixmap = IMAGE_get_gnome_cmd_pixmap (PIXMAP_HOME);
    con->open_pixmap = IMAGE_get_gnome_cmd_pixmap (PIXMAP_HOME);
    con->close_pixmap = IMAGE_get_gnome_cmd_pixmap (PIXMAP_HOME);

    path = gnome_cmd_plain_path_new (g_get_home_dir ());
    dir = gnome_cmd_dir_new (con, path);

    gnome_cmd_con_set_default_dir (con, dir);
    gnome_cmd_con_set_cwd (con, dir);
}



/***********************************
 * Public functions
 ***********************************/

GtkType
gnome_cmd_con_home_get_type         (void)
{
    static GtkType type = 0;

    if (type == 0)
    {
        GtkTypeInfo info =
        {
            "GnomeCmdConHome",
            sizeof (GnomeCmdConHome),
            sizeof (GnomeCmdConHomeClass),
            (GtkClassInitFunc) class_init,
            (GtkObjectInitFunc) init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            (GtkClassInitFunc) NULL
        };

        type = gtk_type_unique (gnome_cmd_con_get_type (), &info);
    }
    return type;
}


GnomeCmdCon *
gnome_cmd_con_home_new (void)
{
    GnomeCmdConHome *con;

    con = gtk_type_new (gnome_cmd_con_home_get_type ());

    return GNOME_CMD_CON (con);
}


