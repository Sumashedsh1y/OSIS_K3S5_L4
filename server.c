#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct info{
    int id;
    double x;
    double y;
    double z;
    double time;
    double temp;
    double dx;
};

int main()
{
    int sock, listener;
    struct sockaddr_in address;
    int bytes_read;
    pid_t proc;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        printf("Socket dont created");
        exit(1);
    }
    printf("Socket created\n");

    fcntl(listener, F_SETFL, O_NONBLOCK);
    
    address.sin_family = AF_INET;
    address.sin_port = htons(3425);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&address.sin_zero,sizeof(address.sin_zero));
    if(bind(listener, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("Bind error");
        exit(2);
    }
    printf("Bind completed\n");

    if(listen(listener, 10)==-1)
    {
        printf("Server dont started listening");
        exit(3);
    }
    printf("Server started listening\n");

    int clients[10];
    int clientaunt[10] = {0};
    int current = 0; 

    while(true) {   
        fd_set set;
        FD_ZERO(&set);
        FD_SET(listener, &set);

        for(int i = 0;i < current;i++)
            FD_SET(clients[i], &set);

        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int mx;
        int maxcurr = 0;
        for(int i = 0;i < current;i++){
            if(maxcurr < clients[i])
                maxcurr =  clients[i];
        }
        if(maxcurr > listener)
            mx = maxcurr;
        else
            mx = listener;

        if(select(mx+1, &set, NULL, NULL, (struct timeval *)&timeout) <= 0)
        {
            printf("Select error");
            continue;
        }
        printf("Select complete\n");

        for(int i = 0;i < current; i++)
        {
            if(FD_ISSET(clients[i], &set) && clientaunt[i] == 0)
            {
                char aunt[] = "Enter password (3 syms)\n";
                char bufpass[3];
                char pass[] = "onf";
                bytes_read = recv(clients[i], bufpass, sizeof(bufpass), 0);
                if(bytes_read <= 0){
                    printf("AUNT DONT OK\n");
                    bool r = false;
                    send(clients[i], &r, sizeof(r), 0);
                    clients[i] = 0;
                    continue;
                }
                if(bufpass[0]==pass[0] && bufpass[1]==pass[1] && bufpass[2]==pass[2]){
                    printf("AUNT OK\n");
                    clientaunt[i] = 1;
                    bool r = true;
                    send(clients[i], &r, sizeof(r), 0);
                    continue;
                }
                else {
                    printf("AUNT DONT OK\n");
                    bool r = false;
                    send(clients[i], &r, sizeof(r), 0);
                    clients[i] = 0;
                    continue;
                }
            }
            if(FD_ISSET(clients[i], &set) && clientaunt[i] == 1)
            {
                printf("Wait XYZ\n");
                double XYZ[3];
                recv(clients[i], &XYZ, sizeof(XYZ), 0);
                printf("x : %lf\n",XYZ[0]);
                printf("y : %lf\n",XYZ[1]);
                printf("z : %lf\n",XYZ[2]);

                struct info allinfo[10825];
                FILE *BD_Coords;

                printf("Start reading files\n");
                BD_Coords = fopen("BD_Coords.txt", "r");
                char _id1[100], _id2[100] , _x[100], _y[100], _z[100],_time[100],_temp[100],_dx1[100],_dx2[100];
                fscanf(BD_Coords, "%s\t%s\t%s\t%s\t%s\n", &_id1,&_id2,&_x,&_y,&_z);
                for (int i=0;i<10824;i++) {
                    fscanf(BD_Coords, "%i\t%lf\t%lf\t%lf\n", &allinfo[i].id, &(allinfo[i].x), &(allinfo[i].y), &(allinfo[i].z));
                }

                FILE *BD;
                BD = fopen("BD.txt", "r");
                fscanf(BD, "%s\t%s\t%s\t%s\t%s\t%s\n", &_time,&_id1,&_id2,&_temp,&_dx1,&_dx2);
                for (int i=0;i<10824;i++) {
                    fscanf(BD, "%lf\t%i\t%lf\t%lf\n", &allinfo[i].time, &allinfo[i].id, &allinfo[i].temp, &allinfo[i].dx);
                }
                printf("End reading files\n");

                printf("%s %s\t%s\t%s %s\t%s\n", _id1,_id2,_time,_dx1,_dx2,_temp);

                int z = 0;
                for (int i = 0;i < 10824; i++){
                    if (allinfo[i].x==XYZ[0] && allinfo[i].z==XYZ[2] && allinfo[i].y==XYZ[1]){
                        z++;
                        printf("%i\t%3lf\t%2le\t%2lf\n",allinfo[i].id,allinfo[i].time,allinfo[i].dx,allinfo[i].temp);
                    }
                }
                send(clients[i], &z, sizeof(z), 0);
                for (int j = 0;j < 10824; j++){
                    if (allinfo[j].x==XYZ[0] && allinfo[j].z==XYZ[2] && allinfo[j].y==XYZ[1]) {
                        send(clients[i], &allinfo[j].time, sizeof(allinfo[j].time), 0);
                        send(clients[i], &allinfo[j].dx, sizeof(allinfo[j].dx), 0);
                        send(clients[i], &allinfo[j].temp, sizeof(allinfo[j].temp), 0);
                    }
                }

                fclose(BD_Coords);
                fclose(BD);
                close(clients[i]);
                clients[i] = 0;
            }
        }
        if(FD_ISSET(listener, &set))
        {
            int sock = accept(listener, NULL, NULL);
            if(sock < 0){
                printf("Dont accepted");
                exit(4);
            }
            printf("Accept completed\n");
            
            fcntl(sock, F_SETFL, O_NONBLOCK);

            clients[current] = sock;
            current++;

            char aunt[] = "Enter password (3 syms)\n";
            send(clients[current-1], aunt, sizeof(aunt), 0);
        }
    }
    close(listener);
    return 0;
}