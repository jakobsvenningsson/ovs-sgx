1. Create private key `openssl genrsa -out rootCA.key 4096`
2. Create CA certificate `openssl req -x509 -new -nodes -key rootCA.key -sha256 -days 1024 -out rootCA.crt \
-subj "/C=SE/ST=STOCKHOLM/O=RISE/CN=CA_SERVER"`
3. make
4. ./CA_server
