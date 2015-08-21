/*
 * src/shortcuts.c
 */

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <gdk/gdkkeysyms.h>

#include "include/gvimsurfer.h"
#include "include/shortcuts.h"
#include "include/client.h"
#include "include/utilities.h"
#include "include/completion.h"

#include "config/config.h"

//--- Shortcuts -----
void sc_abort(Argument* argument) {

   // Clear hints
   gchar* cmd = "clear()";
   run_script(cmd, NULL, NULL);

   // Stop loading website
   if(webkit_web_view_get_progress(GET_CURRENT_TAB()) == 1.0)
      webkit_web_view_stop_loading(GET_CURRENT_TAB());

   // Set back to normal mode
   change_mode(NORMAL);

   // Hide inputbar
   set_inputbar_visibility(HIDE);

   // Unmark search results
   webkit_web_view_unmark_text_matches(GET_CURRENT_TAB());

   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
}

void sc_change_mode(Argument* argument) {
   if(argument)
      change_mode(argument->i);
}

void sc_close_tab(Argument* argument) {
   close_tab(gtk_notebook_get_current_page(Client.UI.webview));
}

void sc_copy_uri(Argument* argument) {
   gchar* uri = (gchar*) webkit_web_view_get_uri(GET_CURRENT_TAB());
   gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), uri, -1);

   notify(INFO, g_strdup_printf("Copied %s", uri), FALSE, -1);
}

void sc_focus_input(Argument* argument){
   gchar *value=NULL, *message = NULL;
   run_script("focus_input()", &value, &message);

   if(value && strncmp(value, "INSERT", 7)==0){
      change_mode(INSERT);
      gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB()));
   } 
}

void sc_focus_inputbar(Argument* argument) {
   if(argument->data) {
      gchar* data = argument->data;
      if(argument->b)
         data = g_strdup_printf("%s%s", data, webkit_web_view_get_uri(GET_CURRENT_TAB()));
      else
         data = g_strdup(data);

      gtk_entry_set_text(Client.UI.inputbar, data);
      g_free(data);

      /* we save the X clipboard that will be clear by "grab_focus" */
      gchar* x_clipboard_text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

      gtk_widget_grab_focus(GTK_WIDGET(Client.UI.inputbar));
      gtk_editable_set_position(GTK_EDITABLE(Client.UI.inputbar), -1);

      if (x_clipboard_text != NULL) {
         /* we reset the X clipboard with saved text */
         gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), x_clipboard_text, -1);

         g_free(x_clipboard_text);
      }
   }

   set_inputbar_visibility(TOGGLE);
}

void sc_follow_link(Argument* argument) {
   static gboolean follow_links = FALSE;
   static int      open_mode    = -1;
   GdkEventKey *key = (GdkEventKey*)argument->data;

   // update open mode
   if(argument->i < 0)
      open_mode = argument->i;

   /* show all links */
   if(!follow_links || Client.Global.mode != FOLLOW) {
      run_script("show_hints()", NULL, NULL);
      change_mode(FOLLOW);
      follow_links = TRUE;
      return;
   }

   char* value = NULL;
   char* cmd   = NULL;

   if (argument && argument->i == 10)
      cmd = g_strdup("get_active()");
   else if (key && key->keyval == GDK_Tab) {
      if ( key->state & GDK_CONTROL_MASK)
         cmd = g_strdup("focus_prev()");
      else
         cmd = g_strdup("focus_next()");
   } else if(key){
      char* keyval = gdk_keyval_name(key->keyval);
      cmd = g_strdup_printf("update_hints(\"%s\")", keyval);
   }

   run_script(cmd, &value, NULL);
   g_free(cmd);

   if(value && strcmp(value, "undefined")) {
      if(open_mode == -1)  open_uri(GET_CURRENT_TAB(), value);
      else                 create_tab(value, TRUE);

      sc_abort(NULL);
   }
}

void sc_go_home(Argument* argument) {
   if(argument->b)   create_tab(home_page, FALSE);
   else              open_uri(GET_CURRENT_TAB(), home_page);
}

void sc_go_parent(Argument* argument) {
   char* current_uri = (char*) webkit_web_view_get_uri(GET_CURRENT_TAB());
   if(!current_uri)      return;

   /* calcuate root */
   int   o = g_str_has_prefix(current_uri, "https://") ? 8 : 7;
   int  rl = 0;
   char* r = current_uri + o;

   while(r && *r != '/') 
      rl++, r++;

   char* root = g_strndup(current_uri, o + rl + 1);
   char* buffer = Client.Global.buffer->str;

   /* go to the root of the website */
   if(!strcmp(buffer, "g"))
      open_uri(GET_CURRENT_TAB(), root);
   else {
      int count = 1;

      if(strlen(buffer) > 2)
         count = atoi(g_strndup(buffer, strlen(buffer) - 2));

      if(count <= 0) count = 1;

      char* directories = g_strndup(current_uri + strlen(root), strlen(current_uri) - strlen(root));

      if(strlen(directories) <= 0)
         open_uri(GET_CURRENT_TAB(), root);
      else{
         gchar **tokens = g_strsplit(directories, "/", -1);
         int     length = g_strv_length(tokens) - 1;

         GString* tmp = g_string_new("");

         int i;
         for(i = 0; i < length - count; i++)
            g_string_append(tmp, tokens[i]);

         char* new_uri = g_strconcat(root, tmp->str, NULL);
         open_uri(GET_CURRENT_TAB(), new_uri);

         g_free(new_uri);
         g_string_free(tmp, TRUE);
         g_strfreev(tokens);
      }
      g_free(directories);
   }
   g_free(root);
}

void sc_navigate(Argument* argument) {
   webkit_web_view_go_back_or_forward(GET_CURRENT_TAB(), argument->b?1:-1);
}

void sc_navigate_tabs(Argument* argument) {
   int current_tab     = gtk_notebook_get_current_page(Client.UI.webview);
   int number_of_tabs  = gtk_notebook_get_n_pages(Client.UI.webview);
   int step            = argument->i==PREVIOUS ? -1 : 1;

   int new_tab = (current_tab + step) % number_of_tabs;

   if(argument->i == SPECIFIC) {
      gchar* buffer;
      if (argument->data)  buffer = argument->data;
      else                 buffer = Client.Global.buffer->str;

      new_tab = get_int_from_buffer(buffer) - 1;
   }

   gtk_notebook_set_current_page(Client.UI.webview, new_tab);
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));

   update_client(new_tab);
}

void sc_pagemark(Argument* argument) {
   gint id = get_int_from_buffer(Client.Global.buffer->str);

   GList* list;
   for(list = Client.Global.pagemarks; list; list = g_list_next(list)) {
      PMark* pmark = (PMark*) list->data;

      if(pmark->id == id) {
         gtk_notebook_set_current_page(Client.UI.webview, pmark->tab_id);
         GtkAdjustment* adjustment;
         adjustment = gtk_scrolled_window_get_vadjustment(GET_CURRENT_TAB_WIDGET());
         gtk_adjustment_set_value(adjustment, pmark->vadjustment);
         adjustment = gtk_scrolled_window_get_hadjustment(GET_CURRENT_TAB_WIDGET());
         gtk_adjustment_set_value(adjustment, pmark->hadjustment);
         webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), pmark->zoom_level);
         update_client(gtk_notebook_get_current_page(Client.UI.webview));
         return;
      }
   }
}

void sc_paste(Argument* argument) {
   gchar* text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

   if(argument->b)   create_tab(text, FALSE);
   else              open_uri(GET_CURRENT_TAB(), text);

   g_free(text);
}

void sc_quickmark(Argument* argument) {
   gint id = get_int_from_buffer(Client.Global.buffer->str);

   GList* list;
   for(list = Client.Global.quickmarks; list; list = g_list_next(list)) {
      QMark* qmark = (QMark*) list->data;

      if(qmark->id == id) {
         open_uri(GET_CURRENT_TAB(), qmark->uri);
         return;
      }
   }
}

void sc_reload(Argument* argument) {
   if(argument->b)
      webkit_web_view_reload_bypass_cache(GET_CURRENT_TAB());
   else
      webkit_web_view_reload(GET_CURRENT_TAB());
}

void sc_reopen(Argument* argument) {

   if(Client.Global.last_closed) {
      create_tab(Client.Global.last_closed, FALSE);

      Client.Global.last_closed = NULL; 
   }
}

void sc_run_script(Argument* argument) {
   if(argument->data)
      run_script(argument->data, NULL, NULL);
}

void sc_scroll(Argument* argument) {
   GtkAdjustment* adjustment;

   if( (argument->i == LEFT) || (argument->i == RIGHT) || (argument->i == MAX_LEFT) || (argument->i == MAX_RIGHT) )
      adjustment = gtk_scrolled_window_get_hadjustment(GET_CURRENT_TAB_WIDGET());
   else
      adjustment = gtk_scrolled_window_get_vadjustment(GET_CURRENT_TAB_WIDGET());

   gdouble view_size  = gtk_adjustment_get_page_size(adjustment);
   gdouble value      = gtk_adjustment_get_value(adjustment);
   gdouble max        = gtk_adjustment_get_upper(adjustment) - view_size;

   if(argument->i == SPECIFIC){
      char* buffer = Client.Global.buffer->str;
      int number     = atoi(g_strndup(buffer, strlen(buffer) - 1));
      int percentage = (number < 0) ? 0 : (number > 100) ? 100 : number;
      gdouble value  = (max / 100.0f) * (float) percentage;

      gtk_adjustment_set_value(adjustment, value);
   } else if(argument->i == FULL_UP)
      gtk_adjustment_set_value(adjustment, (value - view_size) < 0 ? 0 : (value - view_size));
   else if(argument->i == FULL_DOWN)
      gtk_adjustment_set_value(adjustment, (value + view_size) > max ? max : (value + view_size));
   else if((argument->i == LEFT) || (argument->i == UP))
      gtk_adjustment_set_value(adjustment, (value - scroll_step) < 0 ? 0 : (value - scroll_step));
   else if(argument->i == TOP || argument->i == MAX_LEFT)
      gtk_adjustment_set_value(adjustment, 0);
   else if(argument->i == BOTTOM || argument->i == MAX_RIGHT)
      gtk_adjustment_set_value(adjustment, max);
   else
      gtk_adjustment_set_value(adjustment, (value + scroll_step) > max ? max : (value + scroll_step));
}

void sc_search(Argument* argument) {
   search_and_highlight(argument->b, argument->data);
}

void sc_toggle_sourcecode(Argument* argument) {
   gchar* uri    = (gchar*) webkit_web_view_get_uri(GET_CURRENT_TAB());

   if(argument->b) {
      char* command = g_strdup_printf("%s %s", external_editor, uri);

      g_spawn_command_line_async(command, NULL);
      g_free(command);
   } else {
      gboolean is_vsm = webkit_web_view_get_view_source_mode(GET_CURRENT_TAB());
      webkit_web_view_set_view_source_mode(GET_CURRENT_TAB(), !is_vsm);

      open_uri(GET_CURRENT_TAB(), uri);
   }
}

void sc_zoom(Argument* argument) {
   float zoom_level = webkit_web_view_get_zoom_level(GET_CURRENT_TAB());
   char* buffer = Client.Global.buffer->str;

   if(argument->i == ZOOM_IN)
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), zoom_level + (float) (zoom_step / 100));
   else if(argument->i == ZOOM_OUT)
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), zoom_level - (float) (zoom_step / 100));
   else if(argument->i == ZOOM_ORIGINAL)
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), 1.0f);
   else if(argument->i == SPECIFIC) {
      char* number = g_strndup(buffer, strlen(buffer) - 1);
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), (float) (atoi(number) / 100));
      g_free(number);
   }
}

