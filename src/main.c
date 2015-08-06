/*
 * src/main.c: Main code
 */

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/utilities.h"
#include "include/client.h"

//--- Local variable -----
static const gchar default_config_file[] = ".config/gvimsurfer/configrc";

//------------------------------------------------------------------------
// Main function
//----------------
int main(int argc, char* argv[]) {

   gtk_init(&argc, &argv);

   if(argc==2 && !strcmp("-v", argv[1]))
      say(INFO, "Version "VERSION, EXIT_SUCCESS);
   else if(argc==2 && !strcmp("-h", argv[1]))
      say(INFO, "Usage: gvimsurfer [-v][-h][-c configfile][URL1 URL2 ...]", EXIT_SUCCESS);

   gint ioffset=0;
   gchar* configfile=NULL;
   if(argc>2 && !strcmp("-c", argv[1])){
      configfile = argv[2];
      ioffset+=2;
   } else
      configfile = g_build_filename(g_get_home_dir(), default_config_file, NULL);

   if(!read_configuration(configfile)) 
      say(ERROR, g_strdup_printf("Invalid configuration file: %s", configfile), EXIT_FAILURE);

   // init client and read configuration
   init_search_items();
   init_client();
   init_client_settings();
   init_client_data();

   // create tab 
   if(argc-ioffset < 2)
      create_tab(NULL, TRUE);
   else
      for(gint i=ioffset+1; i < argc; i++)
         create_tab(argv[i], TRUE);

   gtk_widget_show_all(GTK_WIDGET(Client.UI.window));
   gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));

   gtk_main();

   return EXIT_SUCCESS;
}
