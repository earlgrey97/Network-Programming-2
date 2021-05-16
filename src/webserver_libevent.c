#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <event2/event.h>
#include <fcntl.h>

#define MAXBUFLEN 65535
#define BACKLOG 100

void ev_redis_callback(int sock, short which, void* data);
void accept_callback(int sock, short which, void* arg);

struct Multi_arg{
	int mysock_fd;//my_sockfd
	//int cli_fd;//cli_sockfd
	char* argv_str_2; //argv[2]
	char* argv_str_3; //argv[3]
	struct sockaddr_in pass_cli_addr;//
	struct event_base* one_base;
	//pthread_t my_pthread;
};

struct ev_redis_args{
	//int mysock_fd;
	int ser_sockfd;
	int cli_sockfd;
	int error_flag;
	int req_flag;
	//struct Multi_arg* multi_ptr;
	//struct event_base* one_base;
};

void ev_redis_callback(int sock, short which, void* data){
	struct ev_redis_args* args_ptr = (struct ev_redis_args*)data;
	int ser_sockfd_1 = args_ptr->ser_sockfd;
       	int cli_sockfd = args_ptr->cli_sockfd;
	int error_flag = args_ptr->error_flag;
	int req_flag = args_ptr->req_flag;
	//int my_sockfd = args_ptr->mysock_fd;
	//struct event_base* base = args_ptr->one_base;
	//struct Multi_arg* multiple_arg = args_ptr->multi_ptr;
	//struct event* ev_accept;
	//---------------------------------------------------------
	char redis_reply[MAXBUFLEN];
	char final_redis_reply[MAXBUFLEN];
	//---------------------------------------------------------
	char* found_str;
	char reply_to_cli[MAXBUFLEN];
	//---------------------------------------------------------
	//printf("redis callback function start!---------------------\n");
	// cli // receive reply from redis server, 4: read
	if(error_flag == 0){
		//printf("get redis reply\n");
		memset(redis_reply, 0, sizeof(redis_reply));
		memset(final_redis_reply, 0, sizeof(final_redis_reply));
		sleep(0.1);///
		while (recv(ser_sockfd_1, redis_reply, MAXBUFLEN, MSG_DONTWAIT) != -1){
			//printf("reading ...\n");
			//printf("redis_reply: %s\n",redis_reply);
			strcat(final_redis_reply, redis_reply);
			//sleep(0.1);
			//printf("final_redis_reply: %s\n",final_redis_reply);
		};
		//printf("came out!\n");
		//printf("\n\nfinal redis reply: %s\n\n", final_redis_reply);			
	}	
	// cli // deliver this reply to client, 5: write
	//----------------------------------------------------------
	//    make reply format to send to client
	//----------------------------------------------------------
	memset(reply_to_cli, 0, MAXBUFLEN);
	if(req_flag == 1){ // SET case
		//printf("-----SET CASE-----\n");
		if(error_flag == 1){
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");		
		} 
		else if(strstr(final_redis_reply, "OK") != NULL){ // success
			 strcpy(reply_to_cli, "HTTP/1.1 200 OK\r\nContent-Type: test/plain\r\nContent-Length: 2\r\n\r\nOK");
		 }
		 else{ // error
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
		 }
	}
	else if(req_flag == 2){ // GET case
		//printf("-----GET CASE-----, reply to client\n");
		if(strstr(final_redis_reply, "$-1") != NULL){
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
				}
		else if(strstr(final_redis_reply, "$") != NULL){ // success
			// get string from redis_reply
		 	found_str = strstr(redis_reply, "\n");
			found_str++;
			found_str = strtok(found_str, "\r");
			//printf("\n\nfound str: %zu\n\n",strlen(found_str));///
			sprintf(reply_to_cli, "HTTP/1.1 200 OK\r\nContent-Type: test/plain\r\nContent-Length: %zu\r\n\r\n%s", strlen(found_str), found_str);
		 }
		 else{ // error
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
		 }
	}
	else{ // Not SET or GET case
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
				 }
	//----------------------------------------------------------
	//    send reply to client
	//----------------------------------------------------------
	//printf("reply to cli: %s\n", reply_to_cli);///
	while(write(cli_sockfd, reply_to_cli, sizeof(reply_to_cli)) == -1) ;
	//ev_accept = event_new(base, my_sockfd, EV_READ|EV_PERSIST, accept_callback, (struct Multi_arg*)multiple_arg);
	//event_add(ev_accept, NULL);	
	//printf("redis event done!!------------------\n");
	close(cli_sockfd);
	close(ser_sockfd_1);
	//event_free(event_self_cbarg());
	//printf("closed ser_sockfd!!------------------\n");
}

void accept_callback(int sock, short which, void* arg){
	//----------------------------------------------------------
	//   libevent
	//----------------------------------------------------------
	//printf("accept callback start!===========================\n");
	struct Multi_arg* multi_ptr = (struct Multi_arg*)arg;
	struct event_base* base = multi_ptr->one_base;
	int cli_sockfd, cli_len;
	int my_sockfd = multi_ptr->mysock_fd;
	struct sockaddr_in cli_addr = multi_ptr->pass_cli_addr;//
	struct event* ev_cli_1;
	struct event* ev_cli_2;
	struct event* ev_redis;	
	
	struct ev_redis_args* redi_call_args = (struct ev_redis_args*)malloc(sizeof(struct ev_redis_args));
	//---------------create thread-----------------------------
	//multiple_arg->my_pthread = pthread[thread_num];
	//pthread_create(&pthread[thread_num], NULL, t_func, (void*)(multiple_arg));
	//thread_num++;
	// parent process
	//int cli_sockfd = multi_ptr->cli_fd;
	//pthread_t thread_self = multi_ptr->my_pthread;
	//int cli_sockfd = multi_ptr->cli_fd;
	//----------------------------------------------------------
	//----------------------------------------------------------
	char req_from_cli[MAXBUFLEN];
	int req_len_cli;
	char *method, *url, *ver, *body;
	char* str_helper;
	char* sub_helper;
	char* redis_req = (char*)malloc(sizeof(char)*MAXBUFLEN);
	char *set_helper, *get_helper;
	//int word_count;
	char word_buf[MAXBUFLEN];
	int req_flag; // 1: post, 2: get
	//char reply_to_cli[MAXBUFLEN];
	//----------------------------------------------------------
	char key_buf[MAXBUFLEN];
	char value_buf[MAXBUFLEN];
	char *key_helper, *value_helper;
	int end_flag = 0;
	//int full_flag = 0;
	int start_flag = 0;
	int pkt_end_flag = 0;
	int key_flag = 1;
	int still_working = 0;
	int error_flag = 0;
	int key_cnt, value_cnt;
	int found_eq_sign = 0;
	char* content_len_str;
	char content_len_buf[100]; 
	int i = 0;
	int whole_size;
	int read_left;
	//int reply_cnt;
	//----------------------------------------------------------
	int ser_sockfd;
	struct sockaddr_in ser_addr;
	struct hostent* ser_ip;
	int ser_port;
	//char redis_reply[MAXBUFLEN];
	//char final_redis_reply[MAXBUFLEN];
	//char* found_str;

	//printf("now, execute accept callback!\n");
	//----------------------------------------------------------
	//    work as a client (to send req to redis)
	//----------------------------------------------------------
	//printf("cli: socket, connect\n");
	// accept client
	cli_len = sizeof(cli_addr);
	cli_sockfd = accept(my_sockfd, (struct sockaddr*)&cli_addr, &cli_len);
	if(cli_sockfd < 0){
		fprintf(stderr, "ser / accept: failed\n");
		exit(1);
	}
	//fcntl(cli_sockfd, F_SETFL, O_NONBLOCK);
	//close(my_sockfd);
	
	// cli // 1 : socket
	//printf("cli / socket\n");	
	ser_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(ser_sockfd < 0){
		fprintf(stderr, "cli / socket: failed\n");
		exit(1);
	}
	//fcntl(ser_sockfd, F_SETFL, O_NONBLOCK);
	// cli // 2: connect
	//printf("ser_sockfd: %d\n",ser_sockfd);
	ser_ip = gethostbyname(multi_ptr->argv_str_2);
	ser_port = atoi(multi_ptr->argv_str_3);
	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(ser_port);
	ser_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)ser_ip->h_addr));
					
	if(connect(ser_sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) < 0){
			fprintf(stderr, "cli / connect: failed\n");
			exit(1);
	}
	// exit_flag : whether there's more packet to receive
	// end flag : until req meets null character
	while((end_flag == 0) && (error_flag == 0)){
		//read_loop_count++;///erase this
		
		error_flag = 0;
		//printf("read from client loop\n");
		//----------------------------------------------------------
		//	if it's the very first request packet
		//----------------------------------------------------------	
		if(start_flag == 0){
			//printf("very first\n");///
			//ev_cli_1 = event_new(base, cli_sockfd, EV_READ|EV_PERSIST, cli_read_callback, (void*)(multiple_arg));
			//event_add(ev_cli_1, NULL);
			//------read from cli callback---------------
			//printf("here!\n");
			while((req_len_cli = recv(cli_sockfd, req_from_cli, MAXBUFLEN, 0)) == - 1) ;
			//printf("here!\n");
			req_from_cli[req_len_cli] = '\0';
			pkt_end_flag == 0;
			
			//printf("req_from_cli: %s\n",req_from_cli);
			//----------------------------------------------------------
			//	parse method and body	
			//----------------------------------------------------------
			if(req_from_cli[0] == 'P') req_flag = 1; // POST
			else req_flag = 2; // GET
					
			if(req_flag == 1){
				content_len_str = strstr(req_from_cli, "ength") + 7;
				//printf("!!!!!\n");
				//printf("content_len_str: %s\n", content_len_str);
				i = 0;
				while(1){
					if((*content_len_str) == '\r') break;
					content_len_buf[i] = (*content_len_str);
					content_len_str++;
					i++;
				}
				content_len_buf[i] = '\0';
				//printf("content_len_buf: %s\n",content_len_buf);
				// get whole content size
				whole_size = atoi(content_len_buf);
				body = strstr(req_from_cli, "\r\n\r\n") + 4;///
				read_left = whole_size - strlen(body);
				//printf("whole size: %d body: %zu\n", whole_size,strlen(body));
			}
		}
		//----------------------------------------------------------
		//	more reading from client
		//----------------------------------------------------------	
		if(pkt_end_flag == 1){
			//ev_cli_2 = event_new(base, my_sockfd, EV_READ|EV_PERSIST, accept_callback, (void*)(multiple_arg));
			//event_add(ev_cli_2, NULL);
			while((req_len_cli = recv(cli_sockfd, req_from_cli, MAXBUFLEN, 0)) == -1) ;
			req_from_cli[req_len_cli] = '\0';
			pkt_end_flag = 0;

			read_left = read_left - strlen(req_from_cli);
		}
				
		//printf("\nread_left: %d\n", read_left);
				
		/////////////////////////////////////////////////////////
		// POST case (SET) //////////////////////////////////////
		/////////////////////////////////////////////////////////
		if(req_flag == 1){ // POST
			//printf("set case\n");///
			//printf("body: %s\n", body);
			//----------------------------------------------------------
			//	save key
			//----------------------------------------------------------
			if(key_flag == 1){
				//printf("save key\n");
				if(start_flag == 0){
					// parse body
					body = strstr(req_from_cli, "\r\n\r\n") + 4;///
					sub_helper = body;
					start_flag = 1; // means it's not very first loop anymore
				}
				if(still_working == 0){
					key_helper = key_buf;
					memset(key_buf, 0, MAXBUFLEN);
				}
				else{//still_working == 1
					sub_helper = req_from_cli;
				}
						
				//word_count = 1;
				if(still_working == 0) key_cnt = 0;
				found_eq_sign = 0;
						
				still_working = 1;
				while(1){
					if((*sub_helper) == '='){
						key_flag = 0;
						still_working = 0;
						sub_helper++;
						key_helper++;
						found_eq_sign = 1;
						break;
					}
					if((*sub_helper) == '\0'){
						//printf("here!\n");
						pkt_end_flag = 1;
						key_helper++;
						break;
					}
					if((*sub_helper) == '&'){
						error_flag = 1;
						break;
					}
					if(key_cnt > MAXBUFLEN){
						error_flag = 1;
						break;
					}
					(*key_helper) = (*sub_helper);
					sub_helper++;
					key_helper++;
					key_cnt++;
				}	
				//printf("\n\nkey buf: %s len: %zu\n\n", key_buf, strlen(key_buf));
				// send key_buf to redis
				if(still_working == 0){
					//printf("make redis request: key \n");
					if(found_eq_sign == 0) error_flag = 1;
					while(write(ser_sockfd,"*3\r\n$3\r\nSET\r\n", strlen("*3\r\n$3\r\nSET\r\n")) == -1) ;
					memset(word_buf, 0, MAXBUFLEN);
					sprintf(word_buf, "$%zu\r\n", strlen(key_buf));
					while(write(ser_sockfd, word_buf, strlen(word_buf)) == -1) ;
					while(write(ser_sockfd, key_buf, strlen(key_buf)) == -1) ;
					while(write(ser_sockfd, "\r\n", 2) == -1) ;
				}
			}
			//----------------------------------------------------------
			//	save value
			//----------------------------------------------------------
			else{ // key_flag == 0
				//printf("save value\n");
				if(still_working == 0){
					value_helper = value_buf;
					memset(value_buf, 0, MAXBUFLEN);
				}
				else{ //means it's new packet
					sub_helper = req_from_cli;
					//printf("sub_helper reset %c %d\n",(*sub_helper), (*value_helper));
				}
					
				if(still_working == 0) value_cnt = 0;
				still_working = 1;
						
				while(1){
					if((*sub_helper) == '&'){
						key_flag = 1;
						still_working = 0;
						sub_helper++;
						value_helper++;
						break;
					}
					if((*sub_helper) == '\0'){ //packet ended
						//printf("here!\n");
						pkt_end_flag = 1;
						if(read_left == 0) still_working = 2; //it's all done
							//value_helper++;
							break;
					}
					if((*sub_helper) == '='){
						error_flag = 1;
						break;
					}
					if(value_cnt > MAXBUFLEN){
						error_flag = 1;
						break;
					}
					(*value_helper) = (*sub_helper);
					sub_helper++;
					value_helper++;
					value_cnt++;
				}
				//printf("\n\nvalue buf: %s len: %zu\n\n", value_buf, strlen(value_buf));
				//printf("read left: %d, still_working: %d\n",read_left, still_working);
				if((still_working == 0) || (still_working == 2)){
					//printf("make redis request\n");
							
					memset(word_buf, 0, MAXBUFLEN);
					sprintf(word_buf, "$%zu\r\n", strlen(value_buf));
					while(write(ser_sockfd, word_buf, strlen(word_buf)) == -1) ;
					while(write(ser_sockfd, value_buf, strlen(value_buf)) == -1) ;
					while(write(ser_sockfd, "\r\n", 2) == -1) ;
				}
			}	
			// set end flag
			if((read_left == 0) && (pkt_end_flag == 1)) end_flag = 1;
			if((end_flag == 1) && (key_flag == 1)) error_flag = 1;
			//printf("error_flag: %d\n",error_flag);
		}
		///////////////////////////////////////////////////////
		// GET case (GET) /////////////////////////////////////
		///////////////////////////////////////////////////////
		if(req_flag == 2){
			//printf("get case!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			end_flag = 1;

			method = strtok(req_from_cli, " ");
			url = strtok(NULL, " ");
			url++;
			//printf("key: %s %zu\n",url, strlen(url));
			memset(redis_req, 0, MAXBUFLEN);
			strcpy(redis_req, "*2\r\n$3\r\nGET\r\n");
			memset(word_buf, 0, MAXBUFLEN);
			sprintf(word_buf, "$%zu\r\n", strlen(url));
			strcat(redis_req, word_buf);
			strcat(redis_req, url);
			strcat(redis_req, "\r\n");
					
			while(write(ser_sockfd, redis_req, strlen(redis_req)) == -1) ;
			//printf("GET redis_req: %s\n", redis_req);
		}
	} // end of recv while loop
	//printf("receive ended\n");
	
	//-------------------------------------------------------------------
	// 			lib event
	//-------------------------------------------------------------------
	redi_call_args->ser_sockfd = ser_sockfd;
	redi_call_args->cli_sockfd = cli_sockfd;
	redi_call_args->error_flag = error_flag;
	redi_call_args->req_flag = req_flag;
	//redi_call_args->mysock_fd = my_sockfd;
	//redi_call_args->multi_ptr = multi_ptr;
	//redi_call_args->one_base = base;
	//printf("add redis event\n");
	ev_redis = event_new(base, ser_sockfd, EV_READ, ev_redis_callback, (void*)(redi_call_args));
	event_add(ev_redis, NULL);	
	//event_base_dispatch(base);
	//-------------------------------------------------------------------
			
	// 6: close
	//close(ser_sockfd);
	//close(cli_sockfd);
	
	// detach
	//pthread_detach(thread_self);
	
	// free libevent
	//event_free(ev_redis);
	//close(my_sockfd);
	//printf("accept event done!================================================\n");
	//return 0;		
	free(redis_req);
	//free(ev_redis);
}

int main(int argc, char *argv[]) {
	//----------------------------------------------------------
	//     libevent
	//----------------------------------------------------------
	struct event_base* base = event_base_new();
	struct event* ev_accept;
	int flag;
	//struct timeval seconds = {10,0};
	//----------------------------------------------------------
	int my_port = atoi(argv[1]);//
	int my_sockfd;
	//int cli_sockfd;//
	struct sockaddr_in cli_addr;//
	int cli_len;//
	//pthread_t pthread[1000];
	//int thread_num = 0;
	//int status;
	//int i=0;
	//int read_loop_count = 0;//for debugging
	//int req_count = 0;//for debugging
	struct Multi_arg* multiple_arg = (struct Multi_arg*)malloc(sizeof(struct Multi_arg));
	multiple_arg->argv_str_2 = argv[2];
	multiple_arg->argv_str_3 = argv[3];
	//=================================================================================
	//----------------------------------------------------------
	//    work as a server
	//----------------------------------------------------------
	// ser // 1 : socket
	//printf("ser / socket\n");
	my_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(my_sockfd < 0){
		fprintf(stderr, "socket: failed\n");
		return 2;
	}

	flag = fcntl(my_sockfd, F_GETFL, 0);
	fcntl(my_sockfd, F_SETFL, flag|O_NONBLOCK);

	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(my_port);
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// ser // 2 : bind
	//printf("ser / bind\n");
	if(bind(my_sockfd, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0){
		fprintf(stderr, "ser / bind: failed\n");
		return 2;
	}
	// ser // 3 : listen
	//printf("ser / listen\n");
	if(listen(my_sockfd, BACKLOG) < 0){
		fprintf(stderr, "ser / listen: failed\n");
		return 2;
	}
	//=================================================================================
	// ser // 4 : accept
	
	//----------------------------------------------------------
	//     libevent
	//----------------------------------------------------------
	//multiple_arg->cli_fd = cli_sockfd;
	//printf("libevent in main start! ev_accept\n");
	multiple_arg->mysock_fd = my_sockfd;
	multiple_arg->one_base = base;
	ev_accept = event_new(base, my_sockfd, EV_READ|EV_PERSIST, accept_callback, (struct Multi_arg*)multiple_arg);
	//printf("multi_arg: %d\n",multiple_arg->mysock_fd);
	//printf("made new event! ev_accept\n");
	event_add(ev_accept, NULL);
	//printf("added new event ev_accept\n");
	//----------------------------------------------------------
	//printf("ser / accept\n");
	//cli_len = sizeof(cli_addr);
	//cli_sockfd = accept(my_sockfd, (struct sockaddr*)&cli_addr, &cli_len);
	//if(cli_sockfd < 0){
	//	fprintf(stderr, "ser / accept: failed\n");
	//	return 2;
	//}
	//close(cli_sockfd);

	//----------------------------------------------------------
	//     libevent
	//----------------------------------------------------------
	event_base_dispatch(base); //dispatch(event_loop)
	//printf("dispatched!!\n");
	// free all
	//event_free(ev_redis);
	close(my_sockfd);
	event_free(ev_accept);
	//---------------------------------------------------------
    return 0;
}