#include "Server.hpp"
#include <fcntl.h>	// for fcntl()
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

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
		return (return_cerr("listen() failed"));
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
			if (evList[i].ident == server_fd)  // 해당 이벤트의 fd 가 서버 fd면
											   // -> 클라이언트 새로 연결된거
			{
				int client_fd = accept(server_fd, NULL, NULL);
				// 클라이언트 주소 정보를 기록할 필요가 딱히 없기 때문에
				// sockadd_in 구조체를 사용하지 않음
				if (client_fd < 0)
				{
					std::cerr << "client accept failed" << std::endl;
					continue;
				}

				std::cout << "new client connected" << std::endl;
				user_list.push_back(new User(client_fd));
			}
		}
	}
}

int return_cerr(const std::string &err_msg)
{
	std::cerr << err_msg << std::endl;
	return (EXIT_FAILURE);
}
