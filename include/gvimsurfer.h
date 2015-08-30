/*
 * include/vimsurfer.h
 */

#ifndef VIMSURFER_H
#define VIMSURFER_H

//--- Macros -----
#define LENGTH(x) sizeof(x)/sizeof((x)[0])
#define ALL_MASK (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)

#define GET_CURRENT_TAB_WIDGET() GET_NTH_TAB_WIDGET(gtk_notebook_get_current_page(Client.UI.webview))
#define GET_NTH_TAB_WIDGET(n) GTK_SCROLLED_WINDOW(gtk_notebook_get_nth_page(Client.UI.webview, n))
#define GET_CURRENT_TAB() GET_NTH_TAB(gtk_notebook_get_current_page(Client.UI.webview))
#define GET_NTH_TAB(n) GET_WEBVIEW(gtk_notebook_get_nth_page(Client.UI.webview, n))
#define GET_WEBVIEW(x) WEBKIT_WEB_VIEW(gtk_bin_get_child(GTK_BIN(x)))

enum mode {
   NORMAL             = 1 << 0,
   INSERT             = 1 << 1,
   VISUAL             = 1 << 2,
   FOLLOW             = 1 << 3,
   PASS_THROUGH       = 1 << 4,
   PASS_THROUGH_NEXT  = 1 << 5,
   ALL                = 0x5fffffff
};

#define FORWARD      TRUE
#define BACKWARD     FALSE
#define EXTERNAL     TRUE
#define APPEND       TRUE
#define NEW_TAB      TRUE
#define BYPASS_CACHE TRUE
enum { 
   SPECIFIC,
   INFO, WARNING, ERROR, DEBUG,
   TOGGLE, SHOW, HIDE, HIGHLIGHT,
   PREVIOUS_GROUP, PREVIOUS, NEXT, NEXT_GROUP,
   ZOOM_ORIGINAL, ZOOM_IN, ZOOM_OUT,
   TOP, BOTTOM, UP, DOWN, LEFT, RIGHT, FULL_UP, FULL_DOWN, MAX_LEFT, MAX_RIGHT
};

typedef struct {
   gboolean b;
   gint     i;
   void*    data;
} Argument;

typedef struct {
   gchar*   name;
   gchar*   uri;
} SearchEngine;

typedef struct {
   gchar*   path;
   gchar*   content;
} Script;

typedef struct {
   gchar*   name;
   gchar*   uris;
} Session;

typedef struct {
	gchar*   name;
	void*    variable;
	gchar*   webkitvar;
   gchar    type;
} Setting;

typedef struct {
   WebKitWebView  *wv;
   GList          *pagemarks;
} Page;
   
typedef struct {
   gint     id;
   gdouble  hadjustment;
   gdouble  vadjustment;
   gfloat   zoom_level;
} PMark;

typedef struct {
   gint     id;
   gchar*   uri;
} QMark;

//--- variables -----
gboolean private_browsing;

#endif
