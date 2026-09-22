#ifndef DISPLAY_SSD1306_H
#define DISPLAY_SSD1306_H

#define DISPLAY_WIDTH   128u
#define DISPLAY_HEIGHT  64u
#define DISPLAY_PAGES   8u

void display_init(void);
void display_off(void);
void display_all_pixels_on(void);
void display_resume_ram(void);
void display_clear(void);
void display_fill(unsigned char pattern);
void display_set_cursor(unsigned char column, unsigned char page);
void display_write_char(char c);
void display_write_string(const char *s);
void display_print_at(unsigned char column, unsigned char page, const char *s);
void display_hline(unsigned char page, unsigned char pattern);

#endif
