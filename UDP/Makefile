compile_server: server.c
	gcc server.c -o server

compile_client: client.c
	gcc client.c -o client

run_server: compile_server
	./server

run_client : compile_client
	./client

compile: compile_server compile_client