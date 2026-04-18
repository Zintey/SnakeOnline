#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <windows.h>
#include <string.h>

#define FG_BLACK 0
#define FG_DARK_BLUE FOREGROUND_BLUE
#define FG_DARK_GREEN FOREGROUND_GREEN
#define FG_DARK_CYAN (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define FG_DARK_RED FOREGROUND_RED
#define FG_DARK_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE)
#define FG_DARK_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)
#define FG_GRAY (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define FG_DARK_GRAY FOREGROUND_INTENSITY
#define FG_BLUE (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FG_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FG_CYAN (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FG_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FG_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FG_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FG_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

#define BG_BLACK 0
#define BG_DARK_BLUE BACKGROUND_BLUE
#define BG_DARK_GREEN BACKGROUND_GREEN
#define BG_DARK_CYAN (BACKGROUND_GREEN | BACKGROUND_BLUE)
#define BG_DARK_RED BACKGROUND_RED
#define BG_DARK_MAGENTA (BACKGROUND_RED | BACKGROUND_BLUE)
#define BG_DARK_YELLOW (BACKGROUND_RED | BACKGROUND_GREEN)
#define BG_GRAY (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define BG_DARK_GRAY BACKGROUND_INTENSITY
#define BG_BLUE (BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define BG_GREEN (BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define BG_CYAN (BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define BG_RED (BACKGROUND_RED | BACKGROUND_INTENSITY)
#define BG_MAGENTA (BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define BG_YELLOW (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define BG_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY)

#define COLOR(fg, bg) ((fg) | (bg))

struct DoubleBuffer {
    HANDLE front;
    HANDLE back;
    int width, height;
    COORD cursor;
    WORD current_color;
    CHAR_INFO* buffer;
};



class Canvas
{
public:
    Canvas(int width, int height);
    ~Canvas();
    void set_color(WORD color);
    void clear_buffer();
    void move_to(int x, int y);
    void draw_text(const char* text);
    void draw_text_at(int x, int y, const char* text, WORD color);
    void draw_char(char c);
    void draw_char_at(int x, int y, char c, WORD color);
    void draw_hline(int x, int y, int length, char c, WORD color);
    void draw_vline(int x, int y, int length, char c, WORD color);
    void draw_rect(int x, int y, int width, int height, char border, WORD border_color);
    void fill_rect(int x, int y, int width, int height, char fill, WORD fill_color);
    void swap_buffers();
private:
    void free_double_buffer();
    void init_double_buffer(int width, int height);
private:
    DoubleBuffer* db;
};



#endif // _CANVAS_H_