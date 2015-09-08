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
   - Store bookmarks (with tags) for future reference
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

## Screenshots:

<a href='http://s6.postimg.org/yrjhkoqn5/Screenshot_190815_02_21_42_AM.png' target='_blank'><img src='http://s6.postimg.org/yrjhkoqn5/Screenshot_190815_02_21_42_AM.png' width="350" /></a>
<a href='http://s6.postimg.org/iukpnyy8x/Screenshot_190815_02_22_09_AM.png' target='_blank'><img src='http://s6.postimg.org/iukpnyy8x/Screenshot_190815_02_22_09_AM.png' width="350" /></a><br /><br />


## Version Log:

  - 0.4 (Work-in-progress):
   - Implement flash block
    - BG/FG/Border colours can be configured in configrc
   - Bookmarks support tags
   - Adding "add new tab" button
   - Completion row displays helpful information
   - Completion row for active downloads
    - Use command ":download"
      - Command ":cancel" is now ":download cancel"
      - Command to list active download is ":download list <TAB>"
   - Revert splitting of "open" command (was done in version 0.3)
   - Fix broken HINTS mode (also renamed from FOLLOW mode)
   - Bug fixes
  - 0.3 (2015-08-30) (<a href='https://github.com/kcirick/gvimsurfer/archive/0.3.zip'>download .zip</a>):
   - Correctly updates when clicking on tabs
   - ~~Split "open" command to "open" and "history"~~:
     - ~~"open" will open search engine and bookmarks completion list~~
     - ~~"history" will open history completion list~~
   - Each page has its own pagemark (previously all one giant session)
   - Fix several seg faults and weird crashes
   - General code clean-up
  - 0.2 (2015-08-19) (<a href='https://github.com/kcirick/gvimsurfer/archive/0.2.zip'>download .zip</a>):
   - Adding close button to tabs
   - Better support for focus during INSERT mode
   - Fix completion method
   - Simplify some functions
   - Adding default javascript file and setup script
  - 0.1 (2015-08-06) (<a href='https://github.com/kcirick/gvimsurfer/archive/0.1.zip'>download .zip</a>): A working web browser

## To do / Known issues:

  - TODO: Add command to reorder tabs
  - TODO: Code cleanup
  - DONE: ~~Several GLib-GObject warnings to address~~ -> Ignore
  - DONE: ~~Implement flash block~~ (could still be buggy)
  - DONE: ~~Implement adblock~~ -> use css stylesheet
  - DONE: ~~Make install/uninstall routine to Makefile~~

