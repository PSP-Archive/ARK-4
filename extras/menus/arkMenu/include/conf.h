#ifndef ARKMENU_CONF_H
#define ARKMENU_CONF_H

#define CONFIG_PATH "ARKMENU.BIN"

typedef struct {
    unsigned char fast_gameboot; // skip pmf/at3 and gameboot animation
    unsigned char language; // default language for the menu
    unsigned char font; // default font (either the ones in flash0 or the custom one in THEME.ARK
    unsigned char plugins; // enable or disable plugins in game
    unsigned char scan_save; // enable or disable scanning savedata
    unsigned char scan_cat; // allow scanning for categorized content in /ISO and /PSP/GAME
    unsigned char scan_dlc; // allow scanning for DLC files (PBOOT.PBP)
    unsigned char swap_buttons; // whether to swap Cross and Circle
    unsigned char animation; // the background animation of the menu
    unsigned char main_menu; // default menu opened at startup (game by default)
    unsigned char sort_entries; // sort entries by name
    unsigned char show_recovery; // show recovery menu entry
    unsigned char show_fps; // show menu FPS
    unsigned char text_glow; // text glowing function scale
    unsigned char screensaver; // Screensaver time (or disabled)
    unsigned char redirect_ms0; // redirect ms0 to ef0
    unsigned char vsh_fg_color; // Advanced VSH Menu Forground color
    unsigned char vsh_bg_color; // Advanced VSH Menu Background color
    unsigned char menusize; // Change size of dropdown system menu (triangle)
    unsigned char force_update; // Force update (disable update version check)
    unsigned char battery_percent; // show remaing battery percent next to battery icon
    unsigned char startbtn; // Default (normal start button behaviour or boot last game)
    char last_game[128];    // last played game
    unsigned char vsh_font; // font used by VSH Menu
    char browser_dir[128]; // default directory when using file browser as main app
    unsigned char show_hidden; // show hidden files/folders
    unsigned char browser_icon0; // display ICON0 in File Browser
    unsigned char show_size; // show file size in browser
    unsigned char show_path; // show device in game manager title for GO, whether the game is on ef0 or ms0 
    unsigned char window_mode; // Choose whether to use the Classic VSH Menu Design or the new look
    unsigned char advanced_vsh; // Choose to autoload into advanced vsh menu
    unsigned char avm_hidden[256]; // Hiden items in advanced vsh menu
} t_conf;

#endif
