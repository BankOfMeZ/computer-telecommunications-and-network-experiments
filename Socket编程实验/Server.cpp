#pragma once
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

void sendFile(char* path, SOCKET sessionSocket) {
	//解析文件类型
	char* t = strrchr(path, '.');
	char* fileType = t != nullptr ? t + 1 : t;
	char contentType[20] = "text/plain";
	if (t == nullptr) {
		printf("invalid file\n");
		goto inx;
	}
	if (!strcmp(fileType, "html")) {
		strcpy(contentType, "text/html");
	}
	else if (!strcmp(fileType, "jpg")) {
		strcpy(contentType, "image/jpg");
	}
	else if (!strcmp(fileType, "png")) {
		strcpy(contentType, "image/png");
	}
	else if (!strcmp(fileType, "gif")) {
		strcpy(contentType, "image/gif");
	}
	else {
		printf("cannot be tackled file\n");
	}
	//发送http文件状态
inx:
	int flag = 0;
	FILE* file = fopen(path, "rb");
	if (file == nullptr) {
		printf("cannot open file\n");
		strcpy(contentType, "text/html");
		file = fopen("D:\\netExp\\Server\\error.html", "rb");
		flag = -1;
	}
	int res = (flag == 0 ? send(sessionSocket, "HTTP/1.1 200 OK\r\n",
		strlen("HTTP/1.1 200 OK\r\n"), 0) : send(sessionSocket, "HTTP/1.1 404 OK\r\n",
		strlen("HTTP/1.1 404 OK\r\n"), 0));
	if (res == SOCKET_ERROR) {
		printf("send state error\n");
		WSAGetLastError();
		return;
	}
	//发送http文件内容类型
	if (contentType) {
		char t1[100] = "Content-type: ";
		strcat(t1, strcat(contentType, "\r\n\r\n"));
		if (send(sessionSocket, t1, strlen(t1), 0) == SOCKET_ERROR) {
			printf("send content-type error");
			WSAGetLastError();
			return;
		}
	}
	//发送文件内容
	//计算文件长度
	fseek(file, 0L, SEEK_END);
	long fileLength = ftell(file);
	//将文件内容复制至数组发送
	char* t2 = (char*)malloc(sizeof(char) * (fileLength + 1));
	if (t2 == nullptr) {
		printf("malloc fail");
		return;
	}
	fseek(file, 0L, SEEK_SET);
	fread(t2, fileLength, 1, file);
	if (send(sessionSocket, t2, fileLength, 0) == SOCKET_ERROR) {
		printf("send file error");
		WSAGetLastError();
	}
}

void main() {
	WSADATA wsaData;
	int nRc = WSAStartup(0x0202, &wsaData);
	if (nRc == WSAEINVAL) {
		printf("Winsock startup failed with error!\n");
		WSAGetLastError();
	}
	if (wsaData.wVersion != 0x0202) {
		printf("Winsock version is not correct!\n");
	}

	//监听socket
	SOCKET srvSocket;

	//服务器地址和客户端地址
	sockaddr_in addr, clientAddr;

	//会话socket，负责和client进程通信
	SOCKET sessionSocket;

	//ip地址长度
	int addrLen;

	//创建监听socket
	srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (srvSocket == INVALID_SOCKET) {
		printf("Socket create fail!\n");
		WSAGetLastError();
	}
	//设置服务器的端口和地址
	string ip, mainDir;
	int port;
	printf("输入监听地址、监听端口和主目录：");
	std::cin >> ip >> port >> mainDir;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	//binding
	int rtn = bind(srvSocket, (LPSOCKADDR)&addr, sizeof(addr));
	if (rtn == SOCKET_ERROR) {
		printf("Socket bind fail!\n");
		WSAGetLastError();
	}
	//监听
	rtn = listen(srvSocket, 5);
	if (rtn == SOCKET_ERROR) {
		printf("Socket listen fail!\n");
		WSAGetLastError();
	}
	clientAddr.sin_family = AF_INET;
	addrLen = sizeof(clientAddr);

	//设置接收缓冲区
	char recvBuf[4096];

	printf("waiting for request\n");

	while (true) {
		//产生会话SOCKET
		sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
		if (sessionSocket == INVALID_SOCKET) {
			printf("connect fail\n");
			WSAGetLastError();
		}
		printf("请求的来源（IP 地址、端口号）:%s:%u\n", inet_ntoa(clientAddr.sin_addr),
			htons(clientAddr.sin_port));
		memset(recvBuf, '\0', 2048);
		rtn = recv(sessionSocket, recvBuf, 2048, 0);
		if (rtn >= 0) {
			printf("Received %d bytes from client:\n%s\n", rtn, recvBuf);
			int i = 0;
			while (recvBuf[i] != '/') {
				i++;
			}
			char dir[256] = "";
			int j;
			for (j = 0; recvBuf[i + 1] != ' '; i++, j++) {
				dir[j] = recvBuf[i + 1];
			}
			dir[j] = '\0';
			char path[256] = "";
			strcpy(path, mainDir.c_str());
			strcat(path, dir);
			printf("send file: %s\n", path);
			sendFile(path, sessionSocket);
		}
		else {
			printf("receive fail\n");
			WSAGetLastError();
		}
		closesocket(sessionSocket);
	}
	printf("Client leaving ...\n");
	closesocket(srvSocket);  //既然client离开了，就关闭sessionSocket
	WSACleanup();
}