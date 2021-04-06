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

bool fulllog = false; //маркер расширенного логировани€

int InitSock();

int Loop();

// прототип функции, обслуживающий
// подключившихс€ пользователей
DWORD WINAPI ServClient(LPVOID client_socket);

void PrintLog(char *type, const char *ip, const char *nick, char *message);

void PrintLog(char *message);

//возвращает список пользователей (дл€ статистики сервера)
const char *PrintUsersTable();

//возвращает список подключенных клиентов
//(форматированный дл€ вывода на экран, передаетс€ клиентам)
const char *GetClientList();

UINT DelClient(SOCKET s);

UINT UpdClientInfo(SOCKET cl_socket, UINT, const char* param_val);

const char *GetClientIP(SOCKET s);

const char *GetClientNick(SOCKET s);

const char *GetClientLeader(SOCKET s);

const char *GetClientStatus(SOCKET s);

SOCKET GetClientSocketByNick(const char *nick);

SOCKET GetClientSocketByIP(const char *ip);

//отправл€ет данные хосту, возвращает "ложь" в случае ошибки коннекта
bool SendToHost(SOCKET host, const char *sendstr);

//читает данные с хоста, возвращает кол-во прочитанных байт или -1 в случае ошибки
//выводит расширенную инфу об ошибке на экран
int ReadFromHost(SOCKET host, char *read_str);

//провер€ет значение во вход€щем буфере на соответствие триггерам
//и формирует значение дл€ исход€щего буфера
//возвращает дескриптор сокета, которому нужно отправить исх. буфер,
//read_sock - если данные предназначаютс€ текущему сокету
//и SOCKET_ERROR в случае ошибки
SOCKET Interprent(SOCKET read_sock, char *read_str, char *send_str);

//провер€ет свободен ли ник
bool IsNickFree(const char *nick);

void StopServer();

SOCKET mysocket;
DWORD thID;
SOCKET client_socket;    // сокет дл€ клиента
sockaddr_in client_addr; // адрес клиента (заполн€етс€ системой)
int client_addr_size;
UINT s_port;
string s_ip;
string s_capt;
bool restart, halt;
char msg[256];

// глобальна€ переменна€ Ц количество
// активных пользователей
int nclients = 0;

//структура, в которой хранитс€ инфа о клиенте
struct CLIENT
{
  SOCKET cl_sock;
  char ip[16];
  char hostname[32];
  char nickname[16];
  char status[8];
  char option[12];
};

std::vector <CLIENTS> vecUList; //вектор в котором хранитс€ таблица
                                //подключенных клиентов

// макрос дл€ печати количества активных пользователей
#define PRINTNUSERS if (nclients)\
printf("\n[%d] user on-line:\n%s\n", nclients, PrintUsersTable());\
else printf("\n[0] user on-line\n\n");

#endif // MAIN_H_INCLUDED
