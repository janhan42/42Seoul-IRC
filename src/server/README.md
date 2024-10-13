# server
IRC 개체에서 가지고 있을 서버 클래스를 정의한다.

서버 클래스에는 port, pass 및 sockaddr_in의 정보를 가지고 있고
run()함수에서 각 이벤트를 처리할 kqueue를 가지고 있는다.
