/*
 * include/utilities.h
 */

#ifndef UTILITIES_H
#define UTILITIES_H

void        notify(int, char*, int);

void        change_mode(int);
void        download_content(WebKitDownload*, char*);
void        open_uri(WebKitWebView*, char*);
gboolean    read_configuration(char*);
char*       reference_to_string(JSContextRef, JSValueRef);
void        run_script(char*, char**, char**);
gboolean    search_and_highlight(Argument*);
gboolean    sessionload(char*);
gboolean    sessionsave(char*);
void        set_proxy(gboolean);

#endif
