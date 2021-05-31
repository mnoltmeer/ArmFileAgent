#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <iterator>
#include <time.h>
#include "..\..\..\work-functions\somefunc.h"
#include "main.h"

int main(int argc, char* argv[])
{
  char initdir [256], config[256];

  try
     {
       GetCurrentDirectoryA(sizeof(initdir), initdir);
       sprintf(config, "%s\\server.ini", initdir);

       s_port = atoi(GetConfigLine(config, "Port").c_str());
       s_ip = GetConfigLine(config, "IP");
       s_capt = GetConfigLine(config, "Caption");

       SetConsoleTitleA("Net Server");
       HANDLE hndl = GetStdHandle(STD_OUTPUT_HANDLE);
       SetConsoleTextAttribute(hndl, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
       printf("%s\n", s_capt.c_str());

       if (s_ip == "0")
         printf("[IP: %s] [Port: %d]\n\n", "any", s_port);
       else
         printf("[IP: %s] [Port: %d]\n\n", s_ip.c_str(), s_port);

       if (argc > 1)
         {
           if (0 == strcmp("-fl", argv[1]))
             {
               fulllog = true;
               printf("(Using full logging mode)\n\n");
             }
         }

       if (InitSock() < 1)
         return -1;

       int lr;

       while (lr = Loop())
         {
           if (lr == 1) //halt
             {
               PrintLog("server stopping...\n");
               StopServer();

               return 0;
             }
           else if (lr == 2) //restart
             {
               PrintLog("server restarting...\n");
               StopServer();

               if (InitSock() < 1)
                 return -1;
             }
         }
     }
  catch (std::exception &e)
     {
       std::cout << "ClientList::Remove: " << e.what() << std::endl;
     }

  return 0;
}
//-------------------------------------------------------------------------

int InitSock()
{
  char tmp[400]; // Буфер для различных нужд

// Шаг 1 - Инициализация Библиотеки Сокетов
// Т.к. возвращенная функцией информация
// не используется ей передается указатель на
// рабочий буфер, преобразуемый
// к указателю  на структуру WSADATA.
// Такой прием позволяет сэкономить одну
// переменную, однако, буфер должен быть не менее
// полкилобайта размером (структура WSADATA
// занимает 400 байт)
  if (WSAStartup(0x0202,(WSADATA *) tmp))
    {
      // Ошибка!
	      printf("Error WSAStartup %d\n",
             WSAGetLastError());
      system("pause");

      return -1;
    }

// Шаг 2 - создание сокета
// AF_INET     - сокет Интернета
// SOCK_STREAM  - потоковый сокет (с установкой соединения)
// 0 - по умолчанию выбирается TCP протокол
  if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Error socket %d\n", WSAGetLastError());
      WSACleanup(); // Деиницилизация библиотеки Winsock
      system("pause");

      return -1;
    }

// Шаг 3 связывание сокета с локальным адресом
  sockaddr_in local_addr;

  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(s_port);

  if (s_ip == "0")
    local_addr.sin_addr.s_addr = ADDR_ANY;
  else
    local_addr.sin_addr.s_addr = inet_addr(s_ip.c_str());

// вызываем bind для связывания
  if (bind(mysocket,(sockaddr *) &local_addr, sizeof(local_addr)))
    {
      printf("Error bind %d\n", WSAGetLastError());
      closesocket(mysocket);  // закрываем сокет!
      WSACleanup();
      system("pause");

      return -1;
    }

// Шаг 4 ожидание подключений
// размер очереди – 16
  if (listen(mysocket, 16))
    {
      printf("Error listen %d\n", WSAGetLastError());
      closesocket(mysocket);
      WSACleanup();
      system("pause");

      return -1;
    }

// Шаг 5 извлекаем сообщение из очереди
  client_addr_size = sizeof(client_addr);

  restart = false;
  halt = false;

  Clients = new ClientList();

  PrintLog("Waiting for connections...\n");

  return 1;
}
//-------------------------------------------------------------------------

int Loop()
{
// цикл извлечения запросов на подключение из очереди
  int res = 0;
  char ip[20], host[256];

  try
     {
       while(client_socket = accept(mysocket, (sockaddr *) &client_addr, &client_addr_size))
         {
// пытаемся получить имя хоста
           HOSTENT *hst = gethostbyaddr((char *) &client_addr.sin_addr.s_addr, 4, AF_INET);

           strcpy(ip, inet_ntoa(client_addr.sin_addr));
           strcpy(host, hst->h_name);

// вывод сведений о клиенте
           sprintf(msg, "%s [%s] connected\n", host, ip);
           PrintLog(msg);

           Clients->Add(new Client(client_socket, ip, host, "n/a"));

// Вызов нового потока для обслуживания клиента
           _beginthreadex(NULL, 0, &ServClient, &client_socket, 0, &thID);

           if (halt)
             {
               res = 1;
               break;
             }
           else if (restart)
             {
               res = 2;
               break;
             }
         }
     }
  catch (std::exception &e)
     {
       std::cout << "Loop: " << e.what() << std::endl;
       res = -1;
     }

  return res;
}
//-------------------------------------------------------------------------

// Эта функция создается в отдельном потоке и
// обслуживает очередного подключившегося клиента
// независимо от остальных
unsigned WINAPI ServClient(void *client_socket)
{
  SOCKET current;
  char in_buff[BUFFSIZE], out_buff[BUFFSIZE];
  SOCKET send_sock; //сокет на который пересылаются данные

  try
     {
       current = ((SOCKET *) client_socket)[0];

       Client *cl = Clients->FindBySocket(current);

       while (ReadFromHost(current, in_buff) != SOCKET_ERROR)
         {
//проверяем полученное сообщение на соотв. триггерам
           /*send_sock = Interprent(current, in_buff, out_buff);

           if (send_sock != SOCKET_ERROR)
             SendToHost(send_sock, out_buff);
           else
             break;*/
         }

       Clients->Remove(cl);
     }
  catch (std::exception &e)
     {
       std::cout << "ServClient: " << e.what() << std::endl;
     }

  return 0;
}
//-------------------------------------------------------------------------

bool SendToHost(SOCKET s, const char *sendstr)
{
  bool res;

  try
     {
       int sent = 0;
       UINT len = strlen(sendstr) + 1, cnt = 0;

       while (cnt < len)
         {
           sent = send(s, sendstr + cnt, len - cnt, 0);

           if (sent < 0)
             {
               res = false;
               break;
             }

           cnt += sent;
         }

       res = true;
     }
  catch (std::exception &e)
     {
       std::cout << "SendToHost: " << e.what() << std::endl;
       res = false;
     }

  return res;
}
//-------------------------------------------------------------------------

int ReadFromHost(SOCKET s, char *read_str)
{
  int recvd = 0;
  int cnt = 0;

  try
     {
       Client *cl = Clients->FindBySocket(s);
       char data[BUFFSIZE];
       char *buf;
       stringstream str;

       while (recvd < BUFFSIZE)
         {
           //recvd = recv(s, read_str + cnt, BUFFSIZE - cnt, 0);

           recvd = recv(s, data, BUFFSIZE, 0);


           if (recvd > 0)
             {
               buf = new char[recvd];

               strncpy(buf, data, recvd);

               str << buf;

               delete[] buf;
             }
           else if (recvd == 0)
             {
               std::cout << str.str() << std::endl;
               recvd = SOCKET_ERROR;
               break;
             }
            else if (recvd < 0)
             {
               sprintf(msg, "Connection with [%s, %s] is closed, code: %d\n", cl->GetIP(), cl->GetHost(), WSAGetLastError());
               PrintLog(msg);
               recvd = SOCKET_ERROR;
               break;
             }

           /*if (recvd == 0)
             {
               sprintf(msg, "End connection with [%s, %s]\n", cl->GetIP(), cl->GetHost());
               PrintLog(msg);
               cnt = SOCKET_ERROR;
               break;
             }
           else if (recvd < 0)
             {
               sprintf(msg, "Connection with [%s, %s] is closed, code: %d\n", cl->GetIP(), cl->GetHost(), WSAGetLastError());
               PrintLog(msg);
               cnt = SOCKET_ERROR;
               break;
             }
           else
             {
               cnt += recvd;

               if (read_str[cnt - 1] == '\0')
                 break;
             }*/
         }
     }
  catch (std::exception &e)
     {
       std::cout << "ReadFromHost: " << e.what() << std::endl;
     }

  return recvd;
}
//-------------------------------------------------------------------------

SOCKET Interprent(SOCKET read_sock, char *read_str, char *send_str)
{
  #define AUTH "#AUTH\n"            //авторизация успешна
  #define QUIT "#QUIT\n"            //успешный выход из системы
  #define ULIST GetClientList()     //список пользователей онлайн
  #define N_CONN "#NO_CONN\n"       //нет соединения с игроком
  #define N_USER "#NO_USR\n"        //нет такого пользователя
  #define STUPD "#STATUS_UPD\n"     //статус успешно изменен
  #define ACCDND "#ACCESS_DENIED\n" //отказано в доступе (неверный пароль админа)

  try
     {
       Client *cl = Clients->FindBySocket(read_sock);

//если включен режим полного логирования - выведем в консоль сообщение
       if (fulllog)
         PrintLog(FR, cl->GetIP(), cl->GetHost(), read_str);

       std::vector <std::string> vecList;

       StrToList(&vecList, std::string(read_str), "_", NODELIMEND);

//---------------------------------------------------//
//клієнтські повідомлення
//---------------------------------------------------//
       if ("LOGIN" == vecList.at(0)) //клиент логинится, указывая ник
         {
           sprintf(msg, "Getting value: %s\n", read_str);
           PrintLog(msg);

           return read_sock;
         }
//-----------------------------------------------------------//
//администраторские команды для управления сервером          //
//---\/------------------------------------------------------//
       else if ("ENDSESSION" == vecList.at(0))//отключение клиента от сервера
         {
//ENDSESSION_клиент_adminpass
           if (vecList.size() != 3)
             {
               sprintf(msg, "Getting incorrect value: %s\n", read_str);
               PrintLog(msg);

               return read_sock;
             }

           if (ADMINPASS == vecList.at(2))
             {
               Client *receiver = Clients->FindByHost(vecList.at(1).c_str());
               SOCKET res = receiver->GetSocket();

               if (INVALID_SOCKET != res)
                 {
                   if (fulllog)
                     {
                       PrintLog(SYS, cl->GetIP(), cl->GetHost(), "uses admin password\n");
                       PrintLog(SYS, receiver->GetIP(), receiver->GetHost(), "connection closed\n");
                     }

                   sprintf(send_str, "Session of %s ip= %s terminated\n", receiver->GetHost(), receiver->GetIP());
                   shutdown(res, SD_BOTH);
                   closesocket(res);

                   return read_sock;
                 }
               else
                 {
                   strcpy(send_str, N_USER);

                   if (fulllog)
                     PrintLog(TO, cl->GetIP(), cl->GetHost(), send_str);

                   return read_sock;
                 }
             }
           else
             {
               strcpy(send_str, ACCDND);

               if (fulllog)
                 PrintLog(TO, cl->GetIP(), cl->GetHost(), send_str);

               return read_sock;
             }
         }
       else if ("NETSTAT" == vecList.at(0))//просмотр расширенного списка подключений
         {
//NETSTAT_adminpass
           if (vecList.size() != 2)
             {
               sprintf(msg, "Getting incorrect value: %s\n", read_str);
               PrintLog(msg);

               return read_sock;
             }

           if (ADMINPASS == vecList.at(1))
             {
               if (fulllog)
                 PrintLog(SYS, cl->GetIP(), cl->GetHost(), "uses admin password\n");

               strcpy(send_str, Clients->PrintList());
             }
           else
             strcpy(send_str, ACCDND);

           if (fulllog)
             PrintLog(TO, cl->GetIP(), cl->GetHost(), send_str);

           return read_sock;
         }
       else if ("RESTART" == vecList.at(0))//перезапуск сервера
         {
//RESTART_adminpass
           if (vecList.size() != 2)
             {
               sprintf(msg, "Getting incorrect value: %s\n", read_str);
               PrintLog(msg);
             }

           if (ADMINPASS == vecList.at(1))
             {
               if (fulllog)
                 PrintLog(SYS, cl->GetIP(), cl->GetHost(), "uses admin password\n");

               restart = true;
             }

           return SOCKET_ERROR;
         }
       else if ("HALT" == vecList.at(0))//полное отключение
         {
//HALT_adminpass
           if (vecList.size() != 2)
             {
               sprintf(msg, "Getting incorrect value: %s\n", read_str);
               PrintLog(msg);
             }

           if (ADMINPASS == vecList.at(1))
             {
               if (fulllog)
                 PrintLog(SYS, cl->GetIP(), cl->GetHost(), "uses admin password\n");

               halt = true;
             }

           return SOCKET_ERROR;
         }
       else
         {
           sprintf(msg, "Getting incorrect value: %s\n", read_str);
           PrintLog(msg);
         }
     }
  catch (std::exception &e)
     {
       std::cout << "Interprent: " << e.what() << std::endl;
       return SOCKET_ERROR;
     }

  return SOCKET_ERROR;
}
//-------------------------------------------------------------------------

void StopServer()
{
  try
     {
       delete Clients;

       closesocket(mysocket);
       WSACleanup();
     }
  catch (std::exception &e)
     {
       std::cout << "StopServer: " << e.what() << std::endl;
     }
}
//-------------------------------------------------------------------------

void PrintLog(char *type, const char *ip, const char *host, char *message)
{
  time_t rawtime;
  struct tm *timeinfo;
  char strtime[80], out_msg[BUFFSIZE], prnt_msg[BUFFSIZE];

  try
     {
       time(&rawtime);
       timeinfo = localtime(&rawtime);
       strftime(strtime, 80, "%Y-%m-%d:%H:%M:%S", timeinfo);
       strcpy(out_msg, message);

       if (0 == strcmp(type, FR))
         strcat(out_msg, "\n");

       sprintf(prnt_msg, "[%s] %s (%s / %s): %s", strtime, type, ip, host, out_msg);

       CPTransOut(CHAROEM, prnt_msg, BUFFSIZE);
     }
  catch (std::exception &e)
     {
       std::cout << "PrintLog: " << e.what() << std::endl;
     }
}
//-------------------------------------------------------------------------

void PrintLog(char *message)
{
  time_t rawtime;
  struct tm *timeinfo;
  char strtime[80], prnt_msg[BUFFSIZE];

  try
     {
       time(&rawtime);
       timeinfo = localtime(&rawtime);
       strftime(strtime, 80, "%Y-%m-%d:%H:%M:%S", timeinfo);

       sprintf(prnt_msg, "[%s] %s: %s", strtime, SYS, message);

       CPTransOut(CHAROEM, prnt_msg, BUFFSIZE);
     }
  catch (std::exception &e)
     {
       std::cout << "PrintLog: " << e.what() << std::endl;
     }
}
//-------------------------------------------------------------------------
