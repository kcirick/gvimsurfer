/*
 * include/utilities.h
 */

#ifndef UTILITIES_H
#define UTILITIES_H

void        notify(gint, gchar*);
void        say(gint, gchar*, gint);

void        clear_input();
gchar*      build_proper_path(gchar*);
void        change_mode(int);
gboolean    download_content(WebKitDownload*, const gchar*);
gint        get_int_from_buffer(gchar*);
gboolean    load_script(gchar*);
void        open_uri(WebKitWebView*, const gchar*);
gboolean    read_configuration(gchar*);
void        run_script(char*, char**, char**);
void        search_and_highlight(gboolean, gchar*);
gboolean    sessionload(char*);
gboolean    sessionsave(char*);
void        set_proxy(gboolean);
Page*       get_current_page();

#endif
