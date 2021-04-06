#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define BUFFSIZE 1024
#define FR "get from"
#define TO "send to"
#define SYS "SYSTEM EVENT"
#define CNFRM "OK\n"

#define CL_IP 1
#define CL_HOST 2
#define CL_NICK 3
#define CL_STATUS 4
#define CL_OPTION 5

const char *ADMINPASS = "morte";

bool fulllog = false; //������ ������������ �����������

int InitSock();

int Loop();

// �������� �������, �������������
// �������������� �������������
DWORD WINAPI ServClient(LPVOID client_socket);

void PrintLog(char *type, const char *ip, const char *nick, char *message);

void PrintLog(char *message);

//���������� ������ ������������� (��� ���������� �������)
const char *PrintUsersTable();

//���������� ������ ������������ ��������
//(��������������� ��� ������ �� �����, ���������� ��������)
const char *GetClientList();

UINT DelClient(SOCKET s);

UINT UpdClientInfo(SOCKET cl_socket, UINT, const char* param_val);

const char *GetClientIP(SOCKET s);

const char *GetClientNick(SOCKET s);

const char *GetClientLeader(SOCKET s);

const char *GetClientStatus(SOCKET s);

SOCKET GetClientSocketByNick(const char *nick);

SOCKET GetClientSocketByIP(const char *ip);

//���������� ������ �����, ���������� "����" � ������ ������ ��������
bool SendToHost(SOCKET host, const char *sendstr);

//������ ������ � �����, ���������� ���-�� ����������� ���� ��� -1 � ������ ������
//������� ����������� ���� �� ������ �� �����
int ReadFromHost(SOCKET host, char *read_str);

//��������� �������� �� �������� ������ �� ������������ ���������
//� ��������� �������� ��� ���������� ������
//���������� ���������� ������, �������� ����� ��������� ���. �����,
//read_sock - ���� ������ ��������������� �������� ������
//� SOCKET_ERROR � ������ ������
SOCKET Interprent(SOCKET read_sock, char *read_str, char *send_str);

//��������� �������� �� ���
bool IsNickFree(const char *nick);

void StopServer();

SOCKET mysocket;
DWORD thID;
SOCKET client_socket;    // ����� ��� �������
sockaddr_in client_addr; // ����� ������� (����������� ��������)
int client_addr_size;
UINT s_port;
string s_ip;
string s_capt;
bool restart, halt;
char msg[256];

// ���������� ���������� � ����������
// �������� �������������
int nclients = 0;

//���������, � ������� �������� ���� � �������
struct CLIENT
{
  SOCKET cl_sock;
  char ip[16];
  char hostname[32];
  char nickname[16];
  char status[8];
  char option[12];
};

std::vector <CLIENTS> vecUList; //������ � ������� �������� �������
                                //������������ ��������

// ������ ��� ������ ���������� �������� �������������
#define PRINTNUSERS if (nclients)\
printf("\n[%d] user on-line:\n%s\n", nclients, PrintUsersTable());\
else printf("\n[0] user on-line\n\n");

#endif // MAIN_H_INCLUDED
