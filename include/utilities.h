/*
 * include/utilities.h
 */

#ifndef UTILITIES_H
#define UTILITIES_H

void        say(int, const char*, int);
void        notify(int, char*);

void        change_mode(int);
void        download_content(WebKitDownload*, char*);
void        load_all_scripts();
void        open_uri(WebKitWebView*, char*);
gboolean    read_configuration(char*);
char*       reference_to_string(JSContextRef, JSValueRef);
void        run_script(char*, char**, char**);
gboolean    search_and_highlight(Argument*);
gboolean    sessionload(char*);
gboolean    sessionsave(char*);
void        set_proxy(gboolean);

#endif
