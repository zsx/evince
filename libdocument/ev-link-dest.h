/* this file is part of evince, a gnome document viewer
 *
 *  Copyright (C) 2006 Carlos Garcia Campos <carlosgc@gnome.org>
 *  Copyright (C) 2005 Red Hat, Inc.
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

#ifndef EV_LINK_DEST_H
#define EV_LINK_DEST_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _EvLinkDest        EvLinkDest;
typedef struct _EvLinkDestClass   EvLinkDestClass;
typedef struct _EvLinkDestPrivate EvLinkDestPrivate;

#define EV_TYPE_LINK_DEST              (ev_link_dest_get_type())
#define EV_LINK_DEST(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), EV_TYPE_LINK_DEST, EvLinkDest))
#define EV_LINK_DEST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EV_TYPE_LINK_DEST, EvLinkDestClass))
#define EV_IS_LINK_DEST(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), EV_TYPE_LINK_DEST))
#define EV_IS_LINK_DEST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EV_TYPE_LINK_DEST))
#define EV_LINK_DEST_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), EV_TYPE_LINK_DEST, EvLinkDestClass))

#define EV_TYPE_LINK_DEST_TYPE         (ev_link_dest_type_get_type ())

typedef enum {
	EV_LINK_DEST_TYPE_PAGE,
	EV_LINK_DEST_TYPE_XYZ,
	EV_LINK_DEST_TYPE_FIT,
	EV_LINK_DEST_TYPE_FITH,
	EV_LINK_DEST_TYPE_FITV,
	EV_LINK_DEST_TYPE_FITR,
	EV_LINK_DEST_TYPE_NAMED,
	EV_LINK_DEST_TYPE_PAGE_LABEL,
	EV_LINK_DEST_TYPE_UNKNOWN
} EvLinkDestType; 

GType           ev_link_dest_type_get_type  (void) G_GNUC_CONST;
GType           ev_link_dest_get_type       (void) G_GNUC_CONST;

EvLinkDestType  ev_link_dest_get_dest_type  (EvLinkDest  *self);
gint            ev_link_dest_get_page       (EvLinkDest  *self);
gdouble         ev_link_dest_get_top        (EvLinkDest  *self);
gdouble         ev_link_dest_get_left       (EvLinkDest  *self);
gdouble         ev_link_dest_get_bottom     (EvLinkDest  *self);
gdouble         ev_link_dest_get_right      (EvLinkDest  *self);
gdouble         ev_link_dest_get_zoom       (EvLinkDest  *self);
const gchar    *ev_link_dest_get_named_dest (EvLinkDest  *self);
const gchar    *ev_link_dest_get_page_label (EvLinkDest  *self);

EvLinkDest     *ev_link_dest_new_page       (gint         page);
EvLinkDest     *ev_link_dest_new_xyz        (gint         page,
					     gdouble      left,
					     gdouble      top,
					     gdouble      zoom);
EvLinkDest     *ev_link_dest_new_fit        (gint         page);
EvLinkDest     *ev_link_dest_new_fith       (gint         page,
					     gdouble      top);
EvLinkDest     *ev_link_dest_new_fitv       (gint         page,
					     gdouble      left);
EvLinkDest     *ev_link_dest_new_fitr       (gint         page,
					     gdouble      left,
					     gdouble      bottom,
					     gdouble      right,
					     gdouble      top);
EvLinkDest     *ev_link_dest_new_named      (const gchar *named_dest);
EvLinkDest     *ev_link_dest_new_page_label (const gchar *page_label);

G_END_DECLS

#endif /* EV_LINK_DEST_H */