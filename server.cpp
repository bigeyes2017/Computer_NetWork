//#include <stdlib.h>
#include <Winsock.h>
#pragma comment(lib,"ws2_32.lib")
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;

fd_set allsocks;


BOOL WINAPI funClear(DWORD CtrlType) {
    switch (CtrlType)
    {
    case CTRL_CLOSE_EVENT:
        for (u_int i = 0; i < allsocks.fd_count; i++)
        {
            closesocket(allsocks.fd_array[i]);
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
    sAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    /*sAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
    sAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b4 = 1;*/
    //inet_pton()
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


    //fd_set allsocks;
    FD_ZERO(&allsocks);
    FD_SET(sockServer, &allsocks);

    timeval find_time = { 3,0 };
    timeval clear_time{ 0,0 };

    while (true)
    {
        fd_set acptReadSocks = allsocks;
        fd_set errSocks = allsocks;

        int se_Res = select(0, &acptReadSocks, NULL, &errSocks, &find_time);
        //关闭客户端仍然返回1个recv事件。

        if (0 == se_Res) {      /*没有recv，accept,继续寻找*/
            continue;
        }
        else if (SOCKET_ERROR == se_Res)
        {
            int a = WSAGetLastError();
            cout << "服务器端：" << a << endl;
            //continue;
        }
        else
        {
            if (acptReadSocks.fd_count) {
                SOCKET  tempsock;
                for (u_int i = 0; i < acptReadSocks.fd_count; i++)
                {
                    tempsock = acptReadSocks.fd_array[i];

                    if (tempsock == sockServer) {
                        //接收新客户端.若带地址信息:SOCKADDR_IN cAddr;int cAddrlen = sizeof(cAddr);

                        SOCKET sockClient = accept(sockServer, NULL, NULL);
                        if (sockClient == INVALID_SOCKET) {
                            cout << "服务器：无法连接客户端" << endl;
                        }
                        else
                        {

                            FD_SET(sockClient, &allsocks);
                            cout << "服务器：置入" << sockClient << endl;

                        }
                    }
                    else {  //处理recv，或者关于客户端的关闭！！

                        char buf[1500] = { 0 };

                        int rRes = recv(tempsock, buf, sizeof(buf) - 1, 0);
                        if (SOCKET_ERROR == rRes) { /*可能需要关闭链接*/
                            int a = WSAGetLastError();
                            cout << "服务器：" << tempsock << "recv错误" << a << endl;

                            if (10054 == a) {
                                cout << "服务器：客户端手动关闭" << endl;
                                closesocket(tempsock);
                                FD_CLR(tempsock, &allsocks);
                            }                            
                        }
                        else if (0 == rRes) {       /*正常关闭连接*/
                            cout << "服务器：客户端退出" << endl;

                            closesocket(tempsock);
                            FD_CLR(tempsock, &allsocks);
                        }
                        else
                        {
                            cout << "服务器收到（" << rRes << "）:" << buf << endl;
                        }
                    }
                }
            }
            else if(errSocks.fd_count){ //管的方面这里没法测
                for (int i = 0; i < errSocks.fd_count ; i++)
                {
                    char optdata[100] = { 0 };
                    int optlen = sizeof(optdata) - 1;       

                    int erRes = getsockopt(errSocks.fd_array[i],
                        SOL_SOCKET, SO_ERROR,
                        optdata, &optlen
                    );
                    if (SOCKET_ERROR == erRes) {
                        cout << "获取错误信息失败" << endl;
                    }
                    else
                    {
                        cout << "SOCKET  :" << errSocks.fd_array[i] <<
                                "错误信息:" << optdata << endl;
                    }
                } 
            }
        }    
    }


    for (u_int i = 0; i < allsocks.fd_count; i++)
    {
        closesocket(allsocks.fd_array[i]);
    }

    WSACleanup();
    return 0;
}
