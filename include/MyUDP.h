
#ifndef MYUDP_H
#define MYUDP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <memory.h>
#include "Public.h"



#define ERR_EXIT(m) \
    do { \
    perror(m); \
    exit(EXIT_FAILURE); \
    } while (0)

struct TRemote_port
{
    string ip;
    int port;
};

class TUDP : public Thread
{
private:
    int  sock;
    vector<string> s_buf;

    void Send(char *str);
    void Send(string str);
    void SendFormat(char *fmt, ...);
    void Send(char *ip, int port, char *str);

public:
    char receive_buf[1024];
    int rec_flag;
    int rec_count;
    vector<TRemote_port> remote_client;
    pthread_mutex_t mute;

    TUDP(char *filename="");

    //int Receive(void);
    void Init(int port);
    void Close(void);
    void SendBuf();
    void AddSendStr(char *str);

    void run();
};


#endif //TEST_WSK_MYUDP_H
