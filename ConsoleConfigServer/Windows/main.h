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

bool fulllog = false; //маркер расширенного логирования

int InitSock();

int Loop();

//функція для окремого потоку, виконує обмін даними з клієнтом
unsigned WINAPI ServClient(void *client_socket);

void PrintLog(char *type, const char *ip, const char *nick, char *message);
void PrintLog(char *message);

//отправляет данные хосту, возвращает "ложь" в случае ошибки коннекта
bool SendToHost(SOCKET s, const char *sendstr);

//читает данные с хоста, возвращает кол-во прочитанных байт или -1 в случае ошибки
//выводит расширенную инфу об ошибке на экран
int ReadFromHost(SOCKET s, char *read_str);

//проверяет значение во входящем буфере на соответствие триггерам
//и формирует значение для исходящего буфера
//возвращает дескриптор сокета, которому нужно отправить исх. буфер,
//read_sock - если данные предназначаются текущему сокету
//и SOCKET_ERROR в случае ошибки
SOCKET Interprent(SOCKET read_sock, char *read_str, char *send_str);

void StopServer();

SOCKET mysocket;
SOCKET client_socket;    // сокет для клиента
sockaddr_in client_addr; // адрес клиента (заполняется системой)
int client_addr_size;
UINT s_port;
string s_ip;
string s_capt;
bool restart, halt;
char msg[256];
unsigned thID;

ClientList *Clients;

#endif // MAIN_H_INCLUDED
