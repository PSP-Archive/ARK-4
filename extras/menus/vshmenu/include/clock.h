#ifndef _CLOCK_H
#define _CLOCK_H


int cpu2no(int cpu);
int bus2no(int cpu);
void swap_readonly(int dir);
void change_bg_color(int dir);
void change_fg_color(int dir);
void change_font(int dir);
void change_design(int dir);
void change_menu(int dir);
void change_usb(int dir);
void change_umd_mode(int dir);
void change_umd_mount_idx(int dir);
void change_umd_region(int dir, int max);
void change_region(int dir, int max);
void change_bool_option(int *p, int direction);


#endif
