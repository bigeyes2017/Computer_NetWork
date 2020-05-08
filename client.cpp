#if 0
#include <iostream>
#include "winsock2.h"
#include <string>
#pragma comment(lib,"ws2_32.lib")

using namespace std;
bool startUp(SOCKET& sockServer);
bool SEND(SOCKET sockServer);
int main()
{
    SOCKET sockServer;
    if (!startUp(sockServer))return 0;

    //connect
    SOCKADDR_IN sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(12345);
    sAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
    sAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b4 = 1;

    //事件选择模型客户端
    WSAEVENT eventServer = WSACreateEvent();
    if (WSA_INVALID_EVENT == eventServer) {
        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    if (SOCKET_ERROR == WSAEventSelect(sockServer, eventServer, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE)) {
        WSACloseEvent(eventServer);
        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    connect(sockServer, (const SOCKADDR*)&sAddr, sizeof(sAddr));
    //无论是否连接成功，都会发送一个信号FD_CONNECT。若连接失败，会出现异常。且看下方关于连接的判断

    bool connectOK = false;
    while (true)
    {
        //等待消息
        DWORD wRes = WSAWaitForMultipleEvents(1, &eventServer, false, 0, false);
        if (WSA_WAIT_FAILED == wRes) {
            continue;
        }
        if (WSA_WAIT_TIMEOUT == wRes) {  //实现核心功能：没有消息时，就发送消息
            if (connectOK)
                if (!SEND(sockServer))
                    break;
            continue;
        }

        //分类处理
        WSANETWORKEVENTS netWorkEvents;
        if (SOCKET_ERROR == WSAEnumNetworkEvents(sockServer, eventServer, &netWorkEvents)) {
            continue;
        }

        if (netWorkEvents.lNetworkEvents & FD_CONNECT) {
            if (!netWorkEvents.iErrorCode[FD_CONNECT_BIT])
            {
                cout << "连接服务器成功" << endl;
                connectOK = true;
            }
            else
            {
                connect(sockServer, (const SOCKADDR*)&sAddr, sizeof(sAddr));
            }
            continue;
        }

        if (netWorkEvents.lNetworkEvents & FD_WRITE) {
            if (!netWorkEvents.iErrorCode[FD_WRITE_BIT])
            {
                if (!SEND(sockServer))
                    break;
            }
            else
            {
                continue;
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_READ) {
            if (!netWorkEvents.iErrorCode[FD_READ_BIT])
            {
                char buf[1024] = { 0 };
                int rres = recv(sockServer, buf, sizeof(buf) - 1, 0);  // 0表示默认读写，不必等满。
                if (SOCKET_ERROR == rres) {
                    int a = WSAGetLastError();
                    cout << " 客户端：接收失败 " << a << endl;
                    continue;
                }
                else
                {
                    cout << "客户端收到（" << rres << "）" << buf << endl;
                }
            }
            else
            {
                continue;
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_CLOSE)
        {
            cout << "服务器已退出";
            break;
        }
    }

    WSACloseEvent(eventServer);
    closesocket(sockServer);
    WSACleanup();
    cout << "客户端退出" << endl;
    return 0;

}
bool SEND(SOCKET sockServer) {

    string str;        getline(cin, str);

    if (str[0] == '0') return 0;

    int sres = send(sockServer, str.c_str(), str.size(), 0);
    if (SOCKET_ERROR == sres) {
        int a = WSAGetLastError();
        cout << "客户端：发送失败" << a << endl;
    }
    return true;
    //true | false 代表是否退出
}
bool startUp(SOCKET& sockServer) {
    //开启网路库
    WORD wdVersion = MAKEWORD(2, 2);
    WSADATA wdata;

    int wres = WSAStartup(wdVersion, &wdata);

    if (0 != wres) {
        cout << "客户端开启网络库失败：" << wres << endl;
        return 0;
    }

    //检测版本号
    if (2 != LOBYTE(wdata.wVersion) || 2 != HIBYTE(wdata.wVersion)) {
        cout << "版本号不支持" << endl;

        WSACleanup();
        return 0;
    }

    //设置服务器ip信息

    sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sockServer) {
        cout << "客户端：创建socket失败 " << endl;
        WSACleanup();
        return 0;
    }


    return 1;
}

























#endif




















