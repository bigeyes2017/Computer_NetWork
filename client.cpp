#if 0
#include <iostream>
#include "winsock2.h"
#include <string>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

int main()
{
    //������·��
    WORD wdVersion = MAKEWORD(2, 2);
    WSADATA wdata;

    int wres = WSAStartup(wdVersion, &wdata);

    if (0 != wres) {
        cout << "�ͻ��˿��������ʧ�ܣ�" << wres << endl;
        return 0;
    }

    //���汾��
    if (2 != LOBYTE(wdata.wVersion) || 2 != HIBYTE(wdata.wVersion)) {
        cout << "�汾�Ų�֧��" << endl;

        WSACleanup();
        return 0;
    }

    //���÷�����ip��Ϣ

    SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sockServer) {
        cout << "�ͻ��ˣ�����socketʧ�� " << endl;
        WSACleanup();
        return 0;
    }


    //connect
    SOCKADDR_IN sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(12345);
    sAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //Winsock2.hͷ�ļ���֧����������
    /*sAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
    sAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b4 = 1;*/

    int cres = connect(sockServer, (const SOCKADDR*)&sAddr, sizeof(sAddr));
    if (0 != cres) {
        int a = WSAGetLastError();

        switch (a) {
        case WSAECONNREFUSED:
            cout << "�ͻ����޷����ӣ�δ����" << endl;
            break;
        default:
            cout << "�ͻ������ӳ���" << a << endl;
            break;
        }

        closesocket(sockServer);
        WSACleanup();
        return 0;

    }
    //������Ϣ

    while (1) {

        string str;        getline(cin, str);

        if (str[0] == '0') break;
        //����0�ÿͻ��������˳�
        int sres = send(sockServer, str.c_str(), str.size(), 0);
        if (SOCKET_ERROR == sres) {
            int a = WSAGetLastError();
            cout << "�ͻ��ˣ�����ʧ��" << a << endl;
            closesocket(sockServer);
            WSACleanup();
            return 0;
        }

        char buf[1024] = { 0 };
        int rres = recv(sockServer, buf, sizeof(buf) - 1, 0);  // 0��ʾĬ�϶�д�����ص�����
        if (SOCKET_ERROR == rres) {
            int a = WSAGetLastError();
            cout << " �ͻ��ˣ�����ʧ�� " << a << endl;

            closesocket(sockServer);
            WSACleanup();
            return 0;
        }
        else
        {
            cout << "�ͻ����յ���" << rres << "��" << buf << endl;
        }

    }

    closesocket(sockServer);
    WSACleanup();
    return 0;
}




#endif




















