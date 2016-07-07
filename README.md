gcc -o chat_client.c -o chat_client -lpthread
gcc -o chat_sevrer.c -o chat_server -lpthread

1. ./chat_sevrer
2. check server operation
	$ netstat -atp | grep chat_server
3. ./chat_client ip
	ex) $ ./chat_client 127.0.0.1
	      ./chat_client xxx.xxx.xxx.xxx

if you want to exit chatting, input 'bye'
