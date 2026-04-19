#include "game_net.h"   
//#include "online.h"   

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <string>
#include <vector>
#include <atomic>
#include "canvas.h"
#include "mapinfo.h"
#include "snake.h"
#include "score_spawner.h"
#include "wall_spawner.h"

const int FPS = 30;

Canvas* canvas;

MapItemType map[HEIGHT][WIDTH];
Vector2 map_anchor = { 0, 5 };

//Snake* player_1;
//Snake* player_2;
std::vector<std::shared_ptr<Snake>> players;
std::vector<ScoreSpawner*> score_spawner_list;
//WallSpawner* wall_spawner;

GameServer* game_server = nullptr;
GameClient* game_client = nullptr;

void map_init()
{
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            map[i][j] = MapItemType::Null;
    for (int i = 0; i < WIDTH; i++)
        map[0][i] = map[HEIGHT - 1][i] = MapItemType::Null;
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
    case Direction::Left:  c = '<'; break;
    case Direction::Right: c = '>'; break;
    case Direction::Up:    c = '^'; break;
    case Direction::Down:  c = 'v'; break;
    default: break;
    }
    return c;
}



void create_new_game()
{
    bool is_game_over = false;
    map_init();
    players.clear();
    auto configs = game_client->get_players_config();
    for (int i = 0; i < game_client->get_all_player_cnt(); ++i) {
        auto& cfg = configs[i];
        players.push_back(std::make_shared<Snake>(
            (PlayerID)cfg.player_id, 15, (Direction)cfg.dir, 4,
            Vector2{ cfg.spawn_x, cfg.spawn_y },
            cfg.eyes_color, cfg.color, cfg.color
        ));
    }

    score_spawner_list.push_back(new ScoreSpawner(10, 1000, 9000,
        BG_GREEN, BG_DARK_GREEN, Vector2{ map_anchor.x + 2 * (WIDTH + 1), map_anchor.y }));
    score_spawner_list.push_back(new ScoreSpawner(50, 5000, 5000,
        BG_CYAN, BG_DARK_CYAN, Vector2{ map_anchor.x + 2 * (WIDTH + 2) + 1, map_anchor.y }));

    //wall_spawner = new WallSpawner(10000);

    clock_t last_tick_time = clock();
    clock_t timer = 0;
    while (!is_game_over)
    {
        clock_t frame_start_time = clock();

        if (_kbhit())
        {
            int ch = _getch();
            while (_kbhit()) _getch();
            if (game_client->get_player_id() < players.size())
            {
				InputMsg input_msg{ game_client->get_player_id(), (char)ch };
				game_client->send_msg<InputMsg>(MsgType::Input, input_msg);

            }
        }
        while (true)
        {
            auto [type, body] = game_client->receive_msg<AllPlayerInputMsg>();
            if (type == MsgType::Input)
            {
                for (int i = 0; i < game_client->get_all_player_cnt(); ++i) {
                    if (body.inputs[i] != 0) {
                        players[i]->on_input(body.inputs[i]);
                    }
                }
                break;
            }
        }
        //clock_t current_tick_time = clock();
        clock_t delta_time = 1000 / 30;
        //last_tick_time = current_tick_time;

        for (auto& p : players) p->on_update(delta_time);

        for (ScoreSpawner* score_spawner : score_spawner_list)
            score_spawner->on_update(delta_time);

        //wall_spawner->on_update(delta_time);

        canvas->clear_buffer();

        timer += delta_time;
        int timer_text_len = strlen("time: ") + std::to_string(timer).length();
        canvas->draw_text_at(map_anchor.x + WIDTH - timer_text_len / 2, map_anchor.y - 4,
            ("time: " + std::to_string(timer / 1000) + " s").c_str(), FG_GRAY);

        for (int i = 0; i < players.size(); i++) {
            canvas->draw_text_at(map_anchor.x + 5 + i * 18, map_anchor.y - 3,
                (" P" + std::to_string(i + 1) + " Score: " + std::to_string(players[i]->get_score()) + " ").c_str(),
                players[i]->get_body_color());
        }

        draw_map();

        for (auto& p : players) p->on_draw(canvas, map_anchor);

        for (ScoreSpawner* score_spawner : score_spawner_list)
            score_spawner->on_draw(canvas, map_anchor);

        clock_t frame_end_time = clock();
        clock_t frame_draw_time = frame_end_time - frame_start_time;

        int dead_count = 0;
        int max_score = -1, winner = -1;
        for (int i = 0; i < players.size(); i++) {
            if (players[i]->check_is_die()) dead_count++;
            if (players[i]->get_score() > max_score) {
                max_score = players[i]->get_score();
                winner = i + 1;
            }
        }

        if (dead_count == players.size() && players.size() > 0)
        {
            is_game_over = true;
            canvas->draw_text_at(map_anchor.x + WIDTH - 4, map_anchor.y - 2,
                (std::to_string(winner) + "P Win!!!").c_str(), FG_YELLOW);
        }
        else
            canvas->swap_buffers();

        //if (frame_draw_time < 1000 / FPS)
            //Sleep(1000 / FPS - frame_draw_time);
    }

    canvas->draw_text_at(map_anchor.x + WIDTH - 44 / 2, 0,
        "Game Over!! Press R to restart, Esc to quit.", FG_DARK_RED);
    canvas->swap_buffers();

    players.clear();
    score_spawner_list.clear();
    //delete wall_spawner;
}
void run();
void run_game()
{
    srand(game_client->get_game_seed());
    bool is_runing = true;
    canvas = new Canvas(WIDTH * 2 + 20, HEIGHT + 20);
    while (is_runing)
    {
        create_new_game();

        char c = _getch();
        while (!(c == 27 || c == 'r')) c = _getch();

        if (c == 27)
            is_runing = false;
        else {
			delete canvas;
            return run();
        }
    }
    delete canvas;
}


void create_room()
{
    game_server = new GameServer();
    system("cls");
    std::cout << "waiting for other player to join..." << std::endl;
    std::cout << "press Enter to start" << std::endl;

    game_server->create_room(8080, [](std::string ip) {
        std::cout << ip << " join" << std::endl;
        },
        [&]() {
            std::thread([]() {
                game_client = new GameClient();
                game_client->list_room();
                game_client->join_room(0);
                game_client->wait_start();
                }).detach();
        });

    std::cout << "start game!\n" << std::endl;
    game_server->start_game();
    run_game();
}

void join_room()
{
    system("cls");
    std::cout << "searching for room..." << std::endl;

    game_client = new GameClient();
    auto ips = game_client->list_room();

    if (ips.empty())
    {
        std::cout << "no room found. press any key to exit." << std::endl;
        _getch();
        return;
    }

    for (size_t i = 0; i < ips.size(); ++i)
    {
        std::cout << "[" << i << "] Room found: " << ips[i] << std::endl;
    }

    int choice = 0;
    if (ips.size() > 1) {
        std::cout << "Enter room number to join: ";
        std::cin >> choice;
    }
    else {
        std::cout << "Joining room 0..." << std::endl;
    }

    if (!game_client->join_room(choice))
    {
        std::cout << "connect failed or invalid choice, press any key to exit." << std::endl;
        _getch();
        return;
    }

    std::cout << "connected! waiting for host to start..." << std::endl;

    if (game_client->wait_start())
    {
        run_game();
    }
}

void run()
{
    system("cls");
    std::cout << "1. create room" << std::endl;
    std::cout << "2. join room" << std::endl;
    char c = _getch();
    while (c != '1' && c != '2') c = _getch();
    if (c == '1')
        create_room();
    else
        join_room();
}

int main()
{
    Logger::instance().set_file("log.txt");
    LOG_INFO("start");
    run();
    return 0;
}