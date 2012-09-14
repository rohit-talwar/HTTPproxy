/* tcpserver.c */

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include<pthread.h>
#include <map>
#include <semaphore.h>

using namespace std;
sem_t x,hit,miss,ratio;
typedef map<int,char*> Mymap;
Mymap cache;
int cacheCount = 0, cacheHit  = 0, cacheMiss = 0;
float hitRatio, missRatio;

void logFile(char in[100], int i) {
	FILE *fp;
	//char in[10], out[100];
	//fileNew(i,in);
	//strcpy(out,"./proxyServerCache/");
	//out = strcat(out,in);
	fp = fopen("logFile.txt","a");
	if(i>=0){
		fprintf(fp,"%s::served from cache\n",in);
	}
	else if (i == -1){
		fprintf(fp,"%s::contacted origin server\n",in);
	}
	else{
		fprintf(fp,"%s",in);
	}
	fclose(fp);
}
void hostname(char host[1024],char res[100]){
	char *i;
	i = strstr(host,"http:");
	int j,k=0;
	for(j=7;j<100;j++){
		if(i[j]=='/' || i[j] == ' '){
			//res[k] = '/'
			res[k] = '\0';
			break;
		}else{
			res[k] = i[j];
			k++;
		}
	}
}
void fullname(char host[1024],char res[100]){
	char *i;
	i = strstr(host,"http:");
	int j,k=0;
	for(j=7;j<100;j++){
		if(i[j] == ' '){
			//res[k] = '/'
			res[k] = '\0';
			break;
		}else{
			res[k] = i[j];
			k++;
		}
	}
}
void fileNew(int val, char conv[10]) {
	if(val>=0 && val<=9) {
		conv[0] = val + '0';
		conv[1] = '\0';
	}
	else {
		conv[0] = (val/10) + '0';
		conv[1] = (val%10) + '0';
		conv[2] = '\0';
	}
}
char* fileSaving(char host[1024000], int count) {
	char *fileName;
	fileName = (char*)malloc(100*sizeof(char));
	strcpy(fileName,"./proxyServerCache/");
	char abc[10];
	fileNew(count,abc);
	fileName = strcat(fileName,abc);

	FILE *fp;
	fp = fopen(fileName,"w");
	fclose(fp);
	fp = fopen(fileName,"a");
	fputs(host,fp);
	fflush(NULL);
	fclose(fp);
	return fileName;
}
void num2string(int num, char out[100]) {
	int i = 0;
	for(i=0; num>0; i++) {
		int res = num%10;
		out[i] = res + '0';
		num = num/10;
	}
	out[i] = '\0';
	char in[100];
	int j=0;
	for(i=strlen(out)-1; i>=0; i--) {
		in[j] =out[i];
		j++;
	}
	in[j] = '\0';
	strcpy(out,in);
}
int change(char recv_data[1024]) {
	char host[100];
	fullname(recv_data,host);
	printf("host: %s\n",host);

	for(int i=0;i<100;i++) {
		if(cache[i]) {
			if(!strcmp(cache[i],host)) {
				sem_wait(&hit);
				cacheHit++;
				sem_post(&hit);
				logFile(host,i);
				return i;
			}
		}
		else if(i == 99) {
			//cyclic
			sem_wait(&x);
			cacheCount = (cacheCount+1) % 100;
			sem_post(&x);
			cache[cacheCount] = (char *)malloc(110*sizeof(char ));
			strcpy(cache[cacheCount],host);
			logFile(host,-1);
			sem_wait(&miss);
			cacheMiss++;
			sem_post(&miss);
		}
	}
	return -1;
}
int forward(char send_data[1024], char **recv_data) {
	int sock, bytes_recieved;
	struct hostent *host;
	struct sockaddr_in server_addr;

	char hostName[100];
	hostname(send_data,hostName);

	host = gethostbyname(hostName);
	//host = gethostbyname("127.0.0.1");

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	server_addr.sin_family = AF_INET;    // AF_IENET, AF_IENET6
	server_addr.sin_port = htons(80);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);

	if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1) {
		perror("Connect");
		exit(1);
	}

	send(sock,send_data,strlen(send_data), 0);

	int i = 0;
	while(1) {
		bytes_recieved=recv(sock,recv_data[i],1024,0);
		printf("bytes: %d\n", bytes_recieved);
		recv_data[i][bytes_recieved] = '\0';
		if(!bytes_recieved) {
			break;
		}
		i++;
	}
	return i;
}
void *threadFn(void *arg) {

	int *connected = (int*) arg;
	//receiving variables
	int bytes_recieved;
	char **send_data = (char **)malloc(100*sizeof(char*));
	char recv_data[1024];

	//	*send_data = (char **)malloc(100*sizeof(char *));
	for(int i=0; i<100; i++) {
		send_data[i] = (char *)malloc(1024*sizeof(char));
	}

	//receiving data data from clint
	bytes_recieved = recv(*connected,recv_data,1024000,0);

	recv_data[bytes_recieved] = '\0';
	if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0) {
		close(*connected);
		pthread_exit(0);
		return NULL;
	} else {
		//				("\n RECIEVED DATA = %s " , recv_data);
	}
	fflush(stdout);

	//correctness of code
	if(strstr(recv_data,"GET http") == NULL && strstr(recv_data,"Print Cache") == NULL && strstr(recv_data,"Print Log") == NULL && strstr(recv_data,"Search Key") == NULL) {
		printf("please request http\n");
		pthread_exit(0);
		return NULL;
	} else if( strstr(recv_data,"Print Cache") ) {
			printf("1\n");
		for(int i=0; i<100; i++) {
			if(cache[i]) {
				printf("ca: %d\n",i);
				send(*connected, cache[i],strlen(cache[i]), 0);
			}
		}
		close(*connected);
		pthread_exit(0);
		return NULL;
	} else if( strstr(recv_data,"Print Log") ){
		printf("2\n");
		FILE *fp1;
		char scan_file[1024];
		fp1 = fopen("./logFile.txt","r");
		while(fscanf(fp1,"%s",scan_file) != EOF) {
			send(*connected, scan_file,strlen(scan_file), 0);
		}
		fclose(fp1);
		close(*connected);
		pthread_exit(0);
		return NULL;
	} else if(strstr(recv_data,"Search Key")){
		printf("3\n");
		printf("%s \n",recv_data);
		char search[1024];
		int i = 15, j = 0;
		while(recv_data[i] != ' ') {
			search[j] = recv_data[i];
			i++,j++;
		}
		search[j] = '\0';
		printf("search: %s\n",search);
		for(int i=0; i<100; i++) {
//			printf("i: %d %s\n",i,);
			if(cache[i]) {
				if(strstr(cache[i],search)) {
					printf("i: %d cache: %s",i,cache[i]);
					strcpy(search,cache[i]);
					send(*connected, search,strlen(search), 0);
					close(*connected);
					pthread_exit(0);
					return NULL;
					break;
				}
			}
		}
		strcpy(search,"Not Present\n");
		send(*connected, search,strlen(search), 0);
		close(*connected);
		pthread_exit(0);
		return NULL;
	} else{

		fflush(stdout);

		int value = change(recv_data);

		char *hitchar = (char*) malloc(10*sizeof(char));
		char *send_data_extra = (char*)malloc(1024*sizeof(char));


		if(value == -1) {
			printf("No Match\n");
			//forwarding
			int count = forward(recv_data,send_data);

			int fileSavingFlag = 1;
			if(strstr(send_data[0],"no-cache")) {
				fileSavingFlag = 0;

			}

			char *big_send;
			big_send = (char*)malloc(1024000*sizeof(char));
			for(int i=0; i<count ;i++) {
				big_send = strcat(big_send,send_data[i]);
			}


			if(fileSavingFlag) {
				char *filepath = fileSaving(big_send, cacheCount);
				char *full;
				full = (char*)malloc(100*sizeof(char));
				char dum[100];
				full[0] = '\0';
				fullname(recv_data,full);
				full = strcat(full,"::");
				full = strcat(full,filepath);
				full = strcat(full," ");
				num2string(strlen(big_send),dum);
				full = strcat(full,dum);

				char *dum2, dum3[100];
				dum2 = strstr(big_send,"Expires");
				if(dum2) {
					full = strcat(full," ");
					int j=0;
					for(int i=8; dum2[i] != '\r'; i++) {
						dum3[j] = dum2[i];
						j++;
					}
					dum3[j]= '\0';
					full = strcat(full,dum3);
				}
				full = strcat(full,"\n");
				logFile(full,-2);
			}
			free(big_send);

			for(int i=0 ;i<count; i++) {
				//sending data to clint
				send(*connected, send_data[i],strlen(send_data[i]), 0);
				if(i == count-1) {

					strcpy(send_data_extra,"Hit Ratio: ");
//					printf("%d %d %f %f\n",cacheHit,cacheMiss,(float)cacheHit,(float)(cacheHit + cacheMiss));
					hitRatio = 100 * ((float)cacheHit)/((float)(cacheHit + cacheMiss));
					if(!cacheHit) {
						hitchar[0] = '0', hitchar[1] = '.', hitchar[2] = '0', hitchar[3] = '0', hitchar[4] = '\0';
					}
					else {
						num2string((int)(100*hitRatio),hitchar);
						if((int)hitRatio <10) {
							hitchar[4] = '\0', hitchar[3] = hitchar[2], hitchar[2] = hitchar[1], hitchar[1] = '.';
						}
						else {
							hitchar[5] = '\0', hitchar[4] = hitchar[3], hitchar[3] = hitchar[2], hitchar[2] = '.';
						}
					}
					send_data_extra = strcat(send_data_extra,hitchar);
					missRatio = 100 - hitRatio;
					if(cacheHit == 0){
						hitchar[0] = '1', hitchar[1] = '0', hitchar[2] = '0', hitchar[3] = '.', hitchar[4] = '0',hitchar[5] = '0', hitchar[6] = '\0';
					}else{
						num2string((int)(100*missRatio),hitchar);
						if((int)missRatio <10) {
							hitchar[4] = '\0', hitchar[3] = hitchar[2], hitchar[2] = hitchar[1], hitchar[1] = '.';
						}
						else {
							hitchar[5] = '\0', hitchar[4] = hitchar[3], hitchar[3] = hitchar[2], hitchar[2] = '.';
						}
					}
					send_data_extra = strcat(send_data_extra," Miss Ratio: ");
					send_data_extra = strcat(send_data_extra,hitchar);
					send_data_extra = strcat(send_data_extra,"\n");
					send(*connected, send_data_extra,strlen(send_data_extra), 0);
				}
			}
			fflush(NULL);
			close(*connected);
			pthread_exit(0);
			return NULL;
		}
		else {
			printf("MAtch\n");
			FILE *fp;
			char  xin[10];
			char *path, *dummyX = (char*)malloc(1024*sizeof(char));
			path = (char*)malloc(100*sizeof(char));
			dummyX[0] = '\0';
			fileNew(value,xin);
			strcpy(path,"./proxyServerCache/");
			path = strcat(path,xin);
			fp = fopen(path,"r");
			while(fscanf(fp,"%[^\n]\n",dummyX) != EOF) {
				dummyX = strcat(dummyX,"\n");
				send(*connected, dummyX,strlen(dummyX), 0);
			}
			free(path);
			fclose(fp);
			fflush(NULL);
			free(send_data);

			strcpy(send_data_extra,"Hit Ratio: ");
			hitRatio = 100*((float)cacheHit)/((float)(cacheHit + cacheMiss));

			if(!cacheHit) {
				hitchar[0] = '0', hitchar[1] = '.', hitchar[2] = '0', hitchar[3] = '0', hitchar[4] = '\0';
			} else {
				num2string((int)(100*hitRatio),hitchar);
				if((int)hitRatio <10) {
					hitchar[4] = '\0', hitchar[3] = hitchar[2], hitchar[2] = hitchar[1], hitchar[1] = '.';
				} else {
					hitchar[5] = '\0', hitchar[4] = hitchar[3], hitchar[3] = hitchar[2], hitchar[2] = '.';
				}
			}
			send_data_extra = strcat(send_data_extra,hitchar);
			missRatio = 100 - hitRatio;
			if(cacheHit == 0){
				hitchar[0] = '1', hitchar[1] = '0', hitchar[2] = '0', hitchar[3] = '.', hitchar[4] = '0',hitchar[5] = '0', hitchar[6] = '\0';
			}else{
				num2string((int)(100*missRatio),hitchar);
				if((int)missRatio <10) {
					hitchar[4] = '\0', hitchar[3] = hitchar[2], hitchar[2] = hitchar[1], hitchar[1] = '.';
				}
				else {
					hitchar[5] = '\0', hitchar[4] = hitchar[3], hitchar[3] = hitchar[2], hitchar[2] = '.';
				}
			}
			send_data_extra = strcat(send_data_extra," Miss Ratio: ");
			send_data_extra = strcat(send_data_extra,hitchar);
			send_data_extra = strcat(send_data_extra,"\n");
			send(*connected, send_data_extra,strlen(send_data_extra), 0);

			close(*connected);
			pthread_exit(0);
			return NULL;
		}
	}
}
int main() {
	int starsock,connected;
	struct sockaddr_in server_addr;
	pthread_t threads[100];
	socklen_t sin_size;
	struct sockaddr_in client_addr;

	sem_init(&x,0,1);
	sem_init(&hit,0,1);
	sem_init(&miss,0,1);
	sem_init(&ratio,0,1);

	if ((starsock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero),8);

	if (bind(starsock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}

	printf("\nTCPServer Waiting for client on port 5000");
	fflush(stdout);

	if (listen(starsock, 5) == -1) {
		perror("Listen");
		exit(1);
	}

	int i=0;
	while(1) {
		fflush(stdout);
		sin_size = sizeof(struct sockaddr_in);
		fflush(stdout);
		connected = accept(starsock, (struct sockaddr *)&client_addr,&sin_size);
		fflush(stdout);
		fflush(stdout);
		printf("\n I got a connection from (%s , %d)\n" ,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		fflush(stdout);
		pthread_create(&threads[i],NULL,threadFn,&connected);
		fflush(stdout);
		//pthread_join(threads[i],NULL);
		i = (i+1)%100;
		fflush(stdout);
	}
	printf("\nTCPServer Waiting for client on port 5000");
	fflush(stdout);

	close(starsock);

	return 0;
}
