#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>

#define BUFSIZE 1024 //Taille du buffer pour les transfert
#define MAX_COMMANDE 256 //Taille maximum de la commande
#define MAX_ERREUR 256 //Taille maximum des messages de control
#define MAX_SYSTEM 1024 //taille maximun des reponse commandes systeme distants


typedef struct {
  char * commande; //La commande
  char * param1; //le premier parametre de la commande
  int local; //1 si c'est une commande local , 0 sinon.
}Commande;


/*
Recoit un entier de control 0: Erreure et 1: OK, sur la socket sock.
Affiche egalement le message associe au controle recu.
sock: socket a utiliser
return: 1 ou 0
*/
int rcvControl (int sock);

/*
Passe en minuscule la chaine ligne et renvoi la chaine en minuscule.
ligne: ligne d'origine.
return: la ligne en minusucle.
*/
char * enMinuscule ( char * ligne);

/*
Decompose la ligne et la rentre dans une structure Commande.
ligne: liogne de commande.
return: La commande decompose.
*/
Commande * analyseCommande (char * ligne);

/*
Envoi un entier de control ctr et un message txt sur la socket sock
ctr: entier de control 0: Erreur ou 1:Ok
txt: message correspondant au controle effectue
sock: socket utilisee
*/
void sendControl(int ctr, char * txt,int sock);


/*
 Rencoi 1 si la commande est reconnue par le client comme systeme 0 sinon.
cmd: Struture commande a identifier
return : 1 si la commande est systeme, 0 sinon.
*/
int idCommande (Commande * cmd);

/*
Execute la commande contenue dans la structure cmd, et renvoi -1 si la commande a ehouee.
cmd: commande a executer en local
return: -1 si la commande a echouee.
*/
void execLocal(Commande * cmd);



/*
Execute la commande de la structure cmd en utilisant le socket sock et renvoi un entier d'erreur.
cmd : commande a executer
sock : socket a utiliser pour le communication avec le client 
return: -1 si la commande a echoue.
*/
int execDistant(Commande * cmd, int sock);

/*
Execute la commande PWD en distan en utilisant la socket sock retourne un code erreur.
sock: Socket de communication avec le client
return : 1 si OK ,  0 sinon.
*/
int execDistantPwd (int sock);

/*
Execute la commande LS en distant (sur le serveur) en utilisant la socket sock et retourne un code d'erreur.
sock: socket de communication avec le client
return: 1 si OK, O sinon

*/
int execDistantLs (int sock);

/*
Execute la commande CD en distant (sur le serveur) en utilisant la socket sock et sur le repertoire dir.
sock: socket de communication avec le client
dir: parametre de cd
return: 1 si OK , 0 sinon
*/
int execDistantCd (int sock, char * dir);

/*
Affiche l'aide
return: 1 dans tou les cas.
*/
int help();
