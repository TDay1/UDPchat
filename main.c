/*
TO DO NEXT:
Format messages before sending
recieve formatted shit properly
*/

#include <ncurses.h> /* ncurses.h includes stdio.h */
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h> /* UDP proccessing */
#include <sys/socket.h> /* Socket library */
#include <stdlib.h> //exit(0);
#include <sys/types.h>

#define PORT 8888 //The port on which to listen for incoming data
#define BUFLEN 512 //Max length of buffer
#define BODYLEN 256 //size of the body length, should be no more then 75% of the Buffer length

void userAddToBuffer(char userName[15], char message[255]);
void remoteAddToBuffer(char userName[15], char remmoteMessage[255]);
void die(char* s);

void processMessage(char input[511]);
char* parseMessage(const char* str, const char* p1, const char* p2);

void udpSender(char ipaddr[16], char username[15], int type, char body[255]);
void* udpListner(void* arg);

int main(int argc, char** argv)
{
    // Network config
    char ipaddr[16];
    int port = 8888;

    //Define shit
    char message[255];
    char userName[32];
    int row, col; /* to store the number of rows and the number of colums of the screen */

    //Thread setup
    pthread_t udpListnerThread; // this is our thread identifier

    //Curses setup
    initscr(); /* start the curses mode */
    raw(); /* Line buffering disabled	*/
    scrollok(stdscr, TRUE); /* Enable scrolling on window */
    getmaxyx(stdscr, row, col); /* get the number of rows and columns */

    //init
    int ret;
    struct sockaddr_in addrtest;
    addrtest.sin_family = AF_INET;

    if (argc == 1) {
        while (1) {
            clear();
            mvprintw((LINES - 1) / 2, (COLS - 17) / 2, "Enter IP address:");
            move(LINES / 2, (COLS - 14) / 2);
            getstr(ipaddr);

            ret = inet_aton(ipaddr, &addrtest.sin_addr);
            if (ret != 0) {
                clear();
                break;
            }
            else {
                clear();
                mvprintw((LINES - 1) / 2, (COLS - 23) / 2, "Not a valid IP address");
                mvprintw(LINES / 2, (COLS - 26) / 2, "Press any key to continue");
                getch();
            }
        }
        while (1) {
            clear();
            mvprintw((LINES - 1) / 2, (COLS - 18) / 2, "Enter a username:");
            move(LINES / 2, (COLS - 14) / 2);
            getstr(userName);
            if (strlen(userName) < 33 && strlen(userName) > 0) {
                clear();
                break;
            }
            else {
                clear();
                mvprintw((LINES - 1) / 2, (COLS - 21) / 2, "Not a valid UserName");
                mvprintw(LINES / 2, (COLS - 26) / 2, "Press any key to continue");
                getch();
            }
        }
    }
    else {
        if (strlen(argv[1]) > 16) {
            die("IP is too long!");
        }
        strcpy(ipaddr, argv[1]);
        ret = inet_aton(ipaddr, &addrtest.sin_addr);
        if (ret != 0) {
            clear();
        }
        else {
            clear();
            mvprintw((LINES - 1) / 2, (COLS - 23) / 2, "Not a valid IP address");
            mvprintw(LINES / 2, (COLS - 26) / 2, "Press any key to continue");
            getch();
            clear();
            die("Invalid IP");
        }
        if (strlen(argv[2]) < 33) {
            strcpy(userName, argv[2]);
        }
        else {
            die("Username max length is 32");
        }
    }

    pthread_create(&udpListnerThread, NULL, udpListner, &port);

      udpSender(ipaddr, userName, 1, "");


    // Program loop
    while (1) {
        move(LINES - 1, 0);
        getstr(message);
        move(LINES - 1, 0);
        udpSender(ipaddr, userName, 0, message);
        userAddToBuffer(userName, message);
    }

    endwin();

    return 0;
}

void processMessage(char input[511])
{
    // Format of a message = "<user>TDay</user><type>0</type><body>hello world</body>";

    // 0 = message
    // 1 = joining
    // 2 = ack

    // Parse Message Used for getting char contents
    char* userName = parseMessage(input, "<user>", "</user>");
    int type = atoi(parseMessage(input, "<type>", "</type>"));
    char* body = parseMessage(input, "<body>", "</body>");
    char joining[64];

    switch (type) {
    case 0:
        //Message
        remoteAddToBuffer(userName, body);
        break;
    case 1:
        sprintf(joining, "User %s has joined the chat", userName);
        remoteAddToBuffer("System", joining);
        //joining
        break;
    case 2:
        break;
    default:
        remoteAddToBuffer("system", "Recieve error: Malformed packet error.");
    }
}

char* parseMessage(const char* str, const char* p1, const char* p2)
{
    const char* i1 = strstr(str, p1);
    if (i1 != NULL) {
        const size_t pl1 = strlen(p1);
        const char* i2 = strstr(i1 + pl1, p2);
        if (p2 != NULL) {
            /* Found both markers, extract text. */
            const size_t mlen = i2 - (i1 + pl1);
            char* ret = malloc(mlen + 1);
            if (ret != NULL) {
                memcpy(ret, i1 + pl1, mlen);
                ret[mlen] = '\0';
                return ret;
            }
        }
    }
}

// add message to chat log (local)
void userAddToBuffer(char userName[32], char message[255])
{
    move(LINES - 2, 0);
    clrtoeol();

    mvprintw(LINES - 2, 0, "[%s] %s", userName, message);
    if (strlen(message) > COLS) {
        scroll(stdscr);
    }
    move(LINES - 1, 0);
}

// add message to chat log (remote)
void remoteAddToBuffer(char userName[], char remoteMessage[255])
{
    char inProgress[255];
    move(LINES - 1, 0);
    instr(inProgress);
    clrtoeol();
    scroll(stdscr);
    mvprintw(LINES - 2, 0, "[%s] %s", userName, remoteMessage);
    if (strlen(inProgress) > 0) {
        move(LINES - 1, 0);
        printw(inProgress);
        scrl(-1);
        move(LINES - 1, strlen(strcpy(inProgress, "\0")));
    }
    wrefresh(stdscr);
}

 
/* UDP send function */
void udpSender(char ipaddr[16], char username[15], int type, char body[255])
{
    char message[BUFLEN]; // create string for formatting
    sprintf(message, "<user>%s</user><type>%i</type><body>%s</body>", username, type, body); // put data into correct packet format

    int s;
    int ret;
    char* buf;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    // attempt to convert ip from char to binary, will exit on error
    ret = inet_aton(ipaddr, &addr.sin_addr);
    if (ret == 0) {
        die("inet_aton");
    }
    addr.sin_port = htons(PORT);
    buf = message;

    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        die("socket");
    }

    ret = sendto(s, buf, strlen(buf), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        die("sendto");
    }
}

/* 
UDP Listner thread 
Listening to a socket is a blocking call so threading is required.
*/
void* udpListner(void* arg)
{
    struct sockaddr_in si_me, si_other;

    int s, i, slen = sizeof(si_other), recv_len;
    char buf[BUFLEN];

    //create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket error");
    }

    // zero out the structure
    memset((char*)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
        die("bind");
    }

    //keep listening for data
    while (1) {
        char buf[BUFLEN] = "";
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)) == -1) {
            die("recvfrom()");
        }

        //remoteAddToBuffer("tday", buf);
        processMessage(buf);
    }

    close(s);

    return NULL;
}

//UDP pairing function
void pairing()
{
}

/* Error handling */
void die(char* s)
{
    char error[] = "Exiting on error: \n ";
    strcat(error, s);
    endwin();
    perror(error);
    exit(1);
}
