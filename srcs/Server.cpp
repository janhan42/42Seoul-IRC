#include "Server.hpp"
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>	// for fcntl()
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "User.hpp"

#define MAX_EVENT 10

Server::Server(int port, const std::string& password)
: port(port)
, passwd(password)
, server_name("sirc")
{
}

Server::~Server()
{
}

int Server::init()
{
	if (SetServerSock()
	||	SetServerAddr()
	||	SetServerBind())
	{
		std::cerr << "Server Init Fail" << std::endl;
		return (EXIT_FAILURE);
	}
	return (0);
}

bool Server::SetServerSock()
{
	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
		return (return_cerr("SetServerSock socket() fail"));

	int opt = 1;
	if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		return (return_cerr("SetServerSock setsockopt() fail"));
	return (EXIT_SUCCESS);
}

bool Server::SetServerAddr()
{
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	return (EXIT_SUCCESS);
}

bool Server::SetServerBind()
{
	if (bind(serverSock, reinterpret_cast<struct sockaddr *>(&server_addr),	sizeof(server_addr)) == -1)
		return (return_cerr("server bind failed!"));
	return (EXIT_SUCCESS);
}

int Server::run()
{
	if (listen(serverSock, SOMAXCONN) == -1)
	{
		return (return_cerr("listen() failed"));
	}
	std::cout << "Server: Waiting for clinet's connection..." << std::endl;
	fcntl(serverSock, F_SETFL, O_NONBLOCK);
	// F_SETFL:		re-set file fd flag with arg
	// O_NONBLOCK:	set fd NONBLOCK

	kq = kqueue();
	if (kq < 0) return (return_cerr("kqueue init failed"));

	struct kevent evSet;  // 이벤트 등록하는데 쓰는 구조체
	EV_SET(&evSet, serverSock, EVFILT_READ, EV_ADD, 0, 0, NULL);
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
		/**
		 * @brief
		 * kq에서 변화사항이 생긴거를 evList 배열에 넣을거고 배열 크기는 MAX_EVENT.
		 * 이벤트가 하나라도 생길때까지 기다린다는(타임아웃이 없다)
		 */
		int event_count = kevent(kq, NULL, 0, evList, MAX_EVENT, NULL);
		if (event_count < 0)
			return (return_cerr("kevent() failed!"));

		for (int i = 0; i < event_count; i++)
		{
			if (static_cast<int>(evList[i].ident) == serverSock)
			{
				// 해당 이벤트의 fd 가 서버 fd면
				// -> 클라이언트 새로 연결된거
				accept_new_client();
				continue;
			}
			if (evList[i].filter & EVFILT_READ)	 // 클라이언트 읽기 가능
				read_client_message(evList[i].ident);
			if (evList[i].filter & EVFILT_WRITE)  // 클라이언트 쓰기 가능
				send_client_message(evList[i].ident);
		}
	}
}
void Server::send_pass_prompt(int client_fd)
{
	std::string pass_msg = ":" + server_name + " 464 :Password incorrect\r\n";
	send(client_fd, pass_msg.c_str(), pass_msg.size(), 0);
}

void Server::send_client_message(int client_fd)
{
	if (send_buffer[client_fd].empty())	 // 버퍼가 비어있으면 넘어감
		return;

	std::string &message = send_buffer[client_fd];
	int bytes_sent = send(client_fd, message.c_str(), message.size(), 0);

	if (bytes_sent < 0)
	{
		std::cerr << "Error send() to client" << std::endl;
		delete_client(client_fd);
		return;
	}

	message.erase(0, bytes_sent);  // 보낸만큼 제거
}

void Server::read_client_message(int client_fd)
{
	char readbuffer[BUFFER_MAX];

	memset(readbuffer, 0, BUFFER_MAX);
	int bytes_read = recv(client_fd, readbuffer, BUFFER_MAX, 0);
	if (bytes_read > 0)
		handle_message(client_fd, readbuffer);
	else if (bytes_read <= 0)  // 클라이언트 연결 종료
		delete_client(client_fd);
}

void Server::delete_client(int client_fd)
{
	struct kevent evSet;

	std::cout << "User [" << user_list_by_fd[client_fd]->nickname
			  << "] disconnected" << std::endl;

	// 이벤트 삭제
	EV_SET(&evSet, client_fd, EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0,
		   NULL);
	kevent(kq, &evSet, 1, NULL, 0, NULL);

	// 유저 리스트에서 해당 클라이언트 삭제
	// read write 둘다 삭제
	user_list_by_nick.erase(user_list_by_fd[client_fd]->nickname);
	delete user_list_by_fd[client_fd];
	user_list_by_fd.erase(client_fd);
	send_buffer.erase(client_fd);
}

void Server::handle_message(int client_fd, std::string message)
{
	buffer_tmp += message; // 요것도 유저별로 하나씩 가지게

	std::vector<std::string> messages = split_message(buffer_tmp);

	if (buffer_tmp.back() != '\n')
	{
		buffer_tmp = messages.back();
		messages.pop_back();
		// 불완전한 메세지는 버퍼에 저장해두고
		// 다음에 처리
	}
	else
		buffer_tmp.clear();	 // 불완전한 메세지 없으면 저장용 버퍼는 초기화

	std::vector<std::string>::iterator it;

	for (it = messages.begin(); it != messages.end(); it++)
	{
		std::cout << *it << std::endl;
		if (it->size() >= 4 && it->substr(0, 4) == "NICK")
		{
			handle_nick(client_fd, *it);
		}
		else if (it->size() >= 4 && it->substr(0, 4) == "USER")
		{
			// handle user
		}
		else if (it->size() >= 4 && it->substr(0, 4) == "JOIN")
		{
			// handle join
		}
		else if (it->size() >= 7 && it->substr(0, 7) == "PRIVMSG")
		{
			// handle privmsg
		}
	}
	client_fd++;
}

void Server::handle_nick(int client_fd, std::string &command)
{
	std::string prev_nick = user_list_by_fd[client_fd]->nickname;
	if (command.size() == 4)  // "NICK" 딸랑 이렇게만 왔을 때
	{
		std::string err_msg =
			":" + server_name + " 431 " + prev_nick + " :No nickname given\r\n";
		// send(client_fd, err_msg.c_str(), err_msg.size(), 0);
		send_buffer[client_fd] += err_msg;
		return;
	}
	std::string name = command.substr(5);
	name.erase(name.find_last_not_of("\r\n") + 1);

	// 닉네임이 중복되면 ERR_NICKNAMEINUSE(433) 보내고 무시하기 -> 처리완료
	// 닉네임에 이상한 글자(#으로 시작, :으로 시작, 스페이스 등)가 오면
	// ERR_ERRONEUSNICKNAME(432) 보내고 명령어 무시
	// 이긴 한데 걍 응답안하고 무시해도 작동하긴 함
	// 닉네임 안에 .!@ 들어오면 무시해야함
	// 키위에서는 닉네임에 스페이스 오면 걍 앞 단어만 보내주네
	if (is_nickname_invalid(name))
	{
		std::string err_msg = ":" + server_name + " 432 " + prev_nick +
							  " :Nickname '" + name + "' is invalid!\r\n";
		// send(client_fd, err_msg.c_str(), err_msg.size(), 0);
		send_buffer[client_fd] += err_msg;
		return;
	}
	if (is_nickname_taken(name))
	{
		std::string err_msg =
			":" + server_name + " 433 " + prev_nick + " " + name + "\r\n";
		// send(client_fd, err_msg.c_str(), err_msg.size(), 0);
		send_buffer[client_fd] += err_msg;
		return;
	}
	user_list_by_fd[client_fd]->set_nickname(name);
	user_list_by_nick.erase(prev_nick);
	user_list_by_nick[name] = user_list_by_fd[client_fd];

	std::string msg = ":" + prev_nick + " NICK " + name + "\r\n";
	// send(client_fd, msg.c_str(), msg.size(), 0);
	send_buffer[client_fd] += msg;
	// 같은 채널에 있는 다른 유저에게도 전달해야함 ㅠ
}

bool Server::is_nickname_invalid(const std::string &name)
// 금지된 문자 있는지 확인
// 처음에는 이름 길이 넘어도 만들어지긴함;;
{
	if (name.empty() || name.size() > 9) return (true);

	if (name.front() == '#' || name.front() == ':') return (true);

	if (name.front() >= '0' && name.front() <= '9') return (true);

	std::string invalid_char = ".!@";
	if (name.find_first_of(invalid_char) != std::string::npos) return (true);

	if (name.find(' ') != std::string::npos) return (true);

	return (false);
}

bool Server::is_nickname_taken(const std::string &name)
{
	return (user_list_by_nick.count(name) > 0);
}

int Server::accept_new_client()
{
	int client_fd = accept(serverSock, NULL, NULL);
	// 클라이언트 주소 정보를 기록할 필요가 딱히 없기 때문에
	// sockadd_in 구조체를 사용하지 않음
	if (client_fd < 0)
	{
		std::cerr << "client accept failed" << std::endl;
		return (0);	 // 클라이언트 연결 실패해도 계속 돌긴 해야될것같음
	}
	std::cout << "new client connected" << std::endl;

	// send_pass_prompt(client_fd);

	User *new_user = new User(client_fd);		// Comment: 굳이 new 로 할당해서 만들어야 하는 이유
	user_list_by_fd[client_fd] = new_user;
	user_list_by_nick[new_user->nickname] = new_user;

	send_welcome_message(*new_user);
	fcntl(client_fd, F_SETFL, O_NONBLOCK);	// 클라이언트도 비동기 처리
	send_buffer[client_fd] = "";  // 이 클라이언트의 send_buffer 생성

	struct kevent evSet;
	EV_SET(&evSet, client_fd, EVFILT_READ | EVFILT_WRITE, EV_ADD, 0, 0,
		   new_user);
	kevent(kq, &evSet, 1, NULL, 0, NULL);
	return (0);
}

void Server::send_welcome_message(User &new_user)
{
	std::string welcome_msg =
		":sirc 001 " + new_user.nickname + " :Welcome to the IRC server!\r\n";
	send(new_user.fd, welcome_msg.c_str(), welcome_msg.size(), 0);
	//  001: 클라이언트가 성공적으로 연결되었음을 알림
	//  002: 서버의 호스트 네임과 버전 정보 알림
	//  003: 서버 생성 날짜와 시간 알림
	//  004: RPL_MYINFO
	//  005: RPL_ISUPPORT
	//  375: MOTD 메세지 시작
	//  376: MOTD 메세지 종료
	//  --> 005 까지는 필수사항
}

int Server::return_cerr(const std::string &err_msg)
{
	std::cerr << err_msg << std::endl;
	return (EXIT_FAILURE);
}


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
