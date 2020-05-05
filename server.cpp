#include <Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;


struct fd_es
{
    unsigned short count;
    SOCKET allsock[64];
    WSAEVENT allevent[64];
}esSet; //全局变量不初始化，会被置0


BOOL WINAPI funClear(DWORD CtrlType) {
    switch (CtrlType)
    {
    case CTRL_CLOSE_EVENT:
        for (unsigned short i = 0; i < esSet.count; i++)
        {
            WSACloseEvent(esSet.allevent[i]);
            closesocket(esSet.allsock[i]);
            printf("服务器：close%d", esSet.allsock[i]);
        }
        WSACleanup();
    default:
        break;
    }
    return true;
}

int main()
{
    SetConsoleCtrlHandler(funClear, true);
    //打开网络库
    WORD wdVersion = MAKEWORD(2, 2);
    WSADATA wdSockMsg;

    int wres = WSAStartup(wdVersion, &wdSockMsg);

    if (0 != wres) {
        switch (wres)
        {
        case WSASYSNOTREADY:
            cout << "系统配置问题，请检查ws2_32.lib是否在工作目录" << endl;
            break;
        case WSAVERNOTSUPPORTED:
            cout << "服务器：网络库版本号不支持" << endl;
            break;
        case WSAEINPROGRESS:
            cout << "服务器忙" << endl;
            break;
        case WSAEPROCLIM:
            cout << "Windows套接字实现所支持的任务数量已达到限制。请关闭不必要软件" << endl;
            break;
        case WSAEFAULT:
            cout << "服务器：网络库启动第二个参数错了" << endl;
            break;
        default:
            break;
        }

        return 0;
    }
    //检测版本
    if (2 != LOBYTE(wdSockMsg.wVersion) || 2 != HIBYTE(wdSockMsg.wVersion)) {

        WSACleanup();
        return 0;
    }

    //创建socket
    SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (INVALID_SOCKET == sockServer) {
        int a = WSAGetLastError();  //0--success
        cout << "服务器：创建socket失败：" << a << endl;

        WSACleanup();
        return 0;
    }

    //绑定服务器socket
    SOCKADDR_IN sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(12345);
    //sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
    sAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b4 = 1;

    int bres = bind(sockServer, (const SOCKADDR*)&sAddr, sizeof(sAddr));
    if (SOCKET_ERROR == bres) {
        int a = WSAGetLastError();

        cout << "服务器：bind失败：" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //监听
    int lres = listen(sockServer, SOMAXCONN);

    if (SOCKET_ERROR == lres) {
        int a = WSAGetLastError();

        cout << "服务器：listen失败(无法获得用户连接)：" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }
    cout << "服务器端已启动..." << endl;


    //运用事件选择模型
    
    WSAEVENT eventServer = WSACreateEvent();
    if (WSA_INVALID_EVENT == eventServer) {
        int a = WSAGetLastError();
        printf("WSACreateEvent失败：%d", a);

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //绑定：把socket与event绑定起来，并指定Os监测内容
    if (SOCKET_ERROR == WSAEventSelect(sockServer, eventServer, FD_ACCEPT)) {
        WSACloseEvent(eventServer);

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //放入监测结构体中
    esSet.allevent[0] = eventServer;
    esSet.allsock[0] = sockServer;
    esSet.count++;


    //循环查看事件是否有信号并处理
    while (true)
    {   
        //查看
        DWORD nRes = 
            WSAWaitForMultipleEvents(esSet.count, esSet.allevent, false, WSA_INFINITE, false);

        if (WSA_WAIT_FAILED== nRes) {//这是等待的失败。
            break;
        }
        DWORD nIndex = nRes - WSA_WAIT_EVENT_0; //得到下标

        
        WSANETWORKEVENTS netWorkEvents;         //将信号值按位或后放在结构体中。
        if (SOCKET_ERROR ==
            WSAEnumNetworkEvents(esSet.allsock[nIndex], esSet.allevent[nIndex], &netWorkEvents)) {
            continue;
        }

        if (netWorkEvents.lNetworkEvents & FD_ACCEPT) {
            if (0 == netWorkEvents.iErrorCode[FD_ACCEPT_BIT]) {
                SOCKET sockClient = accept(esSet.allsock[nIndex], NULL, NULL);
                if (INVALID_SOCKET == sockClient) {
                    printf("服务器：创建客户端socket失败\n");
                    continue;
                }
                //每个socket都要绑定事件。
                WSAEVENT   eventClient = WSACreateEvent();
                if (WSA_INVALID_EVENT == eventClient) {
                    closesocket(sockClient);
                    continue;
                }
                
                if (SOCKET_ERROR == WSAEventSelect(sockClient, eventClient, FD_READ | FD_WRITE | FD_CLOSE)) {
                    closesocket(sockClient);
                    WSACloseEvent(eventClient);
                    continue;
                }

                esSet.allevent[esSet.count] = eventClient;
                esSet.allsock[esSet.count] = sockClient;
                esSet.count++;

                printf("服务器：accept%d\n",sockClient);
            }
            else {
                continue;
            }
        } //ifNet

        //write可在accept之后激活一次，此后默认可发送，所以不会再被激活。
        if (netWorkEvents.lNetworkEvents & FD_WRITE) {
            if (0 == netWorkEvents.iErrorCode[FD_WRITE_BIT]) {
                if (SOCKET_ERROR == send(esSet.allsock[nIndex], "你好，这里是服务器", 100, 0)) {
                    continue;
                }   
                printf("服务器：send%d\n", esSet.allsock[nIndex]);    //这里发送了客户端也不一定收的到。
            }
            else
            {
                printf("服务器send错误：%d\n", netWorkEvents.iErrorCode[FD_WRITE_BIT]);
            }
        }


        if (netWorkEvents.lNetworkEvents & FD_READ) {
            if (0 == netWorkEvents.iErrorCode[FD_READ_BIT]) {
                char str[1500] = { 0 };
                if (SOCKET_ERROR == recv(esSet.allsock[nIndex], str, 1499, 0)) {
                    continue;
                }
                printf("服务器recv :%s\n", str);
            }
            else {
                printf("服务器send错误：%d\n", netWorkEvents.iErrorCode[FD_READ_BIT]);
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_CLOSE) {

            printf("服务器：close%d", esSet.allsock[nIndex]);

            WSACloseEvent(esSet.allevent[nIndex]);
            closesocket(esSet.allsock[nIndex]);
            //删除
            esSet.allevent[nIndex] = esSet.allevent[esSet.count - 1];
            esSet.allsock[nIndex] = esSet.allsock[esSet.count - 1];
            esSet.count--;
        }
    }

    //点X关闭。
    cin.get();
    return 0;
}
