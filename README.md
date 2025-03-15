# Inception 프로젝트 - README

## 📌 프로젝트 개요
**Inception** 프로젝트는 Docker를 활용하여 컨테이너 기반의 **멀티 서비스 인프라**를 구축하는 과제입니다. 이를 통해 가상화 및 컨테이너 기술에 대한 이해를 높이고, 시스템 설계 및 배포 자동화 경험을 쌓는 것을 목표로 합니다.

## 🛠️ 기술 스택
- **Docker & Docker Compose**
- **Nginx** (Reverse Proxy, Load Balancing)
- **WordPress** (CMS)
- **MariaDB** (Database)
- **PHP-FPM** (PHP 실행 환경)
- **Debian**

## 🎯 요구사항
1. **Docker Compose를 활용하여 인프라 구성**
   - 각각의 서비스(웹 서버, 데이터베이스, 애플리케이션 서버 등)를 별도의 컨테이너로 배포

2. **보안 강화**
   - 각 컨테이너는 비루트(Non-root) 사용자로 실행
   - SSL을 적용하여 HTTPS 통신 지원

3. **데이터 지속성 (Persistence)**
   - MariaDB, WordPress 등의 데이터를 유지하기 위해 **볼륨(Volumes)** 사용

4. **네트워크 분리**
   - 외부 접근이 필요한 서비스(Nginx)와 내부 서비스(MariaDB, WordPress 등)를 분리하여 네트워크 보안 강화

## 📂 프로젝트 구조
```
📦 inception
 ┣ 📂 srcs                # 소스 코드 및 설정 파일
 ┃ ┣ 📂 nginx             # Nginx 설정
 ┃ ┣ 📂 wordpress         # WordPress 설정
 ┃ ┣ 📂 mariadb           # MariaDB 설정
 ┃ ┣ 📜 docker-compose.yml  # Docker Compose 구성 파일
 ┃ ┣ 📜 .env                # 환경 변수 파일
 ┣ 📜 Makefile            # Makefile
 ┣ 📜 README.md           # 프로젝트 설명서
```

## 🚀 실행 방법
### 1. 프로젝트 클론
```bash
git clone
cd inception
```

### 2. Docker Compose 빌드 및 실행
```bash
make
```

### 3. 컨테이너 상태 확인
```bash
docker ps
```

### 4. 서비스 접속
- **WordPress 접속:** `https://127.0.0.1.com`

## 🧹 정리 및 종료
```bash
make fclean
```

## 🔍 주요 기능
- Nginx를 통한 Reverse Proxy 설정 및 SSL 지원
- WordPress 설치 및 데이터 지속성을 위한 MariaDB 연동
- Redis를 활용한 성능 최적화
- Docker Compose를 활용한 손쉬운 서비스 배포
