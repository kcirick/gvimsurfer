/*
 * config/binds.h
 */

#ifndef CONFIG_BINDS_H
#define CONFIG_BINDS_H

//--- Mouse buttons -----
Mouse mouse[] = {
  // mask,             button,   function,   mode,      argument 
  {0,                  1,        NULL,       NORMAL,    { 0 } },
};

//--- Keyboard shortcuts -----
#define CMASK GDK_CONTROL_MASK
#define SMASK GDK_SHIFT_MASK
#define CSMASK (CMASK | SMASK)

Shortcut shortcuts[] = {
  // mask,  regex        key,              function,            argument
  {0,       NULL,        GDK_Escape,       sc_abort,            { 0 } },
  {CMASK,   NULL,        GDK_w,            sc_close_tab,        { 0 } },
  {0,       NULL,        GDK_n,            sc_search,           { .b=FORWARD  } },
  {0,       NULL,        GDK_N,            sc_search,           { .b=BACKWARD } },
  {0,       NULL,        GDK_slash,        sc_focus_inputbar,   { .data="/" } },
  {0,       NULL,        GDK_colon,        sc_focus_inputbar,   { .data=":" } },
  {0,       NULL,        GDK_o,            sc_focus_inputbar,   {             .data=":open "   } },
  {0,       NULL,        GDK_O,            sc_focus_inputbar,   { .b=APPEND,  .data=":open "   } },
  {0,       NULL,        GDK_t,            sc_focus_inputbar,   {             .data=":tabnew " } },
  {0,       NULL,        GDK_T,            sc_focus_inputbar,   { .b=APPEND,  .data=":tabnew " } },
  {0,       NULL,        GDK_w,            sc_focus_inputbar,   {             .data=":winnew " } },
  {0,       NULL,        GDK_W,            sc_focus_inputbar,   { .b=APPEND,  .data=":winnew " } },
  {CMASK,   0,           GDK_v,            sc_change_mode,      { .i=VISUAL            } },
  {CMASK,   0,           GDK_z,            sc_change_mode,      { .i=PASS_THROUGH      } },
  {CMASK,   0,           GDK_v,            sc_change_mode,      { .i=PASS_THROUGH_NEXT } },
  {0,       0,           GDK_f,            sc_follow_link,      { .i=-1 } },
  {0,       0,           GDK_F,            sc_follow_link,      { .i=-2 } },
  {0,       "^g$",       GDK_h,            sc_go_home,          { 0           } },
  {0,       "^g$",       GDK_H,            sc_go_home,          { .b=NEW_TAB  } },
  {0,       NULL,        GDK_L,            sc_navigate,         { .b=FORWARD  } },
  {0,       NULL,        GDK_H,            sc_navigate,         { .b=BACKWARD } },
  {0,       "^g$",       GDK_t,            sc_navigate_tabs,    { .i=NEXT     } },
  {0,       "^g$",       GDK_T,            sc_navigate_tabs,    { .i=PREVIOUS } },
  {0,       "^[0-9]+g$", GDK_t,            sc_navigate_tabs,    { .i=SPECIFIC } },
  {0,       "^g$",       GDK_u,            sc_go_parent,        { 0 } },
  {0,       "^[0-9]+g$", GDK_u,            sc_go_parent,        { 0 } },
  {0,       0,           GDK_p,            sc_paste,            { 0           } },
  {0,       0,           GDK_P,            sc_paste,            { .b=NEW_TAB  } },
  {0,       0,           GDK_r,            sc_reload,           { 0                 } },
  {0,       0,           GDK_R,            sc_reload,           { .b=BYPASS_CACHE   } },
  {0,       0,           GDK_u,            sc_reopen,           { 0 } },
  {0,       "^[0-9]+%$", GDK_g,            sc_scroll,           { .i=SPECIFIC    } },
  {0,       "^g$",       GDK_g,            sc_scroll,           { .i=TOP         } },
  {0,       0,           GDK_G,            sc_scroll,           { .i=BOTTOM      } },
  {0,       0,           GDK_Left,         sc_scroll,           { .i=LEFT        } },
  {0,       0,           GDK_Up,           sc_scroll,           { .i=UP          } },
  {0,       0,           GDK_Down,         sc_scroll,           { .i=DOWN        } },
  {0,       0,           GDK_Right,        sc_scroll,           { .i=RIGHT       } },
  {CMASK,   0,           GDK_f,            sc_scroll,           { .i=FULL_DOWN   } },
  {CMASK,   0,           GDK_b,            sc_scroll,           { .i=FULL_UP     } },
  {0,       0,           GDK_0,            sc_scroll,           { .i=MAX_LEFT    } },
  {0,       0,           GDK_dollar,       sc_scroll,           { .i=MAX_RIGHT   } },
  {CMASK,   0,           GDK_s,            sc_toggle_sourcecode,{ 0           } },
  {CMASK,   0,           GDK_S,            sc_toggle_sourcecode,{ .b=EXTERNAL } },
  {0,       0,           GDK_y,            sc_copy_uri,         { 0 } },
  {0,       "^z$",       GDK_i,            sc_zoom,             { .i=ZOOM_IN        } },
  {0,       "^z$",       GDK_o,            sc_zoom,             { .i=ZOOM_OUT       } },
  {0,       "^z$",       GDK_0,            sc_zoom,             { .i=ZOOM_ORIGINAL  } },
  {0,       0,           GDK_i,            sc_focus_input,      { 0 } },
  {0,       "^[0-9]+q$", GDK_m,            sc_quickmark,        { 0 } },
  {0,       "^[0-9]+p$", GDK_m,            sc_pagemark,         { 0 } },
};

#endif
