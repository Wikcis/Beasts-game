#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>
#include "func.h"
#define NUMBER_OF_PLAYERS 4


//Threads
pthread_t threads[2];
pthread_t beasts_threads[5];

//Server
struct server_t server_socket;
struct sockaddr_in server_adr;

//Map
char** t;

//Game play
int round_number=0;
int row,col;

//Players
struct acc_sock gracze[5];
struct send_t recv_s_arr[5];

//Beast
struct beast_t b[5];
int beast_counter=0;

//Drop
struct drop_t drop[1000];
int drop_counter=0;
int beast_moves_arr[5];

//Mutex
pthread_mutex_t mutex;

//Threads
void create_threads()
{
    pthread_create(&threads[0], NULL, accept_client, NULL);
    pthread_create(&threads[1], NULL, server_actions, NULL);
}

//Server
void *server_actions()
{
    while(server_socket.is_active)
    {
        struct poz_t rand_pos=rand_position_in_arr();
        int znak=getchar();

        if(znak=='c')
        {
            t[rand_pos.x][rand_pos.y]='c';
        }
        else if(znak=='t')
        {
            t[rand_pos.x][rand_pos.y]='t';
        }
        else if(znak=='T')
        {
            t[rand_pos.x][rand_pos.y]='T';
        }
        else if(znak=='b' || znak=='B')
        {
            if(beast_counter<5)
            {
                pthread_create(&beasts_threads[beast_counter],NULL,beast_moves,NULL);
            }
        }
        else if(znak=='q' || znak=='Q')
        {
            close_sockets();
            server_socket.is_active=0;
            helpful_connection();

        }
    }

    return NULL;
}

void client_wants_to_quit(struct acc_sock *gracz)
{
    close(gracz->sock);
    struct poz_t zero;
    zero.x=0,zero.y=0;

    gracz->sock=0;
    gracz->server_pid=0;

    gracz->sign=0;
    gracz->money_found=0;
    gracz->money_brought=0;
    gracz->deaths=0;
    gracz->round_number=0;

    gracz->pos=zero;
    gracz->spawn=zero;
}

void close_sockets()
{
    for(int i=0; i<NUMBER_OF_PLAYERS; i++)
    {
        if(gracze[i].sock!=0)
        {
            gracze[i].server_pid=-1;
            send(gracze[i].sock,&gracze[i],sizeof(struct acc_sock),0);
        }
    }
}

void create_server()
{
    server_socket.server_adres = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket.server_adres == -1) {
        printf("socket creation failed\n");
        exit (1);
    }
    bzero(&server_adr, sizeof(server_adr));

    server_adr.sin_family = AF_INET;
    server_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_adr.sin_port = htons(PORT);

    int res = bind(server_socket.server_adres, (SA *) &server_adr, sizeof(server_adr));
    if (res != 0) {
        printf("socket bind failed\n");
        exit (1);
    }

    server_socket.is_active=1;
    server_socket.server_pid=getpid();
    res = listen(server_socket.server_adres, 5);
    if (res != 0) {
        printf("Listen failed\n");
        exit (1);
    }

    for (int i = 0; i < NUMBER_OF_PLAYERS; i++) {
        gracze[i].sock = 0;
    }
}

int helpful_connection()
{
    struct sockaddr_in server_adr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return -1;
    }
    bzero(&server_adr, sizeof(server_adr));

    server_adr.sin_family = AF_INET;
    server_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_adr.sin_port = htons(PORT);

    int res=connect(sock, (SA*)&server_adr, sizeof(server_adr));

    if (res!= 0) {
        return -1;
    }
    return 0;
}

//Map & info
char **create_map(char* filename)
{
    char** t=calloc(25,sizeof(char *));
    for(int i=0; i<25; i++)
        t[i]=calloc(53,sizeof(char));

    FILE *plik=fopen(filename,"r");

    int i=0;
    while(!feof(plik))
    {
        fscanf(plik,"%[^\n]\n",*(t+i));
        i++;
    }

    return t;
}

void printf_info_server()
{
    //server
    mvprintw(row/2+1,col/2+53,"Server's PID: %d",server_socket.server_pid);
    mvprintw(row/2+2,col/2+54,"Campsite X/Y: %d/%d",23,11);
    mvprintw(row/2+3,col/2+54,"Round number: %d",round_number);
    mvprintw(row/2+5,col/2+53,"Parameter:   Player1  Player2  Player3  Player4");
    mvprintw(row/2+6,col/2+54,"PID: ");
    mvprintw(row/2+7,col/2+54,"Type: ");
    clrtoeol();
    mvprintw(row/2+8,col/2+54,"Cur X/Y: ");
    clrtoeol();
    mvprintw(row/2+9,col/2+54,"Deaths: ");
    clrtoeol();
    mvprintw(row/2+10,col/2+54,"Coins: ");
    clrtoeol();
    mvprintw(row/2+11,col/2+55,"carried: ");
    clrtoeol();
    mvprintw(row/2+12,col/2+55,"brought: ");
    clrtoeol();

    //players
    for(int i=0; i<NUMBER_OF_PLAYERS; i++)
    {
        if(gracze[i].sock!=0)
        {
            mvprintw(row/2+6,col/2+(66+i*8+i),"%d",recv_s_arr[i].pid);
            clrtoeol();
            mvprintw(row/2+7,col/2+(66+i*8+i),"%s",gracze[i].type);
            mvprintw(row/2+8,col/2+(66+i*8+i),"%d/%d",gracze[i].pos.x,gracze[i].pos.y);
            clrtoeol();
            mvprintw(row/2+9,col/2+(66+i*8+i),"%d",gracze[i].deaths);
            clrtoeol();
            mvprintw(row/2+11,col/2+(66+i*8+i),"%d",gracze[i].money_found);
            clrtoeol();
            mvprintw(row/2+12,col/2+(66+i*8+i),"%d",gracze[i].money_brought);
            clrtoeol();
        }
        else
        {
            mvprintw(row/2+6,col/2+(66+i*8+i),"-");
            clrtoeol();
            mvprintw(row/2+7,col/2+(66+i*8+i),"-");
            clrtoeol();
            mvprintw(row/2+8,col/2+(66+i*8+i),"-/-");
            clrtoeol();
            mvprintw(row/2+9,col/2+(66+i*8+i),"-");
            clrtoeol();
            mvprintw(row/2+11,col/2+(66+i*8+i),"-");
            clrtoeol();
            mvprintw(row/2+12,col/2+(66+i*8+i),"-");
            clrtoeol();
        }
    }

    //Legend
    mvprintw(row/2+15,col/2+54,"Legend: ");
    mvprintw(row/2+16,col/2+54,"1234 - players");
    mvprintw(row/2+17,col/2+54,"|    - wall");
    mvprintw(row/2+18,col/2+54,"#    - bushes (slow down)");
    mvprintw(row/2+19,col/2+54,"*    - wild beast");
    mvprintw(row/2+20,col/2+54,"c    - one coin");
    mvprintw(row/2+21,col/2+54,"t    - treasure (10 coins)");
    mvprintw(row/2+22,col/2+54,"T    - large treasure (50 coins)");
    mvprintw(row/2+23,col/2+54,"A    - campsite");
    mvprintw(row/2+24,col/2+54,"D    - dropped treasure");
    refresh();
}

void printf_new_map()
{

    for(int i=0; i<25; i++)
    {
        for(int j=0; j<51; j++)
        {
            if(t[i][j]=='|')
                mvaddch(i,j, ACS_CKBOARD);
            else if(isdigit(t[i][j]))
            {
                mvaddch(i,j,t[i][j]);
            }
            else
                mvprintw(i,j,"%c",t[i][j]);
        }

    }
    mvprintw(row/2+25,col/2,"\n");
    mvprintw(row/2+3,col/2+54,"Round number: %d",round_number);
    refresh();
}

void re_write_map(char* p_map, struct acc_sock *gracz)
{
    struct poz_t p=gracz->pos;
    if(p.x-2<0 && p.y-2<0)
    {
        int j1=0;
        for(int i=p.x-2; i<0; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
        for(int i=0; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<0; j++,j1++)
            {
                p_map[j1]=' ';
            }
            for(int j=0; j<p.y+3; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.x-2<0 && p.y+2>=51)
    {
        int j1=0;
        for(int i=p.x-2; i<0; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
        for(int i=0; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<51; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            for(int j=51; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.x+2>24 && p.y+2>=51)
    {
        int j1=0;
        for(int i=p.x-2; i<25; i++)
        {
            for(int j=p.y-2; j<51; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            for(int j=51; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
        for(int i=24; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.x+2>24 && p.y-2<0)
    {
        int j1=0;
        for(int i=p.x-2; i<25; i++)
        {
            for(int j=p.y-2; j<0; j++,j1++)
            {
                p_map[j1]=' ';
            }
            for(int j=0; j<p.y+3; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }

            p_map[j1]='\n';
            j1++;
        }
        for(int i=25; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.x-2<0)
    {
        int j1=0;
        for(int i=p.x-2; i<0; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
        for(int i=0; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.x+2>24)
    {
        int j1=0;
        for(int i=p.x-2; i<25; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            p_map[j1]='\n';
            j1++;
        }
        for(int i=25; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=' ';
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.y-2<0)
    {
        int j1=0;
        for(int i=p.x-2; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<0; j++,j1++)
            {
                p_map[j1]=' ';
            }
            for(int j=0; j<p.y+3; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            p_map[j1]='\n';
            j1++;
        }
    }
    else if(p.y+2>=51)
    {
        int j1=0;
        for(int i=p.x-2; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<51; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            for(int k=51; k<p.y+3; k++,j1++)
                p_map[j1]=' ';

            p_map[j1]='\n';
            j1++;
        }
    }
    else{
        int j1=0;
        for(int i=p.x-2; i<p.x+3; i++)
        {
            for(int j=p.y-2; j<p.y+3; j++,j1++)
            {
                p_map[j1]=t[i][j];
            }
            p_map[j1]='\n';
            j1++;
        }
    }

}

//Game play
enum moving is_move_possible(struct acc_sock *gracz, enum direction dir)
{
    struct poz_t *p=&gracz->pos;
    if(dir==UP)
    {
        if(t[p->x-1][p->y]=='|')
            return WALL;
        else if(t[p->x-1][p->y]=='#')
            return BUSH;
        else if(t[p->x-1][p->y]=='*')
            return BEAST;
        else if(t[p->x-1][p->y]=='A')
            return CAMPSITE;
        else if(t[p->x-1][p->y]=='t')
            return SMALL_T;
        else if(t[p->x-1][p->y]=='T')
            return BIG_T;
        else if(t[p->x-1][p->y]=='c')
            return COIN;
        else if(t[p->x-1][p->y]=='D')
            return DROP;
        else if(isdigit(t[p->x-1][p->y]))
            return HUMAN;
        else
            return FREE;
    }
    else if(dir==DOWN)
    {
        if(t[p->x+1][p->y]=='|')
            return WALL;
        else if(t[p->x+1][p->y]=='#')
            return BUSH;
        else if(t[p->x+1][p->y]=='*')
            return BEAST;
        else if(t[p->x+1][p->y]=='A')
            return CAMPSITE;
        else if(t[p->x+1][p->y]=='t')
            return SMALL_T;
        else if(t[p->x+1][p->y]=='T')
            return BIG_T;
        else if(t[p->x+1][p->y]=='c')
            return COIN;
        else if(t[p->x+1][p->y]=='D')
            return DROP;
        else if(isdigit(t[p->x+1][p->y]))
            return HUMAN;
        else
            return FREE;
    }
    else if(dir==LEFT)
    {
        if(t[p->x][p->y-1]=='|')
            return WALL;
        else if(t[p->x][p->y-1]=='#')
            return BUSH;
        else if(t[p->x][p->y-1]=='*')
            return BEAST;
        else if(t[p->x][p->y-1]=='A')
            return CAMPSITE;
        else if(t[p->x][p->y-1]=='t')
            return SMALL_T;
        else if(t[p->x][p->y-1]=='T')
            return BIG_T;
        else if(t[p->x][p->y-1]=='c')
            return COIN;
        else if(t[p->x][p->y-1]=='D')
            return DROP;
        else if(isdigit(t[p->x][p->y-1]))
            return HUMAN;
        else
            return FREE;
    }
    else if(dir==RIGHT){
        if(t[p->x][p->y+1]=='|')
            return WALL;
        else if(t[p->x][p->y+1]=='#')
            return BUSH;
        else if(t[p->x][p->y+1]=='*')
            return BEAST;
        else if(t[p->x][p->y+1]=='A')
            return CAMPSITE;
        else if(t[p->x][p->y+1]=='t')
            return SMALL_T;
        else if(t[p->x][p->y+1]=='T')
            return BIG_T;
        else if(t[p->x][p->y+1]=='c')
            return COIN;
        else if(t[p->x][p->y+1]=='D')
            return DROP;
        else if(isdigit(t[p->x][p->y+1]))
            return HUMAN;
        else
            return FREE;
    }
    else{
        if(t[p->x][p->y]=='|')
            return WALL;
        else if(t[p->x][p->y]=='#')
            return BUSH;
        else if(t[p->x][p->y]=='*')
            return BEAST;
        else if(t[p->x][p->y]=='A')
            return CAMPSITE;
        else if(t[p->x][p->y]=='t')
            return SMALL_T;
        else if(t[p->x][p->y]=='T')
            return BIG_T;
        else if(t[p->x][p->y]=='c')
            return COIN;
        else if(t[p->x][p->y]=='D')
            return DROP;
        else if(isdigit(t[p->x][p->y]))
            return HUMAN;
        else
            return FREE;
    }

}

struct poz_t rand_position_in_arr()
{
    struct poz_t tmp_poz;
    while(1)
    {
        tmp_poz.x=rand()%25;
        tmp_poz.y=rand()%51;
        if(t[tmp_poz.x][tmp_poz.y]==' ')
            break;
    }
    return tmp_poz;
}

//Player creation
void spawn_player(struct acc_sock *gracz)
{
    gracz->spawn= rand_position_in_arr();
    t[gracz->spawn.x][gracz->spawn.y]=gracz->sign;
    gracz->pos=gracz->spawn;
}

int amount_of_players()
{
    int count=0;
    for(int i=0; i<NUMBER_OF_PLAYERS; i++)
    {
        if(gracze[i].sock!=0)
            count++;
    }
    return count;
}

void* accept_client()
{
    while(server_socket.is_active)
    {
        for(int i=0; i<NUMBER_OF_PLAYERS+1; i++)
        {
            int amount = amount_of_players();
            if (server_socket.is_active == 1 && amount < 4)
            {
                if (gracze[i].sock == 0)
                {
                    int client = accept(server_socket.server_adres, NULL, NULL);
                    if(server_socket.is_active==0)
                    {
                        break;
                    }
                    gracze[i].sock = client;
                    gracze[i].deaths = 0;
                    gracze[i].money_found = 0;
                    gracze[i].money_brought = 0;
                    gracze[i].spawn.x = 0;
                    gracze[i].in_bush = 0;
                    gracze[i].server_pid=getpid();
                    gracze[i].in_campsite = 0;
                    strcpy(gracze[i].type, "HUMAN");
                    gracze[i].sign = (i + 1) + '0';
                    spawn_player(&gracze[i]);
                    re_write_map(gracze[i].map, &gracze[i]);
                    printf_new_map();
                    send(gracze[i].sock, &gracze[i], sizeof(struct acc_sock), 0);
                    break;
                }
            } else if (amount == 4) {
                int client = accept(server_socket.server_adres, NULL, NULL);
                struct acc_sock cl;
                cl.sock = -1;
                send(client, &cl, sizeof(struct acc_sock), 0);
                break;
            } else {
                break;
            }
        }
    }
    return NULL;
}

//Drop

void get_drop(struct acc_sock* gracz)
{
    for(int i=0; i<drop_counter; i++)
    {
        if(gracz->pos.x==drop[i].p.x && gracz->pos.y==drop[i].p.y)
        {
            gracz->money_found+=drop[i].value;
            drop[i].p.x=0;
            drop[i].p.y=0;
            drop[i].value=0;
            break;
        }
    }
}

void print_drop()
{
    for(int i=0; i<drop_counter; i++)
    {
    if(drop[i].p.x!=0 && drop[i].p.y!=0 && drop[i].value!=0)
        {
            struct poz_t p=drop[i].p;
            t[p.x][p.y]='D';
        }
    }
}

//Beast

enum direction search_for_player(struct poz_t p){

    if(isdigit(t[p.x-1][p.y]))
    {
        return UP;
    }
    else if(isdigit(t[p.x+1][p.y]))
    {
        return DOWN;
    }
    else if (isdigit(t[p.x][p.y-1]))
    {
        return LEFT;
    }
    else if (isdigit(t[p.x][p.y+1]))
    {
        return RIGHT;
    }
    else if(t[p.x-1][p.y]==' ')
    {
        if(isdigit(t[p.x-2][p.y]))
        {
            return UP;
        }
    }
    else if(t[p.x+1][p.y]==' ')
    {
        if(isdigit(t[p.x+2][p.y]))
        {
            return DOWN;
        }
    }
    else if(t[p.x][p.y+1]==' ')
    {
        if(isdigit(t[p.x][p.y+2]))
        {
            return RIGHT;
        }
    }
    else if(t[p.x][p.y-1]==' ')
    {
        if(isdigit(t[p.x][p.y-2]))
        {
            return LEFT;
        }
    }
    return SPAWN;
}

void chase_player(enum direction dir, struct beast_t *beast)
{
    struct poz_t p=beast->pos;
    int znak;
    if(dir == DOWN)
    {
        znak = KEY_DOWN;
        p.x++;
        beast_moves_arr[beast->id]=znak;
    }
    else if(dir == UP)
    {
        znak = KEY_UP;
        p.x--;
        beast_moves_arr[beast->id]=znak;
    }
    else if(dir == RIGHT)
    {
        znak = KEY_RIGHT;
        p.y++;
        beast_moves_arr[beast->id]=znak;
    }
    else if(dir == LEFT)
    {
        znak = KEY_LEFT;
        p.y--;
        beast_moves_arr[beast->id]=znak;
    }
}

void *beast_moves()
{
    struct poz_t rand_pos=rand_position_in_arr();
    b[beast_counter].pos=rand_pos;
    b[beast_counter].id=beast_counter;
    struct beast_t *beast=&b[beast_counter];
    t[beast->pos.x][beast->pos.y]='*';
    beast_counter++;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(0,&fd);

    while(server_socket.is_active)
    {
        enum direction dir=search_for_player(beast->pos);
        if(dir!=SPAWN)
        {
            pthread_mutex_lock(&mutex);

            chase_player(dir,beast);

            pthread_mutex_unlock(&mutex);
        }
        else
        {
            struct timeval tv;

            tv.tv_sec=0;
            tv.tv_usec=500000;

            select(1,&fd,NULL,NULL,&tv);

            pthread_mutex_lock(&mutex);
            struct poz_t p=beast->pos;
            int znak;
            while(1)
            {
                struct poz_t old=p;
                znak=rand()%5;
                if(znak==0)
                {
                    znak=KEY_LEFT;
                    p.y--;
                }
                else if(znak==1)
                {
                    znak=KEY_UP;
                    p.x--;
                }
                else if(znak==2)
                {
                    znak=KEY_RIGHT;
                    p.y++;
                }
                else
                {
                    znak=KEY_DOWN;
                    p.x++;
                }
                if(t[p.x][p.y]==' ' || isdigit(t[p.x][p.y]))
                    break;
                p=old;
            }
            beast_moves_arr[beast->id]=znak;
            pthread_mutex_unlock(&mutex);

            FD_ZERO(&fd);
            FD_SET(0,&fd);
        }
    }
    return NULL;
}

void make_beast_moves()
{
    for(int i=0; i<beast_counter; i++)
    {
        if(b[i].pos.x!=0)
        {
            int znak = beast_moves_arr[i];
            struct beast_t *beast = &b[i];
            struct poz_t p;
            if(znak==KEY_LEFT)
            {
                p=beast->pos;
                if(t[p.x][p.y-1]==' ')
                {
                    t[p.x][p.y-1]='*';
                    t[p.x][p.y]=' ';
                    beast->pos.y--;
                }
                else if(isdigit(t[p.x][p.y-1]))
                {
                    p.y--;
                    killed_player(p);
                    p.y++;
                    t[p.x][p.y-1]='*';
                    t[p.x][p.y]=' ';

                    beast->pos.y--;
                }

            }
            else if(znak==KEY_UP)
            {
                p=beast->pos;
                if(t[p.x-1][p.y]==' ')
                {
                    t[p.x-1][p.y]='*';
                    t[p.x][p.y]=' ';
                    beast->pos.x--;
                }
                else if(isdigit(t[p.x-1][p.y]))
                {
                    p.x--;
                    killed_player(p);
                    p.x++;
                    t[p.x-1][p.y]='*';
                    t[p.x][p.y]=' ';

                    beast->pos.x--;
                }
            }
            else if(znak==KEY_RIGHT)
            {
                p=beast->pos;
                if(t[p.x][p.y+1]==' ')
                {
                    t[p.x][p.y+1]='*';
                    t[p.x][p.y]=' ';
                    beast->pos.y++;
                }
                else if(isdigit(t[p.x][p.y+1]))
                {
                    p.y++;
                    killed_player(p);
                    p.y--;
                    t[p.x][p.y+1]='*';
                    t[p.x][p.y]=' ';

                    beast->pos.y++;
                }
            }
            else if(znak==KEY_DOWN)
            {
                p=beast->pos;
                if(t[p.x+1][p.y]==' ')
                {
                    t[p.x+1][p.y]='*';
                    t[p.x][p.y]=' ';
                    beast->pos.x++;
                }
                else if(isdigit(t[p.x+1][p.y]))
                {
                    p.x++;
                    killed_player(p);
                    p.x--;
                    t[p.x+1][p.y]='*';
                    t[p.x][p.y]=' ';

                    beast->pos.x++;
                }
            }
        }
    }
}

void kill_player_and_set_to_spawn(struct acc_sock *gracz)
{
    gracz->pos=gracz->spawn;
    t[gracz->pos.x][gracz->pos.y]=gracz->sign;
    gracz->money_found=0;
    gracz->in_bush=0;
    gracz->in_bush_wait=0;
    gracz->in_campsite=0;
    gracz->deaths++;
}

void killed_player(struct poz_t p)
{
    if(t[p.x][p.y]=='1')
    {
        drop[drop_counter].p=p;
        drop[drop_counter].value=gracze[0].money_found;
        drop_counter++;

        kill_player_and_set_to_spawn(&gracze[0]);
    }
    else if(t[p.x][p.y]=='2')
    {
        drop[drop_counter].p=p;
        drop[drop_counter].value=gracze[1].money_found;
        drop_counter++;

        kill_player_and_set_to_spawn(&gracze[1]);
    }
    else if(t[p.x][p.y]=='3')
    {
        drop[drop_counter].p=p;
        drop[drop_counter].value=gracze[2].money_found;
        drop_counter++;

        kill_player_and_set_to_spawn(&gracze[2]);
    }
    else
    {
        drop[drop_counter].p=p;
        drop[drop_counter].value=gracze[3].money_found;
        drop_counter++;

        kill_player_and_set_to_spawn(&gracze[3]);
    }
}

//Player game play

void checker_bush_camp_free(struct acc_sock *gracz)
{
    struct poz_t p=gracz->pos;
    if(gracz->in_bush)
    {
        gracz->in_bush=0;
        t[p.x][p.y]='#';
    }
    else if(gracz->in_campsite)
    {
        gracz->in_campsite=0;
        t[p.x][p.y]='A';
    }
    else
        t[p.x][p.y]=' ';
}

int checker(struct acc_sock *gracz, enum moving dir)
{
    if(dir==WALL) return 0;

    struct poz_t p=gracz->pos;
    if(dir==COIN)
    {
        gracz->money_found++;
        checker_bush_camp_free(gracz);
    }
    else if(dir==BIG_T)
    {
        gracz->money_found+=50;
        checker_bush_camp_free(gracz);
    }
    else if(dir==SMALL_T)
    {
        gracz->money_found+=10;
        checker_bush_camp_free(gracz);
    }
    else if(dir==BUSH)
    {
        if(gracz->in_bush==0)
        {
            gracz->in_bush=1;
            t[p.x][p.y]=' ';
            gracz->in_bush_wait=1;
        }
        else
        {
            t[p.x][p.y]='#';
            gracz->in_bush_wait=1;
        }
    }
    else if(dir==CAMPSITE)
    {
        t[p.x][p.y]=' ';
        gracz->in_campsite=1;
        gracz->money_brought+=gracz->money_found;
        gracz->money_found=0;
    }
    else if(dir == BEAST)
    {
        killed_player(p);
    }
    else if(dir == DROP)
    {
        return 3;
    }
    else if(dir == HUMAN)
    {
        return 4;
    }
    else{
        checker_bush_camp_free(gracz);
    }
    return 1;
}

//Game
void send_struct_to_client()
{
    for(int i=0; i<NUMBER_OF_PLAYERS; i++)
    {
        re_write_map(gracze[i].map,&gracze[i]);
        send(gracze[i].sock,&gracze[i],sizeof(struct acc_sock),0);
        gracze[i].round_number=round_number;
    }
}

void recv_from_players()
{
    for(int i=0; i<NUMBER_OF_PLAYERS; i++)
    {
        if(gracze[i].sock!=0){
            int res=recv(gracze[i].sock, &recv_s_arr[i], sizeof(struct send_t),0);
            if(res<0)
            {
                client_wants_to_quit(&gracze[i]);
                close(gracze[i].sock);
            }
        }
    }
}

void make_player_moves()
{
    int znak;
    for(int i=0; i<NUMBER_OF_PLAYERS; i++)
    {
        if(gracze[i].sock!=0)
        {
            if(gracze[i].spawn.x==0)
            {
                spawn_player(&gracze[i]);
                re_write_map(gracze[i].map,&gracze[i]);
                printf_new_map();
            }

            znak=recv_s_arr[i].znak;
            struct poz_t p;
            if(znak==KEY_LEFT)
            {
                p=gracze[i].pos;
                int dir=is_move_possible(&gracze[i],LEFT);
                int res=checker(&gracze[i],dir);
                if(res==1)
                {
                    t[p.x][p.y-1]=gracze[i].sign;
                    gracze[i].pos.y--;
                }
                else if(res == 3)
                {
                    t[p.x][p.y]=' ';
                    gracze[i].pos.y--;
                    t[p.x][p.y-1]=gracze[i].sign;
                    get_drop(&gracze[i]);
                }
                else if(res == 4)
                {
                    t[p.x][p.y] = ' ';
                    gracze[i].pos.y--;
                    p = gracze[i].pos;
                    killed_player(p);
                    drop[drop_counter-1].value+=gracze[i].money_found;
                    kill_player_and_set_to_spawn(&gracze[i]);
                }

            }
            else if(znak==KEY_UP)
            {
                p=gracze[i].pos;
                int dir=is_move_possible(&gracze[i],UP);
                int res=checker(&gracze[i],dir);
                if(res==1)
                {
                    t[p.x-1][p.y]=gracze[i].sign;
                    gracze[i].pos.x--;
                }
                else if(res==3)
                {
                    t[p.x][p.y]=' ';
                    gracze[i].pos.x--;
                    t[p.x-1][p.y]=gracze[i].sign;
                    get_drop(&gracze[i]);
                }
                else if(res == 4)
                {
                    t[p.x][p.y] = ' ';
                    gracze[i].pos.x--;
                    p = gracze[i].pos;
                    killed_player(p);
                    drop[drop_counter-1].value+=gracze[i].money_found;
                    kill_player_and_set_to_spawn(&gracze[i]);
                }
            }
            else if(znak==KEY_DOWN)
            {
                p=gracze[i].pos;
                int dir=is_move_possible(&gracze[i],DOWN);
                int res=checker(&gracze[i],dir);
                if(res==1)
                {
                    t[p.x+1][p.y]=gracze[i].sign;
                    gracze[i].pos.x++;
                }
                else if(res==3)
                {
                    t[p.x][p.y]=' ';
                    gracze[i].pos.x++;
                    t[p.x+1][p.y]=gracze[i].sign;
                    get_drop(&gracze[i]);
                }
                else if(res == 4)
                {
                    t[p.x][p.y] = ' ';
                    gracze[i].pos.x++;
                    p = gracze[i].pos;
                    killed_player(p);
                    drop[drop_counter-1].value+=gracze[i].money_found;
                    kill_player_and_set_to_spawn(&gracze[i]);
                }
            }
            else if(znak==KEY_RIGHT)
            {
                p=gracze[i].pos;
                int dir=is_move_possible(&gracze[i],RIGHT);
                int res=checker(&gracze[i],dir);
                if(res==1)
                {
                    t[p.x][p.y+1]=gracze[i].sign;
                    gracze[i].pos.y++;
                }
                else if(res==3)
                {
                    t[p.x][p.y]=' ';
                    gracze[i].pos.y++;
                    t[p.x][p.y+1]=gracze[i].sign;
                    get_drop(&gracze[i]);
                }
                else if(res == 4)
                {
                    t[p.x][p.y] = ' ';
                    gracze[i].pos.y++;
                    p = gracze[i].pos;
                    killed_player(p);
                    drop[drop_counter-1].value+=gracze[i].money_found;
                    kill_player_and_set_to_spawn(&gracze[i]);
                }
            }
            else if(znak=='b')
            {
                gracze[i].in_bush_wait=0;
            }
            else if(znak=='q' || znak=='Q')
            {
                t[gracze[i].pos.x][gracze[i].pos.y]=' ';
                client_wants_to_quit(&gracze[i]);
            }
        }

    }
}

void *game()
{
    printf_new_map();

    while(server_socket.is_active)
    {
        recv_from_players();

        make_player_moves();

        make_beast_moves();

        print_drop();
        printf_new_map();

        send_struct_to_client();

        printf_info_server();
        round_number++;

        usleep(500000);
    }

    close(server_socket.server_adres);
    return NULL;
}

int main() {

    srand(time(NULL));

    pthread_mutex_init(&mutex,NULL);

    getmaxyx(stdscr, row, col);
    initscr();
    noecho();

    curs_set(0);

    create_server();

    t = create_map("mapa.txt");
    printf_new_map();
    printf_info_server();

    create_threads();

    game();

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    for(int i=0; i<25;i++)
        free(t[i]);
    free(t);

    pthread_mutex_destroy(&mutex);

    endwin();

    printf("\n\tServer has been closed!\n\n");

    return 0;
}
