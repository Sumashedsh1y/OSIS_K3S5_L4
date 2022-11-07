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

char message[] = "End of work\n";
char buf[sizeof(message)];

int main()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        printf("Socket dont created");
        exit(1);
    }
    printf("Socket created\n");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Dont connected");
        exit(2);
    }
    printf("Connection completed\n");

    char aunt[3];
    char bufreg[100];
    recv(sock, bufreg, sizeof(bufreg), 0);
    printf(bufreg);
    scanf("%s",&aunt);
    send(sock, aunt, sizeof(aunt), 0);

    bool r;
    recv(sock, &r, sizeof(r), 0);
    printf(bufreg);
    if(!r){
        printf("Uncorrect pass\n");
        exit(0);
    }

    double X,Y,Z;
    double XYZ[3];
    printf("Enter X: ");
    scanf("%lf",&X);
    XYZ[0] = X;
    printf("Enter Y: ");
    scanf("%lf",&Y);
    XYZ[1] = Y;
    printf("Enter Z: ");
    scanf("%lf",&Z);
    XYZ[2] = Z;
    send(sock, &XYZ, sizeof(XYZ), 0);  

    int z;
    recv(sock, &z, sizeof(z), 0);
    printf("Time\tTemperature\tDisplacement X\n");
    for(int i=0;i<z;i++){
        double time,dx,temp;
        recv(sock, &time, sizeof(time), 0);
        recv(sock, &dx, sizeof(dx), 0);
        recv(sock, &temp, sizeof(temp), 0);
        printf("%3lf\t%2le\t%2lf\n",time,dx,temp);
    }
    
    close(sock);

    return 0;
}