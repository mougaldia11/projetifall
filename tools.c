
#include "tools.h"
#include <ctype.h>

/**************/
/*EN MINUSCULE*/
/**************/
//PAS UTILISE
char * enMinuscule (char * ligne) {

  int i;

  char * nouvLigne = (char*) malloc (sizeof(char)*strlen(ligne));

  printf("taille ligne : %d\n",strlen(ligne));
  
  for(i=0;i<strlen(ligne);i++){

    nouvLigne[i]=tolower(ligne[i]);  

  }

  return nouvLigne;

}

/************************/
/*ANALYSE DE LA COMMANDE*/
/************************/

Commande * analyseCommande ( char * ligne) {

  //A AMELIORER => PARAMETRE DEUX DYNAMIQUE
  Commande * cmd; //Commande decoupee
  char * parcour; //premiere partie de la commande
  int i=0;

  fflush(stdout);
  parcour=ligne;
  cmd=(Commande*)malloc(sizeof(Commande));
  cmd->local=(int)malloc(sizeof(int));
  cmd->local = 0;
  
  /*Premier parametre de commande recupere*/
  while ( parcour[i] != ' '){ //Jusqua un espace
    
    i++;
    
  }
  
  cmd->commande=(char*)malloc(sizeof(char)*(i)); 
  
  strncpy(cmd->commande,ligne,i); //commande recuperee
  *((cmd->commande)+i) = '\0';
  
  /*Second parametre de commande recupere*/
  
  cmd->param1 = (char*) malloc(sizeof(char) * (strlen(ligne)-(i+1)) );
  
  strcpy(cmd->param1,&parcour[i+1]); //Parametre 1 recupere
  
  if (((cmd->commande)[0]) == '!'){

    cmd->local = 1;
    cmd->commande= (cmd->commande)+1;
    
  }

  printf("Commande: |%s| Parametre: |%s|  \n",cmd->commande,cmd->param1);   
  
  return cmd;

}


/*****************/
/*RECEIVE CONTROL*/
/*****************/

int rcvControl (int sock) {

  int ctr;
  char txt [MAX_ERREUR];

  if (read(sock,&ctr,sizeof(int))<0) {
    perror("Read socket control");
    exit(7);
  }

  if (read(sock,&txt,MAX_ERREUR)<0) {
    perror("Read socket control");
    exit(7);
  }

  //Message en retour du serveur
  printf("-->%s\n",txt);
  fflush(stdout);
  return ctr; //1 -> OK ; 0->ERREUR

}


/**************/
/*SEND CONTROL*/
/**************/

void sendControl(int ctr, char * txt,int sock) {
  
  write(sock,&ctr,sizeof(int));
  write(sock,txt,MAX_ERREUR);
  
}

/********************/
/*EXECUTION EN LOCAL*/
/********************/

void execLocal(Commande * cmd) {

int pid=0;

  if (( pid=fork()) == 0) {

    if (strcmp(cmd->commande,"ls") == 0) { //LS

      execlp(cmd->commande,cmd->commande,NULL);

    }else if (strcmp(cmd->commande,"pwd") == 0) { //PWD

      execlp(cmd->commande,cmd->commande,NULL);

    }else if (strcmp(cmd->commande,"help") == 0) { //AIDE
      help();
    }else if (strcmp(cmd->commande,"cd") == 0) { //CD
      
      chdir(cmd->param1);

    }
  }

  wait(&pid); //on attend la fin du fils sinon la boucle du shell sera plus rapide
              //et affichera le shell avant la fin du execlp
}

/********************/
/*IDENTIFIE COMMANDE*/
/********************/

int idCommande (Commande * cmd) {
  
  if (strcmp(cmd->commande,"ls") == 0) {
    return 1;
  }else if (strcmp(cmd->commande,"pwd") == 0) {
    return 1;
  }else if (strcmp(cmd->commande,"cd") == 0) {
    return 1;
  }else if (strcmp(cmd->commande,"help") == 0) {
    return 1;
  }else{ //la commande nexiste pas
    return 0;
  }
}

/*******************************/
/*EXECUTE SUR LE SYSTEM DISTANT*/
/*******************************/

int execDistant(Commande * cmd, int sock) {

  if (strcmp(cmd->commande,"ls") == 0) { //LS
    return execDistantLs(sock);
  }else if (strcmp(cmd->commande,"pwd") == 0) { //PWD
    return execDistantPwd(sock);
  }else if (strcmp(cmd->commande,"cd") == 0) { //CD xx
    return execDistantCd(sock,cmd->param1);
  }else{
    return 0;
  }
}

/*************/
/*EXECUTER CD*/
/*************/

int execDistantCd (int sock, char * dir) {
  
  int res=0;

  res = chdir(dir); //-1 -> erreur
  
  if ((write(sock,"Commande CD executee avec succes\n", MAX_SYSTEM))<0){
    perror("write System");
    exit(6);
  }

  return res;
}

/*************/
/*Commande LS*/
/*************/

int execDistantLs (int sock) {
  
  DIR * rep;
  struct dirent * dansRep;
  char buf [MAX_SYSTEM];
  
  if ((rep = opendir(".")) < 0 ) {  //ouverture du rep courant
    perror("Opendir Ls");
    return 0;
  }

  strcpy(buf,"");

  if ((dansRep = readdir(rep)) < 0) {
    perror("Readdir Ls");
    return 0;
  }
  
  while (dansRep != NULL) {

    //ATTENTION NON POSIX

    fflush(stdout);

    strcat(buf,dansRep->d_name);
    strcat(buf,"\n");
    
    if ((dansRep = readdir(rep)) <0) {
      perror("Readdir Ls");
      return 0;
    }
 
  }

  printf("LS donne :%s\n",buf);
  
  if ((write(sock,buf,MAX_SYSTEM)) < 0){
    perror("write system");
    return 0;
  }
  
  closedir(rep);

  return 1;
}

/**************/
/*Commande Pwd*/
/**************/

int execDistantPwd (int sock) {

  char * var;

  var=getenv("cwd");
  
  fflush(stdout);
  printf("PWD: %s \n",var);

  if ((write(sock,var,MAX_SYSTEM)) < 0){
    perror("write system");
    return 0;
  }
  
  return 1;
  //ATTENTION NE MARCHE PAS SOUS BASH
}

/***************/
/*Commande HELP*/
/***************/

int help () {

  fflush(stdout);
  printf("Commandes disponibles : \n");
  printf("| Local | Distant |\n");
  printf("|-----------------|\n");
  printf("| cd    | !cd     |\n");
  printf("| ls    | !ls     |\n");
  printf("| pwd   | !pwd    |\n");
  printf("|-----------------|\n");
  printf("get \n");
  printf("put \n");
  printf("quit \n");
  fflush(stdout);

  return 1;

}
