#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <iterator>
#include <time.h>
#include "..\..\work-functions\somefunc.h"
#include "main.h"

int main(int argc, char* argv[])
{
  char initdir [256], config[256];

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
    if (0 == strcmp("-fl", argv[1]))
      {
        fulllog = true;
        printf("(Using full logging mode)\n\n");
      }

  Init:if (InitSock() < 1)
    return -1;

  int lr = Loop();

  if (1 == lr) //halt
    {
      PrintLog("server stopping...\n");
      StopServer();

      return 0;
    }
  else if (2 == lr) //restart
    {
      PrintLog("server restarting...\n");
      StopServer();
      goto Init;
    }

  return 0;
}
//-------------------------------------------------------------------------

int InitSock()
{
  char tmp[400]; // ����� ��� ��������� ����

// ��� 1 - ������������� ���������� �������
// �.�. ������������ �������� ����������
// �� ������������ �� ���������� ��������� ��
// ������� �����, �������������
// � ���������  �� ��������� WSADATA.
// ����� ����� ��������� ���������� ����
// ����������, ������, ����� ������ ���� �� �����
// ������������ �������� (��������� WSADATA
// �������� 400 ����)
  if (WSAStartup(0x0202,(WSADATA *) tmp))
    {
      // ������!
	      printf("Error WSAStartup %d\n",
             WSAGetLastError());
      system("pause");

      return -1;
    }

// ��� 2 - �������� ������
// AF_INET     - ����� ���������
// SOCK_STREAM  - ��������� ����� (� ���������� ����������)
// 0 - �� ��������� ���������� TCP ��������
  if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Error socket %d\n", WSAGetLastError());
      WSACleanup(); // �������������� ���������� Winsock
      system("pause");

      return -1;
    }

// ��� 3 ���������� ������ � ��������� �������
  sockaddr_in local_addr;

  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(s_port);

  if (s_ip == "0")
    local_addr.sin_addr.s_addr = ADDR_ANY;
  else
    local_addr.sin_addr.s_addr = inet_addr(s_ip.c_str());

// �������� bind ��� ����������
  if (bind(mysocket,(sockaddr *) &local_addr, sizeof(local_addr)))
    {
      printf("Error bind %d\n", WSAGetLastError());
      closesocket(mysocket);  // ��������� �����!
      WSACleanup();
      system("pause");

      return -1;
    }

// ��� 4 �������� �����������
// ������ ������� � 16
  if (listen(mysocket, 16))
    {
      printf("Error listen %d\n", WSAGetLastError());
      closesocket(mysocket);
      WSACleanup();
      system("pause");

      return -1;
    }

// ��� 5 ��������� ��������� �� �������

  client_addr_size = sizeof(client_addr);

  restart = false;
  halt = false;

  PrintLog("Waiting for connections...\n");

  return 1;
}
//-------------------------------------------------------------------------

int Loop()
{
// ���� ���������� �������� �� ����������� �� �������
  while(client_socket = accept(mysocket, (sockaddr *) &client_addr, &client_addr_size))
    {
      nclients++;  // ����������� ������� �������������� ��������

// �������� �������� ��� �����
      HOSTENT *hst;
      hst = gethostbyaddr((char *) &client_addr.sin_addr.s_addr, 4, AF_INET);

// ����� �������� � �������
      sprintf(msg, "%s [%s] connected\n",
              hst->h_name,
              inet_ntoa(client_addr.sin_addr));
      PrintLog(msg);
//������� ���� � ������� � ������
      CLIENTS new_client;

      strcpy(new_client.hostname, hst->h_name);
      strcpy(new_client.ip, inet_ntoa(client_addr.sin_addr));
      strcpy(new_client.nickname, "n/a");
      strcpy(new_client.option, "n/a");
      strcpy(new_client.status, "n/a");
      new_client.cl_sock = client_socket;

      vecUList.push_back(new_client);

// ����� ������ ������ ��� ����������� �������
// ��, ��� ����� ������������� ������������
// _beginthreadex ��, ��������� ������� �����
// ������� ����������� �� ���������� ����� ��
// ������, ����� �������� � CreateThread
      CreateThread(NULL, NULL, ServClient, &client_socket, NULL, &thID);

      if (halt)
        return 1;
      else if (restart)
        return 2;
    }

  return 0;
}
//-------------------------------------------------------------------------

// ��� ������� ��������� � ��������� ������ �
// ���������� ���������� ��������������� �������
// ���������� �� ���������
DWORD WINAPI ServClient(LPVOID client_socket)
{
  SOCKET current;
  current = ((SOCKET *) client_socket)[0];
  char in_buff[BUFFSIZE], out_buff[BUFFSIZE];
  SOCKET send_sock; //����� �� ������� ������������ ������

  while (ReadFromHost(current, in_buff) != SOCKET_ERROR)
    {
//��������� ���������� ��������� �� �����. ���������
      send_sock = Interprent(current, in_buff, out_buff);

      if (send_sock != SOCKET_ERROR)
        {
          //if (send_sock != current)
            //SendToHost(current, CNFRM);

          SendToHost(send_sock, out_buff);
        }

      else
        break;
    }

  DelClient(current);

  return 0;
}
//-------------------------------------------------------------------------

const char *GetClientList()
{
  static char outstr[BUFFSIZE];
  UINT cnt = 0;
  strcpy(outstr, "No user on line...\n");

  if (vecUList.size() > 0)
    {
      cnt += sprintf(outstr + cnt, "#LIST");
      for (UINT i = 0; i < vecUList.size(); i++)
        {
          cnt += sprintf(outstr + cnt,
                         ":[%d] %s (%s)",
                         i,
                         vecUList[i].nickname,
                         vecUList[i].status);
        }

      strcat(outstr, "\n");
    }

  return outstr;
}
//-------------------------------------------------------------------------

const char *PrintUsersTable()
{
  static char out_str[BUFFSIZE];
  char in_str[BUFFSIZE];

  UINT cnt = 0;

  for (UINT i = 0; i < vecUList.size(); i++)
    {
      cnt += sprintf(in_str + cnt,
                     "[%d] (%d) %s  %s  {login=%s  status=%s  option=%s}\n",
                     i,
                     (UINT)vecUList[i].cl_sock,
                     vecUList[i].ip,
                     vecUList[i].hostname,
                     vecUList[i].nickname,
                     vecUList[i].status,
                     vecUList[i].option);
    }

  //strcat(in_str, "\n");

  CharToOemBuffA(in_str, out_str, BUFFSIZE);

  return out_str;
}
//-------------------------------------------------------------------------

bool IsNickFree(const char *nick)
{
  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (0 == strcmp(vecUList[i].nickname, nick))
        return false;
    }

  return true;
}
//-------------------------------------------------------------------------

UINT UpdClientInfo(SOCKET cl_socket, UINT param, const char* param_val)
{
  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (cl_socket == vecUList[i].cl_sock)
        {
          switch (param)
            {
              case CL_IP: strcpy(vecUList[i].ip, param_val); break;
              case CL_HOST: strcpy(vecUList[i].hostname, param_val); break;
              case CL_NICK: strcpy(vecUList[i].nickname, param_val); break;
              case CL_STATUS: strcpy(vecUList[i].status, param_val); break;
              case CL_OPTION: strcpy(vecUList[i].option, param_val); break;
            }

          return 1;
        }
    }

  return 0;
}
//-------------------------------------------------------------------------

UINT DelClient(SOCKET s)
{
  std::vector<CLIENTS>::iterator it;

  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (s == vecUList[i].cl_sock)
        {
          it = vecUList.begin() + i;
          vecUList.erase(it);

          nclients--; // ��������� ������� �������� ��������
          closesocket(s); // ��������� �����

          PRINTNUSERS

          return 1;
        }
    }

  PRINTNUSERS

  return 0;
}
//-------------------------------------------------------------------------

const char *GetClientIP(SOCKET s)
{
  static char outstr[128];

  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (s == vecUList[i].cl_sock)
        {
          strcpy(outstr, vecUList[i].ip);

          return outstr;
        }
    }

  return "!err";
}
//-------------------------------------------------------------------------

const char *GetClientNick(SOCKET s)
{
  static char outstr[128];

  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (s == vecUList[i].cl_sock)
        {
          strcpy(outstr, vecUList[i].nickname);

          return outstr;
        }
    }

  return "!err";
}
//-------------------------------------------------------------------------

const char *GetClientOption(SOCKET s)
{
  static char outstr[128];

  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (s == vecUList[i].cl_sock)
        {
          strcpy(outstr, vecUList[i].option);

          return outstr;
        }
    }

  return "!err";
}
//-------------------------------------------------------------------------

const char *GetClientStatus(SOCKET s)
{
  static char outstr[128];

  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (s == vecUList[i].cl_sock)
        {
          strcpy(outstr, vecUList[i].status);

          return outstr;
        }
    }

  return "!err";
}
//-------------------------------------------------------------------------

SOCKET GetClientSocketByNick(const char *nick)
{
  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (0 == strcmp(vecUList[i].nickname, nick))
        return vecUList[i].cl_sock;
    }

  return INVALID_SOCKET;
}
//-------------------------------------------------------------------------

SOCKET GetClientSocketByIP(const char *ip)
{
  for (UINT i = 0; i < vecUList.size(); i++)
    {
      if (0 == strcmp(vecUList[i].ip, ip))
        return vecUList[i].cl_sock;
    }

  return INVALID_SOCKET;
}
//-------------------------------------------------------------------------

bool SendToHost(SOCKET host, const char *sendstr)
{
  int sent = 0;
  UINT len = strlen(sendstr) + 1, cnt = 0;

  while (cnt < len)
    {
      sent = send(host, sendstr + cnt, len - cnt, 0);

      if (sent < 0)
        return false;

      cnt += sent;
    }

  return true;
}
//-------------------------------------------------------------------------

int ReadFromHost(SOCKET host, char *read_str)
{
  int recvd;
  int cnt = 0;

  while(cnt < BUFFSIZE)
    {
      recvd = recv(host, read_str + cnt, BUFFSIZE - cnt, 0);

      if (recvd == 0)
        {
          sprintf(msg, "Connection with [%s, %s] is lost...\n",
                  GetClientIP(host),
                  GetClientNick(host));

          PrintLog(msg);

          return SOCKET_ERROR;
        }
      else if (recvd < 0)
        {
          sprintf(msg, "Connection with [%s, %s] is closed with code: %d\n",
                  GetClientIP(host),
                  GetClientNick(host),
                  WSAGetLastError());

          PrintLog(msg);

          return SOCKET_ERROR;
        }
      else
        {
          cnt += recvd;

          if (read_str[cnt - 1] == '\0')
            return cnt;
        }
    }

  return -1;
}
//-------------------------------------------------------------------------

SOCKET Interprent(SOCKET read_sock, char *read_str, char *send_str)
{
  #define AUTH "#AUTH\n"            //����������� �������
  #define QUIT "#QUIT\n"            //�������� ����� �� �������
  #define N_AUTH "#INV_LOGIN\n"     //����� ���������� ��� �����
  #define ULIST GetClientList()     //������ ������������� ������
  #define N_CONN "#NO_CONN\n"       //��� ���������� � �������
  #define N_USER "#NO_USR\n"        //��� ������ ������������
  #define OPTUPD "#OPTION_UPD\n"    //����� ������� �������
  #define STUPD "#STATUS_UPD\n"     //������ ������� �������
  #define ACCDND "#ACCESS_DENIED\n" //�������� � ������� (�������� ������ ������)
  #define LSMSTP "#STPLIST\n"       //������� ��������� ������ ������ ���������
  //#define DLGSTP "#STPDLG\n"        //������� ����������� ����������� ������
  //#define DLGREQ "#REQDLG\n"        //������ �� ��������� ����������� ������
  //#define DLGACC "#ACCDLG\n"        //������������� ����������� ������
  //#define DLGSTP "#DNDDLG\n"        //����� �� ����������� ������

//���� ������� ����� ������� ����������� - ������� � ������� ���������
  if (fulllog)
    PrintLog(FR, GetClientIP(read_sock), GetClientNick(read_sock), read_str);

  std::vector <std::string> vecList;

  StrToList(&vecList, std::string(read_str), "_", NODELIMEND);

//---------------------------------------------------//
//������������� �������, ����������� � ���������� dll//
//---\/----------------------------------------------//
  if ("LOGIN" == vecList.at(0)) //������ ���������, �������� ���
    {
//LOGIN_���
      if (vecList.size() != 2)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      if (IsNickFree(vecList.at(1).c_str())) //��������� �������� �� ���
        {
          UpdClientInfo(read_sock, CL_NICK, vecList.at(1).c_str());
          UpdClientInfo(read_sock, CL_STATUS, "waiting");
          strcpy(send_str, AUTH);
          PRINTNUSERS

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

          return read_sock;
        }
      else
        {
          strcpy(send_str, N_AUTH);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

          return read_sock;
        }
    }
  else if ("LOGOUT" == vecList.at(0)) //������ ������������
    {
      char nick[16], ip[16];

      strcpy(nick, GetClientNick(read_sock));
      strcpy(ip, GetClientIP(read_sock));

      if (0 != strcmp(nick, "!err"))
        {
          char prnt_str[BUFFSIZE];

          sprintf(prnt_str, "\n>> %s [%s] logged out...\n", nick, ip);
          CPTransOut(CHAROEM, prnt_str, BUFFSIZE);
          strcpy(send_str, QUIT);

          if (fulllog)
            PrintLog(TO, ip, nick, send_str);

          UpdClientInfo(read_sock, CL_NICK, "n/a");
          UpdClientInfo(read_sock, CL_STATUS, "n/a");
          UpdClientInfo(read_sock, CL_OPTION, "n/a");
        }

      return read_sock;
    }
  else if ("MSG" == vecList.at(0))//������ ���������� ��������� ������� �������
    {
//MSG_(����)_(�� ����)_(���������)
      if (vecList.size() != 4)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      if (!IsNickFree(vecList.at(1).c_str())) //��������� ��� ��������
        {
          SOCKET s = GetClientSocketByNick(vecList.at(1).c_str());

          if (s != INVALID_SOCKET)
            {
//�������� � ����� ��� ����������� � ��������� #MSG:(�� ����):(�����)
              sprintf(send_str, "#MSG:%s:%s\n",
                      vecList.at(2).c_str(),
                      vecList.at(3).c_str());

              if (fulllog)
                PrintLog(TO, GetClientIP(s), vecList.at(1).c_str(), send_str);

              return s;
            }
          else
            {
              strcpy(send_str, N_CONN);

              if (fulllog)
                PrintLog(TO, GetClientIP(read_sock), vecList.at(2).c_str(), send_str);

              return read_sock;
            }
        }
      else
        {
          strcpy(send_str, N_USER);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), vecList.at(2).c_str(), send_str);

          return read_sock;
        }
    }
  else if ("LIST" == vecList.at(0)) //������ ������ �������������
    {
      strcpy(send_str, ULIST);

      if (fulllog)
        PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

      return read_sock;
    }
  else if ("STATUSSET" == vecList.at(0)) //������ �������� ���� ������
    {
//STATUSSET_������
      if (vecList.size() != 2)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      UpdClientInfo(read_sock, CL_STATUS, vecList.at(1).c_str());
      strcpy(send_str, STUPD);

      if (fulllog)
        PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

      return read_sock;
    }
  else if ("OPTIONSET" == vecList.at(0)) //������ �������� ����� ����� ������
    {
//OPTIONSET_�����
      if (vecList.size() != 2)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      if (!IsNickFree(vecList.at(1).c_str())) //��������� ���������� �� ���
        {
          UpdClientInfo(read_sock, CL_OPTION, vecList.at(2).c_str());
          strcpy(send_str, OPTUPD);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), vecList.at(1).c_str(), send_str);

          return read_sock;
        }
      else
        {
          strcpy(send_str, N_USER);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

          return read_sock;
        }
    }
  else if ("ASKOPTION" == vecList.at(0))//������ ����������� ����� ������ ������� �������
    {
//ASKOPTION_������������
      if (vecList.size() != 2)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      char opt[16];
      SOCKET s = GetClientSocketByNick(vecList.at(1).c_str());
      strcpy(opt, GetClientOption(s));

      if (0 != strcmp(opt, "!err"))
        {
          sprintf(send_str, "OPTION_%s\n", opt);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

          return read_sock;
        }
      else
        {
          strcpy(send_str, N_USER);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

          return read_sock;
        }
    }

//-----------------------------------------------------------//
//����������������� ������� ��� ���������� ��������          //
//---\/------------------------------------------------------//
  else if ("ENDSESSION" == vecList.at(0))//���������� ������� �� �������
    {
//ENDSESSION_������_adminpass
      if (vecList.size() != 3)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      if (ADMINPASS == vecList.at(2))
        {
          SOCKET res = GetClientSocketByNick(vecList.at(1).c_str());

          if (INVALID_SOCKET != res)
            {
              if (fulllog)
                {
                  PrintLog(SYS, GetClientIP(read_sock), GetClientNick(read_sock), "uses admin password\n");
                  PrintLog(SYS, GetClientIP(res), vecList.at(1).c_str(), "connection closed\n");
                }

              sprintf(send_str, "Session of %s ip= %s terminated\n", vecList.at(1).c_str(), GetClientIP(res));
              shutdown(res, SD_BOTH);
              closesocket(res);

              return read_sock;
            }
          else
            {
              strcpy(send_str, N_USER);

              if (fulllog)
                PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

              return read_sock;
            }
        }
      else
        {
          strcpy(send_str, ACCDND);

          if (fulllog)
            PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

          return read_sock;
        }
    }
  else if ("NETSTAT" == vecList.at(0))//�������� ������������ ������ �����������
    {
//NETSTAT_adminpass
      if (vecList.size() != 2)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      if (ADMINPASS == vecList.at(1))
        {
          if (fulllog)
            PrintLog(SYS, GetClientIP(read_sock), GetClientNick(read_sock), "uses admin password\n");

          strcpy(send_str, PrintUsersTable());
        }
      else
        strcpy(send_str, ACCDND);

      if (fulllog)
        PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

      return read_sock;
    }
  else if ("RESTART" == vecList.at(0))//���������� �������
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
            PrintLog(SYS, GetClientIP(read_sock), GetClientNick(read_sock), "uses admin password\n");

          restart = true;
        }

      return SOCKET_ERROR;
    }
  else if ("HALT" == vecList.at(0))//������ ����������
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
            PrintLog(SYS, GetClientIP(read_sock), GetClientNick(read_sock), "uses admin password\n");

          halt = true;
        }

      return SOCKET_ERROR;
    }
  else if ("LMSTOP" == vecList.at(0))//��������� ������ ������ ���������
    {
//LMSTOP_login_adminpass
      if (vecList.size() != 3)
        {
          sprintf(msg, "Getting incorrect value: %s\n", read_str);
          PrintLog(msg);

          return SOCKET_ERROR;
        }

      if (ADMINPASS == vecList.at(2))
        {
          if (fulllog)
            PrintLog(SYS, GetClientIP(read_sock), GetClientNick(read_sock), "uses admin password\n");

          SOCKET s = GetClientSocketByNick(vecList.at(1).c_str());

          if (INVALID_SOCKET != s) //��������� �������� ������� ���������
            {
              strcpy(send_str, LSMSTP);

              if (fulllog)
                PrintLog(TO, GetClientIP(s), vecList.at(1).c_str(), send_str);

              return s;
            }
          else
            {
              strcpy(send_str, N_USER);
            }
        }
      else
        strcpy(send_str, ACCDND);

      if (fulllog)
        PrintLog(TO, GetClientIP(read_sock), GetClientNick(read_sock), send_str);

      return read_sock;
    }

  sprintf(msg, "Getting incorrect value: %s\n", read_str);
  PrintLog(msg);

  return SOCKET_ERROR;
}
//-------------------------------------------------------------------------

void StopServer()
{
  for (UINT i = 0; i < vecUList.size(); i++)
    closesocket(vecUList[i].cl_sock);

  closesocket(mysocket);
  WSACleanup();
}
//-------------------------------------------------------------------------

void PrintLog(char *type, const char *ip, const char *nick, char *message)
{
  time_t rawtime;
  struct tm * timeinfo;
  char strtime[80], out_msg[BUFFSIZE], prnt_msg[BUFFSIZE];

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(strtime, 80, "%Y-%m-%d:%H:%M:%S", timeinfo);
  strcpy(out_msg, message);

  if (0 == strcmp(type, FR))
    strcat(out_msg, "\n");

  sprintf(prnt_msg, "[%s] %s (%s / %s): %s",
         strtime,
         type,
         ip,
         nick,
         out_msg);

  CPTransOut(CHAROEM, prnt_msg, BUFFSIZE);
}
//-------------------------------------------------------------------------

void PrintLog(char *message)
{
  time_t rawtime;
  struct tm * timeinfo;
  char strtime[80], prnt_msg[BUFFSIZE];

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(strtime, 80, "%Y-%m-%d:%H:%M:%S", timeinfo);

  sprintf(prnt_msg, "[%s] %s: %s",
          strtime,
          SYS,
          message);

  CPTransOut(CHAROEM, prnt_msg, BUFFSIZE);
}
//-------------------------------------------------------------------------
