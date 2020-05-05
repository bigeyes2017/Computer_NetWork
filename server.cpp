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
}esSet; //ȫ�ֱ�������ʼ�����ᱻ��0


BOOL WINAPI funClear(DWORD CtrlType) {
    switch (CtrlType)
    {
    case CTRL_CLOSE_EVENT:
        for (unsigned short i = 0; i < esSet.count; i++)
        {
            WSACloseEvent(esSet.allevent[i]);
            closesocket(esSet.allsock[i]);
            printf("��������close%d", esSet.allsock[i]);
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
    //sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
    sAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b4 = 1;

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


    //�����¼�ѡ��ģ��
    
    WSAEVENT eventServer = WSACreateEvent();
    if (WSA_INVALID_EVENT == eventServer) {
        int a = WSAGetLastError();
        printf("WSACreateEventʧ�ܣ�%d", a);

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //�󶨣���socket��event����������ָ��Os�������
    if (SOCKET_ERROR == WSAEventSelect(sockServer, eventServer, FD_ACCEPT)) {
        WSACloseEvent(eventServer);

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //������ṹ����
    esSet.allevent[0] = eventServer;
    esSet.allsock[0] = sockServer;
    esSet.count++;


    //ѭ���鿴�¼��Ƿ����źŲ�����
    while (true)
    {   
        //�鿴
        DWORD nRes = 
            WSAWaitForMultipleEvents(esSet.count, esSet.allevent, false, WSA_INFINITE, false);

        if (WSA_WAIT_FAILED== nRes) {//���ǵȴ���ʧ�ܡ�
            break;
        }
        DWORD nIndex = nRes - WSA_WAIT_EVENT_0; //�õ��±�

        
        WSANETWORKEVENTS netWorkEvents;         //���ź�ֵ��λ�����ڽṹ���С�
        if (SOCKET_ERROR ==
            WSAEnumNetworkEvents(esSet.allsock[nIndex], esSet.allevent[nIndex], &netWorkEvents)) {
            continue;
        }

        if (netWorkEvents.lNetworkEvents & FD_ACCEPT) {
            if (0 == netWorkEvents.iErrorCode[FD_ACCEPT_BIT]) {
                SOCKET sockClient = accept(esSet.allsock[nIndex], NULL, NULL);
                if (INVALID_SOCKET == sockClient) {
                    printf("�������������ͻ���socketʧ��\n");
                    continue;
                }
                //ÿ��socket��Ҫ���¼���
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

                printf("��������accept%d\n",sockClient);
            }
            else {
                continue;
            }
        } //ifNet

        //write����accept֮�󼤻�һ�Σ��˺�Ĭ�Ͽɷ��ͣ����Բ����ٱ����
        if (netWorkEvents.lNetworkEvents & FD_WRITE) {
            if (0 == netWorkEvents.iErrorCode[FD_WRITE_BIT]) {
                if (SOCKET_ERROR == send(esSet.allsock[nIndex], "��ã������Ƿ�����", 100, 0)) {
                    continue;
                }   
                printf("��������send%d\n", esSet.allsock[nIndex]);    //���﷢���˿ͻ���Ҳ��һ���յĵ���
            }
            else
            {
                printf("������send����%d\n", netWorkEvents.iErrorCode[FD_WRITE_BIT]);
            }
        }


        if (netWorkEvents.lNetworkEvents & FD_READ) {
            if (0 == netWorkEvents.iErrorCode[FD_READ_BIT]) {
                char str[1500] = { 0 };
                if (SOCKET_ERROR == recv(esSet.allsock[nIndex], str, 1499, 0)) {
                    continue;
                }
                printf("������recv :%s\n", str);
            }
            else {
                printf("������send����%d\n", netWorkEvents.iErrorCode[FD_READ_BIT]);
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_CLOSE) {

            printf("��������close%d", esSet.allsock[nIndex]);

            WSACloseEvent(esSet.allevent[nIndex]);
            closesocket(esSet.allsock[nIndex]);
            //ɾ��
            esSet.allevent[nIndex] = esSet.allevent[esSet.count - 1];
            esSet.allsock[nIndex] = esSet.allsock[esSet.count - 1];
            esSet.count--;
        }
    }

    //��X�رա�
    cin.get();
    return 0;
}
