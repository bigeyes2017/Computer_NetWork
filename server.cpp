//#include <stdlib.h>
#include <Winsock.h>
#pragma comment(lib,"ws2_32.lib")

#include <iostream>
using namespace std;


int main()
{
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
            cout << "�����汾�Ų�֧��" << endl;
            break;
        case WSAEINPROGRESS:
            cout << "������æ" << endl;
            break;
        case WSAEPROCLIM:
            cout << "Windows�׽���ʵ����֧�ֵ����������Ѵﵽ���ơ���رղ���Ҫ���" << endl;
            break;
        case WSAEFAULT:
            cout << "����������ڶ�����������" << endl;
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
        cout << "�������˴���socketʧ�ܣ�" << a << endl;

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

    //�������ӣ�����һ��socket�ͻ��ˣ�������
    SOCKADDR_IN cAddr;
    int cAddrlen = sizeof(cAddr);
    SOCKET sockClient = accept(sockServer, (SOCKADDR*)&cAddr, &cAddrlen);

    if (INVALID_SOCKET == sockClient) {
        int a = WSAGetLastError();

        cout << "����sockClientʧ�ܣ�" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //��ͻ���ͨ��:��,������

    char buf[1500];
    int rres = recv(sockClient, buf, sizeof(buf) - 1, 0);

    if (SOCKET_ERROR == rres) {

        int a = WSAGetLastError();
        cout << "������recv����" << a << endl;
    }
    else if (0 == rres)
    {
        cout << "�ͻ��˵���" << endl;
    }
    else {
        cout << rres << ":" << buf << endl;
    }

    //��ͻ���ͨ�ţ���
    char sbuf[] = "���յ�����.";

    int sres = send(sockClient, sbuf, sizeof(sbuf), 0);
    if (SOCKET_ERROR == sres) {

        int a = WSAGetLastError();
        cout << "������send����:" << a << endl;

    }

    cin.get();

    closesocket(sockClient);
    closesocket(sockServer);
    WSACleanup();
    return 0;
}
