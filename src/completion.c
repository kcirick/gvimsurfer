/*
 * src/completion.c
 */

#include <stdlib.h>
#include <string.h>

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


CompletionGroup*  cg_create(char*);
void              cg_add_element(CompletionGroup*, char*);

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
   if(!group) gtk_widget_set_size_request(GTK_WIDGET(row),    -1, Client.Style.statusbar_height);

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
   if(!group)      say(ERROR, "Out of memory", EXIT_FAILURE);

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

