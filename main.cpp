#include <cstdio>
#include <iostream>
#include <sys/_types/_in_port_t.h>
#include <sys/_types/_sa_family_t.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>

/* TEST CLASS */
#include "Core.hpp"

// TODO: 서버 Core Class 작성
// TODO: 접속한 유저의 데이터 및 정보를 저장해둘 User Class 작성


// TODO: <나중>
// 채널 클래스 (패스워드, 속해있는 유저, invite only인지 등)

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "ERROR: Usage: ./irc-serv [PORT] [PASS]" << std::endl;
		return 1;
	}
	Core	Core(av[1], av[2]);
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	// AF_INET: IPv4
	// SOCK_STREAM: TCP (소켓의 데이터 전송 방식)
	// 0 -> 소켓이 사용할 프로토콜 정의 (0은 2번째 인자 따라감)
	if (server_fd == -1)
	{
		std::cerr << "server_fd open failed!" << std::endl;
		return -1;
	}
	struct sockaddr_in server_addr; 			// 이제 이게 전화번호가 될꺼에요
	server_addr.sin_family = AF_INET;			// IPv4 주소 체계 사용
	server_addr.sin_addr.s_addr = INADDR_ANY;	// 모든 가능한 IP 주소
	server_addr.sin_port = htons(1599); // 호스트 바이트 순서 -> 네트워크 바이트 순서
	// 이 서버에 접속할때 로컬 IP에서 1577 포트에 접근한다.
	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1)
	{
		std::cerr << "server bind failed!" << std::endl;
		return -1;
	}
	std::cout << "Server: Waiting for clinet's connection..." << std::endl;
	if (listen(server_fd, 10) == -1)
	{
				std::cerr << "listen failed!" << std::endl;
	}
	struct sockaddr_in client_addr;
	socklen_t client_addrlen;
	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen);

	(void)client_fd;
	(void)client_addrlen;
	std::cout << "Server: Accepted connection from "
				<< inet_ntoa(client_addr.sin_addr)  // IP 주소 출력
				<< ":" << ntohs(client_addr.sin_port)  // 포트 번호 출력
				<< std::endl;
	char readbuffer[1024] = {};
	while(1)
	{
		int length = recv(client_fd, readbuffer, sizeof(readbuffer), 0);
		// readbuffer[length] = 0;
		std::cout << "-- Client Message --" << std::endl;
		std::cout << readbuffer << std::endl;
		if (strcmp(readbuffer, "/EXIT") == 0)
		{
			send(client_fd, readbuffer, length, 0);
			return (0);
		}
		int flag = 0;
		if (flag == 0)
		{
			std::string respons = ":server.name CAP * LS :multi-prefix sals\r\n";
			send(client_fd, respons.c_str(), respons.size(), 0);
			flag = 1;
		}
	}
	// 1, 서버의 fd, 2. 클라이언트의 주정보 즉 서버 addr과 같음. 3.클라이언트 구조체의 길이.

}
