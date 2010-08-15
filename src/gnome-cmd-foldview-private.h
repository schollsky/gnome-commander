/*
    GNOME Commander - A GNOME based file manager
    Copyright (C) 2001-2006 Marcus Bjurman
    Copyright (C) 2007-2010 Piotr Eljasiak
    Copyright (C) 2010-2010 Guillaume Wardavoir

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
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __GCMDGTKFOLDVIEW_PRIVATE_H__
#define __GCMDGTKFOLDVIEW_PRIVATE_H__

/******************************************************************************

	C++
	Contain variadic macros

	---------------------------------------------------------------------------
	TODO GWR

		GcmdFileSelector resize automatically resize the main vertical pane
		( FILL / EXPAND gtk settings somewhere ) ?

		Trivial bug as no item is selectionned on foldview showing

		..................... facultative  / later ............................

		sync file_list when it is not active ( in a hidden tab )

		Bookmarks

		Notification from filesystem ( very hard to do, we're really going
		multithread ) -> gdk_threads_enter will be enough I think

DONE	init / destroy cleanup : g_object_ref on hided widgets

DONE	memory leaks hunt

DONE	memory leak : gnome-cmd-foldview-control.cc::814 forgot g_free(str)

DONE	memory leak : root node

DONE	memory leak : GcmdGtkFldview::Model::destroy implemented

DONE	memory leak : gvfs quickstack

DONE	correct ESC[30m for terminal after each gwr_print

		.......................................................................
COMMIT  05bb32e2cfd8eb56bf64e93bc1785ce52f6a100d

DONE	symlinks icons show little white pixel when selected its ugly
		but gcmd's too

DONE	foldview correct position on show / hide ( cmdline, buttonbar, ... )

DONE	(Finally) gcmd crashes when opening access-denied folder from treeview
		in tab, after showing an alert

DONE	Going crazy with correct theming of treeview with selectionned items,
		focus between widgets,...

DONE	combo connections at startup

DONE	gtk_widget_set_sensitive on non-full-functional items of context menu

DONE	Check index_ff on qstack_push()

DONE	? mutex on handles : no, gvfs_async calls are queued
		and since I use gdk_threads_enter/leave, there is no more
		concurrency between gtk & gvfs

DONE	why GVFS_qstack_ff / GVFS_qstack_size  is always n-2 / n ??? should be n / n
		-> because t' affiches ff _avant_ update et qsize au lieu de qsize - 1,
		stupid, ça fait un décalage de 2

DONE	Comments cleanup

DONE	Private logging for gvfs ops

DONE	Cancel all async ops

DONE	Big Bug in checking type of GnomeVFSFileInfo : GNOME_VFS_FILE_TYPE_...
		are distinct values - it is not a bitfield, triple fuck, spended time
		on that

	---------------------------------------------------------------------------
	TODO EPIOTR

DONE	in src/gnome-cmd-foldview.h:
        extern		gboolean			gnome_cmd_foldview_set_root_1(gchar *text);
		...
        extern		GtkWidget		*GcmdWidget();
        'extern' is superfluous here, please remove it

DONE	epiotr.10.07.16.gcmd-foldview
DONE	ua

		epiotr.10.07.22.kill-warnings
DONE	epiotr.10.07.22.gtk_object_ref,unref

DONE	epiotr.10.07.24.000? struct-gvfs_async_caller_data
DONE	epiotr.10.07.24.000? struct-gvfs-async
DONE	epiotr.10.07.24.000? struct-gvfs-async-load-subdirs
DONE	epiotr.10.07.24.000? struct-gvfs-async-load-subdirs-subdirs


DONE	* I've moved view_treeview() to gnome-cmd-user-actions.cc. I'm
		sending you a patch for that.

DONE	* after opening foldview, cmdline is moved from bottom to top -
		that requires fixing

		* foldview should respect global settings for case sensitive
		sorting (gnome_cmd_data.case_sens_sort) and showing hidden dirs
		(gnome_cmd_data.filter_settings.hidden)

		* when folview is opened for the first time, it should initially
		expand to the current dir of active pane:
		main_win->fs(ACTIVE)->get_directory()



*******************************************************************************
						HELPERS LOGGERS
						===============

* gnome-cmd-file.h
gint gnome_cmd_file_get_ref_count(GnomeCmdFile* file);
void gnome_cmd_file_log(gchar *str, GnomeCmdFile *File);

gint gnome_cmd_file_get_ref_count(GnomeCmdFile* file)
{
	return file->priv->ref_cnt;
}

* gnome-cmd-file.cc
void gnome_cmd_file_log(gchar *str, GnomeCmdFile *file)
{
	gchar   temp[1024];

	sprintf(temp, "fil:[0x%08x %3s][%03i-%03i] [h:xxxxxxxx] [mu:xxx] [%s] [%s]\n",
		file,
		g_object_is_floating(file) ? "FLO" : "REF",
		((GInitiallyUnowned*)file)->ref_count,
		gnome_cmd_file_get_ref_count(file),
		GNOME_CMD_FILE_INFO(file)->uri != NULL ? GNOME_CMD_FILE_INFO(file)->uri->text : "no uri",
		str);

	printf(temp);
}

* gnome-cmd-dir.h
void gnome_cmd_dir_log(const gchar *str, GnomeCmdDir *dir);

* gnome-cmd-dir.cc
void gnome_cmd_dir_log(const gchar *str, GnomeCmdDir *dir)
{
	gchar   temp[1024];

	if ( ! dir )
	{
		sprintf(temp, "dir:NULL [%s]\n", str);
		return;
	}

	sprintf(temp, "dir:[0x%08x %3s][%03i-%03i] [h:0x%08x] [mu:xxx] [%s] [%s]\n",
		dir,
		g_object_is_floating(dir) ? "FLO" : "REF",
		((GInitiallyUnowned*)dir)->ref_count,
		gnome_cmd_file_get_ref_count(GNOME_CMD_FILE(dir)),
		gnome_cmd_dir_get_handle(dir),
		//dir->priv->monitor_users,
		GNOME_CMD_FILE_INFO(dir)->uri != NULL ? GNOME_CMD_FILE_INFO(dir)->uri->text : "no uri",
		str);

	printf(temp);
}

* gnome-cmd-file-list.cc
static void on_dir_list_failed (GnomeCmdDir *dir, GnomeVFSResult result, GnomeCmdFileList *fl)
{
	gnome_cmd_dir_log("on_dir_list_failed 00", dir);

    DEBUG('l', "on_dir_list_failed\n");

    if (result != GNOME_VFS_OK)
        gnome_cmd_show_message (NULL, _("Directory listing failed."), gnome_vfs_result_to_string (result));

	gnome_cmd_dir_log("on_dir_list_failed 01", dir);

    g_signal_handlers_disconnect_matched (fl->cwd, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, fl);
	gnome_cmd_dir_log("on_dir_list_failed 01-1", dir);
    fl->connected_dir = NULL;
	gnome_cmd_dir_log("on_dir_list_failed 01-2", dir);
    gnome_cmd_dir_unref (fl->cwd);
	gnome_cmd_dir_log("on_dir_list_failed 01-3", dir);
    set_cursor_default_for_widget (*fl);
	gnome_cmd_dir_log("on_dir_list_failed 01-4", dir);
    gtk_widget_set_sensitive (*fl, TRUE);

	gnome_cmd_dir_log("on_dir_list_failed 02", dir);

    if (fl->lwd && fl->con == gnome_cmd_dir_get_connection (fl->lwd))
    {
        fl->cwd = fl->lwd;
        g_signal_connect (fl->cwd, "list-ok", G_CALLBACK (on_dir_list_ok), fl);
        g_signal_connect (fl->cwd, "list-failed", G_CALLBACK (on_dir_list_failed), fl);
        fl->lwd = NULL;
    }
    else
        g_timeout_add (1, (GSourceFunc) set_home_connection, fl);
}

void GnomeCmdFileList::set_connection (GnomeCmdCon *new_con, GnomeCmdDir *start_dir)
{
    g_return_if_fail (GNOME_CMD_IS_CON (new_con));

    if (con==new_con)
    {
		//if (!gnome_cmd_con_should_remember_dir (new_con)) //original code buggy // __GWR__MARK__
		if ( gnome_cmd_con_should_remember_dir (new_con))
            set_directory (gnome_cmd_con_get_default_dir (new_con));
        else
            if (start_dir)
                set_directory (start_dir);
        return;
    }
	...

void GnomeCmdFileList::set_directory(GnomeCmdDir *dir)
{
    g_return_if_fail (GNOME_CMD_IS_DIR (dir));

	gnome_cmd_dir_log("GnomeCmdFileList::set_directory 00", dir);

    if (cwd==dir)
        return;

    if (realized && dir->state!=DIR_STATE_LISTED)
    {
        gtk_widget_set_sensitive (*this, FALSE);
        set_cursor_busy_for_widget (*this);
    }

    gnome_cmd_dir_ref (dir);

    if (lwd && lwd!=dir)
        gnome_cmd_dir_unref (lwd);

    if (cwd)
    {
        gnome_cmd_dir_cancel_monitoring (cwd);
        lwd = cwd;
        g_signal_handlers_disconnect_matched (lwd, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
        cwd->voffset = gnome_cmd_clist_get_voffset (*this);
    }

    cwd = dir;

    switch (dir->state)
    {
        case DIR_STATE_EMPTY:
            g_signal_connect (dir, "list-ok", G_CALLBACK (on_dir_list_ok), this);
            g_signal_connect (dir, "list-failed", G_CALLBACK (on_dir_list_failed), this);
			gnome_cmd_dir_log("GnomeCmdFileList::set_directory 01", dir);
            gnome_cmd_dir_list_files (dir, gnome_cmd_con_needs_list_visprog (con));
			gnome_cmd_dir_log("GnomeCmdFileList::set_directory 02", dir);
            break;

        case DIR_STATE_LISTING:
        case DIR_STATE_CANCELING:
            g_signal_connect (dir, "list-ok", G_CALLBACK (on_dir_list_ok), this);
            g_signal_connect (dir, "list-failed", G_CALLBACK (on_dir_list_failed), this);
            break;

        case DIR_STATE_LISTED:
            on_dir_list_ok (dir, NULL, this);
		    gnome_cmd_dir_start_monitoring (dir);
            break;
    }

	gnome_cmd_dir_log("GnomeCmdFileList::set_directory 03", dir);

	// in the previous 'case', dir->state may have been modified. So include
	// this line in the 'case'
    //gnome_cmd_dir_start_monitoring (dir);
}



*******************************************************************************

					WIDGET HIERARCHY FOR GTK-FOLDVIEW
					=================================

		vbox
		|
		con_hbox
		|	|
		|	+-----------GtkWidget *con_combo ---------- GtkWidget *vol_label;
		|
		|
		dir_indicator
		|
		scrolled window-------- treeview
		|
		GtkWidget *info_label;


*******************************************************************************

				WIDGET HIERARCHY FOR GNOME-CMD-FILE-SELECTOR
				============================================

		vbox
		|
	GnomeCmdFileSelector
		|
		|
		+--------------------- GnomeCmdFileList


*******************************************************************************

						GTK+ DRAG & DROP SYNOPSIS
						=========================


		"drag-begin"									SRC

		"drag-motion"												DST
			|
			|
			+-----------------------+
			|						|
			|						|
		"drag-drop"				"drag-leave"						DST
			|
			|
		"drag-data-get"									SRC
			|
			|
		"drag-data-received"										DST
			|
			|
		"drag-end"										SRC


	+   "drag-failed"									SRC
		"drag-data-delete"								SRC

******************************************************************************/
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>

//*****************************************************************************
//								Defines
//*****************************************************************************

// undef this if you want foldview to use a GtkTreestore instead
// of a GnomeCmdFoldviewTreestore
//#define USE_GTK_TREESTORE

#ifndef USE_GTK_TREESTORE
	#include "gnome-cmd-foldview-treestore.h"
#endif

//*****************************************************************************
//								Logging
//*****************************************************************************
//#define DEBUG_SHOW_INF
#define DEBUG_SHOW_WNG
#define DEBUG_SHOW_ERR
#define DEBUG_SHOW_VFS

void gwr_print(const char* str);

void gwr_inf(const char* fmt, ...);
void gwr_wng(const char* fmt, ...);
void gwr_err(const char* fmt, ...);

void gwr_inf_vfs(const char* fmt, ...);
void gwr_err_vfs(const char* fmt, ...);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Facilities logging
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ERR_RET(value,...)													\
{																			\
	gwr_err(__VA_ARGS__);													\
	return value;															\
}

#define ERR_FAIL(...)														\
{																			\
	gwr_err(__VA_ARGS__);													\
	exit(1);																\
}
//---------------------------------------------------------------------------
#define ERR_VFS_RET(value,...)												\
{																			\
	if ( sVFSResult != GNOME_VFS_OK )										\
	{																		\
		gwr_err_vfs(__VA_ARGS__);											\
		return value;														\
	}																		\
}

#define ERR_VFS_FAIL(value,...)												\
{																			\
	if ( sVFSResult != GNOME_VFS_OK )										\
	{																		\
		gwr_err_vfs(__VA_ARGS__);											\
		exit(1);															\
	}																		\
}

//*****************************************************************************
//							GcmdGtkFoldview
//*****************************************************************************
#define GCMDGTKFOLDVIEW_TYPE			(gcmdgtkfoldview_get_type ())
#define GCMDGTKFOLDVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCMDGTKFOLDVIEW_TYPE, GcmdGtkFoldview))
#define GCMDGTKFOLDVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GCMDGTKFOLDVIEW_TYPE, GcmdGtkFoldviewClass))
#define IS_GCMDGTKFOLDVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCMDGTKFOLDVIEW_TYPE))
#define IS_GCMDGTKFOLDVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GCMDGTKFOLDVIEW_TYPE))

#ifdef USE_GTK_TREESTORE
	#define __GTS__
#endif

struct gvfs_async;
struct gvfs_file;

struct GcmdGtkFoldview
{
	//=========================================================================
	//  Widgets and gtk inheritance
	//=========================================================================
	GtkVBox		vbox;

	//=========================================================================
	//  Common data, enum, typedefs
	//=========================================================================
	public:

	enum eFileAccess
	{
		// these permissions are user-relatives ( the user that launched gcmd )
		eAccessReadOnly		=   0   ,
		eAccessReadWrite	=   1   ,
		eAccessForbidden	=   2   ,

		eAccessInit			=   0xFE,
		eAccessUnknown		=   0xFF
	};

	public:
	static  eFileAccess Access_from_permissions		(GnomeVFSFilePermissions permissions);

	//=========================================================================
	//  View
	//=========================================================================
	struct View
	{
		// For fixing the moving gutter of the pane
		gulong		a_size_request_handle;
		gint		a_size_request_width;

		// We have a m_parent member because of callbacks, for calling
		// the controller
		private:
		GtkWidget   *a_this;

		//---------------------------------------------------------------------
		// other widgets ( we are a composite widget )
		//---------------------------------------------------------------------
		private:
		GtkWidget *m_con_hbox;
		GtkWidget *m_con_combo;
		GtkWidget *m_vol_label;

		// __GWR__ Unable to reuse the gnome-cmd-dir-indicator since it is
		// crossed with gnome-cmd-file-selector
		GtkWidget   m_dir_indicator;

		GtkWidget   *m_scrolledwindow;
		GtkWidget   *m_info_label;

		public:
		GcmdGtkFoldview *foldview()				{ return (GcmdGtkFoldview*)a_this;  }
		GtkWidget		*connection_combo()		{ return (m_con_combo);				}

		//---------------------------------------------------------------------
		// icons
		//---------------------------------------------------------------------
		private:
		GdkPixbuf		*m_pixbuf[50];

		public:
		enum eIcon
		{
			eIconUnknown					,

			eIconDirReadWrite				,
			eIconDirReadOnly				,
			eIconDirForbidden				,

			eIconSymlinkToDirReadWrite		,
			eIconSymlinkToDirReadOnly		,
			eIconSymlinkToDirForbidden
		};

		private:
		gboolean		icons_load();
		void			icons_unload();

		public:
		GdkPixbuf	   *pixbuf(eIcon icon)		{   return m_pixbuf[icon];	}

		static  eIcon Icon_from_type_permissions	(GnomeVFSFileType type, GnomeVFSFilePermissions permissions);
		static  eIcon Icon_from_type_access			(GnomeVFSFileType type, GcmdGtkFoldview::eFileAccess access);

		//---------------------------------------------------------------------
		// connection combo
		//---------------------------------------------------------------------
		public:
		void connection_combo_add_connection(GnomeCmdCon *con);
		void connection_combo_reset();

		//---------------------------------------------------------------------
		// gtk treeview ( main view )
		//---------------------------------------------------------------------
		private:
		GtkWidget		*m_treeview;
		GtkTreeModel	*m_treemodel;

		GtkTreeView		*treeview() { return GTK_TREE_VIEW (m_treeview); }

		public:
		void			init_instance(GtkWidget *_this, GtkTreeModel *_treemodel);
		private:
		void			raz_pointers();
		gboolean		create(GtkWidget *_this, GtkTreeModel *_treemodel);
		void			destroy();

		private:
		GtkTreeSelection	*selection()		{ return gtk_tree_view_get_selection(treeview()); }
		gint				selection_count()
							{
								// Note: gtk_tree_selection_count_selected_rows() does not
								//   exist in gtk+-2.0, only in gtk+ >= v2.2 !
 								return 	gtk_tree_selection_count_selected_rows(selection());
							}

		//.....................................................................
		static gboolean		signal_test_expand_row(GtkTreeView *tree_view,
			GtkTreeIter		*iter, GtkTreePath *path, gpointer user_data);

		static  void		signal_row_expanded(GtkTreeView *tree_view,
			GtkTreeIter		*iter, GtkTreePath *path, gpointer user_data);

		static 	void		signal_row_collapsed(GtkTreeView *tree_view,
			GtkTreeIter		*iter, GtkTreePath *path,gpointer user_data);

		static  gboolean	signal_button_press_event(GtkWidget *tree_view,
							GdkEventButton *event, gpointer user_data);

		static 	void		signal_drag_begin(GtkWidget *widget,
							GdkDragContext *drag_context, gpointer user_data);

		static  void		signal_size_request(GtkWidget *widget,
							GtkRequisition  *requisition,gpointer data);

		//---------------------------------------------------------------------
		// contextual menu
		//---------------------------------------------------------------------
		public:
		struct  ctx_menu_data
		{
			GcmdGtkFoldview		*a_foldview;
			GtkTreePath			*d_path_clicked;
			GtkTreePath			*d_path_selected;
			guint32				a_time;
			gint				a_event_x;
			gint				a_event_y;
			guint				a_button;

								~ctx_menu_data()
								{
									if ( d_path_selected )
									{
										gtk_tree_path_free(d_path_selected);
									}
									if ( d_path_clicked )
									{
										gtk_tree_path_free(d_path_clicked);
									}
								}

			void	*operator   new(size_t size)	{  return g_new0(ctx_menu_data, 1);  }
			void	operator	delete(void *p)		{  g_free(p);  }
		};
		public:
		struct  ctx_menu_desc
		{
			gboolean	a_connect;
			gchar		*a_text;
			GCallback   a_callback;
		};
		struct  ctx_menu_entry
		{
			ctx_menu_desc   a_desc;
			GtkWidget	   *a_widget;
			gulong			a_handle;
		};
		struct  ctx_menu_section
		{
			gchar			*a_title;
			gint			a_card;
			ctx_menu_entry  a_entry[5];
		};
		struct  ctx_menu
		{
			GtkWidget			*a_widget;
			gint				a_card;
			ctx_menu_section	a_section[5];
		};

		private:
		gboolean		context_menu_pop(ctx_menu_data*);

		gboolean		click_left_single(ctx_menu_data *ctxdata);
		gboolean		click_left_double(ctx_menu_data *ctxdata);

		//---------------------------------------------------------------------
		// Theming
		//---------------------------------------------------------------------
		public:
		void	update_style();

	} view;
	//=========================================================================
	//  Model
	//=========================================================================
	struct Model
	{
		public:
		void			init_instance();
		void			release_objects();
		private:
		void			raz_pointers();
		gboolean		create();

		//---------------------------------------------------------------------
		// divers
		//---------------------------------------------------------------------
		public:
		static GtkTreeIter Iter_zero;

		//---------------------------------------------------------------------
		// treestore
		//---------------------------------------------------------------------
#ifdef __GTS__							// With GtkTreeStore

		private:
		GtkTreeStore	*m_treestore;
		public:
		GtkTreeStore	*treestore()	{ return m_treestore;					}
		GtkTreeModel	*treemodel()	{ return GTK_TREE_MODEL(m_treestore);	}
		GtkTreeSortable	*treesortable()	{ return GTK_TREE_SORTABLE(m_treestore);}

#else								   // With GnomeCmdFoldviewTreestore

		private:
		GnomeCmdFoldviewTreestore	   *m_treestore;
		public:
		GnomeCmdFoldviewTreestore		*treestore()	{ return m_treestore;	}
		GtkTreeModel	*treemodel()	{ return GTK_TREE_MODEL(m_treestore);	}
		GtkTreeSortable	*treesortable()	{ return GTK_TREE_SORTABLE(m_treestore);}

		//
		//  Instead of storing boring G_TYPES vars in the store, we can store
		//  anything that inherits from GnomeCmdFoldviewTreestore::Data
		//  when using GnomeCmdFoldviewTreestore :)
		//
		public:
		struct Rowlike : GnomeCmdFoldviewTreestore::Data
		{
			private:
			gchar   *d_utf8_name;
			gchar   *d_utf8_collate_key;
			gint	a_icon;
			//.................................................................
			public:
			Rowlike(gchar *_utf8_name, gint _icon);
			~Rowlike();
			//.................................................................
			public:
			const   gchar*  utf8_collate_key();
			const   gchar*	utf8_name();
					gint	icon();
		};

#endif

		// Treemodel methods. These methods use either
		// GtkTreestore or CnomeCmdFoldviewTreestore, depending on the
		// #define __GTS__ above.

		//
		// Methods from GtkTreeModelIface
		//
		gboolean		get_iter(GtkTreePath *path /*in*/, GtkTreeIter *iter /*out*/);
		gint			iter_n_children (GtkTreeIter *parent);
		//
		// Custom methods
		//
		private:
		static  gint			Compare(GtkTreeModel*, GtkTreeIter*, GtkTreeIter* , gpointer);
		public:
		static  GtkTreeIter*	Iter_new(GtkTreeIter*);
		static  void			Iter_del(GtkTreeIter*);

		GnomeVFSURI*	iter_get_uri_new	(GtkTreeIter *iter);
		gchar*			iter_get_string_new (GtkTreeIter *iter);

		void			set_value(GtkTreeIter*, gchar*, gint icon);

		void			iter_replace_first_child(GtkTreeIter *parent, GtkTreeIter *child, gchar *name, gint icon);
		void			iter_replace_dummy_child(GtkTreeIter *parent, GtkTreeIter *child, gchar *name, gint icon);

		gboolean		iter_add_child  (GtkTreeIter* parent /*in*/, GtkTreeIter *child /*out*/, const gchar *name, gint icon);

		//gboolean		iter_remove(GtkTreeIter *iter);
		void			iter_remove_all();

		gint			iter_remove_children(GtkTreeIter *parent);

		//---------------------------------------------------------------------
		// store custom data in gtktreestore !
		//---------------------------------------------------------------------

		//---------------------------------------------------------------------
		// root element ( because of samba possibility )
		//---------------------------------------------------------------------
		public:
		struct Root
		{
			GtkTreeIter			m_iter;
			GnomeVFSURI			*m_uri;
			GnomeVFSFileInfo	*m_info;   // __GWR_TODO__ gnome_vfs_dup segfault ???
			gboolean			set(GnomeVFSURI *uri);
			void				unset();
		} root;

		//---------------------------------------------------------------------
		// gnome-cmd-connection
		//---------------------------------------------------------------------
		private:
		GnomeCmdCon		*m_con;

		public:
		void			connection(GnomeCmdCon *con)	{ m_con = con;		}
		GnomeCmdCon*	connection()					{ return m_con;		}

	} model;

	//=========================================================================
	//  Controller
	//=========================================================================
	public:
	void		control_init_instance();
	private:
	void		control_raz_pointers();
	gboolean	control_create();
	void		control_finalize();
	void		control_dispose();

	//-------------------------------------------------------------------------
	// GObject stuff
	//-------------------------------------------------------------------------
	public:
	static GObjectClass *Control_parent_class;
	public:
	static  void	Control_gtk_object_destroy	(GtkObject*);
	static  void	Control_g_object_dispose	(GObject*);
	static  void	Control_g_object_finalize   (GObject*);
	static  void	Control_g_object_init		(GcmdGtkFoldview *foldview);

			gint	control_ref_count()
					{
						return ((GInitiallyUnowned*)this)->ref_count;
					}

	//-------------------------------------------------------------------------
	// divers
	//-------------------------------------------------------------------------
	private:
	gboolean	control_root_uri_set(GnomeVFSURI *uri);

	void		control_gcmd_file_list_set_connection(GnomeCmdFileList*, GnomeVFSURI*);

	void		control_connection_combo_update();

	//-------------------------------------------------------------------------
	// expand  / collapse
	//-------------------------------------------------------------------------
	public:
			void		control_iter_collapsed(GtkTreeIter *parent);

	private:
	static  void		control_check_if_empty_callback_1(gvfs_async*);
			void		control_check_if_empty_p(GnomeVFSURI *parent_uri, gchar *parent_path);
	public:
			void		control_check_if_empty(GtkTreeIter *parent);

	private:
	static  void		control_iter_expand_callback_1(gvfs_async*);
	static  void		control_iter_expand_callback_2(gvfs_async*);
			void		control_iter_expand_p(GnomeVFSURI *parent_uri, gchar *parent_path);
	public:
			void		control_iter_expand(GtkTreeIter *parent);

	//-------------------------------------------------------------------------
	// context menu actions
	//-------------------------------------------------------------------------
	private:
			void		control_set_active_tab	(GtkTreePath *path);
			void		control_open_new_tab	(GtkTreePath *path);
			void		control_set_new_root	(GcmdGtkFoldview::View::ctx_menu_data*);
	public:
	static  void		Control_set_active_tab	(GtkMenuItem*, View::ctx_menu_data*);
	static  void		Control_open_new_tab	(GtkMenuItem*, View::ctx_menu_data*);
	static  void		Control_set_new_root	(GtkMenuItem*, View::ctx_menu_data*);

	private:
	enum eSyncState
	{
		SYNC_Y_LIST_Y,
		SYNC_Y_LIST_N,
		SYNC_N_LIST_Y,
		SYNC_N_LIST_N
	};
	eSyncState	control_sync_check		();
	private:
			void		control_sync_treeview   (GcmdGtkFoldview::View::ctx_menu_data*);
			void		control_unsync_treeview ();
			void		control_sync_update		(GcmdGtkFoldview::View::ctx_menu_data*);
	public:
	static  void		Control_sync_treeview   (GtkMenuItem*, View::ctx_menu_data*);
	static  void		Control_unsync_treeview (GtkMenuItem*, View::ctx_menu_data*);
	static  void		Control_sync_update		(GtkMenuItem*, View::ctx_menu_data*);

	private:
	GnomeCmdFileList		*a_synced_list;
	gboolean				a_synced;

	private:
	static  View::ctx_menu		s_context_menu;

	public:
	void		control_context_menu_populate_add_separator(GtkWidget *widget);
	void		control_context_menu_populate_add_section(GtkWidget *widget, gint i, View::ctx_menu_data *ctxdata);

	//=========================================================================
	//  Interface
	//=========================================================================
	public:
	gboolean	root_uri_set_1(gchar			*text);
	gboolean	root_uri_set_2(GnomeVFSURI		*uri);
};

GType          gcmdgtkfoldview_get_type ();
GtkWidget*     gcmdgtkfoldview_new		();
void	       gcmdgtkfoldview_clear	(GcmdGtkFoldview *foldview);

//*****************************************************************************
//  GcmdGtkFoldviewClass
//*****************************************************************************
struct GcmdGtkFoldviewClass
{
  GtkVBoxClass parent_class;

  void (* gcmdgtkfoldview) (GcmdGtkFoldview* fv);
};

extern		GcmdGtkFoldview		*GcmdFoldview();
extern		GtkWidget			*GcmdWidget();


#endif //__GCMDGTKFOLDVIEW_PRIVATE_H__
