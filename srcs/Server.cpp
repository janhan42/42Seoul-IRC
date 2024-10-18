#include "Server.hpp"
#include <fcntl.h>	// for fcntl()
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "User.hpp"

#define MAX_EVENT 10

Server::Server(int port, std::string passwd) :
	port(port), passwd(passwd), server_name("sirc")
{
}

int Server::init()
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) return (return_cerr("socket() failed!"));

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		return (return_cerr("setsockopt"));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
			 sizeof(server_addr)) == -1)
		return (return_cerr("server bind failed!"));

	return (0);
}

int Server::run()
{
	if (listen(server_fd, SOMAXCONN) == -1)
	{
		return (return_cerr("listen() failed"));
	}
	std::cout << "Server: Waiting for clinet's connection..." << std::endl;
	fcntl(server_fd, F_SETFL, O_NONBLOCK);
	// F_SETFL:		re-set file fd flag with arg
	// O_NONBLOCK:	set fd NONBLOCK

	int kq = kqueue();
	if (kq < 0) return (return_cerr("kqueue init failed"));

	struct kevent evSet;  // 이벤트 등록하는데 쓰는 구조체
	EV_SET(&evSet, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	// 	첫 번째 인자: 이벤트 구조체(evSet)의 포인터
	// ident: 감시할 소켓, 파일 디스크립터, 시그널 번호 등
	// filter: 감지할 이벤트 종류 (예: 읽기 이벤트, 쓰기 이벤트)
	// flags: 이벤트의 동작을 지정하는 플래그 (예: EV_ADD, EV_DELETE)
	// fflags: 필터별로 사용할 수 있는 추가 정보
	// data: 이벤트와 관련된 데이터 (예: 읽을 수 있는 바이트 수)
	// udata: 사용자 정의 포인터(예: 사용자 객체)
	//
	// 서버는 이벤트 발생시 클라이언트 소켓을 만들기만 하면 되고 추가적인
	// 상태는 추적할 필요가 없기 때문에 udata 필드에 null 전달

	kevent(kq, &evSet, 1, NULL, 0, NULL);
	// kq에다가 1가지의 변경사항을 등록할거인데 저장할 배열 따로 없고 배열
	// 크기도 당연히 0이고 이벤트 등록 될때까지는 대기할거라는 뜻

	while (true)
	{
		struct kevent evList[MAX_EVENT];
		int event_count = kevent(kq, NULL, 0, evList, MAX_EVENT, NULL);
		// kq에서 변화사항이 생긴거를 evList 배열에 넣을거고 배열 크기는
		// MAX_EVENT. 이벤트가 하나라도 생길때까지 기다린다는(타임아웃이 없다)

		if (event_count < 0) return (return_cerr("kevent() failed!"));

		for (int i = 0; i < event_count; i++)
		{
			if (static_cast<int>(evList[i].ident) == server_fd)
				// 해당 이벤트의 fd 가 서버 fd면
				// -> 클라이언트 새로 연결된거
				accept_new_client(kq, &evSet);
			else if (evList[i].filter == EVFILT_READ)
			{
				read_client_message(evList[i].ident);
			}
		}
	}
}

void Server::read_client_message(int client_fd)
{
	char readbuffer[1024];

	memset(readbuffer, 0, BUFFER_MAX);
	int bytes_read = recv(client_fd, readbuffer, BUFFER_MAX, 0);
	if (bytes_read > 0)
	{
		handle_message(client_fd, readbuffer);
	}
}

void Server::handle_message(int client_fd, std::string message)
{
	std::cout << "MY NAME: " << user_list[client_fd]->nickname << std::endl;
	buffer += message;

	std::vector<std::string> messages = split_message(buffer);

	if (buffer.back() != '\n')
	{
		buffer = messages.back();
		messages.pop_back();
		// 불완전한 메세지는 버퍼에 저장해두고
		// 다음에 처리
	}
	else
		buffer.clear();	 // 불완전한 메세지 없으면 저장용 버퍼는 초기화

	std::vector<std::string>::iterator it;

	for (it = messages.begin(); it != messages.end(); it++)
	{
		if (it->size() >= 4 && it->substr(0, 4) == "NICK")
		{
			handle_nick(client_fd, *it);
			if (it->size() == 4)
			{
				// NICK 뒤에 파라미터 안온경우ERR_NONICKNAMEGIVEN(431)
			}
			std::string name = it->substr(5);
			name.erase(name.find_last_not_of("\r\n") + 1);
			user_list[client_fd]->set_nickname(name);
		}
		else if (it->size() >= 4 && it->substr(0, 4) == "USER")
			std::cout << *it << std::endl;
		else if (it->size() >= 4 && it->substr(0, 4) == "JOIN")
			std::cout << *it << std::endl;
		else if (it->size() >= 7 && it->substr(0, 7) == "PRIVMSG")
			std::cout << *it << std::endl;
	}
	client_fd++;
}

void Server::handle_nick(int client_fd, std::string &command)
{
	if (command.size() == 4)  // "NICK" 딸랑 이렇게만 왔을 때
	{
		std::string err_msg = ":" + server_name + " 431 " +
							  user_list[client_fd]->nickname +
							  " :No nickname given\r\n";
	}
}

int Server::accept_new_client(int kq, struct kevent *evSet)
// 함수 내부에서 kevent 에 클라이언트 추가하기 위해
// 매개변수로 kq랑 evSet 주소 가져옴
{
	int client_fd = accept(server_fd, NULL, NULL);
	// 클라이언트 주소 정보를 기록할 필요가 딱히 없기 때문에
	// sockadd_in 구조체를 사용하지 않음
	if (client_fd < 0)
	{
		std::cerr << "client accept failed" << std::endl;
		return (0);	 // 클라이언트 연결 실패해도 계속 돌긴 해야될것같음
	}
	std::cout << "new client connected" << std::endl;
	User *new_user = new User(client_fd);
	user_list[client_fd] = new_user;
	send_welcome_message(*new_user);

	EV_SET(evSet, client_fd, EVFILT_READ, EV_ADD, 0, 0, new_user);
	kevent(kq, evSet, 1, NULL, 0, NULL);
	return (0);
}

void Server::send_welcome_message(User &new_user)
{
	std::string welcome_msg =
		":sirc 001 " + new_user.nickname + " :Welcome to the IRC server!\r\n";
	send(new_user.fd, welcome_msg.c_str(), welcome_msg.size(), 0);
	// 001: 클라이언트가 성공적으로 연결되었음을 알림
	// 002: 서버의 호스트 네임과 버전 정보 알림
	// 003: 서버 생성 날짜와 시간 알림
	// 375: MOTD 메세지 시작
	// 376: MOTD 메세지 종료
	// --> 001 말고는 선택사항
}

int Server::return_cerr(const std::string &err_msg)
{
	std::cerr << err_msg << std::endl;
	return (EXIT_FAILURE);
}

Server::~Server() {}

std::vector<std::string> Server::split_message(std::string buffer)
// \r\n으로 쭉 연결된 메세지를 스플릿 때려줌
{
	std::vector<std::string> tmp;
	std::istringstream		 stream(buffer);
	std::string				 line;

	while (std::getline(stream, line))
	{
		if (line.size() >= 2 && line.substr(line.size() - 2) == "\r\n")
			line = line.substr(0, line.size() - 2);
		else if (line.size() >= 1 && line.substr(line.size() - 1) == "\r")
			line = line.substr(0, line.size() - 1);
		tmp.push_back(line);
	}
	return (tmp);
}
