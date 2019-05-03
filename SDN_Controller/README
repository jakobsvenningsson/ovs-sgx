1. Install RYU SDN Controller.

2. Create certificate key `openssl genrsa -out sdn.key 2048`

3. Create Certificate Request `openssl req -new -sha256 -key sdn.key -subj "/C=SE/ST=STOCKHOLM/O=RISE/CN=SDN_CONTROLLER" -out sdn.csr`

3. Sign the Certificate Request `openssl x509 -req -in sdn.csr -CA ../CA_server/rootCA.crt -CAkey ../CA_server/rootCA.key -CAcreateserial -out sdn.crt -days 500 -sha256`

4. Copy rootCA.crt to this directory `cp ../CA_server/rootCA.crt .`

5. Use the Makefile to start the desired SDN controller program
