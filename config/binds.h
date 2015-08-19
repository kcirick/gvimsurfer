/*
 * config/binds.h
 */

#ifndef CONFIG_BINDS_H
#define CONFIG_BINDS_H

//--- Mouse buttons -----
Mouse mouse[] = {
  // mask,             button,   function,   mode,      argument 
  {0,                  1,        NULL,       NORMAL,    { 0,   NULL } },
};

//--- Keyboard shortcuts -----
#define CMASK GDK_CONTROL_MASK
#define SMASK GDK_SHIFT_MASK
#define CSMASK (CMASK | SMASK)

Shortcut shortcuts[] = {
  // mask,  regex        key,              function,            argument
  {0,       NULL,        GDK_Escape,       sc_abort,            { 0,                 NULL }},
  {CMASK,   NULL,        GDK_w,            sc_close_tab,        { 0,                 NULL }},
  {0,       NULL,        GDK_n,            sc_search,           { FORWARD,           NULL }},
  {0,       NULL,        GDK_N,            sc_search,           { BACKWARD,          NULL }},
  {0,       NULL,        GDK_slash,        sc_focus_inputbar,   { 0,                 "/" }},
  {0,       NULL,        GDK_colon,        sc_focus_inputbar,   { 0,                 ":" }},
  {0,       NULL,        GDK_o,            sc_focus_inputbar,   { 0,                 ":open " }},
  {0,       NULL,        GDK_O,            sc_focus_inputbar,   { APPEND_URL,        ":open " }},
  {0,       NULL,        GDK_t,            sc_focus_inputbar,   { 0,                 ":tabnew " }},
  {0,       NULL,        GDK_T,            sc_focus_inputbar,   { APPEND_URL,        ":tabnew " }},
  {0,       NULL,        GDK_w,            sc_focus_inputbar,   { 0,                 ":winnew " }},
  {0,       NULL,        GDK_W,            sc_focus_inputbar,   { APPEND_URL,        ":winnew " }},
  {CMASK,   0,           GDK_v,            sc_change_mode,      { VISUAL,            NULL }},
  {CMASK,   0,           GDK_z,            sc_change_mode,      { PASS_THROUGH,      NULL }},
  {CMASK,   0,           GDK_v,            sc_change_mode,      { PASS_THROUGH_NEXT, NULL }},
  {0,       0,           GDK_f,            sc_follow_link,      { -1,                NULL }},
  {0,       0,           GDK_F,            sc_follow_link,      { -2,                NULL }},
  {0,       "^g$",       GDK_h,            sc_go_home,          { 0,                 NULL }},
  {0,       "^g$",       GDK_H,            sc_go_home,          { NEW_TAB,           NULL }},
  {0,       NULL,        GDK_L,            sc_navigate,         { FORWARD,           NULL }},
  {0,       NULL,        GDK_H,            sc_navigate,         { BACKWARD,          NULL }},
  {0,       "^g$",       GDK_t,            sc_navigate_tabs,    { NEXT,              NULL }},
  {0,       "^g$",       GDK_T,            sc_navigate_tabs,    { PREVIOUS,          NULL }},
  {0,       "^[0-9]+g$", GDK_t,            sc_navigate_tabs,    { SPECIFIC,          NULL }},
  {0,       "^g$",       GDK_u,            sc_go_parent,        { 0,                 NULL }},
  {0,       "^[0-9]+g$", GDK_u,            sc_go_parent,        { 0,                 NULL }},
  {0,       0,           GDK_p,            sc_paste,            { 0,                 NULL }},
  {0,       0,           GDK_P,            sc_paste,            { NEW_TAB,           NULL }},
  {0,       0,           GDK_r,            sc_reload,           { 0,                 NULL }},
  {0,       0,           GDK_R,            sc_reload,           { BYPASS_CACHE,      NULL }},
  {0,       0,           GDK_u,            sc_reopen,           { 0,                 NULL }},
  {0,       "^[0-9]+%$", GDK_g,            sc_scroll,           { SPECIFIC,          NULL }},
  {0,       "^g$",       GDK_g,            sc_scroll,           { TOP,               NULL }},
  {0,       0,           GDK_G,            sc_scroll,           { BOTTOM,            NULL }},
  {0,       0,           GDK_Left,         sc_scroll,           { LEFT,              NULL }},
  {0,       0,           GDK_Up,           sc_scroll,           { UP,                NULL }},
  {0,       0,           GDK_Down,         sc_scroll,           { DOWN,              NULL }},
  {0,       0,           GDK_Right,        sc_scroll,           { RIGHT,             NULL }},
  {CMASK,   0,           GDK_f,            sc_scroll,           { FULL_DOWN,         NULL }},
  {CMASK,   0,           GDK_b,            sc_scroll,           { FULL_UP,           NULL }},
  {0,       0,           GDK_0,            sc_scroll,           { MAX_LEFT,          NULL }},
  {0,       0,           GDK_dollar,       sc_scroll,           { MAX_RIGHT,         NULL }},
  {CMASK,   0,           GDK_s,            sc_toggle_sourcecode,{ 0,                 NULL }},
  {CMASK,   0,           GDK_S,            sc_toggle_sourcecode,{ OPEN_EXTERNAL,     NULL }},
  {0,       0,           GDK_y,            sc_yank,             { 0,                 NULL }},
  {0,       0,           GDK_Y,            sc_yank,             { XA_CLIPBOARD,      NULL }},
  {0,       "^z$",       GDK_i,            sc_zoom,             { ZOOM_IN,           NULL }},
  {0,       "^z$",       GDK_o,            sc_zoom,             { ZOOM_OUT,          NULL }},
  {0,       "^z$",       GDK_0,            sc_zoom,             { ZOOM_ORIGINAL,     NULL }},
  {0,       0,           GDK_i,            sc_focus_input,      { 0,                 NULL }},
  {0,       "^[0-9]+q$", GDK_m,            sc_quickmark,        { 0,                 NULL }},
  {0,       "^[0-9]+p$", GDK_m,            sc_pagemark,         { 0,                 NULL }},
};

#endif
