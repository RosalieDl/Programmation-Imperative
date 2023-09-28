// *******************************************************
// Nom ......... : cx25.0.c
// Rôle ........ : Emulation de l'ordinateur en papier
// Auteur ...... : Rosalie Duteuil
// Version ..... : V1.1 du 20/08/2023
// Licence ..... : réalisé dans le cadre du cours de Programmation impérative
// Compilation.. : gcc -Wall cx25.0.c -o cx25.0
// Usage ....... : Pour exécuter : cx25.0 code.txt
// ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// pour strchr

typedef char * str ;			// type string
typedef unsigned char adr ; 	// type adresse (0 à 255)

#define taille_memoire 256
adr memoire[taille_memoire] ;	// variables stockées comme entiers non signés sur 8 bits
int debut_ram = 32 ;			// adresses 0 à 31 = ROM
adr PC = 32 ;					// début du programme
adr A = 0 ;						// accumulateur 
int iter_max = -1 ;				// nombre max d'itérations ; mettre -1 pour  une boucle infinie

int get_prgm(str) ; 			
int execute() ;
int saut(int) ;
int exe_commande(int, adr) ;
void store(adr) ;
void usage(int) ;


int main(int argc, str argv[])
{	if (argc < 2) {usage(4) ; return -1 ;}	// manque le nom de fichier
	int err = get_prgm(argv[1]) ;			// résultat de la lecture du programme
	if (err) {usage(err) ; return -1 ;}		// échec de lecture -> erreur et fin
	if (execute()) usage(5);				// exécution (affichage si erreur)
	return 0; }

// essaie de récupérer le programme depuis un fichier -> renvoie 0 si ok, 1 si il y a un nombre impair de caractères, 2 si caractère invalide et 3 si trop de data
int get_prgm(str fichier) 
{	FILE * flux = fopen(fichier, "r") ;		// ouvre le texte
	if (! flux) return 1 ;					// impossible de lire le fichier
	int i = debut_ram, lu ;					// index (prochain emplacement disponible en mémoire)
	unsigned int code ;						// opcode (bigramme hexa) lu dans le fichier
	while (i < taille_memoire + 1)
	{	if (!(lu = fscanf(flux, "%2X", &code))) return 2 ;  	// caractère non conforme
		else if (lu == -1) break ;								// fin du fichier
		else memoire[i++] = (adr)code; }						// sinon : inscription de l'octet en mémoire
	fclose(flux) ;
	return (i == taille_memoire + 1) ? 3 : 0 ;}					// si i est > la mémoire disponible, le programme est trop long, sinon tout s'est bien passé

// boucle principale d'éxecution
int execute()
{	for(int k = 0 ; k != iter_max ; k++)		// boucle finie ou non (cf iter_max)
	{	int OP = memoire[PC++] ;				// on récupère le code opération et on avance
		if (OP / 16 == 1) saut(OP) ;			// codes qui commencent par 1 = sauts
		else if (exe_commande(OP, memoire[PC]))	// sinon exécution (code opération + opérande)
			return -1 ;							// instruction non reconnue -> erreur
		if (PC == 0) break ;					// on a atteint la fin du programme
		else if (PC < 32) return -1 ; }  		// tentative accès ROM -> erreur
	return 0 ; }


// gestion des opérations de saut
int saut(int OP)
{	if (OP == 16 || (OP == 17 && (char)A < 0) || (OP == 18 && (char)A == 0))	// vérification de la condition du saut
		PC = memoire[PC] ;			// mise à jour du PC (saut)
	else PC++ ; 					// incrémentation du PC (pas de saut)
	return 0 ;}


// gestion des autres opérations 
int exe_commande(int OP, adr AD)	// opcode et opérande
{	adr vrb ;						// variable utilisée dans l'opération
	switch ((int)(OP / 16)) 		// récupération de la variable avec plus ou moins d'indirection(s)
	{ 	case 0 : case 2 : 	vrb = AD ; break ;						// codes en 0x ou 2x -> opérande = valeur
		case 4 : case 6 :	vrb = memoire[AD] ; break ;				// codes en 4x ou 6x -> opérande = adresse
		case 12 : case 14 : vrb = memoire[memoire[AD]] ; break ; 	// codes en Cx ou Ex -> opérande = adresse d'un pointeur
		default : return -1 ; }										// erreur : code opération non conforme
	switch (OP)						// exécution de la commande
	{	case 0 : case 64 : case 192 : 	A = vrb ; break ; 			// inscrit vrb dans A
		case 32 : case 96 : case 224 : 	A += vrb ; break ;			// ajoute vrb à A
		case 65 : case 193 : printf("%i\n", vrb); break;			// envoie vrb en sortie (comme un entier signé)
		case 33 : case 97 : case 225 :	A -= vrb ; break ;			// soustrait vrb à A
		case 34 : case 98 : case  226 : A =  ~(A & vrb) ; break ;	// fait non-et entre A et vrb
		case 72 : memoire[AD] = A ; break ;							// inscrit A en mémoire
		case 200 : memoire[memoire[AD]] = A ; break ;				// inscrit A en mémoire (avec indirection)
		case 73 : store(AD); break ;								// inscrit l'entrée utilisateur en mémoire 
		case 201 :store(memoire[AD]); break ;						// inscrit l'entrée utilisateur en mémoire (avec indirection)
		default : return -1 ; }
	PC++ ;							// incrémentation du PC
	return 0;
}

// récupère la saisie utilisateur et la stocke en mémoire
void store(adr adresse) 
{	int lu;
	char buffer[3] ;
	while (1)			// attente d'une saisie correcte
	{	fgets(buffer, 3, stdin) ;
		if (sscanf(buffer, "%2X", &lu) == 1) break ;	}
	memoire[adresse] = (adr)lu ; }	// stockage en mémoire

// affichage des messages d'erreur
void usage(int erreur)
{	switch(erreur)
	{	case 1 : fprintf(stderr, "Erreur : le fichier ne peut pas être lu.\nArrêt du programme...\n") ; break ;
		case 2 : fprintf(stderr,"Erreur : le code contient des caractères non conformes.\nArrêt du programme...\n") ; break ;
		case 3 : fprintf(stderr,"Erreur : le code est trop long pour la mémoire (%i o de RAM).\nArrêt du programme...\n", taille_memoire - debut_ram) ; break ; 
		case 4 : fprintf(stderr,"Usage : indiquer le nom du fichier à indexer...\n") ; break ;
		case 5 : fprintf(stderr,"Erreur : problème dans le code (opération invalide, ou accès mémoire interdit)\n") ; } }