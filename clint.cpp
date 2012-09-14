/* tcpclient.c */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

typedef struct pass {
	char url[1024];
	int flag;
	int threadCount;
} pass;

void host_name (char dummy[100],char url[1024]) {
	int k = 0;
	for(int i=0, j=0; i<strlen(url); i++) {
		if(url[i] == '/') {
			j++;
			if(j == 2) {
				i++;
				while(url[i] != '/' && i<strlen(url)) {
					dummy[k] = url[i];
					k++;
					i++;
				}
			}
		}
	}
	dummy[k] = '\0';
}

void path (char rest[100],char url[1024]) {
	int k = 0;
	rest[0] = '/';
	rest[1] = '\0';
	for(int i=0, j=0; i<strlen(url); i++) {
		if(url[i] == '/') {
			j++;
			if(j == 3) {
				rest[k] = url[i];
				i++,  k++;
				while(i != strlen(url)) {
					rest[k] = url[i];
					k++;
					i++;
				}
			}

		}
	}
	if(k!=0) {
		rest[k] = '\0';
	}
}

void build_header(char *result, char *url, int flag) {

	//GET request
	result = strcat(result,"GET ");
	//printf("URL: %s\n",url);
	result = strcat(result,url);
	result = strcat(result," HTTP/1.1\r\nHost: ");
	char dummy[100];
	host_name(dummy, url);
	result = strcat(result,dummy);
	result = strcat(result,"\r\n"
			"User-Agent: Mozilla/5.0 (X11; U; SunOS sun4u; en-US; rv:1.1) Gecko/20020827\r\n"
			"Accept:text/xml,application/xml,application/xhtml+xml,text/html;q=0.9text/plain;q=0.8,video/x-mng,image/png,image/jpeg,image/gif;q=0.2,text/css,*/*;q=0.1\r\n"
			"Accept-Language: en-us, en;q=0.50\r\n"
			"Accept-Charset: ISO-8859-1, utf-8;q=0.66, *;q=0.66\r\n"
			"Keep-Alive: 300\r\n"
			"Proxy-Connection: keep-alive\r\n"
			"Referer: http://");
	result = strcat(result,dummy);
	//	result = strcat(result,"/");
	result = strcat(result,"/\r\n");
	if (flag) {
		result = strcat(result, "Last-Modified: Wed, 01 Sep 2004 13:24:52 GMT\r\n");
	}
	result = strcat(result,"\r\n");

}

void *threadFn(void *arg) {
	pass *a = (pass*)arg;

	int sock, bytes_recieved;
	char send_data[1024], recv_data[1024];
	struct hostent *host;
	struct sockaddr_in server_addr;

	send_data[0] = '\0';

	//host = gethostbyname("cstar.iiit.ac.in");
	host = gethostbyname("127.0.0.1");

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

//	printf("sock: %d\n",sock);

	server_addr.sin_family = AF_INET;    // AF_IENET, AF_IENET6
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);

	if (connect(sock, (struct sockaddr *)&server_addr,
			sizeof(struct sockaddr)) == -1)
	{
		perror("Connect");
		exit(1);
	}

	if (strcmp(a->url , "q") != 0 && strcmp(a->url , "Q") != 0) {
		build_header(send_data, a->url, a->flag);
		//PRINTING HEDER FILE
		//			printf("header: \n%s\n\n", send_data);
		send(sock,send_data,strlen(send_data), 0);
	}
	else {
		send(sock,send_data,strlen(send_data), 0);
		close(sock);
		//			break;
	}

	int i=0;
	while(1) {
		bytes_recieved=recv(sock,recv_data,1024,0);
		//			printf("bytes: %d\n", bytes_recieved);
		recv_data[bytes_recieved] = '\0';
		if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0) {
			close(sock);
			pthread_exit(0);
			break;
		}
		else {
			//PRINTING PACKET DATA
			printf("%s\n",recv_data);
		}
		if(bytes_recieved == 0) {

			pthread_exit(0);
			break;
		}
	}

	//NO OF PACKETS
	//		printf("count: %d\n",count	);
	pthread_exit(0);
}
int main() {
	pthread_t threads[100];
	pass a;
	char newLine;

	//sending URL
	for(int i=0; i<100; i++) {
//		printf("i: %d\n",i);
//		fflush(stdout);
		printf("SEND (q or Q to quit) : ");
		scanf("%[^\n]",a.url);
		scanf("%c",&newLine);
		printf("Type of transmission( GET(0) or CONDITIONAL GET(1) ): ");
		scanf("%d",&a.flag);
		scanf("%c",&newLine);
		pthread_create(&threads[i],NULL,threadFn, &a);
 //       pthread_join(threads[i],NULL);
	}

	return 0;
}
