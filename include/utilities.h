/*
 * include/utilities.h
 */

#ifndef UTILITIES_H
#define UTILITIES_H

gchar* search_item;
gboolean search_item_changed;

gchar* proxy;

void say(int, const char*, int);
void open_uri(WebKitWebView*, char*);

void init_search_items();
gboolean search_and_highlight(Argument*);
gboolean sessionload(char*);
gboolean sessionsave(char*);

void change_mode(int);
void notify(int, char*);
char* reference_to_string(JSContextRef, JSValueRef);
void run_script(char*, char**, char**);

void download_content(WebKitDownload*, char*);
void load_all_scripts();
char* read_file(const char*);
gboolean read_configuration(char*);

#endif
