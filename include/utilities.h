/*
 * include/utilities.h
 */

#ifndef UTILITIES_H
#define UTILITIES_H

void        notify(gint, gchar*, gboolean, gint);

void        abort_input();
gchar*      build_proper_path(gchar*);
void        change_mode(int);
void        download_content(WebKitDownload*, char*);
gboolean    load_script(gchar*);
void        open_uri(WebKitWebView*, gchar*);
gboolean    read_configuration(gchar*);
void        run_script(char*, char**, char**);
gboolean    search_and_highlight(Argument*);
gboolean    sessionload(char*);
gboolean    sessionsave(char*);
void        set_proxy(gboolean);

#endif
