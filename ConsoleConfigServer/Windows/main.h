#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "client.h"

#define BUFFSIZE 4096
#define FR "get from"
#define TO "send to"
#define SYS "SYSTEM EVENT"
#define CNFRM "OK\n"

#define CL_IP 1
#define CL_HOST 2
#define CL_INDEX 3
#define CL_STATUS 4

const char *ADMINPASS = "morte";

bool fulllog = false; //������ ������������ �����������

int InitSock();

int Loop();

//������� ��� �������� ������, ������ ���� ������ � �볺����
unsigned WINAPI ServClient(void *client_socket);

void PrintLog(char *type, const char *ip, const char *nick, char *message);
void PrintLog(char *message);

//���������� ������ �����, ���������� "����" � ������ ������ ��������
bool SendToHost(SOCKET s, const char *sendstr);

//������ ������ � �����, ���������� ���-�� ����������� ���� ��� -1 � ������ ������
//������� ����������� ���� �� ������ �� �����
int ReadFromHost(SOCKET s, char *read_str);

//��������� �������� �� �������� ������ �� ������������ ���������
//� ��������� �������� ��� ���������� ������
//���������� ���������� ������, �������� ����� ��������� ���. �����,
//read_sock - ���� ������ ��������������� �������� ������
//� SOCKET_ERROR � ������ ������
SOCKET Interprent(SOCKET read_sock, char *read_str, char *send_str);

void StopServer();

SOCKET mysocket;
SOCKET client_socket;    // ����� ��� �������
sockaddr_in client_addr; // ����� ������� (����������� ��������)
int client_addr_size;
UINT s_port;
string s_ip;
string s_capt;
bool restart, halt;
char msg[256];
unsigned thID;

ClientList *Clients;

#endif // MAIN_H_INCLUDED
