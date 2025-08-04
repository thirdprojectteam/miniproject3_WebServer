# MySQL SSL 설정하는 방법

1. openssl 설치 

- 윈도우 환경이라면 openssl을 설치합시다 (https://chris1108.tistory.com/36) 

2. Qt 코드 추가 

m_db.setConnectOptions(
    "SSL_KEY=C:/MySQL/certs/client-key.pem;"
    "SSL_CERT=C:/MySQL/certs/client-cert.pem;"
    "SSL_CA=C:/MySQL/certs/ca.pem;"
);

3. Qt 환경설정

- CMD를 열고!!

- "set SSL_CERT_FILE=C:\MySQL\certs\ca.pem" 해준다.

4. SSL 클라이언트 키 생성

-  CA (자체 서명)
- openssl genrsa -out ca-key.pem 2048
- openssl req -new -x509 -days 365 -key ca-key.pem -out ca.pem -subj "/CN=MySQL Test CA"

-  클라이언트 키 + 요청
- openssl genrsa -out client-key.pem 2048
- openssl req -new -key client-key.pem -out client-req.pem -subj "/CN=MySQL Client"

-  클라이언트 인증서 발급
- openssl x509 -req -in client-req.pem -days 365 -CA ca.pem -CAkey ca-key.pem -set_serial 01 -out client-cert.pem

- cd C:\Program Files\MySQL\MySQL Server 8.0\bin
- mysql_ssl_rsa_setup --datadir=C:\MySQL\certs --uid=mysql

- !!!!!!!!!!주의 : 경로는 생성해야합니다. 자동으로 만들어 지는게 아닙니다. C:\MySQL\certs 만든겁니다.

5. SSL 서버 키 생성

- C:\MySQL\certs (클라이언트 키가 있는 경로로 가서)

- openssl genrsa -out server-key.pem 2048
- openssl req -new -key server-key.pem -out server-req.pem -subj "/CN=MySQL_Server"
- openssl x509 -req -in server-req.pem -days 365 -CA ca.pem -CAkey ca-key.pem -set_serial 02 -out server-cert.pem

6. my.ini 파일 수정 (C:\ProgramData\MySQL\MySQL Server 8.0)

[mysqld]
ssl-ca = C:/MySQL/certs/ca.pem
ssl-cert = C:/MySQL/certs/server-cert.pem
ssl-key = C:/MySQL/certs/server-key.pem
require_secure_transport = ON



