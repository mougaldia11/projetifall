/***********************************************************************************
   Nom du fichier : serveur.c
========================================================================
   Description : mini-serveur FTP permettant des fonctionnalites et
		 une interface utilisateur
========================================================================
   Auteur : Chacha22
========================================================================
   Date derniere modification : 20/03/03

   Version : 1
************************************************************************/

#include "serveur.h"
#include "tools.h"

	int socket_RV, socket_service;
	struct sockaddr_in adresseRV;
	int lgadresseRV;
	struct sockaddr_in adresseClient;
	int lgadresseClient;
	struct hostent *hote;
	unsigned short port;


/**************************************************
  Nom de la fonction : kill_zombie(int sig)
============================================
  Description : Permet d eliminer les processus 
	        qui sont en etat zombie
**************************************************/

void kill_zombie(int sig)
{
	printf("terminaison d'un processus de service\n");
	wait(NULL);
}


/*********************************************************************
  Nom de la fonction : Serveur_put(char * nomFichier)
============================================
  Description : recoit lr fichier de nom nomFichier
		en provenance du client et le stocke 
		sous le meme nom dans le repertoire
		ou le serveur a ete lance
**************************************************/

void Serveur_put(char * nomFichier) {

  	int n = 0;
  	int tailleMessage = 0;
  	int fic;
  	int totalRecu = 0;
  	int ctrl=0;
  	char buf [BUFSIZE];
  
  	printf("Lancement de put\n");
  	fflush(stdout);
  
  	if(rcvControl(socket_service)){ //si le client est OK
    
    /* ouverture du fichier a ecrire */
    if ((fic = open(nomFichier, O_WRONLY | O_CREAT | O_TRUNC,0666)) == -1) {
      fflush(stdout);
      perror("Erreur dans l ouverture du fichier: ");
      sendControl(0,"Probleme avec le fichier",socket_service);
      
    }else{
      
      
      sendControl(1,"Fichier cree et ouvert",socket_service);
      
      /*On recupere la taille du message*/
      read(socket_service,&tailleMessage,sizeof(int));
      printf("Taille du message recu (theorique) : %d \n",tailleMessage);
      fflush(stdout);
      
      while(totalRecu < tailleMessage ){
	
	/* lecture dans la socket et ecriture dans le fichier */
	if((n=read(socket_service,buf,BUFSIZE)) < 0) {
	  perror("Read");
	  exit(7);
	}
	
	totalRecu += n;
	
	if (write(fic,buf,n) != n){
	  perror("write");
	  exit(7);
	}
	
      }
      
      if(tailleMessage > totalRecu){
	printf("Message recu IMCOMPLET \n");
	fflush(stdout);
      }
      /* fermeture du fichier a ecrire */
      close(fic);  
    }
  }
}


/******************/
/*GET COTE SERVEUR*/
/******************/

void get (char * nomFichier) {

int n = 0;
int tailleFichier = 0;
int tailleEnvoyee = 0;
struct stat infos;
int src;
int ecrit;
char buf[BUFSIZE];

 printf("GET COMMENCE...\n");
 fflush(stdout);
 
 if (nomFichier == 0){
   
   sendControl(0,"Il manque le nom du fichier",socket_service);
   printf("Manque nom de fichier");
   
 }else{
   /*Ouverture fichier*/
   if ((src = open(nomFichier,O_RDONLY)) == -1) {
     
     sendControl(0,"Le fichier nexiste pas",socket_service); //erreur
     printf("Le fichier n existe pas\n");
     
   }else{ //si le fichier existe
     
     /*Recupere la taille du fichier*/
     fstat(src,&infos);
     
     tailleFichier = infos.st_size;
     
     sendControl(1,"Debut transfert ...",socket_service); //Le fichier existe
     printf("Taille totale du fichier transmis: %d \n",tailleFichier);
     /*Envoi de la taille du fichier*/
     write(socket_service,&tailleFichier,sizeof(int));
     
     while (tailleEnvoyee < tailleFichier ) {
       
       /* lecture du fichier et d'ecriture dans la socket */
       if ((n=read(src,buf,BUFSIZE)) < 0) {
	 perror("Read");
	 exit(5);
       }
       
       tailleEnvoyee += n;
       
       printf("Taille du lot transmis: %d \n",n);
       
       if ((ecrit = write(socket_service, buf, n)) != n){
	 perror("write");
	 exit(6);
       }
       
     }  
     /* fermeture du fichier a lire */
     close(src);   
   }
 }
}

/*****************/
/*TUER LE SERVEUR*/
/*****************/ 

void killServeur () {
   
   close(socket_RV);
   printf("Bye !\n"); 
   
}

/****************************/
/*QUITTER LA SESSION SERVEUR*/
/****************************/

void quit () {

printf("Recu message QUIT\n");
fflush(stdout);
printf("That s all folk\n");
fflush(stdout);
exit(1);
close(socket_service);
}


/******/
/*MAIN*/
/******/

int main (int argc, char *argv [] ) {
  
  char ligne [MAX_COMMANDE];  
  Commande * cmd;
  struct sigaction action;
  struct sigaction tuer;
  int q=0; //Permet de quitter la boucle du fork

/* armement du handler pour eliminer les fils morts */
action.sa_handler=eliminer_zombie;
sigaction(SIGCHLD,&action, NULL);

tuer.sa_handler=killServeur;
/* Armement du Control C pour tuer le serveur*/ 
sigaction(SIGINT,&tuer,NULL);

/* creation de la socket de RV */
if ((socket_RV = socket(AF_INET, SOCK_STREAM, 0)) == -1){
  perror("socket");
  exit(1);
}

/* preparation de l'adresse locale */
port = (unsigned short) atoi(argv[1]);

adresseRV.sin_family = AF_INET;
adresseRV.sin_port = htons(port);
adresseRV.sin_addr.s_addr = htonl(INADDR_ANY);

lgadresseRV = sizeof(adresseRV);

/* attachement de la socket a l'adresse locale */
if ((bind(socket_RV, &adresseRV, lgadresseRV)) == -1){
  perror("bind");
  exit(3);}

/* declaration d'ouverture du service */
if (listen(socket_RV, 10)==-1){
  perror("listen");
  exit(4);}

/* boucle d'attente de connexion */
 while(TRUE){
   printf("CONNEXION PRETE....\n");
   fflush(stdout);
   
   /* attente d'un client */
   lgadresseClient = sizeof(adresseClient);
   
   socket_service=accept(socket_RV, &adresseClient, &lgadresseClient);
   
   if (socket_service==-1 && errno==EINTR)
     /* reception d'un signal (probablement SIGCHLD) */
     continue;
   if (socket_service==-1){
     /* erreur plus grave */
     perror("accept");
     exit(5);
   }
   
   /* un client est arrive */
   printf("Connexion Acceptee\n");
   fflush(stdout);
   
   /* lancement du processus de service */
   if (fork()==0){
     /* il n'utilise plus la socket de RV */
     close(socket_RV);
     
     while(q==0) {
     
       /*Recuperation de la commande Client */ 
       read(socket_service,&ligne,MAX_COMMANDE);
       
       cmd=analyseCommande(ligne);
       
       if (((cmd->local) == 1) && (idCommande(cmd))) { //si la commande est locale

	 sendControl(1,"Commande systeme detectee",socket_service);
	 printf("Execution de %s \n",cmd->commande);
	
	 if (execDistant(cmd,socket_service) >= 0){
	   sendControl(1,"Commande executee",socket_service);
	 }else{
	   sendControl(0,"Commande echouee",socket_service);
	 }

       }else if (((cmd->local) == 1) && (idCommande(cmd))==0) {

	 sendControl(0,"Commande Inconnue",socket_service);

       }else{

	 printf("Operation %s sur fichier %s \n",cmd->commande,cmd->param1);
	 fflush(stdout);
	 
	 if (strcmp(cmd->commande,"get")==0) { /*GET*/
	   
	   sendControl(1,"Commande GET reconnue",socket_service);
	   get(cmd->param1);
	   printf("FIN GET\n");
	   fflush(stdout);
	   
	 }else if (strcmp(cmd->commande,"put")==0) { /*PUT*/
	   
	   sendControl(1,"Commande PUT reconnue",socket_service);
	   put(cmd->param1);
	   printf("FIN PUT\n");
	   fflush(stdout);
	   
	 }else if (strcmp(cmd->commande,"quit")==0) {/*QUIT*/
	   
	   sendControl(1,"Commande QUIT reconnue",socket_service);
	   q=1;
	   quit();
	   printf("FIN QUIT\n");
	   fflush(stdout);
	   
	 }else{
	   printf("COMMANDE RECUE INCONNUE\n");
	   sendControl(0,"Commande inconnue",socket_service);
	 } 
       }
     }
   }

   /* le pere n'utilise pas socket_service */
   close(socket_service);
 }
}









