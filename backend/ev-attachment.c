/* this file is part of evince, a gnome document viewer
 *
 *  Copyright (C) 2006 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * Evince is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Evince is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include "ev-attachment.h"

enum
{
	PROP_0,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_MTIME,
	PROP_CTIME,
	PROP_SIZE,
	PROP_DATA
};

struct _EvAttachmentPrivate {
	gchar                   *name;
	gchar                   *description;
	GTime                    mtime;
	GTime                    ctime;
	gsize                    size;
	gchar                   *data;
	gchar                   *mime_type;

	GnomeVFSMimeApplication *app;
	gchar                   *tmp_uri;
};

#define EV_ATTACHMENT_GET_PRIVATE(object) \
                (G_TYPE_INSTANCE_GET_PRIVATE ((object), EV_TYPE_ATTACHMENT, EvAttachmentPrivate))

G_DEFINE_TYPE (EvAttachment, ev_attachment, G_TYPE_OBJECT)

GQuark
ev_attachment_error_quark (void)
{
	static GQuark error_quark = 0;
	
	if (error_quark == 0)
		error_quark =
			g_quark_from_static_string ("ev-attachment-error-quark");
	
	return error_quark;
}

static void
ev_attachment_finalize (GObject *object)
{
	EvAttachment *attachment = EV_ATTACHMENT (object);

	if (attachment->priv->name) {
		g_free (attachment->priv->name);
		attachment->priv->name = NULL;
	}

	if (attachment->priv->description) {
		g_free (attachment->priv->description);
		attachment->priv->description = NULL;
	}

	if (attachment->priv->data) {
		g_free (attachment->priv->data);
		attachment->priv->data = NULL;
	}

	if (attachment->priv->mime_type) {
		g_free (attachment->priv->mime_type);
		attachment->priv->mime_type = NULL;
	}

	if (attachment->priv->app) {
		gnome_vfs_mime_application_free (attachment->priv->app);
		attachment->priv->app = NULL;
	}

	if (attachment->priv->tmp_uri) {
		g_unlink (attachment->priv->tmp_uri);
		g_free (attachment->priv->tmp_uri);
		attachment->priv->tmp_uri = NULL;
	}

	(* G_OBJECT_CLASS (ev_attachment_parent_class)->finalize) (object);
}

static void
ev_attachment_set_property (GObject      *object,
			    guint         prop_id,
			    const GValue *value,
			    GParamSpec   *param_spec)
{
	EvAttachment *attachment = EV_ATTACHMENT (object);

	switch (prop_id) {
	case PROP_NAME:
		attachment->priv->name = g_value_dup_string (value);
		break;
	case PROP_DESCRIPTION:
		attachment->priv->description = g_value_dup_string (value);
		break;
	case PROP_MTIME:
		attachment->priv->mtime = g_value_get_ulong (value);
		break;
	case PROP_CTIME:
		attachment->priv->ctime = g_value_get_ulong (value);
		break;
	case PROP_SIZE:
		attachment->priv->size = g_value_get_uint (value);
		break;
	case PROP_DATA:
		attachment->priv->data = g_value_get_pointer (value);
		attachment->priv->mime_type =
			g_strdup (gnome_vfs_get_mime_type_for_data (attachment->priv->data,
								    attachment->priv->size));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
						   prop_id,
						   param_spec);
		break;
	}
}

static void
ev_attachment_class_init (EvAttachmentClass *klass)
{
	GObjectClass *g_object_class;

	g_object_class = G_OBJECT_CLASS (klass);

	g_object_class->set_property = ev_attachment_set_property;

	g_type_class_add_private (g_object_class, sizeof (EvAttachmentPrivate));

	/* Properties */
	g_object_class_install_property (g_object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
							      "Name",
							      "The attachment name",
							      NULL,
							      G_PARAM_WRITABLE |
							      G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (g_object_class,
					 PROP_DESCRIPTION,
					 g_param_spec_string ("description",
							      "Description",
							      "The attachment description",
							      NULL,
							      G_PARAM_WRITABLE |
							      G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (g_object_class,
					 PROP_MTIME,
					 g_param_spec_ulong ("mtime",
							     "ModifiedTime", 
							     "The attachment modification date",
							     0, G_MAXULONG, 0,
							     G_PARAM_WRITABLE |
							     G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (g_object_class,
					 PROP_CTIME,
					 g_param_spec_ulong ("ctime",
							     "CreationTime",
							     "The attachment creation date",
							     0, G_MAXULONG, 0,
							     G_PARAM_WRITABLE |
							     G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (g_object_class,
					 PROP_SIZE,
					 g_param_spec_uint ("size",
							    "Size",
							    "The attachment size",
							    0, G_MAXUINT, 0,
							    G_PARAM_WRITABLE |
							    G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (g_object_class,
					 PROP_DATA,
					 g_param_spec_pointer ("data",
							       "Data",
							       "The attachment data",
							       G_PARAM_WRITABLE |
							       G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class->finalize = ev_attachment_finalize;
}

static void
ev_attachment_init (EvAttachment *attachment)
{
	attachment->priv = EV_ATTACHMENT_GET_PRIVATE (attachment);

	attachment->priv->name = NULL;
	attachment->priv->description = NULL;
	attachment->priv->data = NULL;
	attachment->priv->mime_type = NULL;

	attachment->priv->tmp_uri = NULL;
}

EvAttachment *
ev_attachment_new (const gchar *name,
		   const gchar *description,
		   GTime        mtime,
		   GTime        ctime,
		   gsize        size,
		   gpointer     data)
{
	EvAttachment *attachment;

	attachment = g_object_new (EV_TYPE_ATTACHMENT,
				   "name", name,
				   "description", description,
				   "mtime", mtime,
				   "ctime", ctime,
				   "size", size,
				   "data", data,
				   NULL);

	return attachment;
}

const gchar *
ev_attachment_get_name (EvAttachment *attachment)
{
	g_return_val_if_fail (EV_IS_ATTACHMENT (attachment), NULL);

	return attachment->priv->name;
}

const gchar *
ev_attachment_get_description (EvAttachment *attachment)
{
	g_return_val_if_fail (EV_IS_ATTACHMENT (attachment), NULL);

	return attachment->priv->description;
}

GTime
ev_attachment_get_modification_date (EvAttachment *attachment)
{
	g_return_val_if_fail (EV_IS_ATTACHMENT (attachment), 0);

	return attachment->priv->mtime;
}

GTime
ev_attachment_get_creation_date (EvAttachment *attachment)
{
	g_return_val_if_fail (EV_IS_ATTACHMENT (attachment), 0);

	return attachment->priv->ctime;
}

const gchar *
ev_attachment_get_mime_type (EvAttachment *attachment)
{
	g_return_val_if_fail (EV_IS_ATTACHMENT (attachment), NULL);

	return attachment->priv->mime_type;
}

gboolean
ev_attachment_save (EvAttachment *attachment,
		    const gchar  *uri,
		    GError      **error)
{
	GnomeVFSHandle  *handle = NULL;
	GnomeVFSFileSize written;
	GnomeVFSResult   result;
	
	g_return_val_if_fail (EV_IS_ATTACHMENT (attachment), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	result = gnome_vfs_create (&handle, uri,
				   GNOME_VFS_OPEN_WRITE |
				   GNOME_VFS_OPEN_TRUNCATE,
				   FALSE, 0644);
	if (result != GNOME_VFS_OK) {
		g_set_error (error,
			     EV_ATTACHMENT_ERROR, 
			     (gint) result,
			     _("Couldn't save attachment '%s': %s"),
			     uri, 
			     gnome_vfs_result_to_string (result));
		
		return FALSE;
	}

	result = gnome_vfs_write (handle, attachment->priv->data,
				  attachment->priv->size, &written);
	if (result != GNOME_VFS_OK || written < attachment->priv->size){
		g_set_error (error,
			     EV_ATTACHMENT_ERROR,
			     (gint) result,
			     _("Couldn't save attachment '%s': %s"),
			     uri,
			     gnome_vfs_result_to_string (result));
		
		gnome_vfs_close (handle);

		return FALSE;
	}

	gnome_vfs_close (handle);

	return TRUE;
}

static gboolean
ev_attachment_launch_app (EvAttachment *attachment,
			  GError      **error)
{
	GnomeVFSResult result;
	GList         *uris = NULL;

	g_assert (attachment->priv->tmp_uri != NULL);
	g_assert (attachment->priv->app != NULL);

	uris = g_list_prepend (uris, attachment->priv->tmp_uri);
	result = gnome_vfs_mime_application_launch (attachment->priv->app,
						    uris);

	if (result != GNOME_VFS_OK) {
		g_set_error (error,
			     EV_ATTACHMENT_ERROR,
			     (gint) result,
			     _("Couldn't open attachment '%s': %s"),
			     attachment->priv->name,
			     gnome_vfs_result_to_string (result));

		g_list_free (uris);
		
		return FALSE;
	}

	g_list_free (uris);
	
	return TRUE;
}

gboolean
ev_attachment_open (EvAttachment *attachment,
		    GError      **error)
{

	gboolean                 retval = FALSE;
	GnomeVFSMimeApplication *default_app = NULL;

	if (!attachment->priv->app)
		default_app = gnome_vfs_mime_get_default_application (attachment->priv->mime_type);

	if (!default_app) {
		g_set_error (error,
			     EV_ATTACHMENT_ERROR,
			     0,
			     _("Couldn't open attachment '%s'"),
			     attachment->priv->name);
		
		return FALSE;
	}

	attachment->priv->app = default_app;
	
	if (attachment->priv->tmp_uri &&
	    g_file_test (attachment->priv->tmp_uri, G_FILE_TEST_EXISTS)) {
		retval = ev_attachment_launch_app (attachment, error);
	} else {
		gchar *uri, *filename;
		
		filename = g_build_filename (g_get_tmp_dir (), attachment->priv->name, NULL);
		uri = g_filename_to_uri (filename, NULL, NULL);

		if (ev_attachment_save (attachment, uri, error)) {
			if (attachment->priv->tmp_uri)
				g_free (attachment->priv->tmp_uri);
			attachment->priv->tmp_uri = g_strdup (filename);

			retval = ev_attachment_launch_app (attachment, error);
		}

		g_free (filename);
		g_free (uri);
	}

	return retval;
}