#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#define PORT 8000
#define SA struct sockaddr

//Server
int sock;
struct sockaddr_in server_adr;

//Game play
int row,col;
struct poz_t{
    int x;
    int y;
};

//Player
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
struct acc_sock gracz;

//Map
void printf_new_map(char** t)
{
    for(int i=0; i<5; i++)
    {
        for(int j=0; j<6; j++)
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
    refresh();
}

void rewrite_map(char* m,char** dest)
{
    int j1=0;
    for(int i=0; i<5; i++)
    {
        for(int j=0; j<6; j++,j1++)
        {
            dest[i][j]=m[j1];
        }
    }
}

//Info
void printf_info_player(struct acc_sock *gracz)
{
    mvprintw(row/2+1,col/2+53,"Server's PID:  %d",gracz->server_pid);
    mvprintw(row/2+2,col/2+54,"Campsite X/Y: unknown");
    mvprintw(row/2+3,col/2+54,"Round number: %d",gracz->round_number);
    mvprintw(row/2+5,col/2+53,"Player:");
    mvprintw(row/2+6,col/2+54,"Number: \t%c",gracz->sign);
    mvprintw(row/2+7,col/2+54,"Type: \t%s",gracz->type);
    clrtoeol();
    mvprintw(row/2+8,col/2+54,"Cur X/Y: \t%d/%d",gracz->pos.x,gracz->pos.y);
    clrtoeol();
    mvprintw(row/2+9,col/2+54,"Deaths: \t%d",gracz->deaths);
    clrtoeol();
    mvprintw(row/2+11,col/2+54,"Coins found:   %d",gracz->money_found);
    clrtoeol();
    mvprintw(row/2+12,col/2+54,"Coins brought: %d",gracz->money_brought);
    clrtoeol();

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
    mvprintw(row/2+24,col/2+54,"D    - dropped treasure\n");
    refresh();
}

//Game
void *game(int sock)
{
    char** map_2=calloc(5,sizeof(char*));
    for(int i=0; i<5; i++)
        map_2[i]=calloc(6,sizeof(char));

    fd_set fd;

    FD_ZERO(&fd);
    FD_SET(0,&fd);

    struct send_t send_struct;
    send_struct.pid=getpid();


    while(1)
    {
        recv(sock,&gracz,sizeof(struct acc_sock),0);
        if(gracz.sock==-1)
        {
            printf("\n\tServer is full!\n\n");
            refresh();
            break;
        }
        if(gracz.server_pid==-1) break;

        rewrite_map(gracz.map,map_2);
        printf_new_map(map_2);
        printf_info_player(&gracz);

        struct timeval tv;

        tv.tv_sec=0;
        tv.tv_usec=500000;

        int res=select(1,&fd,NULL,NULL,&tv);

        if(res==1)
        {
            if(FD_ISSET(0,&fd))
            {
                if(gracz.in_bush_wait)
                {
                    send_struct.znak = 'b';
                    send(sock,&send_struct,sizeof(struct send_t),0);
                    continue;
                }
                send_struct.znak = getch();
                if (send_struct.znak == '\033')
                {
                    getch();
                    int znak = getch();
                    if(znak =='A') send_struct.znak = KEY_UP;
                    else if(znak == 'B') send_struct.znak = KEY_DOWN;
                    else if(znak == 'C') send_struct.znak = KEY_RIGHT;
                    else if(znak == 'D') send_struct.znak = KEY_LEFT;


                }
                send(sock,&send_struct,sizeof(struct send_t),0);

                if(send_struct.znak=='q' || send_struct.znak=='Q') break;
            }
        }
        else
        {
            send_struct.znak = 0;
            send(sock,&send_struct,sizeof(struct send_t),0);
            FD_ZERO(&fd);
            FD_SET(0,&fd);
        }
    }

    for(int i=0; i<5; i++)
        free(map_2[i]);
    free(map_2);

    return NULL;
}

//server connection
void server_connect()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("\n\tsocket creation failed\n\n");
        exit(1);
    }
    bzero(&server_adr, sizeof(server_adr));

    server_adr.sin_family = AF_INET;
    server_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_adr.sin_port = htons(PORT);

    int res=connect(sock, (SA*)&server_adr, sizeof(server_adr));

    if (res!= 0) {
        printf("\n\tconnection with the server failed\n\n");
        exit(1);
    }
}

int main()
{
    getmaxyx(stdscr,row,col);
    initscr();
    noecho();

    curs_set(0);

    server_connect();

    game(sock);

    close(sock);

    endwin();

    if(gracz.server_pid==-1)
    {
        printf("\n\tServer has been closed\n\n");
    }
    else
    {
        printf("\n\tDisconnected\n\n");
    }

    return 0;
}
