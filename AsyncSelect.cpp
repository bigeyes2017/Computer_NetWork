#if 0

//#include<Windows.h>����������һ��ͷ�ļ���
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <string.h>
#include <iostream>
#define UM_MSGFDACCEPT WM_USER+1
using namespace std;

bool startUp(SOCKET&);
LRESULT WINAPI fnWndProc(HWND hWnd, UINT uId, WPARAM wParam, LPARAM lParam);

#define MAX_SOCK_COUNT 1024
SOCKET allSock[MAX_SOCK_COUNT];
unsigned short cont = 0;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hpreInstance, _In_ LPSTR lpCmdline, _In_ int nShowCmd) {
    SOCKET sockServer;
    if (!startUp(sockServer))return 0;//������⣺���汾�ţ�����socket������Ϣ��listen
    allSock[cont++] = sockServer;
    //������win32Wnd����------------------
    WNDCLASSEX wce;
    wce.cbClsExtra = 0;
    wce.cbWndExtra = 0;
    wce.hInstance = hInstance;
    wce.cbSize = sizeof(WNDCLASSEX);

    wce.style = CS_VREDRAW | CS_HREDRAW;

    wce.lpszMenuName = NULL;
    wce.lpszClassName = TEXT("theFirstInstance");
    wce.lpfnWndProc = fnWndProc;	//�˴�Ҫ׼���ص�������	

    wce.hIcon = NULL;
    wce.hIconSm = NULL;
    wce.hCursor = NULL;

    wce.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

    ATOM res = RegisterClassEx(&wce);
    if (0 == res) { return 1; }

    //��������id
    HWND hWnd = CreateWindowEx(WS_EX_TOPMOST, wce.lpszClassName, TEXT("For ֽ��"), WS_OVERLAPPEDWINDOW, 300, 200, 700, 500,
        NULL, NULL, hInstance, NULL
    );
    if (NULL == hWnd) {
        return 1;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);


    if (SOCKET_ERROR == WSAAsyncSelect(sockServer, hWnd, UM_MSGFDACCEPT, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE)) {//  ���ʱ���Ḳ��
        int a = WSAGetLastError();
        cout << "���������첽�¼���Ϣ��ʧ��" << a << endl;
        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //�ַ���Ϣ
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);

        DispatchMessageW(&msg);
    }

    return 0;
}

LRESULT WINAPI fnWndProc(HWND hWnd, UINT uId, WPARAM wParam, LPARAM lParam) {
    static int y = 0;
    HDC hdc = GetDC(hWnd);

    switch (uId)
    {
    case UM_MSGFDACCEPT:
    {
        //MessageBox(NULL, TEXT("���ź���"), TEXT("��ʾ"),MB_YESNO );
        SOCKET sock = (SOCKET)wParam;   //�õ�socket

        if (0 != HIWORD(lParam)) {
            if (!(WSAECONNABORTED == HIWORD(lParam))) {
                break; //������
            }

        }
        switch (LOWORD(lParam))
        {
        case FD_ACCEPT:
        {
            TextOut(hdc, 0, y, TEXT("Accept"), strlen("Accept"));
            y += 15;
            SOCKET sockClient = accept(sock, NULL, NULL);
            if (INVALID_SOCKET == sockClient) {
                break;
            }
            if (SOCKET_ERROR == WSAAsyncSelect(sockClient, hWnd, UM_MSGFDACCEPT, FD_READ | FD_WRITE | FD_CLOSE))
            {
                closesocket(sockClient);
                break;
            }
            allSock[cont++] = sockClient;
        }
        break;
        case FD_READ:
        {
            TextOut(hdc, 0, y, TEXT("Read"), strlen("Read"));    //strlen������\0��
            char str[1024] = { 0 };
            if (SOCKET_ERROR == recv(sock, str, 1024, 0)) {
                break;
            }
            TextOut(hdc, 40, y, str, strlen(str));    //strlen������\0��
            y += 15;
        }
        break;

        case FD_WRITE:  //��FD_ACCEPT�󴥷�һ�Ρ����ڳ�ʼ�����ӡ�
        {
            TextOut(hdc, 0, y, TEXT("Send"), strlen("Send"));

            if (SOCKET_ERROR == send(sock, TEXT("���ӳɹ�"), strlen("���ӳɹ�"), 0)) {
                break;
            }
            y += 15;
        }
        break;

        //FD_CLOSE�������ͬʱ����
        case FD_CLOSE:  //�ͻ����˳���һɾ����
        {
            TextOut(hdc, 0, y, TEXT("Close"), strlen("Close"));
            //�رո�Socket�ϵ���Ϣ
            WSAAsyncSelect(sock, hWnd, 0, 0);
            //�ر�socket
            closesocket(sock);
            //������ɾ��
            for (unsigned short i = 0; i < cont; i++)
            {
                if (sock == allSock[i]) {
                    allSock[i] = allSock[cont - 1];
                    cont--;
                }
            }
            y += 15;
        }
        break;
        }

        break;
    }

    case WM_CREATE:

        break;
    case WM_CLOSE:	//���X��ť����ʱ�û��ͷ�new�Ŀռ�
        break;

    case WM_DESTROY://ϵͳ�ͷŴ��ڣ���ʱӦ���˳�����
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    ReleaseDC(hWnd, hdc);
    return DefWindowProc(hWnd, uId, wParam, lParam);
}
bool startUp(SOCKET& sockServer) {
    //�������--------------------
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

        for (SOCKET& s : allSock)
        {
            closesocket(s);
        }

        return 0;
    }
    //���汾
    if (2 != LOBYTE(wdSockMsg.wVersion) || 2 != HIBYTE(wdSockMsg.wVersion)) {

        WSACleanup();
        return 0;
    }

    //����socket
    sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    return true;
}
#endif