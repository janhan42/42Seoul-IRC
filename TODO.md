# IRC Check List

# CRASH!!!!!

- [x] /ping hello 하면 터지지는 않는데 pong이 안감
	- Recv Send: PRIVMSG hello :PING 1731785308 736278 이런식으로 privmsg로 들어와서 privmsg처럼 처리함
- [x] 근데 /ping fuck 하면 세그폴트남
- [x] /msg janhan hi 해도 터짐
- [x] 밑에 적힌거처럼 구현은 안해도 무시하게는 해야될거같은데
- [x] 고침 같은 user변수가 두개라서 터지는거였음
- [x] 이제 /msg로 1대1대화도 잘됨

## ISSUE
- [x] 같은 닉네임이 서버에 접속해 있으면 접속이 안됨 -> 이게 맞는 행동인지 확인후 처리를 해야할듯 -> 맞는거임 프로토콜 픽스 했음
- [x] 기본 닉네임으로 채널에 들어간뒤에 /NICK을 사용한뒤 새로운 유저가 기본닉네임으로 이상 현상으로 접속한뒤 기존 첫번째 유저가 exit한뒤 두번째 유저가 eixt하면 badfree 발생 -> NickCollision 처리 해놨음
- [x] channel에 있다보면 mode b라는 명령어를 오퍼레이터가 아닌 유저가 쏘는데 이유가 뭔지 모르겠음 -> 무시 처리함
- [x] @BOT BuckShot z 를 했을때 zinox가 찾아짐 풀 네임이 아닌데도 찾아짐 - logic 문제가 있음 -> 이름 컬러 시절때 로직 문제였음
- [x] /mode +i 하고 나가면 방이 안없어짐
- [x] Privmsg Bot부분 리펙토링 예정....-> 시작
- [x] Error 프로토콜 대부분이 맞지 않음.... 다시 고치는중
- [x] Regist 중에 실패했을때 Evlist에서 삭제 및 Regist Fail 로직 변경 예정
## Command
- [ ] Invite
	- [x] 서버에 없는 사람을 초대한 경우 -> NoSuchNick
	- [x] 채널 #test에 들어가있는 OPUser가 메인창에서 /invite <\user> <\channel-name>을 했을경우
	- [x] 답변 체크
- [ ] Join
	- [x] 답변 체크
- [ ] Kick
	- [x] 자기 자신 킥 했을때 채팅방이 남아 있는 현상 있음 -> TEST /mode +i 한뒤 자기 자신 Kick
		<br>-> 해결 방안1 : ChannelManager 를 만들어서 루프당 인원 체크뒤 close
		<br>-> 해결 방안2 : Kick 부분에서 체크후 Close 처리
	- [x] 채널에 없는 유저 킥 했을때 NoSuchNick 프로토콜 발생 -> 처리함
	- [x] 예상한 메세지 처럼 안오고 irssi 클라이언트 자체에서 개별 KICK으로 들어오기 때문에 로직 변경 요망
	- [x] /kick janhan 님 그거 규칙 위반임 -> 메세지가 [님]만 나옴
	- [x] 답변 체크
- [ ] Mode
	- [X] limit 모드 해제 안되는거 -> -l이 무시되는 코드가 있었음
	- [x] mode #test b 들어오는거 ->ignore 처리 해둠
	- [x] 서버에 janhan이 없을때 /mode janhan +b를 한다 했을때 ERR_NOSUCHNICK (401)일이 떠야하고
		  서버에 janhna이 있는대 zinox가 /mode janhan +b를 하면 ERR_USERSDONTMATCH (502) 가 떠야함 -> 구현 요구사항 모드들 중에서 걸릴 에러가 없음
	- [x] user op모드가 안빠짐 채널 모드만 신경써서 해서 다시 만들어야 할듯 -> /mode #test +o janhan 이 문법임
	- [x] 답변 체크
- [ ] Nick
	- [x] 서버에 접속후 Channel에 접속해있을때 NICK 명령어를 했을떄 #\<channel-name> 으로 바뀌는 상황이 있음.
	- [x] 위의 상황처럼 되면 #test에서 /Part를 하게 되면 이전 닉네임은 나가지고 실적으로 나가지지 않음
	- [x] 답변 체크
- [ ] Part
	- [x] 채널에 두명이 있었는데 한명이 나가고 한명만 남았다면 그 사람에게 오퍼레이터를 줘야하는게 맞지 않을까 싶은데 -> 응안줌
	- [x] 답변 체크
- [ ] Pass
	- [x] 답변 체크
	- [ ]
- [ ] Ping
	- [x]	/PING janhan 을 했을떄 CTCP PING으로 처리 되는데 requsted는 뜨는데 답변이 안가고있음
			근데 이게 irssi 클라이언트 PING과 유저 입력 PING의 서버로 들어오는 메세지가 다름 유저 PING 은 PRIVMSG로 들어옴 -> PING 구현 사항이 아니기 때문에 클라이언트 에서 서버로 쏘는 token Ping만 처리
	- [x] 답변 체크
- [ ] Privmsg
	- [x] 없는 유저에게 채팅을 보냈을때 채팅방이 생기면서 오류메세지가 발생 -> 정상임 Irssi 클라이언트에서 그렇게 처리함
	- [x] /away에 대한 처리 를 해야 하나 모르겠음 -> 구현 사항에 없었기 때문에 안함
	- [x] 답변 체크
- [ ] Quit
	- [ ]
	- [x] 답변 체크 -> 답변하는게 없음
- [ ] Topic
	- [ ]
	- [x] 답변 체크
- [ ] User
	- [ ]
	- [x] 답변 체크
___
## Bot

## File trnasfer

file send:
/dcc send <\nick name> <\file addres>

file get:
/dcc get -> request file send accept

## Refactor

- [x] Server 클래스에 FindUser 함수 두개 만들어서 필요한 부분 전부 교체함

- [x] 읽기 어려운 iterator 읽기 쉽게 변경중

- [x] istringstream 쓰는 부분 split함수 로 바꾸기 -> 그냥 둡시다

- [x] 이제 bot 클래스가 생겼으니까 fd -1인 유저는 안넣어도 되는거 아닌가?
	-> 채널 접속시나 names 했을 때 유저 리스트 보내줄 때 봇 안넣어도 되면
		삭제해도 될 것 같음
	-> 봇이 채널에 유저 처럼 있는 게 맞는것 같아서 그냥 두겠습니다

- [x] strcmp 랑 string 비교연산 섞여있는 부분 stirng 비교연산으로 통일함
	- Pass.cpp, Privmsg.cpp

- [x] @bot buckshot 하고 뒤에 이름 안적었을때 일단 메세지 가게 해놓았음

- [x] Command::ChannelPART 함수 사용안해서 주석처리함

- [x] 서버에서 init 을 따로 분리한건 생성자 실패했는지를 execption으로 밖에 확인하지 못해서 빼놓은건데, execption 쓸거면 메인에서 호출 안한고 생성자에 넣어도 될듯
	- init이 실패하면 catch로 가서 프로그램 종료됨

- [x] 서버 사용하지 않는 멤버변수, 멤버함수 정리
	- [x] GetKqFd 함수 사용하지 않음
	- [x] mbrunning변수 필요없음
	- [x] mEventCount 변수 굳이 멤버로 둘 필요 없을 것 같음
	- [x] mUserSock 변수 accept user 함수 내부에서만 쓰기 때문에 멤버로 둘 필요 없음
	- [x] mUserAddr이랑 mUserAddrLen 역시 일회용처럼 쓰이기 때문에 멤버변수로 둘 필요 없음
	- [x] mServerEvent도 굳이 멤버로 둘 필요 없어보임

- [x] RecvMessage함수 한글자씩 읽는거 너무 느리지 않을까 싶어서 새로 만들어 볼까 했는데 다 뜯어고쳐야되네... 일단 보류

- [x] mMessage 벡터가 1024개나 가지고 있길래 max_user 만들고 100으로 함
	- 유저fd는 5부터 생기니까 max_user+5로함

- [x] 442에러 체크 부분 IsInChannel 함수로 변경 가능한거 변경

- [x] User 클래스 정리
	- mIsCapEnd 멤버변수 안씀 -> 지우는게 맞음
	- mRealName 처음 등록할때 받아오긴하는데 그뒤로 안씀 -> 안쓴다고 빼면 안됨
	- realname 이 필요한 명령어는 구현 대상이 아니긴함


