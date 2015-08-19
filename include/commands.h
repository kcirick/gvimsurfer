/*
 * include/commands.h
 */

#ifndef COMMANDS_H
#define COMMANDS_H

typedef struct {
   gchar*      command;
   gboolean    (*function)(int, char**);
   Completion* (*completion)(gchar*);
} Command;


gboolean cmd_back(int, char**);
gboolean cmd_bookmark(int, char**);
gboolean cmd_forward(int, char**);
gboolean cmd_open(int, char**);
gboolean cmd_settings(int, char**);
gboolean cmd_pagemark(int, char**);
gboolean cmd_print(int, char**);
gboolean cmd_quickmark(int, char**);
gboolean cmd_quit(int, char**);
gboolean cmd_quitall(int, char**);
gboolean cmd_reload(int, char**);
gboolean cmd_saveas(int, char**);
gboolean cmd_load_script(int, char**);
gboolean cmd_session(int, char**);
gboolean cmd_stop(int, char**);
gboolean cmd_tabopen(int, char**);
gboolean cmd_winopen(int, char**);
gboolean cmd_write(int, char**);

#endif
