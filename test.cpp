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

// TODO: 서버 Core Class 작성
// TODO: 접속한 유저의 데이터 및 정보를 저장해둘 User Class 작성


// TODO: <나중>
// 채널 클래스 (패스워드, 속해있는 유저, invite only인지 등)

int main(int ac, char **av)
{
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
	server_addr.sin_port = htons(6667); // 호스트 바이트 순서 -> 네트워크 바이트 순서
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
	int flag = 0;
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
		// CAP LS 응답을 한 번만 보내기
		if (flag == 0) {
			std::string response = ":server_name CAP * LS\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			flag = 1;  // CAP 응답 이후 플래그 설정
		}

		// 이후 프로토콜 처리 추가 (예: NICK, USER 명령 처리)
		// 예: 클라이언트에서 NICK 명령을 받았을 때 처리
		if (strncmp(readbuffer, "CAP REQ", 7) == 0)
		{
			std::string reqResponse = ":server_name CAP * ACK :multi-prefix\r\n";
			send(client_fd, reqResponse.c_str(), reqResponse.size(), 0);
		}

		if (strncmp(readbuffer, "NICK", 4) == 0) {
			std::string nickResponse = ":server_name 001 Welcome to the IRC server\r\n";
			send(client_fd, nickResponse.c_str(), nickResponse.size(), 0);
		}

		// USER 명령 처리 예시
		if (strncmp(readbuffer, "USER", 4) == 0) {
			std::string userResponse = ":server_name 002 User registered\r\n";
			send(client_fd, userResponse.c_str(), userResponse.size(), 0);
		}

		if (strstr(readbuffer, "CAP END")) {
			std::string response = ":server_name 001 zinox :Welcome to the IRC server\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			response = ":server_name 002 zinox :Your host is server_name, running version 1.0\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			// MOTD 전송
			response = ":server_name 375 zinox :- server_name Message of the Day -\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			response = ":server_name 376 zinox :End of /MOTD command\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
		}
    }
}
	// 1, 서버의 fd, 2. 클라이언트의 주정보 즉 서버 addr과 같음. 3.클라이언트 구조체의 길이.
