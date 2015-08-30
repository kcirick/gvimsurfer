/*
 * src/main.c: Main code
 */

#include <stdlib.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/client.h"
#include "include/utilities.h"

//--- Local variable -----
static const gchar default_config_file[] = ".config/gvimsurfer/configrc";

//------------------------------------------------------------------------
// Main function
//----------------
int main(int argc, char* argv[]) {

   static GError *err;
   static gboolean version = FALSE;
   static gboolean private = FALSE;
   static const gchar* cfile = NULL;
   static GOptionEntry opts[] = {
      { "version",      'v', 0, G_OPTION_ARG_NONE, &version, "Print version", NULL },
      { "configfile",   'c', 0, G_OPTION_ARG_STRING, &cfile, "Specify config file", NULL },
      { "private",      'p', 0, G_OPTION_ARG_NONE, &private, "Open in private mode", NULL },
      { NULL } };

   if (!gtk_init_with_args(&argc, &argv, "[URL1 URL2 ...]", opts, NULL, &err))
      say(ERROR, err->message, EXIT_FAILURE);

   if (version)
      say(INFO, "Version "VERSION, EXIT_SUCCESS);

   private_browsing = private;

   // read config file
   gchar* configfile= cfile ?
      g_strdup(cfile): g_build_filename(g_get_home_dir(), default_config_file, NULL);

   if(!read_configuration(configfile)) 
      say(ERROR, g_strdup_printf("Invalid configuration file: %s", configfile), EXIT_FAILURE);

   // init client 
   init_client();

   // create tab 
   if(argc < 2)
      create_tab(NULL, TRUE);
   else
      for(gint i=1; i<argc; i++)
         create_tab(argv[i], TRUE);

   gtk_widget_show_all(GTK_WIDGET(Client.UI.window));
   gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));

   gtk_main();

   return EXIT_SUCCESS;
}
