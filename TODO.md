u# IRC Check List

## ISSUE
- [x] 같은 닉네임이 서버에 접속해 있으면 접속이 안됨 -> 이게 맞는 행동인지 확인후 처리를 해야할듯 -> 맞는거임 프로토콜 픽스 했음
- [x] 기본 닉네임으로 채널에 들어간뒤에 /NICK을 사용한뒤 새로운 유저가 기본닉네임으로 이상 현상으로 접속한뒤 기존 첫번째 유저가 exit한뒤 두번째 유저가 eixt하면 badfree 발생 -> NickCollision 처리 해놨음
- [x] channel에 있다보면 mode b라는 명령어를 오퍼레이터가 아닌 유저가 쏘는데 이유가 뭔지 모르겠음 -> 무시 처리함
- [x] @BOT BuckShot z 를 했을때 zinox가 찾아짐 풀 네임이 아닌데도 찾아짐 - logic 문제가 있음 -> 이름 컬러 시절때 로직 문제였음
- [x] /mode +i 하고 나가면 방이 안없어짐
- [ ] Privmsg Bot부분 리펙토링 예정....
- [ ] Error 프로토콜 대부분이 맞지 않음.... 다시 고치는중
- [ ] Regist 중에 실패했을때 Evlist에서 삭제 및 Regist Fail 로직 변경 예정
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
	- [x] 답변 체크
- [ ] Mode
	- [X] limit 모드 해제 안되는거 -> -l이 무시되는 코드가 있었음
	- [x] mode #test b 들어오는거 ->ignore 처리 해둠
	- [ ] 답변 체크
- [ ] Nick
	- [x] 서버에 접속후 Channel에 접속해있을때 NICK 명령어를 했을떄 #\<channel-name> 으로 바뀌는 상황이 있음.
	- [x] 위의 상황처럼 되면 #test에서 /Part를 하게 되면 이전 닉네임은 나가지고 실직적으로 나가지지 않음
	- [ ] 답변 체크
- [ ] Part
	- [x] 채널에 두명이 있었는데 한명이 나가고 한명만 남았다면 그 사람에게 오퍼레이터를 줘야하는게 맞지 않을까 싶은데 -> 응안줌
	- [ ] 답변 체크
- [ ] Pass
	- [ ] 답변 체크
	- [ ]
- [ ] Ping
	- [ ] 답변 체크
	- [ ]
- [ ] Privmsg
	- [ ] 답변 체크
	- [ ]
- [ ] Quit
	- [ ] 답변 체크
	- [ ]
- [ ] Topic
	- [ ] 답변 체크
	- [ ]
- [ ] User
	- [ ] 답변 체크
	- [ ]
___
## Bot

