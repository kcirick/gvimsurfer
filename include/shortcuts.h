/*
 * include/shortcuts.h
 */

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

typedef struct {
   unsigned int   mask;
   char*          regex;
   unsigned int   key;
   void           (*function)(Argument*);
   Argument       argument;
} Shortcut;

typedef struct {
   unsigned int   mask;
   unsigned int   key;
   void           (*function)(Argument*);
   Argument       argument;
} InputbarShortcut;

typedef struct {
  unsigned int    mask;
  unsigned int    button;
  void            (*function)(Argument*);
  int             mode;
  Argument        argument;
} Mouse;


void sc_abort(Argument*);
void sc_change_mode(Argument*);
void sc_close_tab(Argument*);
void sc_focus_input(Argument*);
void sc_focus_inputbar(Argument*);
void sc_follow_link(Argument*);
void sc_go_home(Argument*);
void sc_go_parent(Argument*);
void sc_navigate(Argument*);
void sc_navigate_tabs(Argument*);
void sc_pagemark(Argument*);
void sc_paste(Argument*);
void sc_quickmark(Argument*);
void sc_quit(Argument*);
void sc_reload(Argument*);
void sc_reopen(Argument*);
void sc_run_script(Argument*);
void sc_scroll(Argument*);
void sc_search(Argument*);
void sc_toggle_sourcecode(Argument*);
void sc_yank(Argument*);
void sc_zoom(Argument*);

void isc_abort(Argument*);
void isc_completion(Argument*);

#endif

