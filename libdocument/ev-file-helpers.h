/*
 *  Copyright (C) 2002 Jorn Baayen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifndef EPHY_FILE_HELPERS_H
#define EPHY_FILE_HELPERS_H

#include <glib.h>

G_BEGIN_DECLS

const gchar *ev_dot_dir               (void);

const gchar *ev_tmp_dir               (void);

void         ev_file_helpers_init     (void);

void         ev_file_helpers_shutdown (void);

gchar*       ev_tmp_filename          (const char *prefix);

gboolean     ev_xfer_uri_simple       (const char *from,
				       const char *to,
				       GError     **error);

G_END_DECLS

#endif /* EPHY_FILE_HELPERS_H */