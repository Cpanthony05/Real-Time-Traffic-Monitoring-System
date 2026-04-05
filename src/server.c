
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
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#define PORT 3124
#define MAX_CLIENTS 30
int sd,client,optval = 1, fd, nfds, len;
struct sockaddr_in adresa_server;
struct sockaddr_in adresa_client;
fd_set marcati;
fd_set all_fds;
struct timeval tv;
int viteza[1024];
int strada[1024];
int abonat[1024];
extern int errno;
int n;
int **g;
int *d;
int *l;
char ** info;
int adiacente(int x,int y)
{
    int i;
    for(i = 0; i < d[x]; i++)
        if(g[x][i] == y)
            return 1;
    return 0;
}
void initializare_graf(char * nume_fisier)
{
    FILE *f;
    f = fopen(nume_fisier,"r");
    if(!f)
    {
        perror("fopen() esuat");
        exit(EXIT_FAILURE);
    }
    fscanf(f,"%d",&n);
    int nod;
    d = malloc(n*sizeof(int));
    l = malloc(n*sizeof(int));
    g = malloc(n*sizeof(int*));
    info = malloc(n*sizeof(char*));
    int i,j;
    for(i = 0; i < n; i++)
    {
        fscanf(f,"%d",&d[i]);
        g[i] = malloc(d[i] * sizeof(int));
        fscanf(f,"%d",&l[i]);
        if(l[i] < 0)
        {
            perror("Limita de viteza negativa! \n");
            exit(EXIT_FAILURE);
        }
        for(j = 0; j < d[i]; j++)
        {
            fscanf(f,"%d",&g[i][j]);
            if(g[i][j] < 0 || g[i][j] >= n)
            {
                perror("Lista de adicenta gresita pentru un nod! \n");
                exit(EXIT_FAILURE);
            }
        }
        info[i] = malloc(512*sizeof(char));
        fgets(info[i],512,f);
        info[i][strcspn(info[i], "\n")] = '\0';
        for(j = 0; info[i][j]; j++)
            info[i][j] = info[i][j+1];
    }
    for(i = 0;i < n; i++)
    {
        printf("Strada %d are limita de viteza %d, informatia asociata %s si %d vecini : ",i,l[i],info[i],d[i]);
        for(j = 0; j < d[i]; j++)
            printf("%d ",g[i][j]);
        printf("\n");
    }
}
void initializari()
{
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        exit(EXIT_FAILURE);
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bzero(&adresa_server, sizeof(adresa_server));
    adresa_server.sin_family = AF_INET;
    adresa_server.sin_addr.s_addr = htonl(INADDR_ANY);
    adresa_server.sin_port = htons(PORT);
    if (bind(sd, (struct sockaddr *) &adresa_server, sizeof(struct sockaddr)) == -1) {
        perror("Eroare la bind().\n");
        exit(EXIT_FAILURE);
    }
    if (listen(sd, MAX_CLIENTS) == -1) {
        perror("Eroare la listen().\n");
        exit(EXIT_FAILURE);
    }
    FD_ZERO(&all_fds);
    FD_SET(sd, &all_fds);
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    nfds = sd;
    
    printf("Am pornit. Port : %d\n", PORT);
    fflush(stdout);
}
void scrie_catre_client(int fd,char mesaj[])
{
    if(write(fd,mesaj,strlen(mesaj))<=0)
    {
        perror("Eroare la write() spre client.\n");
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
void broadcast(int sender_fd, fd_set *all_fds, int nfds, char *message, int sd) {
    int fd;
    
    for (fd = 0; fd <= nfds; fd++) {
        if (FD_ISSET(fd, all_fds) && fd != sd && fd != sender_fd) {
            if (write(fd, message, strlen(message)) < 0) {
                perror("Eroare la trimiterea broadcast.\n");
            }
        }
    }
}

int handle_client_message(int fd, fd_set *all_fds, int nfds, int sd) {
    char buffer[1024];
    char broadcast_msg[1024];
    char rasp[1024];
    int bytes;
    bzero(broadcast_msg,sizeof(broadcast_msg));
    bzero(rasp,sizeof(rasp));
    bzero(buffer, sizeof(buffer));
    bytes = read(fd, buffer, sizeof(buffer) - 1);
    
    if (bytes <= 0) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    if(strstr(buffer,"client nou")== buffer+0)
    {
        if(sscanf(buffer,"client nou %d %d",&strada[fd],&viteza[fd]) == 2)
        {
            printf("Am identificat un sofer nou! \n");
            if(strada[fd] >= n)
                snprintf(rasp,sizeof(rasp),"Strada inexistenta! \n");
            else if (viteza[fd] > l[strada[fd]])
                snprintf(rasp,sizeof(rasp),"Depasesti limita de viteza cu %d km/h! \n",viteza[fd] - l[strada[fd]]);
            else
                snprintf(rasp,sizeof(rasp),"Bine ai venit! \n");
            scrie_catre_client(fd,rasp);
            return 1;
        }
    }
    else if(strstr(buffer,"speed")== buffer + 0)
    {
        printf("Am inregistrat automat viteza %d unui sofer! \n",extragenumar(buffer));
        int viteza = extragenumar(buffer);
        if(viteza > l[strada[fd]])
        {
            snprintf(rasp,sizeof(rasp),"Depasesti limita de viteza cu %d km/h! \n", viteza - l[strada[fd]]);
            scrie_catre_client(fd,rasp);
        }
        return 1;
    }
    else if(strstr(buffer,"where_to")== buffer + 0)
    {
        printf("Am identificat o cerere where_to de pe strada %d! \n", strada[fd]);
        int locatie = strada[fd];
        int i;
        int aux = 0;
        aux = aux + snprintf(rasp,sizeof(rasp),"Strazile adiacente cu strada ta sunt:");
        for(int i=0; i < d[locatie]; i++)
            {
                //printf("%d \n",g[locatie][i]);
                if(g[locatie][i] != -1)
                    aux = aux + snprintf(rasp+aux,sizeof(rasp)-aux," %d",g[locatie][i]);
            }
        aux = aux + snprintf(rasp+aux,sizeof(rasp)-aux,"\n");
        scrie_catre_client(fd,rasp);
        return 1;
    }
    else if(strstr(buffer,"extra_info")== buffer + 0)
    {
        printf("Am identificat o cerere extra_info! \n");
        if(abonat[fd] == 0)
        {
            abonat[fd] = 1;
            int aux = 0;
            aux = aux + snprintf(rasp,sizeof(rasp),"Acum vei primi informatii despre strazile pe care calatoresti!\n");
            scrie_catre_client(fd,rasp);
            bzero(rasp,sizeof(rasp));
            aux = aux + snprintf(rasp,sizeof(rasp),"Informatii extra pentru strada %d : %s \n", strada[fd],info[fd]);
        }
        else
        {
            abonat[fd] = 0;
            snprintf(rasp,sizeof(rasp),"Acum nu mai esti abonat la informatii extra despre strazile pe care calatoresti! \n");
        }
        scrie_catre_client(fd,rasp);
        return 1;
    }
    else if(strstr(buffer,"go_to")== buffer + 0)
    {
        printf("Am identificat o cerere go_to pentru strada %d! \n", extragenumar(buffer));
        int sursa = strada[fd];
        int destinatie = extragenumar(buffer);
        if(adiacente(sursa,destinatie)==0)
            snprintf(rasp,sizeof(rasp),"Nu ai o legatura directa cu strada indicata! \n");
        else
        {
            strada[fd] = destinatie;
            snprintf(rasp,sizeof(rasp),"Pozitie actualizata. Limita de viteza pe noua strada: %d \n", l[destinatie]);
            if(abonat[fd] == 1)
            {
                scrie_catre_client(fd,rasp);
                bzero(rasp,sizeof(rasp));
                snprintf(rasp,sizeof(rasp),"Informatii extra pentru strada %d : %s \n",destinatie,info[destinatie]);
            }
        }
        scrie_catre_client(fd,rasp);
        return 1;
    }
    else if(strstr(buffer,"incident")==buffer + 0)
    {
        printf("Am identificat raportarea unui accident! \n");
        int strada_incident = strada[fd];
        l[strada_incident] = l[strada_incident]/3;
        if(l[strada_incident] == 0)
        {
            int i,j;
            for(i=0;i<n;i++)
                for(j=0;j<d[i];j++)
                    if(g[i][j] == strada_incident)
                        g[i][j] = -1;
        }
        snprintf(rasp,sizeof(rasp),"Incident raportat. Noua limita de viteza pe strada %d : %d \n",strada_incident,l[strada_incident]);
        scrie_catre_client(fd,rasp);
        snprintf(broadcast_msg,sizeof(broadcast_msg),"A avut loc un accident pe strada %d. Noua ei limita de viteza - %d \n", strada_incident, l[strada_incident]);
        broadcast(fd,all_fds,nfds,broadcast_msg,sd);
    }
    return 1;
}

int main(int argc, char * argv[]) {
    if(argc != 2)
    {
        printf("Sintaxa: %s <nume_fisier_configuratie_graf>", argv[0]);
        return(5);
    }
    initializare_graf(argv[1]);
    //return 0;
    initializari();
    while (1) {
        bcopy((char *) &all_fds, (char *) &marcati, sizeof(marcati));
        if (select(nfds + 1, &marcati, NULL, NULL, &tv) < 0) {
            perror("Eroare la select().\n");
            return errno;
        }
        
        if (FD_ISSET(sd, &marcati)) {
            len = sizeof(adresa_client);
            bzero(&adresa_client, sizeof(adresa_client));
            
            client = accept(sd, (struct sockaddr *) &adresa_client, &len);
            
            if (client < 0) {
                perror("Eroare la accept().\n");
                continue;
            }
            
            if (nfds < client)
                nfds = client;
            
            FD_SET(client, &all_fds);
        }
        for (fd = 0; fd <= nfds; fd++) {
            if (fd != sd && FD_ISSET(fd, &marcati)) {
                if (!handle_client_message(fd, &all_fds, nfds, sd)) {
                    printf("Clientul %d s-a deconectat\n", fd);
                    fflush(stdout);
                    close(fd);
                    FD_CLR(fd, &all_fds);
                }
            }
        }
    }
    
    return 0;
}