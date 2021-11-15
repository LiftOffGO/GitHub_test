#include <stdarg.h>
#include <unistd.h>
#include "MyUDP.h"

using namespace std;


TUDP::TUDP(char *filename)
{
    string strValue;
    char str[100];
    vector<string> substr;
    int port;

    Init(8080);

    remote_client.clear();
    TRemote_port p;
    p.port=10500;
    p.ip="10.106.27.249",    remote_client.push_back(p);
    // p.ip="192.168.3.112",    remote_client.push_back(p);
    // p.ip="192.168.3.113",    remote_client.push_back(p);
    // p.ip="192.168.3.114",    remote_client.push_back(p);
    // p.ip="192.168.3.115",    remote_client.push_back(p);

//    if(access(filename, F_OK)==0)
//    {
//        CIniFile cini(filename);
//        remote_client.clear();
//        for(int i=0;i<10;i++)
//        {
//            char keyname[100];
//            sprintf(keyname, "remote%d", i+1);
//            cini.ReadItem("ip_addr", keyname, "",strValue);
//            if(strValue=="") break;
//
//            substr=split(strValue.c_str(), ":");
//            if(substr.size()<2) printf("remote1 ip error!\n");
//            else
//            {
//                TRemote_port p;
//                p.ip=substr[0];  p.port=atoi(substr[1].c_str());;
//                remote_client.push_back(p);
//            }
//        }
//    }

    for(int i=0;i<remote_client.size();i++)
        printf("remote%d %s:%d\n",i+1, remote_client[i].ip.c_str(),remote_client[i].port);

    rec_flag=0;
    rec_count=0;

    start();

    pthread_mutex_init(&mute, NULL);
}

//int TUDP::Receive(void)
//{
//    int flag=rec_flag;
//    rec_flag=0;
//    return(flag);
//}

void TUDP::Init(int port)
{
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)   ERR_EXIT("socket error");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr =htonl(INADDR_ANY);  //inet_addr("127.0.0.1");

    //sin_family指代协议族，在socket编程中只能是AF_INET
    //sin_port存储端口号（使用网络字节顺序）
    //s_addr按照网络字节顺序存储IP地址
    printf("listen to %d port\n",port);
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");
}

void TUDP::Close(void)
{
    close(sock);
}

void TUDP::Send(char *str)
{
    for(int i=0;i<remote_client.size();i++)
    {
        char ip[100];
        strcpy(ip,remote_client[i].ip.c_str());
        Send(ip, remote_client[i].port, str);
    }
}

void TUDP::Send(string str)
{
    if(str.size()==0)
        return;

    char s[1000];
    strcpy(s, str.c_str());

    //printf("%s", str.c_str());
    for(int i=0;i<remote_client.size();i++)
    {
        char ip[100];
        strcpy(ip,remote_client[i].ip.c_str());
        Send(ip, remote_client[i].port, s);
    }
}

void TUDP::SendFormat(char *fmt, ...)
{
    char buf[1000]={0};

    va_list ap;
    int d;
    double f;
    char c;
    char *s;
    char flag;
    va_start(ap,fmt);
    while (*fmt)
    {
        flag=*fmt++;
        if(flag!='%')
        {
            sprintf(buf,"%s%c",buf,flag);
            //putchar(flag);
            continue;
        }
        flag=*fmt++;//记得后移一位
        switch (flag)
        {
            case 's':
                s=va_arg(ap,char*);
                sprintf(buf,"%s%s",buf,s);
                //printf("%s",s);
                break;
            case 'd': /* int */
                d = va_arg(ap, int);
                sprintf(buf,"%s%d",buf,d);
                //printf("%d", d);
                break;
            case 'f': /* double*/
                f = va_arg(ap,double);
                sprintf(buf,"%s%.5f",buf,f);
                //printf("%.5f", f);
                break;
            case 'c': /* char*/
                c = (char)va_arg(ap,int);
                sprintf(buf,"%s%c",buf,c);
                //printf("%c", c);
                break;
            default:
                //putchar(flag);
                sprintf(buf,"%s%c",buf,flag);
                break;
        }
    }
    va_end(ap);

    Send(buf);
    //printf("%s\n",buf);
}

void TUDP::Send(char *ip, int port, char *str)
{
    struct sockaddr_in sendaddr;

    sendaddr.sin_family = AF_INET;
    sendaddr.sin_port = htons(port);
    sendaddr.sin_addr.s_addr = inet_addr(ip);
    sendto(sock, str, strlen(str), 0, (struct sockaddr *)&sendaddr, sizeof(struct sockaddr_in));
}

void TUDP::SendBuf()
{
    string buf="";
    pthread_mutex_lock(&mute);

    for(int i=0;i<s_buf.size();i++)
    {
        if(buf.size()>500)
        {
            Send(buf), buf.clear();
            usleep(10000);
        }
        buf+=s_buf.at(i);
    }
    Send(buf);
    //printf("%s\n", buf.c_str());

    //printf("%d\n", s_buf.size());
    s_buf.clear();
    pthread_mutex_unlock(&mute);
}

void TUDP::AddSendStr(char *str)
{
    pthread_mutex_lock(&mute);
    if(s_buf.size()<1000) s_buf.push_back(str);
    pthread_mutex_unlock(&mute);
}


void TUDP::run()
{
    int len;
    char recvbuf[1024] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    //printf("udp is working!!!!\n");
    while (1)
    {
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));

        //memset某一块内存中的内容全部设置为指定的值
        //函数解释：将recvbuf中当前位置后面的sizeof(recvbuf)个字节，用0替换并返回recvbuf
        len = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&peeraddr, &peerlen);
        //recvfrom函数(经socket接收数据)。
        string ip=inet_ntoa(peeraddr.sin_addr);
        //将一个十进制网络字节序转换为点分十进制IP格式的字符串。

        if(len>0)
        {
            rec_flag=1;
            rec_count++;
            strcpy(receive_buf,recvbuf);
            // printf("%s\n",recvbuf);
        }

        if (len <= 0)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom error");
        }
    }
    close(sock);
}

