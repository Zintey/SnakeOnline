#include <stdio.h>
#include <string.h>
#include <Windows.h>

// 颜色属性定义 (前景色和背景色组合)
// 前景色
#define FG_BLACK 0
#define FG_DARK_BLUE FOREGROUND_BLUE
#define FG_DARK_GREEN FOREGROUND_GREEN
#define FG_DARK_CYAN FOREGROUND_GREEN | FOREGROUND_BLUE
#define FG_DARK_RED FOREGROUND_RED
#define FG_DARK_MAGENTA FOREGROUND_RED | FOREGROUND_BLUE
#define FG_DARK_YELLOW FOREGROUND_RED | FOREGROUND_GREEN
#define FG_GRAY FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
#define FG_DARK_GRAY FOREGROUND_INTENSITY
#define FG_BLUE FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define FG_GREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define FG_CYAN FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define FG_RED FOREGROUND_RED | FOREGROUND_INTENSITY
#define FG_MAGENTA FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define FG_YELLOW FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define FG_WHITE FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY

// 背景色
#define BG_BLACK 0
#define BG_DARK_BLUE BACKGROUND_BLUE
#define BG_DARK_GREEN BACKGROUND_GREEN
#define BG_DARK_CYAN BACKGROUND_GREEN | BACKGROUND_BLUE
#define BG_DARK_RED BACKGROUND_RED
#define BG_DARK_MAGENTA BACKGROUND_RED | BACKGROUND_BLUE
#define BG_DARK_YELLOW BACKGROUND_RED | BACKGROUND_GREEN
#define BG_GRAY BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
#define BG_DARK_GRAY BACKGROUND_INTENSITY
#define BG_BLUE BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define BG_GREEN BACKGROUND_GREEN | BACKGROUND_INTENSITY
#define BG_CYAN BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define BG_RED BACKGROUND_RED | BACKGROUND_INTENSITY
#define BG_MAGENTA BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define BG_YELLOW BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY
#define BG_WHITE BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY

// 组合颜色
#define COLOR(fg, bg) (fg | bg)

// 双缓冲结构体
typedef struct {
    HANDLE front;   // 前台缓冲区（当前显示）
    HANDLE back;    // 后台缓冲区（绘制目标）
    int width;      // 控制台宽度
    int height;     // 控制台高度
    COORD cursor;   // 当前光标位置
    WORD current_color; // 当前颜色属性
} DoubleBuffer;

// 初始化双缓冲系统
DoubleBuffer init_double_buffer(int width, int height) {
    DoubleBuffer db;

    // 创建两个缓冲区
    db.front = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    db.back = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    // 设置缓冲区大小
    COORD size = { (SHORT)width, (SHORT)height };
    SetConsoleScreenBufferSize(db.front, size);
    SetConsoleScreenBufferSize(db.back, size);

    // 设置窗口大小
    SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
    SetConsoleWindowInfo(db.front, TRUE, &rect);
    SetConsoleWindowInfo(db.back, TRUE, &rect);

    // 设置活动缓冲区
    SetConsoleActiveScreenBuffer(db.front);

    // 隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(db.front, &cursorInfo);
    SetConsoleCursorInfo(db.back, &cursorInfo);

    db.width = width;
    db.height = height;
    db.cursor.X = 0;
    db.cursor.Y = 0;
    db.current_color = FG_WHITE | BG_BLACK; // 默认白字黑底

    return db;
}

// 设置当前颜色（影响后续绘制）
void set_color(DoubleBuffer* db, WORD color) {
    db->current_color = color;
}

// 清除缓冲区内容
void clear_buffer(DoubleBuffer* db) {
    COORD coord = { 0, 0 };
    DWORD count;

    // 获取缓冲区信息
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(db->back, &csbi);

    // 填充空格字符
    DWORD length = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(db->back, ' ', length, coord, &count);

    // 使用当前颜色填充属性
    FillConsoleOutputAttribute(db->back, db->current_color, length, coord, &count);

    // 重置光标位置
    SetConsoleCursorPosition(db->back, coord);
    db->cursor.X = 0;
    db->cursor.Y = 0;
}

// 移动到指定位置
void move_to(DoubleBuffer* db, int x, int y) {
    db->cursor.X = (SHORT)x;
    db->cursor.Y = (SHORT)y;
    SetConsoleCursorPosition(db->back, db->cursor);
}

// 绘制文本到缓冲区（使用当前颜色）
void draw_text(DoubleBuffer* db, const char* text) {
    DWORD bytesWritten;
    WriteConsoleOutputCharacterA(db->back, text, strlen(text), db->cursor, &bytesWritten);

    // 设置文本颜色属性
    FillConsoleOutputAttribute(db->back, db->current_color, strlen(text), db->cursor, &bytesWritten);

    // 更新光标位置
    db->cursor.X += (SHORT)strlen(text);
}

// 在指定位置绘制文本（带颜色）
void draw_text_at(DoubleBuffer* db, int x, int y, const char* text, WORD color) {
    COORD save = db->cursor;
    WORD save_color = db->current_color;

    move_to(db, x, y);
    set_color(db, color);
    draw_text(db, text);

    // 恢复位置和颜色
    db->cursor = save;
    db->current_color = save_color;
    SetConsoleCursorPosition(db->back, save);
}

// 绘制字符到缓冲区（使用当前颜色）
void draw_char(DoubleBuffer* db, char c) {
    DWORD bytesWritten;
    WriteConsoleOutputCharacterA(db->back, &c, 1, db->cursor, &bytesWritten);

    // 设置字符颜色属性
    FillConsoleOutputAttribute(db->back, db->current_color, 1, db->cursor, &bytesWritten);

    // 更新光标位置
    db->cursor.X++;
}

// 在指定位置绘制字符（带颜色）
void draw_char_at(DoubleBuffer* db, int x, int y, char c, WORD color) {
    COORD save = db->cursor;
    WORD save_color = db->current_color;

    move_to(db, x, y);
    set_color(db, color);
    draw_char(db, c);

    // 恢复位置和颜色
    db->cursor = save;
    db->current_color = save_color;
    SetConsoleCursorPosition(db->back, save);
}

// 绘制水平线
void draw_hline(DoubleBuffer* db, int x, int y, int length, char c, WORD color) {
    for (int i = 0; i < length; i++) {
        draw_char_at(db, x + i, y, c, color);
    }
}

// 绘制垂直线
void draw_vline(DoubleBuffer* db, int x, int y, int length, char c, WORD color) {
    for (int i = 0; i < length; i++) {
        draw_char_at(db, x, y + i, c, color);
    }
}

// 绘制矩形边框
void draw_rect(DoubleBuffer* db, int x, int y, int width, int height, char border, WORD border_color) {
    // 上边框
    draw_hline(db, x, y, width, border, border_color);

    // 下边框
    draw_hline(db, x, y + height - 1, width, border, border_color);

    // 左边框
    draw_vline(db, x, y + 1, height - 2, border, border_color);

    // 右边框
    draw_vline(db, x + width - 1, y + 1, height - 2, border, border_color);
}

// 填充矩形区域
void fill_rect(DoubleBuffer* db, int x, int y, int width, int height, char fill, WORD fill_color) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            draw_char_at(db, x + col, y + row, fill, fill_color);
        }
    }
}

// 交换缓冲区
void swap_buffers(DoubleBuffer* db) {
    // 将后台缓冲区设为活动（显示）
    SetConsoleActiveScreenBuffer(db->back);

    // 交换前后台缓冲区
    HANDLE temp = db->front;
    db->front = db->back;
    db->back = temp;

    // 重置新后台缓冲区的光标位置
    db->cursor.X = 0;
    db->cursor.Y = 0;
    SetConsoleCursorPosition(db->back, db->cursor);
}

// 释放资源
void free_double_buffer(DoubleBuffer* db) {
    // 恢复标准输出
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleActiveScreenBuffer(hStdOut);

    // 关闭缓冲区
    CloseHandle(db->front);
    CloseHandle(db->back);
}
