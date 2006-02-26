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
#include "gnome-cmd-path.h"


static GtkObjectClass *parent_class = NULL;


/*******************************
 * Gtk class implementation
 *******************************/

static void
destroy (GtkObject *object)
{
    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


static void
class_init (GnomeCmdPathClass *class)
{
    GtkObjectClass *object_class;

    object_class = GTK_OBJECT_CLASS (class);
    parent_class = gtk_type_class (gtk_object_get_type ());

    object_class->destroy = destroy;

    class->get_path = NULL;
    class->get_display_path = NULL;
    class->get_parent = NULL;
    class->get_child = NULL;
}


static void
init (GnomeCmdPath *path)
{
}



/***********************************
 * Public functions
 ***********************************/

GtkType
gnome_cmd_path_get_type         (void)
{
    static GtkType type = 0;

    if (type == 0)
    {
        GtkTypeInfo info =
        {
            "GnomeCmdPath",
            sizeof (GnomeCmdPath),
            sizeof (GnomeCmdPathClass),
            (GtkClassInitFunc) class_init,
            (GtkObjectInitFunc) init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            (GtkClassInitFunc) NULL
        };

        type = gtk_type_unique (gtk_object_get_type (), &info);
    }
    return type;
}


const gchar *
gnome_cmd_path_get_path (GnomeCmdPath *path)
{
    GnomeCmdPathClass *klass;

    g_return_val_if_fail (GNOME_CMD_IS_PATH (path), NULL);

    klass = GNOME_CMD_PATH_GET_CLASS (path);

    return klass->get_path (path);
}


const gchar *
gnome_cmd_path_get_display_path (GnomeCmdPath *path)
{
    GnomeCmdPathClass *klass;

    g_return_val_if_fail (GNOME_CMD_IS_PATH (path), NULL);

    klass = GNOME_CMD_PATH_GET_CLASS (path);

    return klass->get_display_path (path);
}


GnomeCmdPath *
gnome_cmd_path_get_parent (GnomeCmdPath *path)
{
    GnomeCmdPathClass *klass;

    g_return_val_if_fail (GNOME_CMD_IS_PATH (path), NULL);

    klass = GNOME_CMD_PATH_GET_CLASS (path);

    return klass->get_parent (path);
}


GnomeCmdPath *
gnome_cmd_path_get_child (GnomeCmdPath *path, const gchar *child)
{
    GnomeCmdPathClass *klass;

    g_return_val_if_fail (GNOME_CMD_IS_PATH (path), NULL);

    klass = GNOME_CMD_PATH_GET_CLASS (path);

    return klass->get_child (path, child);
}

