/* NOM : brainfuck-interpreter.c
 * Auteur : Goehry Martial
 * Date : 23/11/2021
 * Description : Interpreteur brainfuck
 *
 * 	Nombre de case de la matrice 30000
 * 	
 * 	Lexique :
 * 	char	description
 * 	> 	incrementer curseur
 * 	<	decrementer curseur
 * 	+	incrementer case pointee
 * 	-	decrementer case pointee
 * 	.	afficher le CHAR  de la case pointee
 * 	,	inserer la valeur du char donnee dans la case
 * 	[	aller à l'instruction après le ] correspondant si la case == 0
 * 	]	aller à l'instruction après le [ correspondant si la case != 0
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*======================================================*/
/*			Macro				*/
/*======================================================*/

#define VERSION "1.2"
#define MAX_CASES 30000			// Nombre de case (minimum 30 000)
#define MAX_LIGNE 4096			// taille maximale de la ligne
#define STACK_MAX 8			// taille maximale de la pile de block

/*======================================================*/
/*			Structure Stack			*/
/*======================================================*/
// conservation en memoire des '[]'
struct Block {
	int debut;
	int fin;
} typedef block;

/*======================================================*/
/*			Variables Globales		*/
/*======================================================*/

int yylval = 0;				// Valeur d'une operation

char ligne[MAX_LIGNE] = {0};		// ligne lue
int indexligne;				// index du prochain char a traiter

unsigned char matrice[MAX_CASES] = {0};	// tableau des cases pour brainfuck
int curseur = 0;			// index de la case utilisee

char *ops = "><+-,.][";			// Operateurs acceptes

block stack[STACK_MAX];			// pile pour garder les index des []
int headstack = -1;			// tete de la pile des blocks


/*======================================================*/
/*			Fonctions			*/
/*======================================================*/

// erreur - sortie en cas d'erreur
void erreur(char* message, ...){
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	exit(1);
}

// warning - avertissement de l'interpreteur
void warning(char* message){
	fprintf(stderr, "Warning: %s\n", message);
}


// find_lbark - trouver la ] correspondante
int find_lbark(void){
	int tmpindex = indexligne;
	int ignoresub = 1;
	char c;

	while ((c = ligne[tmpindex]) != 0){
		if (c == '[') ignoresub++;
		if (c == ']') {
			ignoresub--;
			if (ignoresub == 0) return tmpindex;
		}
		tmpindex++;
	}
	return 0;	// Erreur fin de ligne sans ] correspondant
}


// yylex - Lecture des operations
int yylex(void) {
	
	// Recommencer a la fin de la ligne
	if (ligne[indexligne] == '\n'){
		return 0;
	}

	// Retirer tous les char qui ne sont pas des operateurs
	while (!(strchr(ops, ligne[indexligne])))
		indexligne++;

	// Recuperation de l'operateur
	yylval = ligne[indexligne++];
	return yylval;
	
}

// yyparse - analyseur lexical
int yyparse(void) {
	char c;
	int tmpfin;

	while (1) {
		switch (yylex()){
			case '+':
				if (matrice[curseur] >= 127){
					warning("Valeur de la case hors limite ASCII");
					break;
				}
				matrice[curseur]++;
				break;
			
			case '-':
				if (matrice[curseur] <= 0){
					warning("Valeur de la case hors limite ASCII");
					break;
				}
				matrice[curseur]--;
				break;
			
			case '>':
				if (curseur >= MAX_CASES){
					warning("Déplacement du curseur hors limite");
					break;
				}
				curseur++;
				break;
				
			case '<':
				if (curseur <= 0){
					warning("Déplacement du curseur hors limite");
					break;
				}
				curseur--;
				break;

			case ',':
				printf("\t<? INPUT [%i]> ", curseur);
				
				// lecture du premier char 
				c = getchar();

				// fermer l'input sans fermer le programme
				if (c == EOF){
					matrice[curseur] = 0;	
					ungetc('\n', stdin);
				}
				else matrice[curseur] = c;

				// Vider stdin
				do { 
					c = getchar();
				} while (c != '\n');
				break;

			case '.':
				putchar(matrice[curseur]);
				break;
			
			case '[':
				if((tmpfin = find_lbark()) == 0)
					erreur("Error : missing ']' for '[' at index : %i\n", indexligne-1);

				// sauter si la case == 0
				if (matrice[curseur] == 0){
					indexligne = tmpfin+1;
					break;
				}

				// Pas d'empilement si on est dans le même boucle
				if ((headstack >= 0) && (stack[headstack].debut == indexligne-1))
					break;

				// Pile pleine
				if (headstack >= STACK_MAX)
					erreur("Error : Stack overflow, limit '[...]' to : %i\n", STACK_MAX); 
				
				// Empilement
				headstack++;
				stack[headstack].debut = indexligne-1;
				stack[headstack].fin = tmpfin;
				break;

			case ']':
				if (headstack < 0)
					erreur("Error: missing '[' before ']' at index : %i\n", indexligne-1);

				if (matrice[curseur] != 0) {
					indexligne = stack[headstack].debut;
					break;
				}

				// Depilement
				headstack--;
				break;
			
			case 0 :
				return 0;

			default:
				break;
		}
	}
	return 0;
}

void help (char *program){
	printf("Usage: %s\n\n", program);
	printf("Interpreteur en ligne de commande pour le langage BrainFuck\n\n");
	printf("++++++++++[>+>+++>+++++++>++++++++++<<<<-]>>>++.>+.+++++++..+++.<<++.>>+++++.------------.---.+++++++++++++.-------------.\n");
	exit(0);
}

/*======================================================*/
/*			MAIN				*/
/*======================================================*/

int main (int argc, char* argv[]) {
	char *s;
	
	if (argc > 1) help(argv[0]);

	printf("BrainFuck Interpreteur - version : %s\n", VERSION);	
	printf("Sortie : Ctrl + D\n");	

	while (1){
		printf("\n<C:[%i] V:[%i] BFI> ", curseur, matrice[curseur]);

		// Capture de la nouvelle ligne
		s = fgets(ligne, sizeof ligne, stdin);
		indexligne = 0;
		
		// Fermer le programme avec un EOF 
		if (s == NULL) break;

		yyparse();
	}
	return 0;
}

