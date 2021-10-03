#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

struct msg
{
    char  name[30];
    char  message[20];
    char  strzal[10];
};

struct addrinfo hints;
struct addrinfo *result, *rp, *opponent;
struct msg my_msg;
struct msg opponent_msg;
char   ip[16];
char*  numerPortu = "5560";
int    sockfd;
int    mojaPlansza[7][7];
int    trafiony;
char   statek1[4];
char   statek2[4];
char   statek3[2][4];
char   planszaPrzeciwnika[4][4];
char   nowaGra[1]="t";
int    koniec;
int    trafienia;

void sgnhandle(int signal)
{
    freeaddrinfo(result);
    freeaddrinfo(rp);
    freeaddrinfo(opponent);
    close(sockfd);
    exit(0);
}

/* Funkcje zamieniajace podane koordynaty na liczby, przy okazji sprawdzenie czy sa one poprawne */
int zamianaZnakuX(char x)
{
    if(x=='A')
    {
        return 0;
    }
    else if(x=='B')
    {
       return 1;
    }
    else if(x=='C')
    {
       return 2;
    }
    else if(x=='D')
    {
       return 3;
    }
    else
    {
        return -1;
    }
}

int zamianaZnakuY(char x)
{
    if(x=='1')
    {
        return 0;
    }
    else if(x=='2')
    {
       return 1;
    }
    else if(x=='3')
    {
       return 2;
    }
    else if(x=='4')
    {
       return 3;
    }
    else
    {
        return -1;
    }
}

/* Funkcja odpowiedzialna za zamiane na planszy przeciwnika litery t na z jesli dwuamsztowiec zostal dwukrotnie trafiony */
void ZamienTNaZ()
{
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(planszaPrzeciwnika[i][j]=='t')
            {
                planszaPrzeciwnika[i][j]='z';
                break;
            }
        }
    }
}

/* Funkcja odpowiedzialna za wypisanie planszy przeciwnika wraz z oddanymi przez nas strzalami */
void wypisz()
{
        printf("  1   2   3   4\n\n");
        printf("A %c   %c   %c   %c\n\n",planszaPrzeciwnika[0][0],planszaPrzeciwnika[0][1],planszaPrzeciwnika[0][2],planszaPrzeciwnika[0][3]);
        printf("B %c   %c   %c   %c\n\n",planszaPrzeciwnika[1][0],planszaPrzeciwnika[1][1],planszaPrzeciwnika[1][2],planszaPrzeciwnika[1][3]);
        printf("C %c   %c   %c   %c\n\n",planszaPrzeciwnika[2][0],planszaPrzeciwnika[2][1],planszaPrzeciwnika[2][2],planszaPrzeciwnika[2][3]);
        printf("D %c   %c   %c   %c\n\n",planszaPrzeciwnika[3][0],planszaPrzeciwnika[3][1],planszaPrzeciwnika[3][2],planszaPrzeciwnika[3][3]);
}

/* Funkcja pomocnicza przy ustawianiu dwumasztowca */
void minus(char statek[2])
{
    int x,y;
    x = zamianaZnakuX(statek[0])+1;
    y = zamianaZnakuY(statek[1])+1;
    mojaPlansza[x][y] = 0;
}

/* Funkcja odpowiadajaca za ustawienie statku na planszy */
int ustawStatek(char statek[4], int ile)
{
    int x,y;
    int suma;
    x = zamianaZnakuX(statek[0])+1;
    y = zamianaZnakuY(statek[1])+1;
    if(x==0||y==0||strlen(statek)!=2)
    {
        printf("Podano bledne wspolrzedne, podaj je jeszcze raz!!!\n");
        return 0;
    }

    suma = mojaPlansza[x][y-1] + mojaPlansza[x][y] + mojaPlansza[x][y+1] + mojaPlansza[x-1][y-1] + mojaPlansza[x-1][y] + mojaPlansza[x-1][y+1] + mojaPlansza[x+1][y-1] + mojaPlansza[x+1][y] + mojaPlansza[x+1][y+1];

    if(suma==ile)
    {
        mojaPlansza[x][y] = 1;
        return 1;
    }
    else
    {
        printf("Podano bledne wspolrzedne, podaj je jeszcze raz!!!\n");
        return 0;
    }
}

/* Funkcje zerujace zmienne */
void wstawianieZer()
{
    for(int i=0; i<7; i++)
    {
        for(int j=0; j<7; j++)
        {
            mojaPlansza[i][j]=0;
        }
    }
}

void wstawianie(char planszaPrzeciwnika[4][4])
{
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            planszaPrzeciwnika[i][j]=' ';
        }
    }
}

/* Funkcja sprawdzajaca czy podane 2 koordynaty dwumasztowca znajduja sie obok siebie */
int dwumasztowiec(char statek1[2], char statek2[2])
{
    if(statek1[0]==statek2[0])
    {
        if(statek1[1]==(statek2[1]-1))
        {
            return 1;
        }
        else if(statek1[1]==(statek2[1]+1))
        {
            return 1;
        }
    }
    else if(statek1[1]==statek2[1])
    {
        if(statek1[0]==(statek2[0]-1))
        {
            return 1;
        }
        else if(statek1[0]==(statek2[0]+1))
        {
            return 1;
        }
    }
    return 0;
}

/* Funkcja sprawdzajaca czy wszystkie nasze statki zostaly zatopione */
void CheckWin()
{
    if(trafienia==4)
    {
        printf("Niestety PRZEGRALES, czy chcesz przygotowac nowa plansze?(t/n)]\n");
        strcpy(my_msg.message,"WYGRANA");
        koniec = 1;
    }
}

/* Funkcja odpowiadajaca za wyslanie wiadomosci do przeciwnika */
void SendMsg()
{
    ssize_t bytes;
    bytes = sendto(sockfd, &my_msg, sizeof(my_msg), 0 , opponent->ai_addr,opponent->ai_addrlen);
    if(bytes < 0)
    {
        perror("Sendto");
    }
    else if(bytes > 0 && strcmp("NewGame", my_msg.message) == 0)
    {
        printf("[Propozycja gry wyslana]\n");
    }
}

/* Funkcja odpowiadajaca za oddanie strzalu */
void strzal()
{
    int dobry = 0;
    do
    {
        scanf("%s",my_msg.strzal);
        if((zamianaZnakuX(my_msg.strzal[0]))!=-1 && (zamianaZnakuY(my_msg.strzal[1])!=-1) && (planszaPrzeciwnika[zamianaZnakuX(my_msg.strzal[0])][zamianaZnakuY(my_msg.strzal[1])] == ' ' && strlen(my_msg.strzal)==2))
        {
            dobry++;
        }
        else if(strcmp(my_msg.strzal,"<koniec>")==0)
        {
            strcpy(my_msg.message,"KONIEC");
            SendMsg();
            freeaddrinfo(result);
            close(sockfd);
            exit(0);
        }
        else if(strcmp(my_msg.strzal,"wypisz")==0)
        {
            wypisz();
        }
        else
        {
            printf("Podano bledne wspolrzedne strzalu, podaj je jeszcze raz!!!\n");
        }
    }while(dobry==0);

    strcpy(my_msg.message,"Strzal");
    SendMsg();
}

/* Funkcja odpowiadajaca za odebranie wiadomosci oraz w zaleznosci od wiadomosci dalsza czesc programu */
void RecvMsg()
{
    recvfrom(sockfd, &opponent_msg, sizeof(opponent_msg), 0, NULL, NULL);

    if(strcmp(opponent_msg.message,"NewGame")==0)
    {
        printf("[%s (%s) dolaczyl do gry, podaj pole do strzalu]\n",opponent_msg.name, ip);
        strzal();
    }
    else if(strcmp(opponent_msg.message,"Strzal")==0)
    {
        printf("[%s (%s) strzela %s - ", opponent_msg.name, ip, opponent_msg.strzal);

        if(opponent_msg.strzal[0]==statek1[0] && opponent_msg.strzal[1]==statek1[1])
        {
            printf("jednomasztowiec zatopiony]\n");
            trafienia++;
            strcpy(my_msg.message,"1Z");
            CheckWin();
            SendMsg();
        }
        else
        if(opponent_msg.strzal[0]==statek2[0] && opponent_msg.strzal[1]==statek2[1])
        {
            printf("jednomasztowiec zatopiony]\n");
            trafienia++;
            strcpy(my_msg.message,"1Z");
            CheckWin();
            SendMsg();
        }
        else
        if(opponent_msg.strzal[0]==statek3[0][0] && opponent_msg.strzal[1]==statek3[0][1])
        {
            if(trafiony==0)
            {
                printf("dwumasztowiec trafiony]\n");
                trafiony++;
                trafienia++;
                strcpy(my_msg.message,"2T");
                SendMsg();
            }
            else
            {
                printf("dwumasztowiec zatopiony]\n");
                trafienia++;
                strcpy(my_msg.message,"2Z");
                CheckWin();
                SendMsg();
            }
        }
        else
        if(opponent_msg.strzal[0]==statek3[1][0] && opponent_msg.strzal[1]==statek3[1][1])
        {
            if(trafiony==0)
            {
                printf("dwumasztowiec trafiony]\n");
                trafiony++;
                trafienia++;
                strcpy(my_msg.message,"2T");
                SendMsg();
            }
            else
            {
                printf("dwumasztowiec zatopiony]\n");
                trafienia++;
                strcpy(my_msg.message,"2Z");
                CheckWin();
                SendMsg();
            }
        }
        else
        {
            printf("pudlo, podaj pole do strzalu]\n");
            strcpy(my_msg.message,"PUDLO");
            SendMsg();
        }
    }
    else
    if(strcmp(opponent_msg.message,"1Z")==0)
    {
        printf("[%s (%s): zatopiles jednomasztowiec, podaj kolejne pole]\n", opponent_msg.name, ip);
        planszaPrzeciwnika[zamianaZnakuX(my_msg.strzal[0])][zamianaZnakuY(my_msg.strzal[1])] = 'z';
        strzal();
    }
    else
    if(strcmp(opponent_msg.message,"2Z")==0)
    {
        printf("[%s (%s): zatopiles dwumasztowiec, podaj kolejne pole]\n", opponent_msg.name, ip);
        planszaPrzeciwnika[zamianaZnakuX(my_msg.strzal[0])][zamianaZnakuY(my_msg.strzal[1])] = 'z';
        ZamienTNaZ();
        strzal();
    }
    else
    if(strcmp(opponent_msg.message,"2T")==0)
    {
        printf("[%s (%s): trafiles dwumasztowiec, podaj kolejne pole]\n", opponent_msg.name, ip);
        planszaPrzeciwnika[zamianaZnakuX(my_msg.strzal[0])][zamianaZnakuY(my_msg.strzal[1])] = 't';
        strzal();
    }
    else
    if(strcmp(opponent_msg.message,"KONIEC")==0)
    {
        printf("[%s (%s) zakonczyl gre, czy chcesz przygotowac nowa plansze?(t/n)]\n", opponent_msg.name, ip);
        koniec = 1;
    }
    else
    if(strcmp(opponent_msg.message,"PUDLO")==0)
    {
        printf("[Pudlo, ");
        fflush(stdout);
        planszaPrzeciwnika[zamianaZnakuX(my_msg.strzal[0])][zamianaZnakuY(my_msg.strzal[1])] = 'x';
        strcpy(my_msg.message,"OK");
        SendMsg();
    }
    else
    if(strcmp(opponent_msg.message,"OK")==0)
    {
        strzal();
    }

    if(strcmp(opponent_msg.message,"WYGRANA")==0)
    {
        printf("[%s (%s) gratulacje WYGRALES, czy chcesz przygotowac nowa plansze?(t/n)]\n", opponent_msg.name, ip);
        koniec = 1;
    }
}

/* Funkcja odpowiadajaca za uzyskanie adresu IPv4 oraz polaczenie sie z przeciwnikiem*/
void polacz(char *arg)
{
    int s1;
    int s2;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    s1 = getaddrinfo(NULL,numerPortu,&hints,&result);

    hints.ai_flags = 0;

    s2 = getaddrinfo(arg,numerPortu,&hints,&opponent);

    if(s1 != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s1));
        exit(EXIT_FAILURE);
    }

    if(s2 != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s2));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sockfd == -1)
        {
            continue;
        }

        if(bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }

        close(sockfd);
    }
    inet_ntop(AF_INET, &(((struct sockaddr_in *)opponent->ai_addr)->sin_addr),ip,16);
}

int main(int argc, char *argv[])
{
    signal(SIGINT,sgnhandle);

    /* Obsluga podawanych danych*/
    if(argc==3)
    {
        strcpy(my_msg.name,argv[2]);
    }
    else
    if(argc==2)
    {
        strcpy(my_msg.name,"NN");
    }
    else
    {
        printf("Podaj pierwszy argument bedący adresem domenowym lub adresem IP hosta, z którym będziemy prowadzić konwersację\n");
        printf("Opcjonalnie drugi argument będący nazwa uzytkownika\n");
        exit(1);
    }

    polacz(argv[1]);
    close(sockfd);
    printf("Rozpoczynam grę z %s. Napisz <koniec> by zakonczyc. ", ip);

    /* Glowna petla gry */
    while(nowaGra[0]=='t')
    {
        /* Zerowanie zmiennych */
        wstawianieZer();
        wstawianie(planszaPrzeciwnika);
        trafiony = 0;
        koniec = 0;
        trafienia = 0;

        /* Ustawianie statkow */
        printf("Ustal polozenie swoich okretow:\n");

        printf("1. jednomasztowiec: ");
        do
        {
            scanf(" %s",statek1);
        }
        while(ustawStatek(statek1,0)!=1);

        printf("2. jednomasztowiec: ");
        do
        {
            scanf(" %s",statek2);
        }
        while(ustawStatek(statek2,0)!=1);

        printf("3. dwumasztowiec: ");
        int x=0;
        int y=0;
        int d=0;
        while(x!=1 || y!=1 || d!=1)
        {
            scanf("%s%s",statek3[0], statek3[1]);
            x = ustawStatek(statek3[0],0);
            y = ustawStatek(statek3[1],1);
            d = dwumasztowiec(statek3[0],statek3[1]);
            if(x==1 && y==0)
            {
                minus(statek3[0]);
            }
            else if(x==0 && y==1)
            {
                minus(statek3[1]);
            }
            else if(d==0 && x+y==2)
            {
                printf("Podano bledne wspolrzedne, podaj je jeszcze raz!!!\n");
                minus(statek3[0]);
                minus(statek3[1]);
            }
        }
        /* Polaczenie sie z przeciwnikiem i wyslanie mu startowej wiadomosci */
        polacz(argv[1]);
        strcpy(my_msg.message,"NewGame");
        SendMsg();

        /* Petla w ktorej toczy sie gra, wysylamy i odbieramy wiadomosci na przemian */
        while(9)
        {
            RecvMsg();
            if(koniec == 1)
            {
                break;
            }
        }

        /* Petla w ktorej wybieramy czy chcemy zagrac jeszcze raz */
        do
        {
            scanf(" %c",nowaGra);
        }
        while(nowaGra[0]!='t'&& nowaGra[0]!='n');

        /* Wyczyszczenie danych */
        freeaddrinfo(result);
	    close(sockfd);
    }
	return 0;
}

