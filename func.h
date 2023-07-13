//
// Created by wiktor on 15.10.22.
//

#ifndef GIERKA_FUNCTIONS_H
#define GIERKA_FUNCTIONS_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#define PORT 8000
#define SA struct sockaddr

struct server_t{
    int server_adres;
    int is_active;
    int server_pid;
};

struct poz_t{
        int x;
        int y;
};

struct drop_t{
    struct poz_t p;
    int value;
};

struct beast_t{
    struct poz_t pos;

    int id;

    int player_in_range;
};

struct acc_sock{
    //connection
    int  sock;
    int server_pid;
    //game
    char sign;
    char type[6];
    int money_found;
    int money_brought;
    int deaths;
    int round_number;
    char map[36];
    struct poz_t pos;
    struct poz_t spawn;
    int in_bush;
    int in_bush_wait;
    int in_campsite;
};

struct send_t {
    int znak;
    int pid;
};

enum direction{RIGHT,LEFT,UP,DOWN,SPAWN};

enum moving{BEAST,BUSH,WALL,COIN,SMALL_T,BIG_T,CAMPSITE,FREE,DROP,HUMAN};

//Threads
void create_threads();

//Server
void *server_actions();

void client_wants_to_quit(struct acc_sock *gracz);

void close_sockets();

void create_server();

int helpful_connection();

//Map & info
char **create_map(char* filename);

void printf_info_server();

void printf_new_map();

void re_write_map(char* p_map, struct acc_sock *gracz);

//Game play
enum moving is_move_possible(struct acc_sock *gracz, enum direction dir);

struct poz_t rand_position_in_arr();

//Player creation
void spawn_player(struct acc_sock *gracz);

int amount_of_players();

void* accept_client();
//Drop

void get_drop(struct acc_sock* gracz);

void print_drop();

//Beast

enum direction search_for_player(struct poz_t p);

void chase_player(enum direction dir, struct beast_t *beast);

void *beast_moves();

void killed_player(struct poz_t p);

void make_beast_moves();

//Player game play

void checker_bush_camp_free(struct acc_sock *gracz);

int checker(struct acc_sock *gracz, enum moving dir);

//Game
void *game();
void send_struct_to_client();
void recv_from_players();
void make_players_moves();

#endif //GIERKA_FUNCTIONS_H
