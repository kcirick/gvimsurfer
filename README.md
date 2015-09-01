# gVimSurfer

A WebkitGTK-based Web browser that looks and works like gVim


## Description:

  - Features/functions inspired by jumanji and vimprobable
  - organized in a way that makes sense
  - Requires following as dependencies:
   - gtk+-2
   - webkit-1.0
   - libsoup 
  - All configuration is stored under $HOME/.config/gvimsurfer (run the setup script to create it - See installation section below)

## Features:

  - Look and feel:
   - Customize statusbar colours, completion colours and browser font
   - Hide tabs, statusbar and scrollbars for maximum real estate
  - Navigate using keyboard
   - Hinting via javascript
   - Tab completion for URI (bookmarks and history), commands and settings
  - Search engines
   - Can define multiple search engines through config file to search the web
  - Bookmarks
   - Store bookmarks for future reference
  - Quickmarks
   - Quickly mark a webpage you want to come back to. Not stored after the browser is closed
  - Pagemarks
   - Quickly mark a section/view in a webpage you want to come back to. Not stored after the browser is closed
  - Flash block (NEW)
   - Flash frames are replaced with a black rectangle. It can be loaded when the rectangle is clicked 
  - Sessions
   - Able to save sessions (collection of tabs with the opened webpages) to be loaded in the future

## Installation:

  1. Clone a copy: `> git clone https://github.com/kcirick/gvimsurfer.git`
  2. Compile the code: `> make`
  3. Install: `> make install` (as root)
  4. Run the setup script: `> sh /usr/share/gvimsurfer/setup.sh`
  5. Make changes to configrc (located in $HOME/.config/gvimsurfer)
  6. Run 'gvimsurfer' (try 'gvimsurfer --help' for help)
  7. To uninstall: `> make uninstall` (as root)

## A screenshot:

<a href='http://s6.postimg.org/yrjhkoqn5/Screenshot_190815_02_21_42_AM.png' target='_blank'><img src='http://s6.postimg.org/yrjhkoqn5/Screenshot_190815_02_21_42_AM.png' width="350" /></a>
<a href='http://s6.postimg.org/iukpnyy8x/Screenshot_190815_02_22_09_AM.png' target='_blank'><img src='http://s6.postimg.org/iukpnyy8x/Screenshot_190815_02_22_09_AM.png' width="350" /></a><br /><br />


## Version Log:

  - 0.4 (Work-in-progress):
   - Implement flash block
   - Bug fixes
  - 0.3 (2015-08-30):
   - Correctly updates when clicking on tabs
   - Split "open" command to "open" and "history":
     - "open" will open search engine and bookmarks completion list
     - "history" will open history completion list
   - Each page has its own pagemark ~~and soup session~~ (previously all one giant session)
   - Fix several seg faults and weird crashes
   - General code clean-up
  - 0.2 (2015-08-19):
   - Adding close button to tabs
   - Better support for focus during INSERT mode
   - Fix completion method
   - Simplify some functions
   - Adding default javascript file and setup script
  - 0.1 (2015-08-06): A working web browser

## To do / Known issues:

  - BUG: Several GLib-GObject warnings to address
  - TODO: Code cleanup
  - DONE: ~~Implement flash block~~ (could still be buggy)
  - DONE: ~~Implement adblock~~ -> use css stylesheet
  - DONE: ~~Make install/uninstall routine to Makefile~~

