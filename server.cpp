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
    //�������
    WORD wdVersion = MAKEWORD(2, 2);
    WSADATA wdSockMsg;

    int wres = WSAStartup(wdVersion, &wdSockMsg);

    if (0 != wres) {
        switch (wres)
        {
        case WSASYSNOTREADY:
            cout << "ϵͳ�������⣬����ws2_32.lib�Ƿ��ڹ���Ŀ¼" << endl;
            break;
        case WSAVERNOTSUPPORTED:
            cout << "�������������汾�Ų�֧��" << endl;
            break;
        case WSAEINPROGRESS:
            cout << "������æ" << endl;
            break;
        case WSAEPROCLIM:
            cout << "Windows�׽���ʵ����֧�ֵ����������Ѵﵽ���ơ���رղ���Ҫ���" << endl;
            break;
        case WSAEFAULT:
            cout << "������������������ڶ�����������" << endl;
            break;
        default:
            break;
        }

        return 0;
    }
    //���汾
    if (2 != LOBYTE(wdSockMsg.wVersion) || 2 != HIBYTE(wdSockMsg.wVersion)) {

        WSACleanup();
        return 0;
    }

    //����socket
    SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (INVALID_SOCKET == sockServer) {
        int a = WSAGetLastError();  //0--success
        cout << "������������socketʧ�ܣ�" << a << endl;

        WSACleanup();
        return 0;
    }

    //�󶨷�����socket

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

        cout << "��������bindʧ�ܣ�" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //����

    int lres = listen(sockServer, SOMAXCONN);

    if (SOCKET_ERROR == lres) {
        int a = WSAGetLastError();

        cout << "��������listenʧ��(�޷�����û�����)��" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }
    cout << "��������������..." << endl;


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
        //�رտͻ�����Ȼ����1��recv�¼���

        if (0 == se_Res) {      /*û��recv��accept,����Ѱ��*/
            continue;
        }
        else if (SOCKET_ERROR == se_Res)
        {
            int a = WSAGetLastError();
            cout << "�������ˣ�" << a << endl;
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
                        //�����¿ͻ���.������ַ��Ϣ:SOCKADDR_IN cAddr;int cAddrlen = sizeof(cAddr);

                        SOCKET sockClient = accept(sockServer, NULL, NULL);
                        if (sockClient == INVALID_SOCKET) {
                            cout << "���������޷����ӿͻ���" << endl;
                        }
                        else
                        {

                            FD_SET(sockClient, &allsocks);
                            cout << "������������" << sockClient << endl;

                        }
                    }
                    else {  //����recv�����߹��ڿͻ��˵Ĺرգ���

                        char buf[1500] = { 0 };

                        int rRes = recv(tempsock, buf, sizeof(buf) - 1, 0);
                        if (SOCKET_ERROR == rRes) { /*������Ҫ�ر�����*/
                            int a = WSAGetLastError();
                            cout << "��������" << tempsock << "recv����" << a << endl;

                            if (10054 == a) {
                                cout << "���������ͻ����ֶ��ر�" << endl;
                                closesocket(tempsock);
                                FD_CLR(tempsock, &allsocks);
                            }                            
                        }
                        else if (0 == rRes) {       /*�����ر�����*/
                            cout << "���������ͻ����˳�" << endl;

                            closesocket(tempsock);
                            FD_CLR(tempsock, &allsocks);
                        }
                        else
                        {
                            cout << "�������յ���" << rRes << "��:" << buf << endl;
                        }
                    }
                }
            }
            else if(errSocks.fd_count){ //�ܵķ�������û����
                for (int i = 0; i < errSocks.fd_count ; i++)
                {
                    char optdata[100] = { 0 };
                    int optlen = sizeof(optdata) - 1;       

                    int erRes = getsockopt(errSocks.fd_array[i],
                        SOL_SOCKET, SO_ERROR,
                        optdata, &optlen
                    );
                    if (SOCKET_ERROR == erRes) {
                        cout << "��ȡ������Ϣʧ��" << endl;
                    }
                    else
                    {
                        cout << "SOCKET  :" << errSocks.fd_array[i] <<
                                "������Ϣ:" << optdata << endl;
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
