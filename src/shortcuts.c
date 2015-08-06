/*
 * src/shortcuts.c
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <gdk/gdkkeysyms.h>

#include "include/gvimsurfer.h"
#include "include/shortcuts.h"
#include "include/client.h"
#include "include/utilities.h"
#include "include/completion.h"
#include "include/commands.h"

#include "config/config.h"
#include "config/commands.h"

//--- Shortcuts -----
void sc_abort(Argument* argument) {
   // Clear buffer
   if(Client.Global.buffer) {
      g_string_free(Client.Global.buffer, TRUE);
      Client.Global.buffer = NULL;
   }

   // Clear hints
   gchar* cmd = "clear()";
   run_script(cmd, NULL, NULL);

   // Stop loading website
   if(webkit_web_view_get_progress(GET_CURRENT_TAB()) == 1.0)
      cmd_stop(0, NULL);

   // Set back to normal mode
   change_mode(NORMAL);

   // Hide inputbar
   gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));
   gtk_widget_show(GTK_WIDGET(Client.UI.statusbar));

   // Unmark search results
   webkit_web_view_unmark_text_matches(GET_CURRENT_TAB());

   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
}

void sc_change_mode(Argument* argument) {
   if(argument)
      change_mode(argument->n);
}

void sc_close_tab(Argument* argument) {
   gint current_tab = gtk_notebook_get_current_page(Client.UI.webview);
   close_tab(current_tab);
}

void sc_focus_input(Argument* argument){
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB()));

   change_mode(INSERT);
}

void sc_focus_inputbar(Argument* argument) {
   if(argument->data) {
      char* data = argument->data;

      if(argument->n == APPEND_URL)
         data = g_strdup_printf("%s%s", data, webkit_web_view_get_uri(GET_CURRENT_TAB()));
      else
         data = g_strdup(data);

      notify(INFO, data);
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

   if(!(GTK_WIDGET_VISIBLE(GTK_WIDGET(Client.UI.inputbar)))){
      gtk_widget_show(GTK_WIDGET(Client.UI.inputbar));
      gtk_widget_hide(GTK_WIDGET(Client.UI.statusbar));
   }
}

void sc_follow_link(Argument* argument) {
   static gboolean follow_links = FALSE;
   static int      open_mode    = -1;
   GdkEventKey *key = (GdkEventKey*)argument->data;

   // update open mode
   if(argument->n < 0)
      open_mode = argument->n;

   /* show all links */
   if(!follow_links || Client.Global.mode != FOLLOW) {
      run_script("show_hints()", NULL, NULL);
      change_mode(FOLLOW);
      follow_links = TRUE;
      return;
   }

   char* value = NULL;
   char* cmd   = NULL;

   if (argument && argument->n == 10)
      cmd = g_strdup("get_active()");
   else if (key && key->keyval == GDK_Tab) {
      if ( key->state & GDK_CONTROL_MASK)
         cmd = g_strdup("focus_prev()");
      else
         cmd = g_strdup("focus_next()");
   } else if(Client.Global.buffer && Client.Global.buffer->len > 0)
      cmd = g_strconcat("update_hints(\"", Client.Global.buffer->str, "\")", NULL);

   run_script(cmd, &value, NULL);
   g_free(cmd);

   if(value && strcmp(value, "undefined")) {
      if(open_mode == -1)  open_uri(GET_CURRENT_TAB(), value);
      else                 create_tab(value, TRUE);

      sc_abort(NULL);
   }
}

void sc_go_home(Argument* argument) {
   if(argument->n == NEW_TAB)
      create_tab(home_page, FALSE);
   else
      open_uri(GET_CURRENT_TAB(), home_page);
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
   int increment = argument->data ? atoi(argument->data) : 1;
   if(argument->n == BACKWARD)   increment *= -1;

   webkit_web_view_go_back_or_forward(GET_CURRENT_TAB(), increment);
}

void sc_navigate_tabs(Argument* argument) {
   int current_tab     = gtk_notebook_get_current_page(Client.UI.webview);
   int number_of_tabs  = gtk_notebook_get_n_pages(Client.UI.webview);
   int step            = 1;
   char* buffer;

   if(argument->n == PREVIOUS) step = -1;

   int new_tab = (current_tab + step) % number_of_tabs;

   if(argument->n == SPECIFIC) {
      if (argument->data)
         buffer = argument->data;
      else
         buffer = Client.Global.buffer->str;

      int digit_end = 0;
      while(g_ascii_isdigit(buffer[digit_end]))
         digit_end = digit_end + 1;

      char* number = g_strndup(buffer, digit_end);
      new_tab      = atoi(number) - 1;
      g_free(number);
   }

   gtk_notebook_set_current_page(Client.UI.webview, new_tab);
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));

   update_client();
}

void sc_pagemark(Argument* argument) {

   char* buffer = Client.Global.buffer->str;

   int digit_end = 0;
   while(g_ascii_isdigit(buffer[digit_end]))
      digit_end = digit_end + 1;

   char* number = g_strndup(buffer, digit_end);
   int id = atoi(number);
   g_free(number);

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
         update_client();
         return;
      }
   }
}

void sc_paste(Argument* argument) {
   gchar* text;
   text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

   if(argument->n == NEW_TAB)
      create_tab(text, FALSE);
   else
      open_uri(GET_CURRENT_TAB(), text);

   g_free(text);
}

void sc_quickmark(Argument* argument) {
   char* buffer = Client.Global.buffer->str;

   int digit_end = 0;
   while(g_ascii_isdigit(buffer[digit_end]))
      digit_end = digit_end + 1;

   char* number = g_strndup(buffer, digit_end);
   int id = atoi(number);
   g_free(number);

   GList* list;
   for(list = Client.Global.quickmarks; list; list = g_list_next(list)) {
      QMark* qmark = (QMark*) list->data;

      if(qmark->id == id) {
         open_uri(GET_CURRENT_TAB(), qmark->uri);
         update_client();
         return;
      }
   }
}

void sc_quit(Argument* argument) {
   cmd_quitall(0, NULL);
}

void sc_reload(Argument* argument) {
   if(argument->n == BYPASS_CACHE)
      webkit_web_view_reload_bypass_cache(GET_CURRENT_TAB());
   else
      webkit_web_view_reload(GET_CURRENT_TAB());
}

void sc_reopen(Argument* argument) {
   GList *last_closed = g_list_first(Client.Global.last_closed);

   if(last_closed) {
      if(argument && argument->n)
         create_tab(last_closed->data, TRUE);
      else
         create_tab(last_closed->data, FALSE);

      Client.Global.last_closed = g_list_remove(Client.Global.last_closed, last_closed->data);
   }
}

void sc_run_script(Argument* argument) {
   if(argument->data)
      run_script(argument->data, NULL, NULL);
}

void sc_scroll(Argument* argument) {
   GtkAdjustment* adjustment;

   if( (argument->n == LEFT) || (argument->n == RIGHT) || (argument->n == MAX_LEFT) || (argument->n == MAX_RIGHT) )
      adjustment = gtk_scrolled_window_get_hadjustment(GET_CURRENT_TAB_WIDGET());
   else
      adjustment = gtk_scrolled_window_get_vadjustment(GET_CURRENT_TAB_WIDGET());

   gdouble view_size  = gtk_adjustment_get_page_size(adjustment);
   gdouble value      = gtk_adjustment_get_value(adjustment);
   gdouble max        = gtk_adjustment_get_upper(adjustment) - view_size;

   if(argument->n == SPECIFIC){
      char* buffer = Client.Global.buffer->str;
      int number     = atoi(g_strndup(buffer, strlen(buffer) - 1));
      int percentage = (number < 0) ? 0 : (number > 100) ? 100 : number;
      gdouble value  = (max / 100.0f) * (float) percentage;

      gtk_adjustment_set_value(adjustment, value);
   } else if(argument->n == FULL_UP)
      gtk_adjustment_set_value(adjustment, (value - view_size) < 0 ? 0 : (value - view_size));
   else if(argument->n == FULL_DOWN)
      gtk_adjustment_set_value(adjustment, (value + view_size) > max ? max : (value + view_size));
   else if((argument->n == LEFT) || (argument->n == UP))
      gtk_adjustment_set_value(adjustment, (value - scroll_step) < 0 ? 0 : (value - scroll_step));
   else if(argument->n == TOP || argument->n == MAX_LEFT)
      gtk_adjustment_set_value(adjustment, 0);
   else if(argument->n == BOTTOM || argument->n == MAX_RIGHT)
      gtk_adjustment_set_value(adjustment, max);
   else
      gtk_adjustment_set_value(adjustment, (value + scroll_step) > max ? max : (value + scroll_step));
}

void sc_search(Argument* argument) {
   search_and_highlight(argument);
}

void sc_toggle_proxy(Argument* argument) {
   // TODO Properly implement this function
   /*
   static gboolean enable = FALSE;

   if(enable) {
      g_object_set(Client.Global.soup_session, "proxy-uri", NULL, NULL);

      notify(INFO, "Proxy deactivated");
   } else {
      char* purl = (proxy) ? proxy : (char*) g_getenv("http_proxy");
      if(!purl)   purl = (char*)g_getenv("HTTP_PROXY");

      if(!purl) {
         say(WARNING, "No proxy defined", -1);
         return;
      }

      char* uri = strstr(purl, "://") ? g_strdup(purl) : g_strconcat("http://", purl, NULL);
      SoupURI* proxy_uri = soup_uri_new(uri);

      g_object_set(Client.Global.soup_session, "proxy-uri", proxy_uri, NULL);

      soup_uri_free(proxy_uri);
      g_free(uri);

      notify(INFO, "Proxy activated");
   }

   enable = !enable;
   */
}

void sc_toggle_sourcecode(Argument* argument) {
   gchar* uri    = (gchar*) webkit_web_view_get_uri(GET_CURRENT_TAB());

   if(argument->n == OPEN_EXTERNAL) {
      char* command = g_strdup_printf("%s %s", external_editor, uri);

      g_spawn_command_line_async(command, NULL);
      g_free(command);
   } else {
      gboolean is_vsm = webkit_web_view_get_view_source_mode(GET_CURRENT_TAB());
      webkit_web_view_set_view_source_mode(GET_CURRENT_TAB(), !is_vsm);

      open_uri(GET_CURRENT_TAB(), uri);
   }
}

void sc_yank(Argument* argument) {
   gchar* uri = (gchar*) webkit_web_view_get_uri(GET_CURRENT_TAB());
   if (argument->n == XA_CLIPBOARD)
      gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), uri, -1);
   else if (argument->n == XA_SECONDARY)
      gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_SECONDARY), uri, -1);
   else
      gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), uri, -1);

   gchar* message = g_strdup_printf("Yanked %s", uri);
   notify(INFO, message);
   g_free(message);
}

void sc_zoom(Argument* argument) {
   float zoom_level = webkit_web_view_get_zoom_level(GET_CURRENT_TAB());
   char* buffer = Client.Global.buffer->str;

   if(argument->n == ZOOM_IN)
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), zoom_level + (float) (zoom_step / 100));
   else if(argument->n == ZOOM_OUT)
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), zoom_level - (float) (zoom_step / 100));
   else if(argument->n == ZOOM_ORIGINAL)
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), 1.0f);
   else if(argument->n == SPECIFIC) {
      char* number = g_strndup(buffer, strlen(buffer) - 1);
      webkit_web_view_set_zoom_level(GET_CURRENT_TAB(), (float) (atoi(number) / 100));
      g_free(number);
   }
}

//--- Input Shortcuts -----
void isc_abort(Argument* argument) {
   Argument arg = { HIDE, NULL };
   isc_completion(&arg);

   notify(INFO, "");
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
   gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));
   gtk_widget_show(GTK_WIDGET(Client.UI.statusbar));
}

// TODO Needs major clean up
void isc_completion(Argument* argument) {
   gchar *input      = gtk_editable_get_chars(GTK_EDITABLE(Client.UI.inputbar), 0, -1);
   gchar  identifier = input[0];
   gchar *input_m    = input + 1;
   int    length     = strlen(input_m);

   if(!length && !identifier) {
      g_free(input);
      return;
   }

   /* get current information*/
   char* first_space = strstr(input_m, " ");
   char* current_command;
   char* current_parameter;
   int   current_command_length;

   if(!first_space) {
      current_command          = input_m;
      current_command_length   = length;
      current_parameter        = NULL;
   } else {
      int offset               = abs(input_m - first_space);
      current_command          = g_strndup(input_m, offset);
      current_command_length   = strlen(current_command);
      current_parameter        = input_m + offset + 1;
   }

   /* if the identifier does not match the command sign and
    * the completion should not be hidden, leave this function */
   if((identifier != ':') && (argument->n != HIDE)) {
      g_free(input);
      return;
   }

   /* static elements */
   static GtkBox        *results = NULL;
   static CompletionRow *rows    = NULL;

   static int current_item = 0;
   static int n_items      = 0;

   static char *previous_command   = NULL;
   static char *previous_parameter = NULL;
   static int   previous_id        = 0;
   static int   previous_length    = 0;

   static gboolean command_mode = TRUE;

   /* delete old list iff
    *   the completion should be hidden
    *   the current command differs from the previous one
    *   the current parameter differs from the previous one
    */
   if( (argument->n == HIDE) ||
         (current_parameter && previous_parameter && strcmp(current_parameter, previous_parameter)) ||
         (current_command && previous_command && strcmp(current_command, previous_command)) ||
         (previous_length != length)
     ) {
      if(results)    gtk_widget_destroy(GTK_WIDGET(results));

      results = NULL;

      if(rows)       free(rows);

      rows         = NULL;
      current_item = 0;
      n_items      = 0;
      command_mode = TRUE;

      if(argument->n == HIDE) {
         g_free(input);
         return;
      }
   }

   /* create new list iff
    *  there is no current list
    *  the current command differs from the previous one
    *  the current parameter differs from the previous one
    */
   if( (!results) ) {
      results = GTK_BOX(gtk_vbox_new(FALSE, 0));

      /* create list based on parameters iff
       *  there is a current parameter given
       *  there is an old list with commands (or not)
       *  the current command does not differ from the previous one
       *  the current command has an completion function
       */
      if(strchr(input_m, ' ')) {
         gboolean search_matching_command = FALSE;

         for(unsigned int i = 0; i < LENGTH(commands); i++) {
            int cmd_length  = commands[i].command ? strlen(commands[i].command) : 0;

            if( ((current_command_length <= cmd_length)  && !strncmp(current_command, commands[i].command, current_command_length)) ) {
               if(commands[i].completion) {
                  previous_command = current_command;
                  previous_id = i;
                  search_matching_command = TRUE;
               } else {
                  g_free(input);
                  return;
               }
            }
         }

         if(!search_matching_command) {
            g_free(input);
            return;
         }

         Completion *result = commands[previous_id].completion(current_parameter ? current_parameter : "");
         if(!result || !result->groups) {
            g_free(input);
            return;
         }

         command_mode               = FALSE;
         CompletionGroup* group     = NULL;

         rows = malloc(sizeof(CompletionRow));
         if(!rows) say(ERROR, "Out of memory", EXIT_FAILURE);

         for(GList* grlist = result->groups; grlist; grlist = g_list_next(grlist)) {
            group = (CompletionGroup*)grlist->data;
            int group_elements = 0;

            for(GList* element = group->elements; element; element = g_list_next(element)) {
               if(element->data) {
                  if (group->value && !group_elements) {
                     rows = realloc(rows, (n_items + 1) * sizeof(CompletionRow));
                     rows[n_items].command     = group->value;
                     rows[n_items].command_id  = -1;
                     rows[n_items].is_group    = TRUE;
                     rows[n_items++].row       = GTK_WIDGET(cr_create(results, group->value, TRUE));
                  }

                  rows = realloc(rows, (n_items + 1) * sizeof(CompletionRow));
                  rows[n_items].command     = element->data;
                  rows[n_items].command_id  = previous_id;
                  rows[n_items].is_group    = FALSE;
                  rows[n_items++].row       = GTK_WIDGET(cr_create(results, element->data, FALSE));
                  group_elements++;
               }
            }
         }
         // clean up
         completion_free(result);
      }
      /* create list based on commands */
      else {
         command_mode = TRUE;

         rows = malloc(LENGTH(commands) * sizeof(CompletionRow));
         if(!rows)   say(ERROR, "Out of memory", EXIT_FAILURE);

         for(unsigned int i = 0; i < LENGTH(commands); i++) {
            int cmd_length  = commands[i].command ? strlen(commands[i].command) : 0;

            /* add command to list iff
             *  the current command would match the command
             */
            if( ((current_command_length <= cmd_length)  && !strncmp(current_command, commands[i].command, current_command_length)) ) {
               rows[n_items].command     = commands[i].command;
               rows[n_items].command_id  = i;
               rows[n_items].is_group    = FALSE;
               rows[n_items++].row       = GTK_WIDGET(cr_create(results, commands[i].command, FALSE));
            }
         }
         rows = realloc(rows, n_items * sizeof(CompletionRow));
      }

      gtk_box_pack_start(Client.UI.box, GTK_WIDGET(results), FALSE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(results));

      current_item = (argument->n == NEXT) ? -1 : 0;
   }

   /* update coloring iff there is a list with items */
   if( (results) && (n_items > 0) ) {
      if(!rows[current_item].is_group)
         cr_set_color(results, NORMAL, current_item);
      char* temp;
      int i = 0, next_group = 0;

      for(i = 0; i < n_items; i++) {
         if(argument->n == NEXT || argument->n == NEXT_GROUP)
            current_item = (current_item + n_items + 1) % n_items;
         else if(argument->n == PREVIOUS || argument->n == PREVIOUS_GROUP)
            current_item = (current_item + n_items - 1) % n_items;

         if(rows[current_item].is_group) {
            if(!command_mode && (argument->n == NEXT_GROUP || argument->n == PREVIOUS_GROUP))
               next_group = 1;
            continue;
         } else {
            if(!command_mode && (next_group == 0) && (argument->n == NEXT_GROUP || argument->n == PREVIOUS_GROUP))
               continue;
            break;
         }
      }
      cr_set_color(results, HIGHLIGHT, current_item);

      /* hide other items */
      int uh = ceil(n_completion_items / 2);
      int lh = floor(n_completion_items / 2);

      for(i = 0; i < n_items; i++) {
         if((n_items > 1) && (
                  (i >= (current_item - lh) && (i <= current_item + uh)) ||
                  (i < n_completion_items && current_item < lh) ||
                  (i >= (n_items - n_completion_items) && (current_item >= (n_items - uh))))
           )
            gtk_widget_show(rows[i].row);
         else
            gtk_widget_hide(rows[i].row);
      }

      if(command_mode)
         temp = g_strconcat(":", rows[current_item].command, (n_items == 1) ? " "  : NULL, NULL);
      else
         temp = g_strconcat(":", previous_command, " ", rows[current_item].command, NULL);

      gtk_entry_set_text(Client.UI.inputbar, temp);
      gtk_editable_set_position(GTK_EDITABLE(Client.UI.inputbar), -1);
      g_free(temp);

      previous_command   = (command_mode) ? rows[current_item].command : current_command;
      previous_parameter = (command_mode) ? current_parameter : rows[current_item].command;
      previous_length    = strlen(previous_command);
      if(command_mode)
         previous_length += length - current_command_length;
      else
         previous_length += strlen(previous_parameter) + 1;

      previous_id        = rows[current_item].command_id;
   }
   g_free(input);
}

