/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8 -*- */
/*
 * Implements hyperlink functionality for Djvu files.
 * Copyright (C) 2006 Pauli Virtanen <pav@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "djvu-document.h"
#include "djvu-links.h"
#include "djvu-document-private.h"
#include "ev-document-links.h"

#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>
#include <glib.h>
#include <string.h>

static gboolean number_from_miniexp(miniexp_t sexp, int *number)
{
	if (miniexp_numberp (sexp)) {
		*number = miniexp_to_int (sexp);
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean string_from_miniexp(miniexp_t sexp, const char **str)
{
	if (miniexp_stringp (sexp)) {
		*str = miniexp_to_str (sexp);
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean number_from_string_10(const gchar *str, guint64 *number)
{
	gchar *end_ptr;

	*number = g_ascii_strtoull(str, &end_ptr, 10);
	if (*end_ptr == '\0') {
		return TRUE;
	} else {
		return FALSE;
	}
}

static EvLinkDest *
get_djvu_link_dest (const DjvuDocument *djvu_document, const gchar *link_name, int base_page)
{
	guint64 page_num = 0;
	gchar *end_ptr;

	/* #pagenum, #+pageoffset, #-pageoffset */
	if (g_str_has_prefix (link_name, "#")) {
		if (base_page > 0 && g_str_has_prefix (link_name+1, "+")) {
			if (number_from_string_10 (link_name + 2, &page_num)) {
				return ev_link_dest_new_page (base_page + page_num);
			}
		} else if (base_page > 0 && g_str_has_prefix (link_name+1, "-")) {
			if (number_from_string_10 (link_name + 2, &page_num)) {
				return ev_link_dest_new_page (base_page - page_num);
			}
		} else {
			if (number_from_string_10 (link_name + 1, &page_num)) {
				return ev_link_dest_new_page (page_num - 1);
			}
		}
	} else {
		/* FIXME: component file identifiers */
	}

	return NULL;
}

static EvLinkAction *
get_djvu_link_action (const DjvuDocument *djvu_document, const gchar *link_name, int base_page)
{
	EvLinkDest *ev_dest = NULL;
	EvLinkAction *ev_action = NULL;

	ev_dest = get_djvu_link_dest (djvu_document, link_name, base_page);

	if (ev_dest) {
		ev_action = ev_link_action_new_dest (ev_dest);
	} else if (strstr(link_name, "://") != NULL) {
		/* It's probably an URI */
		ev_action = ev_link_action_new_external_uri (link_name);
	} else {
		/* FIXME: component file identifiers */
	}

	return ev_action;
}

/**
 * Builds the index GtkTreeModel from DjVu s-expr
 *
 * (bookmarks
 *   ("title1" "dest1"
 *     ("title12" "dest12"
 *       ... )
 *     ... )
 *   ("title2" "dest2"
 *     ... )
 *   ... )
 */
static void
build_tree (const DjvuDocument *djvu_document,
	    GtkTreeModel       *model,
	    GtkTreeIter        *parent,
	    miniexp_t           iter)
{
	const char *title, *link_dest;
	char *title_markup;

	EvLinkAction *ev_action = NULL;
	EvLink *ev_link = NULL;
	GtkTreeIter tree_iter;

	if (miniexp_car (iter) == miniexp_symbol ("bookmarks")) {
		/* The (bookmarks) cons */
		iter = miniexp_cdr (iter);
	} else if ( miniexp_length (iter) >= 2 ) {
		/* An entry */
		if (!string_from_miniexp (miniexp_car (iter), &title)) goto unknown_entry;
		if (!string_from_miniexp (miniexp_cadr (iter), &link_dest)) goto unknown_entry;

		title_markup = g_markup_escape_text (title, -1);
		ev_action = get_djvu_link_action (djvu_document, link_dest, -1);
		
		if (g_str_has_suffix (link_dest, ".djvu")) {
			/* FIXME: component file identifiers */
		} else if (ev_action) {
			ev_link = ev_link_new (title, ev_action);
			gtk_tree_store_append (GTK_TREE_STORE (model), &tree_iter, parent);
			gtk_tree_store_set (GTK_TREE_STORE (model), &tree_iter,
					    EV_DOCUMENT_LINKS_COLUMN_MARKUP, title_markup,
					    EV_DOCUMENT_LINKS_COLUMN_LINK, ev_link,
					    EV_DOCUMENT_LINKS_COLUMN_EXPAND, FALSE,
					    -1);
			g_object_unref (ev_link);
		} else {
			gtk_tree_store_append (GTK_TREE_STORE (model), &tree_iter, parent);
			gtk_tree_store_set (GTK_TREE_STORE (model), &tree_iter,
					    EV_DOCUMENT_LINKS_COLUMN_MARKUP, title_markup,
					    EV_DOCUMENT_LINKS_COLUMN_EXPAND, FALSE,
					    -1);
		}

		g_free (title_markup);
		
		iter = miniexp_cddr (iter);
		parent = &tree_iter;
	} else {
		goto unknown_entry;
	}

	for (; iter != miniexp_nil; iter = miniexp_cdr (iter)) {
		build_tree (djvu_document, model, parent, miniexp_car (iter));
	}
	return;

 unknown_entry:
	g_warning ("DjvuLibre error: Unknown entry in bookmarks");
	return;
}

static gboolean
get_djvu_hyperlink_area (ddjvu_pageinfo_t   *page_info,
			 miniexp_t           sexp,
			 EvLinkMapping      *ev_link_mapping)
{
	miniexp_t iter;
        ddjvu_pageinfo_t info;

	iter = sexp;
	
	if ((miniexp_car (iter) == miniexp_symbol ("rect") || miniexp_car (iter) == miniexp_symbol ("oval"))
	    && miniexp_length (iter) == 5) {
		/* FIXME: get bounding box for (oval) since Evince doesn't support shaped links */
		int minx, miny, width, height;

		iter = miniexp_cdr (iter);
		if (!number_from_miniexp (miniexp_car (iter), &minx)) goto unknown_link;
		iter = miniexp_cdr (iter);
		if (!number_from_miniexp (miniexp_car (iter), &miny)) goto unknown_link;
		iter = miniexp_cdr (iter);
		if (!number_from_miniexp (miniexp_car (iter), &width)) goto unknown_link;
		iter = miniexp_cdr (iter);
		if (!number_from_miniexp (miniexp_car (iter), &height)) goto unknown_link;

		ev_link_mapping->x1 = minx;
		ev_link_mapping->x2 = (minx + width);
		ev_link_mapping->y1 = (page_info->height - (miny + height));
		ev_link_mapping->y2 = (page_info->height - miny);
	} else if (miniexp_car (iter) == miniexp_symbol ("poly")
		   && miniexp_length (iter) >= 5 && miniexp_length (iter) % 2 == 1) {
		
		/* FIXME: get bounding box since Evince doesn't support shaped links */
		int minx = G_MAXINT, miny = G_MAXINT;
		int maxx = G_MININT, maxy = G_MININT;

		iter = miniexp_cdr(iter);
		while (iter != miniexp_nil) {
			int x, y;

			if (!number_from_miniexp (miniexp_car(iter), &x)) goto unknown_link;
			iter = miniexp_cdr (iter);
			if (!number_from_miniexp (miniexp_car(iter), &y)) goto unknown_link;
			iter = miniexp_cdr (iter);

			minx = MIN (minx, x);
			miny = MIN (miny, y);
			maxx = MAX (maxx, x);
			maxy = MAX (maxy, y);
		}

		ev_link_mapping->x1 = minx;
		ev_link_mapping->x2 = maxx;
		ev_link_mapping->y1 = (page_info->height - maxy);
		ev_link_mapping->y2 = (page_info->height - miny);
	} else {
		/* unknown */
		goto unknown_link;
	}

	return TRUE;
	
 unknown_link:
	g_warning("DjvuLibre error: Unknown hyperlink area %s", miniexp_to_name(miniexp_car(sexp)));
	return FALSE;
}

static EvLinkMapping *
get_djvu_hyperlink_mapping (DjvuDocument     *djvu_document,
			    int               page,
			    ddjvu_pageinfo_t *page_info,
			    miniexp_t         sexp)
{
	EvLinkMapping *ev_link_mapping = NULL;
	EvLinkAction *ev_action = NULL;
	miniexp_t iter;
	const char *url, *url_target, *comment;

	ev_link_mapping = g_new (EvLinkMapping, 1);

	iter = sexp;

	if (miniexp_car (iter) != miniexp_symbol ("maparea")) goto unknown_mapping;

	iter = miniexp_cdr(iter);

	if (miniexp_caar(iter) == miniexp_symbol("url")) {
		if (!string_from_miniexp (miniexp_cadr (miniexp_car (iter)), &url)) goto unknown_mapping;
		if (!string_from_miniexp (miniexp_caddr (miniexp_car (iter)), &url_target)) goto unknown_mapping;
	} else {
		if (!string_from_miniexp (miniexp_car(iter), &url)) goto unknown_mapping;
		url_target = NULL;
	}

	iter = miniexp_cdr (iter);
	if (!string_from_miniexp (miniexp_car(iter), &comment)) goto unknown_mapping;

	iter = miniexp_cdr (iter);
	if (!get_djvu_hyperlink_area (page_info, miniexp_car(iter), ev_link_mapping)) goto unknown_mapping;

	iter = miniexp_cdr (iter);
	/* FIXME: DjVu hyperlink attributes are ignored */

	ev_action = get_djvu_link_action (djvu_document, url, page);
	if (!ev_action) goto unknown_mapping;

	ev_link_mapping->link = ev_link_new (comment, ev_action);
			
	return ev_link_mapping;

 unknown_mapping:
	if (ev_link_mapping) g_free(ev_link_mapping);
	g_warning("DjvuLibre error: Unknown hyperlink %s", miniexp_to_name(miniexp_car(sexp)));
	return NULL;
}


gboolean
djvu_links_has_document_links (EvDocumentLinks *document_links)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document_links);
	miniexp_t outline;

	while ((outline = ddjvu_document_get_outline (djvu_document->d_document)) == miniexp_dummy)
		djvu_handle_events (djvu_document, TRUE);

	if (outline) {
		ddjvu_miniexp_release (djvu_document->d_document, outline);
		return TRUE;
	}
	
	return FALSE;
}

GList *
djvu_links_get_links (EvDocumentLinks *document_links,
                      gint             page,
                      double           scale_factor)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document_links);
	GList *retval = NULL;
	miniexp_t page_annotations = miniexp_nil;
	miniexp_t *hyperlinks = NULL, *iter = NULL;
	EvLinkMapping *ev_link_mapping;
        ddjvu_pageinfo_t page_info;

	while ((page_annotations = ddjvu_document_get_pageanno (djvu_document->d_document, page)) == miniexp_dummy)
		djvu_handle_events (djvu_document, TRUE);

	while (ddjvu_document_get_pageinfo (djvu_document->d_document, page, &page_info) < DDJVU_JOB_OK)
		djvu_handle_events(djvu_document, TRUE);

	if (page_annotations) {
		hyperlinks = ddjvu_anno_get_hyperlinks (page_annotations);
		if (hyperlinks) {
			for (iter = hyperlinks; *iter; ++iter) {
				ev_link_mapping = get_djvu_hyperlink_mapping (djvu_document, page, &page_info, *iter);
				if (ev_link_mapping) {
					ev_link_mapping->x1 *= scale_factor;
					ev_link_mapping->x2 *= scale_factor;
					ev_link_mapping->y1 *= scale_factor;
					ev_link_mapping->y2 *= scale_factor;
					retval = g_list_prepend (retval, ev_link_mapping);
				}
			}
			free (hyperlinks);
		}
		ddjvu_miniexp_release (djvu_document->d_document, page_annotations);
	}
	
	return retval;
}

EvLinkDest *
djvu_links_find_link_dest (EvDocumentLinks  *document_links,
                           const gchar      *link_name)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document_links);
	EvLinkDest *ev_dest = NULL;
	
	ev_dest = get_djvu_link_dest (DJVU_DOCUMENT (document_links), link_name, -1);

	if (!ev_dest) {
		g_warning ("DjvuLibre error: unknown link destination %s", link_name);
	}
	
	return ev_dest;
}

GtkTreeModel *
djvu_links_get_links_model (EvDocumentLinks *document_links)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document_links);
	GtkTreeModel *model = NULL;
	miniexp_t outline = miniexp_nil;

	while ((outline = ddjvu_document_get_outline (djvu_document->d_document)) == miniexp_dummy)
		djvu_handle_events (djvu_document, TRUE);

	if (outline) {
		model = (GtkTreeModel *) gtk_tree_store_new (EV_DOCUMENT_LINKS_COLUMN_NUM_COLUMNS,
							     G_TYPE_STRING,
							     G_TYPE_OBJECT,
							     G_TYPE_BOOLEAN);
		build_tree (djvu_document, model, NULL, outline);

		ddjvu_miniexp_release (djvu_document->d_document, outline);
	}
	
	return model;
}