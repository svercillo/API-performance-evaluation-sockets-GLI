#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>   // for gettimeofday()

// Set size for buffer size in linked list
#define NODE_SIZE 1


/*  Structs  */
typedef struct {
    char * data;
    int length;
    char * error_code;
    int total_response_length;
} buffer;

typedef struct {
    char * code;
    bool invalid;
} http_code;

typedef struct {
    char *val;
    int length;
    struct node * next;
} node;

typedef struct {
    int num_requests;
    char port [4];
    char request_link [2048];
    char url [1024];
} input_data;



input_data process_input(char ** input1, char ** input2);
buffer process_response(char ** response);
buffer send_HTTP_request(input_data id);
char * list_to_pointer(node ** head, int total_length);
char * check_response(char ** response);
void * print_response(void * param);

//Example inputs
// ./bin/socket --help
// ./bin/socket --url=http://my-worker.snvercil.workers.dev:80/links --profile=4
// ./bin/socket --url=http://dummy.restapiexample.com:80/api/v1/employee/1 --profile=3
// ./bin/socket --url=http://info.cern.ch:80/sfsdf  --profile=5

int cmpfunc (const void * a, const void * b) {
   return ( *(long*)a - *(long*)b );
}



int main( int argc, char** argv ) {
    if (argc < 3){
        perror("Enter --help to find a usage guilde\n");
        exit(1);
    }
    long min_time = 2147483647;
    long max_time = -1;
    long total_time = 0;
    long min_bytes = 2147483647;
    long max_bytes = -1;
    double sucesss = 0;
    int bad_requests = 0;

    struct timeval s, e;
    gettimeofday(&s, NULL);
    struct timeval start, end;
    input_data id = process_input(&argv[1], &argv[2]);

    long time_arr [id.num_requests];

    for (int i =1; i<=id.num_requests; i++){
        gettimeofday(&start, NULL);
            // input_data id = *((input_data *)param);
        buffer buff = send_HTTP_request(id);
        if (buff.total_response_length > max_bytes){
            max_bytes = buff.total_response_length;
        } 
        if (buff.total_response_length < min_bytes){
            min_bytes = buff.total_response_length;
        }
        if ( buff.error_code == NULL){
            printf("%s\n", buff.data);
            free(buff.data);
        } else {
            bad_requests ++;
            printf("{\n    error_code: '%s'\n}\n", buff.error_code);
            free(buff.error_code);
            free(buff.data);
        }


        gettimeofday(&end, NULL);
        long seconds = (end.tv_sec - start.tv_sec);
        long ms = (((seconds * 1000000) + end.tv_usec) - (start.tv_usec))/1000;
        total_time += ms;
        // printf("Time elpased is %ld ms\n", ms);

        if (ms < min_time){
            min_time = ms;   
        }
        if (ms > max_time){
            max_time = ms;
        }
        time_arr[i-1] = ms;
        printf("\n");
    }

    
    gettimeofday(&e, NULL);
    long seconds = (e.tv_sec - s.tv_sec);
    long ms = (((seconds * 1000000) + e.tv_usec) - (s.tv_usec))/1000;
    
    printf("\nStatistics: \n");
    printf("Total execution time: %ld ms\n",ms);
    printf("Number of requests: %d\n", id.num_requests);
    printf("Fastest time: %ld ms\n", min_time);
    printf("Slowest time: %ld ms\n", max_time);


    // Calculate mean 
    double mean  = total_time /id.num_requests;
    printf("Mean time: %ld ms\n",(long) mean);


    // Calculate median
    qsort(time_arr, id.num_requests, sizeof(long), cmpfunc); // sort long array
    double median = 0;
    if (id.num_requests % 2 == 0){
        median += time_arr[id.num_requests/2];
        median += time_arr[id.num_requests/2 -1];
        median /= 2;
    } else {
        median += time_arr[id.num_requests/2];
    }
    printf("Median time: %ld ms\n", (long) median);

    int percent_succ = (int) ((double) ( 1- (double) bad_requests / id.num_requests) *100);
    printf("Successful requests: %d %%\n", percent_succ);

    printf("Bytes recieved in largest response: %ld\n", max_bytes);
    printf("Bytes recieved in smallest response: %ld\n", min_bytes);

    return 0;
}


/* Function: send the HTTP request based on the processed inputs and 
also recieve memory dynamic in an I/O blocking way
*/ 
void * print_response(void * param){


}

/* Function: send the HTTP request based on the processed inputs and 
also recieve memory dynamic in an I/O blocking way
*/ 
buffer send_HTTP_request(input_data id){

    const char * url = id.url;
    const char * request_link = id.request_link;

    struct addrinfo hints; 
    struct addrinfo *serverinfo; 
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype= SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    int result = getaddrinfo( url, id.port, &hints, &serverinfo);

    if (result != 0){
        printf("Could not connect to %s.\n", url);
        exit(1);
    }
    // Connected to url !

    struct sockaddr_in * sain = (struct sockaddr_in*) serverinfo->ai_addr;
    int clientsock = socket( serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    int c = connect( clientsock, serverinfo->ai_addr, serverinfo->ai_addrlen);
    
    if (c == -1){
        perror("Could not connect to server");
        exit(1);
    }
    
    // Socket Connected!
    char * header = malloc(1024);
    memset(header, 0, 1024);
    sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nAccept: application/json\r\nConnection: close\r\nUser-Agent: Mozilla/5.0 (compatible; Rigor/1.0.0; http://rigor.com)\r\n\r\n", request_link, url);
    
    int s_status = send(clientsock, header, strlen(header),0);
    if (s_status ==-1){
        perror("Error sending data");
        exit(1);
    }





    
        /* The purpose of this section is to store an unknown amount of API 
    data dynamically using a linked list one, for the purpose of reducing 
    latency, improving memory usage, and  not overflowing stack memory
    */

    // node * head = NULL;
    // head = (node*) malloc(sizeof(node));
    // memset(head, 0, sizeof(node));
    // head->next = NULL;
    // head->val = malloc(NODE_SIZE);
    // memset(head->val, 0, NODE_SIZE);
    // node * temp = head;
    // int recv_bytes =0;
    // int n =1;
    // // the last parameter indicates it is a blocking proceedure
    // n = recv(clientsock, temp->val, NODE_SIZE, 0);

    // push new node to the top of the list and recieve more data
    // while (n >0){
    //     temp->length = n;
    //     recv_bytes += n;
    //     temp->next = (node*) malloc(sizeof(node));
    //     temp = temp->next; 

    //     temp->val = malloc(NODE_SIZE);
    //     memset(temp->val, 0, NODE_SIZE);
    //     // break;
    //     n = recv(clientsock, temp->val, NODE_SIZE, 0);
    // }
    // free(temp->next);
    // temp->next = NULL;

    
    // char * data;
    // data = list_to_pointer(&head, recv_bytes);    

    
    /* For future reference, use above code. For sake of this project,
    I know the size will be well under 100000 bytes so this is fine*/  
    char * data = malloc(2048);
    memset(data, 0, 2048);
    int recv_bytes =0;
    recv_bytes = recv(clientsock, data, 2048, 0);  
    
    buffer buff;
    char * http_status = check_response(&data);
    if (strcmp(http_status, "200") == 0){
        buff =  process_response(&data);  
        buff.total_response_length = recv_bytes;  
        free(http_status);
    } else {
        buff.error_code = http_status;
        buff.data = data;
        buff.total_response_length = recv_bytes;  
    }

    free(header);    
    close(clientsock);
    freeaddrinfo(serverinfo);
    return buff;
}


/* Function: Convert the linked list of dynamic data to a pointer
*/
char * list_to_pointer(node ** head, int total_length){
    char * p = malloc(total_length);
    memset(p, 0, total_length);
    node * current = *head;
    

    int iter =0;
    while (current != NULL) {
        if (current->length < NODE_SIZE){
            strncpy(p +NODE_SIZE * iter, current->val, current->length);  
        } else {
            strncpy(p +NODE_SIZE * iter, current->val, NODE_SIZE);  
        }
        
        node * t = current;
        current = current->next;
        free(t->val);
        free(t);
        iter ++;
    }

    return p;

}

/* Function: process input and flags into the input_data 
*/
input_data process_input(char ** input1, char ** input2){

    /* process 1st input
    */

    char * s = *input1;
    char * help_str = "--help";
    if (strcmp(s, help_str) ==0){
        printf("Usage: ./exe --url=http://<www.example.com><:80>/<requestlinks> --profile=<# of requests desired>\n");
        exit(1);
    }

    char * token = strtok(*input1, ":");
    input_data id;
    
    if(strlen(token) < 5 || (token[0] != '-' || token[1] != '-' || token[2] != 'u' || token[3] != 'r' || token[4] != 'l')){
        perror("Missing --url flag!");
        exit(1);
    }
    int iter=0; 
    
    // loop through the string to extract all other tokens
    while( token != NULL ) {
        int i =0;
        switch (iter){

            // parse the url
            case 1:
                strncpy(id.url, token +2, strlen(token)-2);
                break;
                
            // parse the port number and request link 
            case 2:
                for (; i<strlen(token); i++){
                    if (token[i] == '/'){
                        break;
                    }
                }
                if (i>3){
                    perror("Invalid port number entered!");
		            exit(1);
                }

                strncpy(id.port, token, i);
                strncpy(id.request_link, token +i, strlen(token)-i);
                break;
        }
        if (iter > 2){
            perror("Invalid URL entered!");
		    exit(1);
        }
        iter ++;
        token = strtok(NULL, ":"); // get each token
    }
    if (iter <=2){
        perror("Invalid URL entered (enter https:// or http:// and include port number!");
        exit(1);

    }

    /* process 2nd input
    */
    char * str2 = *input2;
    if(strlen(str2) < 10|| (str2[0] != '-' || str2[1] != '-' || str2[2] != 'p' || str2[3] != 'r' || str2[4] != 'o' || str2[5] != 'f' || str2[6] != 'i' || str2[7] != 'l' || str2[8] != 'e')){
        perror("Missing --profile flag!");
        exit(1);
    }
    
    int i =8;
    for (; i< strlen(str2); i++){
        if (str2[i] == '='){
            break;
        }
    }

    char * num_req_str = malloc(4);  // size of int on most machines 
    memset(num_req_str, 0, 4 );
    strncpy(num_req_str, str2 + i +1, strlen(str2) -i -1);
    id.num_requests = atoi(num_req_str);
    free(num_req_str);

    return id;
}

/* Function:  returns the HTTP response code, parsed from the response 
*/
char * check_response(char ** response){
    char * string = malloc(20);
    memset(string, 0, 20);
    strncpy(string, *response, 20);
    
    char * token = strtok(string, " ");
    if (token == NULL){
        free(string);
        perror("Incomplete response received");
        exit(1);
    }

    token = strtok(NULL, " ");
    if(token == NULL){
        free(string);
        perror("Incomplete response received");
        exit(1);
    }

    // free the temp string 
    free(string);
    char * res = malloc(strlen(token));
    memset(res, 0, strlen(token));
    strcpy(res, token);

    return res;
}


/* Function: process the raw HTTP response, and return the valid
    JSON array or object if applicable
*/
buffer process_response(char ** response){
    
    // used to define when the json data has started
    char delimiter = '~'; 
    char * string = *response;

    int stack_count = 0;  // used to determine when the json obj/arr is done.
    int i;

    // using a counter to find when the start and end of json object/arraay is
    for (i= strlen(string)-1; i >=0; i--){
        if (delimiter == '~'){
            if (string[i] == ']' || string[i] == '}'){
                delimiter = string[i];
                stack_count ++;
            }
        } else {
            if ( delimiter == ']'){
                if(string[i] == ']'){
                    stack_count ++;
                } else if (string[i] == '['){
                    stack_count--;
                    if (stack_count == 0 && i != strlen(string) -1) break; 
                }
            } else if (delimiter == '}') { 
                if(string[i] == '}'){
                    stack_count ++;
                } else if (string[i] == '{'){
                    stack_count--;
                    if (stack_count == 0 && i != strlen(string) -1) break; 
                }
            } else {
                perror("Error 1: request does not return a valid JSON object or array");
                exit(1);
            }
        }
    }
    if (i < 0){
        perror("Error 2: request does not return a valid JSON object or array");
        exit(1);
    }

    // fill all of the relevant btyes into a buffer
    int bytes_recv = strlen(string)-i;
    char * json = malloc(bytes_recv);
    memset(json, 0, bytes_recv);
    strncpy(json, string+i, bytes_recv);
    // printf("%s\n", json);
    
    free(*response);
    buffer b;
    b.data = json;
    b.length = bytes_recv;
    b.error_code = NULL;
    return b;
}
