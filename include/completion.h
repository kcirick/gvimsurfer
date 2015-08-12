/*
 * include/completion.h
 */

#ifndef COMPLETION_H
#define COMPLETION_H

typedef struct {
   gchar    *value;
   GList    *elements;
} CompletionGroup;

typedef struct {
   GList*   groups;
} Completion;

typedef struct {
   gchar*      command;
   gint        command_id;
   gboolean    is_group;
   GtkWidget*  row;
} CompletionRow;


// Completion 
Completion*       completion_init();
void              completion_free(Completion*);

// Completion Row
GtkEventBox*      cr_create(GtkBox*, char*, gboolean);
void              cr_set_color(GtkBox*, int, int);

// Completion Command
Completion*       cc_open(char*);
Completion*       cc_session(char*);
Completion*       cc_settings(char*);

#endif

