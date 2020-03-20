
/*
Affiche le Prompt ftp et permet a l;utilisateur de saisir des commandes au clavier.
S'occupe de l'analyse de l'envoi et des commandes locales.
*/
void shell();

/*
Execute un Put du cote client (lecture d'un fichier cote client).
nomFichier: Fichier a utiliser pour la commande (a lire)
*/
void put (char * nomFichier);

/*
Execute un get cote client (ecriture d'un fichier sur le client).
nomFichier: Fichier a ecrire sur le client.
*/
void get (char * nomFichier);

/*
Quitte le client en cas de signal d'interuption
et quitte la session.
*/
void quitCtrlC ();

/*
Quitte le client lorsque l'utilisateur a tape la commande
*/
void quit();
