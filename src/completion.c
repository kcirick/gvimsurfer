/*
 * src/completion.c
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/client.h"
#include "include/utilities.h"
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
CompletionGroup*  cg_create(gchar*);
void              cg_add_element(CompletionGroup*, gchar*, gchar*);
// Completion Row
GtkEventBox*      cr_create(GtkBox*, gchar*, gchar*, gboolean);
void              cr_set_color(GtkBox*, int, int);

//--- Completion -----
Completion* completion_init() {
   Completion *completion = malloc(sizeof(Completion));
   if(!completion)   say(ERROR, "Out of memory", EXIT_FAILURE);

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
GtkEventBox* cr_create(GtkBox* results, gchar* command, gchar* text, gboolean group) {
   GtkBox      *icol = GTK_BOX(gtk_hbox_new(FALSE, 0));
   GtkBox      *col = GTK_BOX(gtk_vbox_new(FALSE, 0));
   GtkEventBox *row = GTK_EVENT_BOX(gtk_event_box_new());
   GtkWidget   *separator = gtk_hseparator_new();
   gtk_widget_modify_bg(separator, GTK_STATE_NORMAL, &(Client.Style.completion_fg));

   GtkLabel *show_command  = GTK_LABEL(gtk_label_new(NULL));
   GtkLabel *show_text     = GTK_LABEL(gtk_label_new(NULL));   

   gtk_misc_set_alignment(GTK_MISC(show_command),  0.0, 0.0);
   gtk_misc_set_alignment(GTK_MISC(show_text),     0.0, 0.0);
   gtk_misc_set_padding(GTK_MISC(show_command),    1.0, 1.0);
   gtk_misc_set_padding(GTK_MISC(show_text),       1.0, 1.0);

   gtk_label_set_use_markup(show_command, TRUE);
   gtk_label_set_use_markup(show_text,    TRUE);

   gtk_label_set_markup(show_command, group ? 
      g_markup_printf_escaped("<b>%s</b>",   command ? command : "") :
      g_markup_printf_escaped("%s",          command ? command : "") );
   gtk_label_set_markup(show_text, g_markup_printf_escaped("<i>%s</i>", text ? text:""));

   gtk_widget_modify_bg(GTK_WIDGET(row),              GTK_STATE_NORMAL, &(Client.Style.completion_bg));
   gtk_widget_modify_fg(GTK_WIDGET(show_command),     GTK_STATE_NORMAL, &(Client.Style.completion_fg));
   gtk_widget_modify_fg(GTK_WIDGET(show_text),        GTK_STATE_NORMAL, &(Client.Style.completion_fg));
   gtk_widget_modify_font(GTK_WIDGET(show_command),   Client.Style.font);
   gtk_widget_modify_font(GTK_WIDGET(show_text),      Client.Style.font);

   gtk_box_pack_start(GTK_BOX(icol), GTK_WIDGET(show_command),  TRUE,  TRUE,  2);
   gtk_box_pack_end(GTK_BOX(icol), GTK_WIDGET(show_text),     FALSE, FALSE, 2);
   gtk_box_pack_start(GTK_BOX(col), GTK_WIDGET(icol), TRUE, TRUE, 0);
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
CompletionGroup* cg_create(gchar* name) {
   CompletionGroup* group = malloc(sizeof(CompletionGroup));
   if(!group)      say(ERROR, "Out of memory", EXIT_FAILURE);

   group->value    = name;
   group->elements = NULL;

   return group;
}

void cg_add_element(CompletionGroup* group, gchar* command, gchar* text) {
   CompletionElement *element = malloc(sizeof(CompletionElement));
   element->command  = g_strdup(command);
   element->text     = g_strdup(text);

   group->elements = g_list_append(group->elements, element);
}

//--- Completion Command -----
Completion* cc_commands(gchar* input) {
   Completion* completion = completion_init();

   CompletionGroup* group = cg_create("Commands");
   completion->groups = g_list_append(completion->groups, group);

   for(unsigned int i = 0; i < LENGTH(commands); i++) {
      if( strlen(input) <= strlen(commands[i].command)  && 
               !strncmp(input, commands[i].command, strlen(input)) ) 
         cg_add_element(group, commands[i].command, "");
   }
   return completion;
}

Completion* cc_open(gchar* input) {
   Completion* completion = completion_init();

   //--- search engines -----
   if(Client.Global.search_engines){
      CompletionGroup* search_engines = cg_create("Search Engines");
      completion->groups = g_list_append(completion->groups, search_engines);
      
      for(GList* l = Client.Global.search_engines; l; l=g_list_next(l)) {
         SearchEngine* se =  (SearchEngine*)l->data;

         if(strstr(se->name, input))
            cg_add_element(search_engines, se->name, se->uri);
      }
   }

   // make bookmark completion case insensitive
   gchar* lowercase_input = g_utf8_strdown(input, -1);

   //--- bookmarks -----
   if(Client.Global.bookmarks) {
      CompletionGroup* bookmarks = cg_create("Bookmarks");
      completion->groups = g_list_append(completion->groups, bookmarks);

      for(GList* l = Client.Global.bookmarks; l; l = g_list_next(l)) {
         BMark* bmark = (BMark*) l->data;
         gchar* lowercase_uri = g_utf8_strdown(bmark->uri, -1);
         gchar* lowercase_tags = bmark->tags? g_utf8_strdown(bmark->tags, -1) : "";

         if(strstr(lowercase_uri, lowercase_input) || strstr(lowercase_tags, lowercase_input))
            cg_add_element(bookmarks, bmark->uri, bmark->tags);

         g_free(lowercase_uri); 
         if(strlen(lowercase_tags)) g_free(lowercase_tags);
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
            cg_add_element(history, uri, "");

         g_free(lowercase_uri);
      }
   }

   g_free(lowercase_input);

   return completion;
}

Completion* cc_session(gchar* input) {
   Completion* completion = completion_init();

   CompletionGroup* group = cg_create("Sessions");
   completion->groups = g_list_append(completion->groups, group);

   for(GList* l = Client.Global.sessions; l; l = g_list_next(l)) {
      Session* se = (Session*)l->data;

      if(strstr(se->name, input))
         cg_add_element(group, se->name, "");
   }
   return completion;
}

Completion* cc_settings(gchar* input) {
   Completion* completion = completion_init();
   
   CompletionGroup* group = cg_create("Browser Settings");
   completion->groups = g_list_append(completion->groups, group);

   unsigned int input_length = input ? strlen(input) : 0;

   for(unsigned int i = 0; i < LENGTH(settings); i++) {
      if( (input_length <= strlen(settings[i].name)) &&
            !strncmp(input, settings[i].name, input_length) ){
         gchar* type_name="";
         switch(settings[i].type){
            case 'b': type_name=strdup("boolean"); break;
            case 'i': type_name=strdup("integer"); break;
            case 'f': type_name=strdup("float"); break;
            case 's': type_name=strdup("string"); break;
            case 'c': type_name=strdup("character"); break;
            default:  type_name=strdup("special"); break;
         }
         cg_add_element(group, settings[i].name, type_name);
      }
   }
   return completion;
}

Completion* cc_downloads(gchar* input) {
   Completion* completion = completion_init();

   CompletionGroup* group = cg_create("Active Downloads");
   completion->groups = g_list_append(completion->groups, group);

   for(int i=0; i<g_list_length(Client.Global.active_downloads); i++){

      WebKitDownload* this_download = (WebKitDownload*)g_list_nth_data(Client.Global.active_downloads, i);
      if(this_download){
         const gchar* filename = webkit_download_get_suggested_filename(this_download);

         gchar* istring = g_strdup_printf("%d",i);
         gchar* text    = g_strdup_printf("%s - %.0f%% completed", filename, 100*webkit_download_get_progress(this_download));

         if(strstr(istring, input))
            cg_add_element(group, istring, text);

         g_free(istring); g_free(text);
      }
   }
   return completion;
}

void run_completion(gint arg) {
   gchar *input      = gtk_editable_get_chars(GTK_EDITABLE(Client.UI.inputbar), 0, -1);
   if(strlen(input)==0 || (input[0]!=':' && arg!=HIDE)) {
      g_free(input); return;
   }

   gchar *input_m    = input + 1;
   if(strlen(input_m)==0){
      g_free(input); return;
   }

   // get current information
   gchar **entries = g_strsplit_set(input_m, " ", -1);
   gint  n_entries = g_strv_length(entries);
   gboolean is_command = n_entries==1;

   gchar* new_command   = entries[0];
   gchar* new_parameter = is_command ? NULL : g_strdup(entries[n_entries-1]);

   // static elements
   static GtkBox        *results = NULL;
   static CompletionRow *rows    = NULL;

   static gint current_item = 0;
   static gint n_items      = 0;

   static gchar *this_command   = NULL;
   static gchar *this_parameter = NULL;

   static gboolean first_instance   = TRUE;

   /* delete old list if
    *   the completion should be hidden
    *   the current command differs from the previous one
    *   the current parameter differs from the previous one
    */
   //notify(DEBUG, g_strdup_printf("%s %s / %s %s", this_command, new_command,
   //         this_parameter, new_parameter));
   if( (arg == HIDE) ||
         (new_parameter && this_parameter && strcmp(new_parameter, this_parameter)) ||
         (new_command && this_command && strcmp(new_command, this_command)) ||
         (!is_command && strlen(new_parameter)==0)
     ) {
      if(results)    gtk_widget_destroy(GTK_WIDGET(results));
      results        = NULL;

      if(rows)       free(rows);
      rows           = NULL;
      current_item   = 0;
      n_items        = 0;

      first_instance = TRUE;

      if(arg == HIDE) {
         g_free(input); return;
      }
   }

   //--- create new list -----
   if( !results ) {
      results = GTK_BOX(gtk_vbox_new(FALSE, 0));

      Completion *result = NULL;
      if(is_command){      // create list based on commands
         result = cc_commands(new_command);
      } else {             // create list based on parameters 
         gint command_id=-1;
         for(unsigned int i = 0; i < LENGTH(commands); i++) {
            if( g_strcmp0(new_command, commands[i].command)!=0 ) continue;

            if(commands[i].completion) {
               this_command = commands[i].command;
               command_id = i;
               break;
            } 
         }
         if(command_id<0){
            g_free(input); return;
         }
         result = commands[command_id].completion(new_parameter);
      }

      rows = malloc(sizeof(CompletionRow));
      if(!rows) say(ERROR, "Out of memory", EXIT_FAILURE);

      for(GList* grlist = result->groups; grlist; grlist = g_list_next(grlist)) {
         CompletionGroup* group = (CompletionGroup*)grlist->data;
         gboolean header_empty = TRUE;

         for(GList* element = group->elements; element; element = g_list_next(element)) {
            CompletionElement* ce = (CompletionElement*)element->data;
            if (group->value && header_empty) {
               rows = realloc(rows, (n_items + 1) * sizeof(CompletionRow));
               rows[n_items].command     = group->value;
               rows[n_items].is_group    = TRUE;
               rows[n_items++].row       = GTK_WIDGET(cr_create(results, group->value, NULL, TRUE));
               header_empty = FALSE;
            }

            rows = realloc(rows, (n_items + 1) * sizeof(CompletionRow));
            rows[n_items].command     = ce->command;
            rows[n_items].is_group    = FALSE;
            rows[n_items++].row       = GTK_WIDGET(cr_create(results, ce->command, ce->text, FALSE));
         }
      }
      // clean up
      completion_free(result);

      gtk_box_pack_start(Client.UI.box, GTK_WIDGET(results), FALSE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(results));
   }

   // update row colour if there is a list
   if( results && n_items > 1 ) {
      gboolean next_group = FALSE;

      cr_set_color(results, NORMAL, current_item);

      for(int i = 0; i < n_items; i++) {
         if(arg==NEXT || arg==NEXT_GROUP)
            current_item = (current_item + n_items + 1) % n_items;
         else if(arg==PREVIOUS || arg==PREVIOUS_GROUP)
            current_item = (current_item + n_items - 1) % n_items;

         if(is_command){
            if(rows[current_item].is_group)  continue;
            else                             break;
         } else {
            if(rows[current_item].is_group) {
               if((arg==NEXT_GROUP || arg==PREVIOUS_GROUP)) next_group = TRUE;
               continue;
            } else {
               if(first_instance) break;
               if((arg==NEXT_GROUP || arg==PREVIOUS_GROUP) && !next_group) continue;
               break;
            }
         }
      }
      cr_set_color(results, HIGHLIGHT, current_item);

      // hide other items
      gint uh = ceil(n_completion_items / 2);
      gint lh = floor(n_completion_items / 2);

      for(gint i = 0; i < n_items; i++) {
         if( (i >= (current_item - lh) && (i <= current_item + uh)) ||
               (i < n_completion_items && current_item < lh) ||
               (i >= (n_items - n_completion_items) && (current_item >= (n_items - uh)))
           )
            gtk_widget_show(rows[i].row);
         else
            gtk_widget_hide(rows[i].row);
      }

      // Set input bar text and previous_* variables
      gchar* new_input_text;
      if(is_command){
         gboolean match = g_strcmp0(this_command, rows[current_item].command)==0;
         new_input_text = g_strconcat(":", rows[current_item].command, match ? " " : NULL, NULL);

         this_command   = rows[current_item].command;
      } else {
         gchar* other_parameters = "";
         for(gint i=1; i<n_entries-1; i++)
            other_parameters = g_strconcat(" ", entries[i], NULL);
         new_input_text = g_strconcat(":", this_command, other_parameters, " ", rows[current_item].command, NULL);
         if(strlen(other_parameters)>0) g_free(other_parameters);

         this_parameter = g_strdup(rows[current_item].command);
      }
      gtk_entry_set_text(Client.UI.inputbar, new_input_text);
      gtk_editable_set_position(GTK_EDITABLE(Client.UI.inputbar), -1);
      g_free(new_input_text); 
      
      first_instance = FALSE;
   }
   g_free(input);
}

