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

// Completion Command
Completion*       cc_open(char*);
Completion*       cc_session(char*);
Completion*       cc_settings(char*);
Completion*       cc_history(char*);

void run_completion(gint);

#endif

