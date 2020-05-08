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

    //�¼�ѡ��ģ�Ϳͻ���
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
    //�����Ƿ����ӳɹ������ᷢ��һ���ź�FD_CONNECT��������ʧ�ܣ�������쳣���ҿ��·��������ӵ��ж�

    bool connectOK = false;
    while (true)
    {
        //�ȴ���Ϣ
        DWORD wRes = WSAWaitForMultipleEvents(1, &eventServer, false, 0, false);
        if (WSA_WAIT_FAILED == wRes) {
            continue;
        }
        if (WSA_WAIT_TIMEOUT == wRes) {  //ʵ�ֺ��Ĺ��ܣ�û����Ϣʱ���ͷ�����Ϣ
            if (connectOK)
                if (!SEND(sockServer))
                    break;
            continue;
        }

        //���ദ��
        WSANETWORKEVENTS netWorkEvents;
        if (SOCKET_ERROR == WSAEnumNetworkEvents(sockServer, eventServer, &netWorkEvents)) {
            continue;
        }

        if (netWorkEvents.lNetworkEvents & FD_CONNECT) {
            if (!netWorkEvents.iErrorCode[FD_CONNECT_BIT])
            {
                cout << "���ӷ������ɹ�" << endl;
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
                int rres = recv(sockServer, buf, sizeof(buf) - 1, 0);  // 0��ʾĬ�϶�д�����ص�����
                if (SOCKET_ERROR == rres) {
                    int a = WSAGetLastError();
                    cout << " �ͻ��ˣ�����ʧ�� " << a << endl;
                    continue;
                }
                else
                {
                    cout << "�ͻ����յ���" << rres << "��" << buf << endl;
                }
            }
            else
            {
                continue;
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_CLOSE)
        {
            cout << "���������˳�";
            break;
        }
    }

    WSACloseEvent(eventServer);
    closesocket(sockServer);
    WSACleanup();
    cout << "�ͻ����˳�" << endl;
    return 0;

}
bool SEND(SOCKET sockServer) {

    string str;        getline(cin, str);

    if (str[0] == '0') return 0;

    int sres = send(sockServer, str.c_str(), str.size(), 0);
    if (SOCKET_ERROR == sres) {
        int a = WSAGetLastError();
        cout << "�ͻ��ˣ�����ʧ��" << a << endl;
    }
    return true;
    //true | false �����Ƿ��˳�
}
bool startUp(SOCKET& sockServer) {
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

    sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sockServer) {
        cout << "�ͻ��ˣ�����socketʧ�� " << endl;
        WSACleanup();
        return 0;
    }


    return 1;
}

























#endif




















