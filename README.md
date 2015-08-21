# gVimSurfer

Web browser that looks and works like gVim


## Description:

  - Features/functions inspired by jumanji and vimprobable
  - organized in a way it makes sense
  - Requires following as dependencies:
   - gtk+-2
   - webkit-1.0
   - libsoup 
  - Not meant to be installed but compiled to a local folder (eg. $HOME/.local/bin)
   - All configuration is stored under $HOME/.config/gvimsurfer (run the setup script to create it)


## Installation:

  1. Clone a copy: `> git clone https://github.com/kcirick/gvimsurfer.git`
  2. Compile the code: `> make`
  3. Make a link to the excutable (Optional): `> ln -s gvimsurfer ~/.local/bin/gvimsurfer`
  4. Run the setup script: `> sh setup.sh`
  5. Make changes to configrc
  6. Run 'gvimsurfer' (try 'gvimsurfer --help' for help)


## A screenshot:

<a href='http://s6.postimg.org/yrjhkoqn5/Screenshot_190815_02_21_42_AM.png' target='_blank'><img src='http://s6.postimg.org/yrjhkoqn5/Screenshot_190815_02_21_42_AM.png' width="350" /></a>
<a href='http://s6.postimg.org/iukpnyy8x/Screenshot_190815_02_22_09_AM.png' target='_blank'><img src='http://s6.postimg.org/iukpnyy8x/Screenshot_190815_02_22_09_AM.png' width="350" /></a><br /><br />


## Version Log:

  - 0.3 (Work-in-progress):
   - Correctly updates when clicking on tabs
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

  - Lots (to be added)

