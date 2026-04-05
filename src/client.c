
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <utmp.h>
extern int errno;
int viteza;
int strada;
int cnt;
void scrie_catre_server (int sd,char mesaj[])
{
     if (write(sd, mesaj, strlen(mesaj)) <= 0) 
        {
            perror("Eroare la write() spre server.\n");
            exit(EXIT_FAILURE);
        }
}
int extragenumar(char s[])
{
    int rasp = 0;
    for(int i=0;s[i];i++)
    {
        if(s[i]<'0' || s[i] >'9')
            continue;
        while(s[i]>='0' && s[i]<='9')
        {
            rasp = 10*rasp + s[i] - '0';
            i++; 
        }
        break;
    }
    return rasp;
}
int main(int argc, char *argv[]) {
    printf("Introdu strada pe care te afli si viteza cu care mergi \n");
    scanf("%d %d",&strada, &viteza);
    int sd;
    struct sockaddr_in server;
    char msg[1024];
    int port;
    fd_set readfds;
    int maxfd;
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    
    port = atoi(argv[2]);
    
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);
    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("Eroare la connect().\n");
        return errno;
    }
    
    printf("Conectat la server!\n");
    char mesaj_initial[1024];
    snprintf(mesaj_initial,sizeof(mesaj_initial),"client nou %d %d \n",strada,viteza);
    scrie_catre_server(sd,mesaj_initial);
    fflush(stdout);
    
    maxfd = sd;
    if (STDIN_FILENO > maxfd)
        maxfd = STDIN_FILENO;
    struct timeval acum, ultimul_update;
    gettimeofday(&ultimul_update,NULL);
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);  
        FD_SET(sd, &readfds);            
        
        gettimeofday(&acum,NULL);
        struct timeval tv;
        int timp_trecut = acum.tv_sec - ultimul_update.tv_sec;
        if(timp_trecut >= 60)
        {
            char mesaj_automat[1024];
            bzero(mesaj_automat,sizeof(mesaj_automat));
            snprintf(mesaj_automat,sizeof(mesaj_automat),"speed %d\n",viteza);
            scrie_catre_server(sd,mesaj_automat);
            gettimeofday(&ultimul_update,NULL);
            timp_trecut = 0;
        }
        tv.tv_sec = 60 - timp_trecut;
        tv.tv_usec = 0;
        if (select(maxfd + 1, &readfds, NULL, NULL, &tv) < 0) {
            perror("Eroare la select().\n");
            break;
        }
        
        if (FD_ISSET(sd, &readfds)) {
            bzero(msg, sizeof(msg));
            int bytes = read(sd, msg, sizeof(msg) - 1);
            
            if (bytes <= 0) {
                printf("\n Serverul a inchis conexiunea.\n");
                break;
            }
            if(strstr(msg,"Strada inexistenta! ")==msg+0)
            {
                printf("%s", msg);
                break;
            }
            printf("%s", msg);
            fflush(stdout);
        }
        
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            bzero(msg, sizeof(msg));
            
            if (fgets(msg, sizeof(msg), stdin) == NULL) {
                printf("\n Eroare la citire.\n");
                break;
            }
            
            if (strstr(msg,"speed")==msg+0)
            {
                viteza = extragenumar(msg);
                printf("Am schimbat viteza la %d \n", viteza);               
            }
            else if (strstr(msg,"where_to?")==msg+0)
                scrie_catre_server(sd,msg);
            else if (strstr(msg,"go_to") == msg+0)
            {
                scrie_catre_server(sd,msg);
                strada = extragenumar(msg);
            }
            else if (strstr(msg,"incident") == msg+0)
                scrie_catre_server(sd,msg);
            else if (strstr(msg,"extra_info") == msg + 0)
                scrie_catre_server(sd,msg);
            else if (strstr(msg,"help")== msg + 0)
            {
                printf("Comenzi posibile : \n");
                printf("speed - pentru a seta viteza \n");
                printf("where_to? - pentru a primi o lista cu toate strazile adiacente\n");
                printf("extra_info - pentru a te abona la informatii suplimentare despre vreme sau evenimente desfasurate pe strada pe care te afli\n");
                printf("go_to - pentru a schimba strada pe care te afli (te poti muta doar pe o strada adiacenta cu tine)\n");
                printf("incident - pentru a raporta un incident \n");
                printf("quit - pentru a iesi din aplicatie");
            }
            else if (strstr(msg,"quit")== msg + 0)
                break;
            else
            {
                cnt++;
                if(cnt > 1)
                    printf("Comanda necunoscuta! \n");
            }
        }
    }
    close(sd);
    printf("Conexiune inchisa.\n");
    
    return 0;
}