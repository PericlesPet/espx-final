#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h> 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>

#include <arpa/inet.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 

#define MYAEM 8896
// #define MYAEM 7777
// #define MYAEM 6666

#define MAX 278
#define PORT 2288
#define BUFFER_LENGTH 2000
#define MESSAGES_TOTAL 10
#define LOG_BATCH 10
#define BASE_TIME 30    // Messages produced (randomly distributed) every
#define RANDOM_TIME 60 // (BASE_TIME) to (BASE_TIME+RANDOM_TIME) seconds
#define NODES 2


void server(void);
void *client(void *); 
void *receive(void *);
void produceMsg(int);
void sendMsgs(int, int);
void catchSignals(int);
void logger(char *, int);


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;

char IP_ADDR_LIST[][11]= { "10.0.77.77", "10.0.66.66" };

// unsigned int AEM_LIST[] = {8896, 7777, 6666};
unsigned int AEM_LIST[] = {7777, 6666};

// Messages for real node 8896
char msgPool[][256]= {
	"Andre-Marie Ampere",
	"Donald Knuth", 
	"Bjarne Stroustrup", 
	"Richard Stallman",
	"Carl Sassenrath", 
	"Nicolaus Copernicus", 
	"Daniel Bernoulli",
	"Graham Bell", 
	"Aristotle",
	"Archimedes"
};
// Messages for "fake" node 7777
// char msgPool[][256]= {
// 	"Thales of Miletus",
// 	"Michael Faraday",
// 	"Leonhard Euler",
// 	"Nicolas Carnot",
// 	"Isaac Newton",
// 	"Friedrich Gauss",
// 	"Albert Einstein",
// 	"Marie Curie",
// 	"Brian Kernighan",
// 	"Linus Torvalds",
// 	"John Carmack",
// 	"Larry Page"
// };

// Messages for "fake" node 6666
// char msgPool[][256]= {
// 	"Charles Darwin", 
// 	"Rene Descartes",
// 	"Pierre de Fermat", 
// 	"Fibonacci", 
// 	"Benjamin Franklin",
// 	"Galileo Galilei", 
// 	"Heinrich Hertz",
// 	"Hypatia",
// 	"James Maxwell",
// 	"Gregor Mendel",
// 	"Alfred Nobel",
// 	"Pythagoras"
// };

//Client stuff
int LastMsgsToOthersIndex[NODES];
char LastMsgsToOthers[NODES][278];

//Buffer stuff
char buff[BUFFER_LENGTH][MAX];
int count=0;
int fullBuffer=0;

//Logging stuff
char log_buffer[LOG_BATCH][MAX];
int curr_log_count = 0;


int main(){

	for (int i=0; i<NODES; i++){
		LastMsgsToOthersIndex[i]=-1;
		strcpy(LastMsgsToOthers[i], "null");
	}


	signal(SIGALRM,produceMsg);
	signal(SIGTERM, catchSignals);
	signal(SIGINT, catchSignals);
	srand(time(NULL));

	printf("Hi, I'm %d, and I send to %d and %d\n", MYAEM, AEM_LIST[0], AEM_LIST[1]);
	produceMsg(0);

	pthread_t clientThread;	
	pthread_create(&clientThread, NULL, client, NULL);
	
	server();
	
	return 0;
}

void catchSignals(int signal){
	logger("", 1);
	printf("\nI'm ... being ... termi..nated x| \n");
	exit(1);
}

void server(void){
	int sockfd,newfd;
	int opt = 1;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("SERVER:\tsocket failed\n"); 
        exit(EXIT_FAILURE); 
    } 
	
	address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
	
	// reuse address and port options
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
         &opt, sizeof(opt))) 
        { 
            perror("SERVER:\tsetsockopt\n"); 
            exit(1); 
        } 

    if (bind(sockfd, (struct sockaddr *)&address,  sizeof(address))<0) {
        perror("SERVER:\tbind failed\n"); 
        exit(EXIT_FAILURE); 
    } 
	
	if (listen(sockfd, 3) < 0) {
        perror("SERVER:\tlisten\n"); 
        exit(EXIT_FAILURE); 
    } 
	for (;;){

		pthread_mutex_lock(&fd_mutex);
		if ((newfd = accept(sockfd, (struct sockaddr *)&address,  (socklen_t*)&addrlen))<0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

		pthread_t receiveThread;
		pthread_create(&receiveThread, NULL, receive, &newfd);
		
		
	}
	
}

void *receive(void *newfd){
	int *temp = (int *) newfd;
	int sock = *temp;


	pthread_mutex_unlock(&fd_mutex);
	
	char receivedMsg[278];

	recv(sock, receivedMsg, sizeof(receivedMsg), 0);


	int i,doubleMsg,endIndex;
	while (strcmp(receivedMsg,"Exit")!=0){
		doubleMsg=0;
		if (count>=BUFFER_LENGTH){
			count=0;
			fullBuffer=1;
		}
		
		if (fullBuffer==0) {
		 	endIndex=count;
		}
		else {
		 	endIndex=BUFFER_LENGTH;
		}
		
		for (i=0; i<endIndex; i++){
			if (strcmp(receivedMsg, buff[i])==0){
		 		doubleMsg=1;
		 		break;
		 	}
		}
		
		if (doubleMsg==0){
			pthread_mutex_lock(&mutex);
		 	strcpy(buff[count], receivedMsg);
			logger(receivedMsg, 0);
			count++;
			pthread_mutex_unlock(&mutex);
		}

		send(sock,"OK",strlen("OK")+1,0);
		recv(sock, receivedMsg, sizeof(receivedMsg), 0);
	}
	
	//Print stuff
	printf("\nMessages in buffer: \n");
	for (int i=0; i<count;i++){
		printf("buff[%d]: \t%s\n",i,buff[i]);
	}
	printf("Count:%d\n", count);

	close(sock);
	
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}


void *client(void *param){
    int sock = 0;
    struct sockaddr_in serv_addr; 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
	int j=0;
	while (j<NODES){

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		{ 
			printf("CLIENT:\tSocket creation error \n"); 
			pthread_exit(NULL); 
		} 
		// Convert IPv4 and IPv6 addresses from text to binary form 
		if(inet_pton(AF_INET, IP_ADDR_LIST[j], &serv_addr.sin_addr)<=0)  
		{ 
			printf("CLIENT:\tInvalid address/ Address not supported \n"); 
		} 
		
		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			printf("CLIENT:\tConnection Failed with IP %s\n", IP_ADDR_LIST[j]);
		} 
		else {
			sendMsgs(sock, j);
		}
		close(sock);
		j++;
		if (j == NODES){
			j = 0;
			sleep(60);
		}
	}
	
	return NULL;
}

void sendMsgs(int sock, int receiver){
	int last_msg_sent_index = LastMsgsToOthersIndex[receiver];
	char *last_msg_sent = LastMsgsToOthers[receiver];
	int endIndex = count;
	if(endIndex >= BUFFER_LENGTH){
		endIndex = 0;
	}
	char ack[3];

	if((last_msg_sent_index != endIndex) || (strcmp(last_msg_sent, buff[endIndex]) != 0)){
		do {
			last_msg_sent_index++;
			if(last_msg_sent_index >= BUFFER_LENGTH){
				last_msg_sent_index = 0;
			}

			
			if (send(sock, buff[last_msg_sent_index], strlen(buff[last_msg_sent_index])+1, 0) == -1){
				break;
			};
			
			recv(sock, ack,sizeof(ack),0);
		

			last_msg_sent = buff[last_msg_sent_index];
			
		} while (last_msg_sent_index != endIndex);

		send(sock, "Exit", strlen("Exit") + 1, 0);
		LastMsgsToOthersIndex[receiver] = last_msg_sent_index;
		strcpy(LastMsgsToOthers[receiver], last_msg_sent);

	} 
	else {
		printf("No need of new messages\n");
	}
}

void produceMsg(int sig){
	
	if (pthread_mutex_trylock(&mutex)!=0){
		alarm(3);
		return;
	}
	
	char message[278];
	unsigned int sender = MYAEM;
	
	sprintf(message, "%d_%d_%d_%s",sender, AEM_LIST[rand()%NODES], (unsigned)time(NULL), msgPool[rand()%MESSAGES_TOTAL]);
	printf("New message produced: %s\n",message);
	if (count >= BUFFER_LENGTH){
		count = 0;
		fullBuffer = 1;
	}	
	strcpy(buff[count], message);
	count++;
	logger(message, 0);
	
	int t = rand() % (RANDOM_TIME) + BASE_TIME; //produce message every 20secs to 60secs
	// FILE *fp;
	// fp = fopen("msgTimes.txt", "a+");
	// fprintf(fp, "%d\n", t);
	// fclose(fp);

	alarm(t);
	
	pthread_mutex_unlock(&mutex);
	
}


void logger(char *receive, int flush_remaining){

	FILE *fp;
	int i = 0, end = 0;
	strcpy(log_buffer[curr_log_count], receive);
	curr_log_count++;
	// determine if logging to file will take place
	if (curr_log_count == LOG_BATCH || flush_remaining){
		
		end = curr_log_count;
		curr_log_count = 0;
		// write to file
		fp = fopen("msgs.txt", "a+");
		for (int i = 0; i < end; i++){
			fprintf(fp, "%s\n", log_buffer[i]);
		}
		fclose(fp);
	}
	return;
}
