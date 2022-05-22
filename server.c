#include "server.h"

int main(int argc, char** argv) 
{
    int global_fd;
    char msg[WHOLE_LEN];
    //create gevent fifo
    mkfifo(PATH, 0777); 

    //receive the SIGUSR1 signal from the clienthandler
    signal(SIGUSR1,sigusr1_handler);

    //clean up zombie process
    signal(SIGCHLD, zombie_child_signal);
    
    
    //create two char array to store the identifier and domain
    char identifier[256];
    char domain[256];
    
    clock_t start = clock();
    clock_t end = clock();
    pid_t pid = -1;
//use one infinite loop to waiting all messages from client
    while (1)
    {       

        global_fd = open(PATH, O_RDONLY);
        // First open in read only and read
        read(global_fd, msg, WHOLE_LEN);
        memcpy(identifier, &msg[2], IDENTIFIER_LEN);
        memcpy(domain, &msg[258], DOMAIN_LEN);
        char _RDPATH[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";
        char _WRPATH[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";
        pid_t parent_pid;
        pid_t child_pid;
        
        if (msg[0] == 0 && msg[1] == 0)
        {
            if (opendir(domain) == NULL)
            {
                mkdir(domain, 0777); 
            }
            //open a fifo for read for the connected client
            
            CREATE_RD_PATH(_RDPATH, domain, identifier);
            mkfifo(_RDPATH, 0666);

            //open a fifo for write for the connected client
            CREATE_WR_PATH(_WRPATH, domain, identifier); 
            mkfifo(_WRPATH, 0666);
            //close gevent pipe
            close(global_fd);
            parent_pid = getpid();
            child_pid = getpid();

            //fork the server to create the client handler
            pid = fork();
            if (pid < 0)
            {
                perror("Unable to fork server\n");
                return 1;
            }

        }
        if (pid == 0)
        {
                //go into the child process
            
            char new_WR[DOMAIN_LEN + IDENTIFIER_LEN + 4] = ""; 
            char new_RD[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";         
            CREATE_WR_PATH(new_WR, domain, identifier);               
            CREATE_RD_PATH(new_RD, domain, identifier);
            
            char receive[WHOLE_LEN] = "";

            
            
            while (1)
            {
                
                fd_set readfd;
                struct timeval timeout;
                int fd_2 = open(new_WR, O_RDONLY|O_NONBLOCK);
                
                timeout.tv_sec = 5;

                FD_ZERO(&readfd);

                FD_SET(fd_2,&readfd);
                
                time_t time_start = time(NULL);
                time_t time_end;
                int return_val = select(fd_2+1,&readfd,NULL,NULL,&timeout);
                
                if (return_val < 0)
                {
                    exit(0);
                }

                if(FD_ISSET(fd_2,&readfd))
                {
                    read(fd_2, receive, sizeof(receive));              
                    
                }
                time_end = time(NULL);
                close(fd_2);

                float difference = time_end - time_start;
                if (difference > 15)
                {
                    //send ping 
                    char PING_MSG[WHOLE_LEN] = {0};
                    PING_MSG[0] = 5;
                    PING_MSG[0] = 0;
                    int fd_2 = open(new_RD, O_WRONLY);
                    write(fd_2, PING_MSG, sizeof(PING_MSG));
                    close(fd_2);


                    char PONG_MSG[WHOLE_LEN] = {0};
                    int ret_3;
                    fd_set readfd_3;
                    struct timeval timeout_3;
                    int fd_3 = open(new_WR, O_RDONLY|O_NONBLOCK);

                    //set time to receive pong. If it cannot receive pong within 2 sec, which means that the client dead.
                    timeout_3.tv_sec = 2;
                    FD_ZERO(&readfd_3);
                    FD_SET(fd_3,&readfd_3);
                    
                    ret_3 = select(fd_3+1,&readfd_3,NULL,NULL,&timeout_3);
                    
                    if (ret_3 < 0)
                    {
                        exit(0);
                    }

                    if(FD_ISSET(fd_3,&readfd_3))
                    {
                        read(fd_3, PONG_MSG, sizeof(PONG_MSG));                 
                    }
                    
                    if (PONG_MSG[0] == 6 && PONG_MSG[1] == 0)
                    {
                        time_start = time(NULL);
                        time_end = time(NULL);
                    }
                    else
                    {
                        
                        unlink(_WRPATH);
                        unlink(_RDPATH);
                        kill(parent_pid, SIGUSR1);
                        kill(child_pid, SIGTERM);
                        break;
                    }   
                }

                //say receive
                if (receive[0] == 1 && receive[1] == 0)
                {
                    char say_msg[1790];
                    memcpy(say_msg, &receive[2], 1790);
                    char result[WHOLE_LEN] = {0};
                    struct dirent *directory;
                    DIR *underlying_file = opendir(domain);

                    while ((directory = readdir(underlying_file)) != NULL)
                    {
                        
                        if(directory->d_name[strlen(directory->d_name)-1] == 'D' && directory->d_name[strlen(directory->d_name)-2] == 'R' && directory->d_name[strlen(directory->d_name)-3] == '_' )
                        {
                            char cmp_ident[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";
                            strcat(cmp_ident, identifier);
                            strcat(cmp_ident, "_RD");
                            if (strcmp(cmp_ident, directory->d_name) != 0)
                            {
                                result[0] = 3;
                                result[1] = 0;
                                memcpy(&result[2], identifier, IDENTIFIER_LEN);
                                memcpy(&result[258], say_msg, 1790);

                                char path[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";
                                strcat(path, domain);
                                strcat(path, "/");
                                strcat(path, directory->d_name);
                                int fd_rd = open(path, O_WRONLY);
                                write(fd_rd, result, sizeof(result));
                                close(fd_rd);

                            }                              
                        }                                          
                    }

                }

                //say_cont rece_cont
                else if (receive[0] == 2 && receive[1] == 0)
                {
                    char say_cont_msg[1789];
                    memcpy(say_cont_msg, &receive[2], 1789);
                    int termination = receive[2047];
                    char result[WHOLE_LEN] = {0};
                    struct dirent *directory;
                    DIR *underlying_dir = opendir(domain);
                    if (underlying_dir == NULL) 
                    {
                        printf("Cannot open directory.\n" );
                        return 1;
                    }
                    while ((directory = readdir(underlying_dir)) != NULL)
                    {
                        if(directory->d_name[strlen(directory->d_name)-1] == 'D' && directory->d_name[strlen(directory->d_name)-2] == 'R' && directory->d_name[strlen(directory->d_name)-3] == '_' )
                        {
                            char cmp_ident[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";

                            strcat(cmp_ident, identifier);
                            strcat(cmp_ident, "_RD");
                            if (strcmp(cmp_ident, directory->d_name) != 0)
                            {
                                result[0] = 4;
                                result[1] = 0;
                                memcpy(&result[2], identifier, IDENTIFIER_LEN);
                                memcpy(&result[258], say_cont_msg, 1789);
                                result[2047] = termination;

                                char path[DOMAIN_LEN + IDENTIFIER_LEN + 4] = "";
                                strcat(path, domain);
                                strcat(path, "/");
                                strcat(path, directory->d_name);

                                int fd_rd = open(path, O_WRONLY);
                                
                                write(fd_rd, result, sizeof(result));              

                                close(fd_rd);
                            }
                        }

                    }

                }
                else if (receive[0] == 7 && receive[1] == 0)
                {                 
                    unlink(_WRPATH);
                    unlink(_RDPATH);
                    kill(parent_pid, SIGUSR1);
                    kill(child_pid, SIGTERM);
                    break;
                }    

                end = clock();
                if ((double)(end - start) == 15)
                {
                    char PING_MSG[WHOLE_LEN] = {0};
                    PING_MSG[0] = 5;
                    PING_MSG[0] = 0;
                    int fd_2 = open(new_RD, O_WRONLY);
                    write(fd_2, PING_MSG, sizeof(PING_MSG));
                    close(fd_2);


                    char PONG_MSG[WHOLE_LEN] = {0};
                    int fd_3 = open(new_WR, O_RDONLY);
                    read(fd_3, PONG_MSG, sizeof(PONG_MSG));
                    if (PONG_MSG[0] == 6 && PONG_MSG[1] == 0)
                    {
                        start = clock();
                        end = clock();
                    }
                    else
                    {
                        
                        // kill(parent_pid, SIGUSR1);
                        exit(0);
                    }          
                }
                
            }          
        }
        //////
        



    }
}



void CREATE_WR_PATH(char* dest, char* domain, char* identifier)
{
    strcat(dest, domain);
    strcat(dest, "/");
    strcat(dest, identifier);
    strcat(dest, "_WR");
}

void CREATE_RD_PATH(char* dest, char* domain, char* identifier)
{
    strcat(dest, domain);
    strcat(dest, "/");
    strcat(dest, identifier);
    strcat(dest, "_RD");
}



void sigusr1_handler(int signal)
{
    if (signal == SIGUSR1)
    {
        wait(NULL);
    }
    
    // if (signal == SIGUSR1)
    // {
    //     printf("ababa\n");
    //     fprintf(stderr, "%i", getpid());
    //     fprintf(stderr, "%i", parent_pid);
    //     fprintf(stderr, "%s","aba\n");
    //     waitpid(getpid(), NULL, WNOHANG);

    //     // if (i == -1)
    //     // {
    //     //     perror("Fail\n");
    //     //     exit(1);
    //     // }
    // }
}

// void signal_handler( int signal_number )
// {
//     if (signal_number == SIGUSR1)
//     {
//         ++signal_counter;
//         int wait_status;
//         pid_t return_pid = wait( &wait_status );
//         if( return_pid == -1 )
//         {
//             perror( "wait()" );
//         }
//         if( WIFEXITED( wait_status ) )
//         {
//             printf ( "job [ %d ] | pid: %d | exit status: %d\n",signal_counter, return_pid, WEXITSTATUS( wait_status ) );
//         }
//         else
//         {
//             printf( "exit abnormally\n" );
//         }
//     }
    

//     // fprintf( stderr, "the signal %d was received\n", signal_number );
// }
void zombie_child_signal(int signal) 
{ 
    pid_t pid; 
    int status; 
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){ 
        printf("Zombie process: child %d terminated\n", pid); 
    } 
    return; 
}

