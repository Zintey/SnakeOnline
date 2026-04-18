#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include "canvas.h"
#include "mapinfo.h"
#include "snake.h"
#include "score_spawner.h"
#include "wall_spawner.h"
#include "online.h"

const int FPS = 30;

Canvas* canvas;

MapItemType map[HEIGHT][WIDTH];
Vector2 map_anchor = {0, 5};

Snake* player_1;
Snake* player_2;
std::vector<ScoreSpawner*> score_spawner_list;
WallSpawner* wall_spawner;

void map_init()
{
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            map[i][j] = MapItemType::Null;
    for (int i = 0; i < WIDTH; i++)
        map[0][i] = map[HEIGHT - 1][i] = MapItemType::Wall;

}

void draw_map()
{
    
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            char c = ' ', nc = ' ';
            WORD color = BG_GRAY;
            switch (map[i][j])
            {
            case MapItemType::Null:
                c = ' ';
                color = BG_DARK_GRAY;
                break;
            case MapItemType::Wall:
                c = ' ';
                color = BG_WHITE;
                break;
            }
            canvas->draw_char_at(map_anchor.x + j * 2, map_anchor.y + i, c, color);
            canvas->draw_char_at(map_anchor.x + j * 2 + 1, map_anchor.y + i, nc, color);
        }
    }
}

char get_direction_char(Direction dir)
{
    char c = ' ';
    switch (dir)
    {
    case Direction::Left:
        c = '<';
        break;
    case Direction::Right:
        c = '>';
        break;
    case Direction::Up:
        c = '^';
        break;
    case Direction::Down:
        c = 'v';
        break;
    default:
        break;
    }
    return c;
}

void create_new_game()
{

    bool is_game_over = false;

    map_init();

    player_1 = new Snake(PlayerID::P1, 15, Direction::Right, 4, Vector2{ 3, 3 },
        'w', 's', 'a', 'd', ' ', FG_GREEN, BG_DARK_BLUE, BG_BLUE);
    player_2 = new Snake(PlayerID::P2, 15, Direction::Left, 4, Vector2{ HEIGHT - 4, WIDTH - 4 },
        72, 80, 75, 77, '.', FG_RED, BG_YELLOW, BG_DARK_YELLOW);
    
    score_spawner_list.push_back(new ScoreSpawner(10, 1000, 9000, 
        BG_GREEN, BG_DARK_GREEN, Vector2 { map_anchor.x + 2 * (WIDTH + 1) , map_anchor.y}));

    score_spawner_list.push_back(new ScoreSpawner(50, 5000, 5000, 
        BG_CYAN, BG_DARK_CYAN, Vector2{ map_anchor.x + 2 * (WIDTH + 2) + 1, map_anchor.y}));

    wall_spawner = new WallSpawner(10000);

    clock_t last_tick_time = clock();
    clock_t timer = 0;
    while (!is_game_over)
    {
        clock_t frame_start_time = clock();

        // on_input()
        if (_kbhit())
        {
            int ch = _getch();
            player_1->on_input(ch);
            player_2->on_input(ch);
        }

        clock_t current_tick_time = clock();
        clock_t delta_time = current_tick_time - last_tick_time;
        last_tick_time = current_tick_time;

        // on_update()
        player_1->on_update(delta_time);
        player_2->on_update(delta_time);

        for (ScoreSpawner* score_spawner : score_spawner_list)
            score_spawner->on_update(delta_time);

        wall_spawner->on_update(delta_time);
        // on_draw()

        canvas->clear_buffer();

        timer += delta_time;
        int timer_text_len = strlen("time: ") + std::to_string(timer).length();
        canvas->draw_text_at(map_anchor.x + WIDTH - timer_text_len / 2, map_anchor.y - 4, 
            ("time: " + std::to_string(timer / 1000) + " s").c_str(), FG_GRAY);

        canvas->draw_text_at(map_anchor.x + 5, map_anchor.y - 3, 
            (" 1P Score: " + std::to_string(player_1->get_score()) + " ").c_str(), 
            player_1->get_body_color());

        canvas->draw_text_at(map_anchor.x + 2 * WIDTH - 20, map_anchor.y - 3, 
            (" 2P Score: " + std::to_string(player_2->get_score()) + " ").c_str(), 
            player_2->get_body_color());

        draw_map();

        player_1->on_draw(canvas, map_anchor);
        player_2->on_draw(canvas, map_anchor);

        for (ScoreSpawner* score_spawner : score_spawner_list)
            score_spawner->on_draw(canvas, map_anchor);

        clock_t frame_end_time = clock();
        clock_t frame_draw_time = frame_end_time - frame_start_time;
        
        
        if (player_1->check_is_die() && player_2->check_is_die())
        {
            is_game_over = true;
            int text_len = 9;
            canvas->draw_text_at(map_anchor.x + WIDTH - text_len / 2, map_anchor.y - 2,
            (std::to_string(player_1->get_score() >= player_2->get_score() ? 1 : 2) + "P Win!!!").c_str()
                , FG_YELLOW);
        }
        else
            canvas->swap_buffers();
        if (frame_draw_time < 1000 / FPS)
            Sleep(1000 / FPS - frame_draw_time);
    }

    canvas->draw_text_at(map_anchor.x + WIDTH - 44 / 2, 0,
        "Game Over!! Press R to restart, Esc to quit.", FG_DARK_RED);
    canvas->swap_buffers();

    delete player_1; player_1 = nullptr;
    delete player_2; player_2 = nullptr;
    score_spawner_list.clear();
    delete wall_spawner;
}


void run_game()
{
    bool is_runing = true;
    canvas = new Canvas(WIDTH * 2 + 20, HEIGHT + 20);
    while (is_runing)
    {
        create_new_game();

        char c = _getch();
        while (!(c == 27 || c == 'r')) c = _getch();

        if (c == 27)
            is_runing = false;
    }
    delete canvas;
}


int main()
{
    run_game();
    return 0;
}