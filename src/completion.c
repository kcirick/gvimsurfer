/*
 * src/completion.c
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/utilities.h"
#include "include/client.h"
#include "include/callbacks.h"
#include "include/completion.h"
#include "include/commands.h"

#include "config/config.h"
#include "config/settings.h"
#include "config/commands.h"


// Completion 
Completion*       completion_init();
void              completion_free(Completion*);

// Completion Group
CompletionGroup*  cg_create(char*);
void              cg_add_element(CompletionGroup*, char*);

// Completion Row
GtkEventBox*      cr_create(GtkBox*, char*, gboolean);
void              cr_set_color(GtkBox*, int, int);

//--- Completion -----
Completion* completion_init() {
   Completion *completion = malloc(sizeof(Completion));
   if(!completion)   die(ERROR, "Out of memory", EXIT_FAILURE);

   completion->groups = NULL;

   return completion;
}

void completion_free(Completion* completion) {
   for(GList* grlist=completion->groups; grlist; grlist=g_list_next(grlist)){
      CompletionGroup* group = (CompletionGroup*)grlist->data;

      g_list_free(group->elements);
      free(group);
   }
}

//--- Completion Row -----
GtkEventBox* cr_create(GtkBox* results, char* command, gboolean group) {
   GtkBox      *col = GTK_BOX(gtk_vbox_new(FALSE, 0));
   GtkEventBox *row = GTK_EVENT_BOX(gtk_event_box_new());
   GtkWidget   *separator = gtk_hseparator_new();
   gtk_widget_modify_bg(separator, GTK_STATE_NORMAL, &(Client.Style.completion_fg));

   GtkLabel *show_command     = GTK_LABEL(gtk_label_new(NULL));

   gtk_misc_set_alignment(GTK_MISC(show_command),  0.0, 0.0);
   gtk_misc_set_padding(GTK_MISC(show_command),    1.0, 1.0);

   gtk_label_set_use_markup(show_command, TRUE);

   gtk_label_set_markup(show_command, group ? 
      g_markup_printf_escaped("<b>%s</b>",   command ? command : "") :
      g_markup_printf_escaped("%s",          command ? command : "") );
   
   gtk_widget_modify_fg(GTK_WIDGET(show_command),     GTK_STATE_NORMAL, &(Client.Style.completion_fg));
   gtk_widget_modify_bg(GTK_WIDGET(row),              GTK_STATE_NORMAL, &(Client.Style.completion_bg));
   gtk_widget_modify_font(GTK_WIDGET(show_command),   Client.Style.font);
   //if(!group) gtk_widget_set_size_request(GTK_WIDGET(row),    -1, Client.Style.statusbar_height);

   gtk_box_pack_start(GTK_BOX(col), GTK_WIDGET(show_command),     TRUE,  TRUE,  0);
   if(group) gtk_box_pack_start(GTK_BOX(col), separator, TRUE, TRUE, 0);
   gtk_container_add(GTK_CONTAINER(row), GTK_WIDGET(col));
   gtk_box_pack_start(results, GTK_WIDGET(row), FALSE, FALSE, 0);

   gtk_widget_show_all(GTK_WIDGET(row));

   return row;
}

void cr_set_color(GtkBox* results, int mode, int id) {
   GtkEventBox *row   = (GtkEventBox*) g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(results)), id);

   if(row) {
      if(mode == NORMAL)
         gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(Client.Style.completion_bg));
      else
         gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(Client.Style.completion_hl));
   }
}

//--- Completion Group -----
void cg_add_element(CompletionGroup* group, char* name) {
   group->elements = g_list_append(group->elements, name);
}

CompletionGroup* cg_create(char* name) {
   CompletionGroup* group = malloc(sizeof(CompletionGroup));
   if(!group)      die(ERROR, "Out of memory", EXIT_FAILURE);

   group->value    = name;
   group->elements = NULL;

   return group;
}

//--- Completion Command -----
Completion* cc_open(char* input) {
   Completion* completion = completion_init();

   //--- search engines -----
   if(Client.Global.search_engines){
      CompletionGroup* search_engines = cg_create("Search Engines");
      completion->groups = g_list_append(completion->groups, search_engines);
      
      for(GList* l = Client.Global.search_engines; l; l=g_list_next(l)) {
         SearchEngine* se =  (SearchEngine*)l->data;

         if(strstr(se->name, input))
            cg_add_element(search_engines, se->name);
      }
   }

   /* we make bookmark and history completion case insensitive */
   gchar* lowercase_input = g_utf8_strdown(input, -1);

   //--- bookmarks -----
   if(Client.Global.bookmarks) {
      CompletionGroup* bookmarks = cg_create("Bookmarks");
      completion->groups = g_list_append(completion->groups, bookmarks);

      for(GList* l = Client.Global.bookmarks; l; l = g_list_next(l)) {
         char* bookmark = (char*) l->data;
         gchar* lowercase_bookmark = g_utf8_strdown(bookmark, -1);

         if(strstr(lowercase_bookmark, lowercase_input))
            cg_add_element(bookmarks, bookmark);

         g_free(lowercase_bookmark);
      }
   }

   //--- history -----
   if(Client.Global.history) {
      CompletionGroup* history = cg_create("History");
      completion->groups = g_list_append(completion->groups, history);

      for(GList* h = Client.Global.history; h; h = g_list_next(h)) {
         char* uri = (char*) h->data;
         gchar* lowercase_uri = g_utf8_strdown(uri, -1);

         if(strstr(lowercase_uri, lowercase_input))
            cg_add_element(history, uri);

         g_free(lowercase_uri);
      }
   }
   g_free(lowercase_input);

   return completion;
}

Completion* cc_session(char* input) {
   Completion* completion = completion_init();

   CompletionGroup* group = cg_create(NULL);
   completion->groups = g_list_append(completion->groups, group);

   for(GList* l = Client.Global.sessions; l; l = g_list_next(l)) {
      Session* se = (Session*)l->data;

      if(strstr(se->name, input))
         cg_add_element(group, se->name);
   }
   return completion;
}

Completion* cc_settings(char* input) {
   Completion* completion = completion_init();
   
   CompletionGroup* group = cg_create(NULL);
   completion->groups = g_list_append(completion->groups, group);

   unsigned int input_length = input ? strlen(input) : 0;

   for(unsigned int i = 0; i < LENGTH(settings); i++) {
      if( (input_length <= strlen(settings[i].name)) &&
            !strncmp(input, settings[i].name, input_length) ){
         cg_add_element(group, settings[i].name);
      }
   }
   return completion;
}

// TODO Needs major clean up
void run_completion(gint arg) {
   gchar *input      = gtk_editable_get_chars(GTK_EDITABLE(Client.UI.inputbar), 0, -1);
   if(strlen(input)==0 || (input[0]!=':' && arg!=HIDE)) {
      g_free(input);
      return;
   }

   gchar *input_m    = input + 1;
   gint   length     = strlen(input_m);
   if(length==0){
      g_free(input);
      return;
   }

   // get current information
   gchar **entries = g_strsplit_set(input_m, " ", -1);
   gint n_entries = g_strv_length(entries);

   gchar* current_command        = entries[0];
   gint   current_command_length = strlen(current_command);
   gchar* current_parameter      =  n_entries==1 ? NULL : entries[n_entries-1];

   // static elements
   static GtkBox        *results = NULL;
   static CompletionRow *rows    = NULL;

   static gint current_item = 0;
   static gint n_items      = 0;

   static gchar *previous_command   = NULL;
   static gchar *previous_parameter = NULL;
   static gint   previous_id        = 0;
   static gint   previous_length    = 0;

   /* delete old list if
    *   the completion should be hidden
    *   the current command differs from the previous one
    *   the current parameter differs from the previous one
    */
   if( (arg == HIDE) ||
         (current_parameter && previous_parameter && strcmp(current_parameter, previous_parameter)) ||
         (current_command && previous_command && strcmp(current_command, previous_command)) ||
         (previous_length != length)
     ) {
      if(results)    gtk_widget_destroy(GTK_WIDGET(results));
      results        = NULL;

      if(rows)       free(rows);
      rows           = NULL;
      current_item   = 0;
      n_items        = 0;

      if(arg == HIDE) {
         g_free(input);
         return;
      }
   }

   //--- create new list -----
   if( !results ) {
      results = GTK_BOX(gtk_vbox_new(FALSE, 0));

      // create list based on parameters 
      if(n_entries>1) {
         for(unsigned int i = 0; i < LENGTH(commands); i++) {
            if( g_strcmp0(current_command, commands[i].command)!=0 ) continue;

            if(commands[i].completion) {
               previous_command = current_command;
               previous_id = i;
               break;
            } 

            g_free(input);
            return;
         }

         Completion *result = commands[previous_id].completion(current_parameter ? current_parameter : "");
         if(!result || !result->groups) {
            g_free(input);
            return;
         }

         rows = malloc(sizeof(CompletionRow));
         if(!rows) die(ERROR, "Out of memory", EXIT_FAILURE);

         for(GList* grlist = result->groups; grlist; grlist = g_list_next(grlist)) {
            CompletionGroup* group = (CompletionGroup*)grlist->data;
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
      // create list based on commands
      else {

         rows = malloc(LENGTH(commands) * sizeof(CompletionRow));
         if(!rows)   die(ERROR, "Out of memory", EXIT_FAILURE);

         for(unsigned int i = 0; i < LENGTH(commands); i++) {
            int cmd_length  = commands[i].command ? strlen(commands[i].command) : 0;

            if( current_command_length <= cmd_length  && !strncmp(current_command, commands[i].command, current_command_length) ) {
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

      current_item = (arg == NEXT) ? -1 : 0;
   }

   /* update coloring iff there is a list with items */
   if( (results) && (n_items > 0) ) {
      int i = 0, next_group = 0;

      if(!rows[current_item].is_group) cr_set_color(results, NORMAL, current_item);

      for(i = 0; i < n_items; i++) {
         if(arg == NEXT || arg == NEXT_GROUP)
            current_item = (current_item + n_items + 1) % n_items;
         else if(arg == PREVIOUS || arg == PREVIOUS_GROUP)
            current_item = (current_item + n_items - 1) % n_items;

         if(rows[current_item].is_group) {
            if(n_entries>1 && (arg == NEXT_GROUP || arg == PREVIOUS_GROUP))
               next_group = 1;
            continue;
         } else {
            if(n_entries>1 && (next_group == 0) && (arg == NEXT_GROUP || arg == PREVIOUS_GROUP))
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

      gchar* temp;
      if(n_entries==1)
         temp = g_strconcat(":", rows[current_item].command, (n_items == 1) ? " "  : NULL, NULL);
      else{
         gchar* other_parameters = "";
         for(i=1; i<n_entries-1; i++)
            other_parameters = g_strconcat(" ", entries[i], NULL);

         temp = g_strconcat(":", previous_command, other_parameters, " ", rows[current_item].command, NULL);
      }

      gtk_entry_set_text(Client.UI.inputbar, temp);
      gtk_editable_set_position(GTK_EDITABLE(Client.UI.inputbar), -1);
      g_free(temp);

      previous_command   = (n_entries==1) ? rows[current_item].command : current_command;
      previous_parameter = (n_entries==1) ? current_parameter : rows[current_item].command;
      previous_length    = strlen(previous_command);
      if(n_entries==1)  previous_length += length - current_command_length;
      else              previous_length += strlen(previous_parameter) + 1;

      previous_id = rows[current_item].command_id;
   }
   g_free(input);
}

