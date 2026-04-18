#include "canvas.h"

Canvas::Canvas(int width, int height)
{
    init_double_buffer(width, height);
}
Canvas::~Canvas()
{
    free_double_buffer();
}
void Canvas::set_color(WORD color) {
    db->current_color = color;
}

void Canvas::clear_buffer() {
    for (int i = 0; i < db->width * db->height; i++) {
        db->buffer[i].Char.UnicodeChar = L' ';
        db->buffer[i].Attributes = db->current_color;
    }
    db->cursor.X = 0;
    db->cursor.Y = 0;
}

void Canvas::move_to(int x, int y) {
    db->cursor.X = (SHORT)x;
    db->cursor.Y = (SHORT)y;
}

void Canvas::draw_text(const char* text) {
    int x = db->cursor.X;
    int y = db->cursor.Y;
    for (size_t i = 0; i < strlen(text); i++) {
        int index = y * db->width + x + i;
        if (index >= 0 && index < db->width * db->height) {
            db->buffer[index].Char.UnicodeChar = (WCHAR)text[i];
            db->buffer[index].Attributes = db->current_color;
        }
    }
    db->cursor.X += (SHORT)strlen(text);
}

void Canvas::draw_text_at(int x, int y, const char* text, WORD color) {
    COORD save = db->cursor;
    WORD save_color = db->current_color;

    move_to(x, y);
    set_color(color);
    draw_text(text);

    db->cursor = save;
    db->current_color = save_color;
}

void Canvas::draw_char(char c) {
    int index = db->cursor.Y * db->width + db->cursor.X;
    if (index >= 0 && index < db->width * db->height) {
        db->buffer[index].Char.UnicodeChar = (WCHAR)c;
        db->buffer[index].Attributes = db->current_color;
    }
    db->cursor.X++;
}

void Canvas::draw_char_at(int x, int y, char c, WORD color) {
    COORD save = db->cursor;
    WORD save_color = db->current_color;

    move_to(x, y);
    set_color(color);
    draw_char(c);

    db->cursor = save;
    db->current_color = save_color;
}

void Canvas::draw_hline(int x, int y, int length, char c, WORD color) {
    for (int i = 0; i < length; i++) {
        draw_char_at(x + i, y, c, color);
    }
}

void Canvas::draw_vline(int x, int y, int length, char c, WORD color) {
    for (int i = 0; i < length; i++) {
        draw_char_at(x, y + i, c, color);
    }
}

void Canvas::draw_rect(int x, int y, int width, int height, char border, WORD border_color) {
    draw_hline(x, y, width, border, border_color);
    draw_hline(x, y + height - 1, width, border, border_color);
    draw_vline(x, y + 1, height - 2, border, border_color);
    draw_vline(x + width - 1, y + 1, height - 2, border, border_color);
}

void Canvas::fill_rect(int x, int y, int width, int height, char fill, WORD fill_color) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            draw_char_at(x + col, y + row, fill, fill_color);
        }
    }
}


void Canvas::swap_buffers() {
    COORD bufSize = { (SHORT)db->width, (SHORT)db->height };
    COORD coord = { 0, 0 };
    SMALL_RECT rect = { 0, 0, (SHORT)(db->width - 1), (SHORT)(db->height - 1) };

    WriteConsoleOutputW(db->back, db->buffer, bufSize, coord, &rect);
    SetConsoleActiveScreenBuffer(db->back);

    HANDLE tmp = db->front;
    db->front = db->back;
    db->back = tmp;
    db->cursor.X = 0;
    db->cursor.Y = 0;
}

void Canvas::free_double_buffer() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleActiveScreenBuffer(hStdOut);

    CloseHandle(db->front);
    CloseHandle(db->back);
    free(db->buffer);
}

void Canvas::init_double_buffer(int width, int height) {
    db = new DoubleBuffer();
    db->width = width;
    db->height = height;
    db->cursor.X = 0;
    db->cursor.Y = 0;
    db->current_color = FG_WHITE | BG_BLACK;

    db->front = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    db->back = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    COORD size = { (SHORT)width, (SHORT)height };
    SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };

    SetConsoleScreenBufferSize(db->front, size);
    SetConsoleScreenBufferSize(db->back, size);
    SetConsoleWindowInfo(db->front, TRUE, &rect);
    SetConsoleWindowInfo(db->back, TRUE, &rect);

    SetConsoleActiveScreenBuffer(db->front);

    CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
    SetConsoleCursorInfo(db->front, &cursorInfo);
    SetConsoleCursorInfo(db->back, &cursorInfo);

    db->buffer = (CHAR_INFO*)malloc(sizeof(CHAR_INFO) * width * height);
    clear_buffer();
}




//void set_windows_size(int col, int row)
//{
//    char cmd[64];
//    sprintf_s(cmd, "mode con cols=%d lines=%d", col, row);
//    system(cmd);
//    SetWindowLongPtrA(GetConsoleWindow(), GWL_STYLE, GetWindowLongPtrA(GetConsoleWindow(), GWL_STYLE) & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX);
//}

