/*
 * config/commands.h
 */

#ifndef CONFIG_COMMANDS_H
#define CONFIG_COMMANDS_H

//--- Inputbar commands -----
static const Command commands[] = {
  // command,     function,      completion,       description
  {"back",     cmd_back,         0 },              // Go back in history
  {"forward",  cmd_forward,      0 },              // Go forward in history
  {"bmark",    cmd_bookmark,     0 },              // Add bookmark
  {"qmark",    cmd_quickmark,    0 },              // Set quickmark
  {"pmark",    cmd_pagemark,     0 },              // Set pagemark
  {"e",        cmd_open,         cc_open },        // Open URI in current tab
  {"open",     cmd_open,         cc_open },
  {"print",    cmd_print,        0 },              // Print page
  {"q",        cmd_quit,         0 },              // Quit current tab
  {"quit",     cmd_quit,         0 },
  {"qall",     cmd_quitall,      0 },              // Quit all tabs
  {"reload",   cmd_reload,       0 },              // Reload current page
  {"saveas",   cmd_saveas,       0 },              // Save current document to disk
  {"script",   cmd_load_script,  0 },              // Load a javascript file
  {"session",  cmd_session,      cc_session },     // Save session with specified name
  {"set",      cmd_settings,     cc_settings },    // Set an option
  {"stop",     cmd_stop,         0 },              // Stop loading the current page
  {"tabnew",   cmd_tabopen,      cc_open },        // Open URI in a new tab
  {"winnew",   cmd_winopen,      cc_open },        // Open URI in a new window
  {"write",    cmd_write,        0 },              // Write bookmark and history file
};

#endif

