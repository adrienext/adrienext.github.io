#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <wchar.h>
#include <locale.h>
#include <sys/ioctl.h>

// taille d'un carre, et taille max et min du plateau
#define LENGTH 3
#define MAXSIZE 21
#define MINSIZE 3

// taille max des chaines de caracteres
#define BUFFER 12

// differentes valeurs pour le plateau
#define NBRPLAYER 2
#define P1 0
#define P2 1
#define ACTIVE 3
#define INACTIVE 4

#define AI true

// equivalent ASCII
#define LEFT 68
#define RIGHT 67
#define UP 65
#define DOWN 66
#define SPACE 32
#define ENTER 10

// macro pour mouvement
#define FIRST true
#define SECOND false

// different etat de jeu
#define ERR 0
#define MOVE 1
#define PLAY 2
#define QUIT 3
#define REST 4

// couleur
#define BG "48;5"
#define FG "38;5"
#define RED "160"
#define BLUE "27"
#define WHITE "254"
#define MINT "117"
#define LPINK "177"
#define GOLD "222"
#define GRAY "249"
#define SAND "136"
#define PWHITE "255"
#define GREEN "46"
#define MAUVE "171"
#define NAVY "69"
#define COLOR1 "31"
#define COLOR2 "75"
#define COLOR3 "153"
#define CLEAR "0"

#define IS_ODD(x) (x & (uint8_t)1)

struct winsize w;
struct timespec remaining, request = {0, 10500000};
int nanosleep(const struct timespec *req, struct timespec *rem);

uint8_t theme = 0;
typedef struct {
    uint8_t col;
    uint8_t line;
} Position;

void printHelp(char *prgName) {
    printf("%s\n", prgName);
    printf("\tRules :\n");
	printf("\t\t- Player      : First player is Vertical,\n");
	printf("\t\t                Second player is Horizontal.\n");
	printf("\t\t                Each player has is own color.\n");
	printf("\t\t- How to Play : Your position is represented by a square\n");
	printf("\t\t                with a blinking number representing the player turn.\n");
	printf("\t\t                You move by using the arrow keys,\n");
	printf("\t\t                if you want to go diagonaly just press x.\n");
	printf("\t\t                To place a bridge just press SPACE or ENTER.\n");
	printf("\t\t- How to Win  : Make your bridge cross the board,\n");
	printf("\t\t                Or surround your oppenent.\n");
	printf("\t\t- Settings    : Size change the size of the playable board,\n");
	printf("\t\t                Theme change the differents color.\n");
    exit (EXIT_SUCCESS);
}
void printError(char *errStr) {
    fprintf(stderr, "Error : %s\n", errStr);
    exit (EXIT_FAILURE);
}
void centerPrintf(char *s, uint8_t width) {
	uint8_t center = width - strlen(s);
	printf("%*s%s%*s\n", center/2, "", s, center/2, "");
}

/*==== Linked list ========================================================*/
struct link {
    uint8_t node;
    struct link * neighbor;
};
typedef struct link Link;
typedef Link * List;
bool isEmptyList(List l)
{
	return (l == (List)NULL);
}
uint8_t element(List l)
{
    return l->node;
}
List next(List l)
{
    return l->neighbor;
}
List push(uint8_t e, List l)
{
    List head = malloc(sizeof(Link));
    head->node = e;
    head->neighbor = l;
    return head;
}
List pop(List l) {
	List temp = next(l);
	free(l);
	return temp;
}
void freeList(List l) {
    if(!isEmptyList(l)) {
        freeList(next(l));
		free(l);
	}
}
/*=========================================================================*/

/*==== Theorie des graphes ================================================*/
void initAdj(List ** adj,uint8_t nbrNodes) {
	for (size_t i = 0; i < NBRPLAYER; i++)
    {
        for (size_t j = 0; j < nbrNodes; j++)
        {
            adj[i][j] = (List)NULL;
        }
    }
}
void freeAdj(List **adj, uint8_t nbrNodes) {
    for (size_t i = 0; i < NBRPLAYER; i++)
        {
            for (size_t j = 0; j < nbrNodes; j++)
            {
                freeList(adj[i][j]);
            }
            free(adj[i]);
        }
    free(adj);
}

/*---- Remplissage Adjency List -------------------------------------------*/
/*  On considere une Adjency List par joueur                               */
/*  Ils ont chacun des plots qui ont des valeurs de noeuds differents      */
/*  Un bord peut etre considere comme un seul noeud                        */
/*  Ils sont agence dans le sens du joueur                                 */
/*  ce qui a pour effet d'avoir juste a permuter les lignes et colonnes    */
/*-------------------------------------------------------------------------*/
void addBridg(List **adj, uint8_t player, uint8_t size, uint8_t n, uint8_t col, uint8_t lin, uint8_t even, uint8_t start, uint8_t end) {
    /*---- Cas terminaux --------------------------------------------------*/
	if (lin == 0) {
        adj[player][start] = push(1+(col/2)*n, adj[player][start]);
        adj[player][1+(col/2)*n] = push(start, adj[player][1+(col/2)*n]);
    } else if (lin == size - 1) {
        adj[player][(1+col/2)*n] = push(end, adj[player][(1+col/2)*n]);
        adj[player][end] = push((1+col/2)*n, adj[player][end]);
    } 
	/*---------------------------------------------------------------------*/
	else if (even) {
        adj[player][(lin/2)+(col/2)*n] = push((lin/2)+(col/2)*n+1, adj[player][(lin/2)+(col/2)*n]);
        adj[player][(lin/2)+(col/2)*n+1] = push((lin/2)+(col/2)*n, adj[player][(lin/2)+(col/2)*n+1]);
    } else {
        adj[player][(col/2)*n+(lin/2)+1] = push((col/2)*n+(lin/2)+n+1, adj[player][(col/2)*n+(lin/2)+1]);
        adj[player][(col/2)*n+(lin/2)+n+1] = push((col/2)*n+(lin/2)+1, adj[player][(col/2)*n+(lin/2)+n+1]);
    }
}
void remBridg(List **adj, uint8_t player, uint8_t size, uint8_t n, uint8_t col, uint8_t lin, uint8_t even, uint8_t start, uint8_t end) {
    if (lin == 0) {
        adj[player][start] = pop(adj[player][start]);
        adj[player][1+(col/2)*n] = pop(adj[player][1+(col/2)*n]);
    } else if (lin == size - 1) {
        adj[player][(1+col/2)*n] = pop(adj[player][(1+col/2)*n]);
        adj[player][end] = pop(adj[player][end]);
    } else if (even) {
        adj[player][(lin/2)+(col/2)*n] = pop(adj[player][(lin/2)+(col/2)*n]);
        adj[player][(lin/2)+(col/2)*n+1] = pop(adj[player][(lin/2)+(col/2)*n+1]);
    } else {
        adj[player][(col/2)*n+(lin/2)+1] = pop(adj[player][(col/2)*n+(lin/2)+1]);
        adj[player][(col/2)*n+(lin/2)+n+1] = pop(adj[player][(col/2)*n+(lin/2)+n+1]);
    }
}

/*---- Algo de Victoire ------------------------------*/
/*  Parcours le graphe avec un algo DFS               */
/*  trouve les encerclement si un noeud deja visite   */
/*  est différent de son parent                       */
/*----------------------------------------------------*/
void dfs(List *adj, bool *visited, uint8_t node, uint8_t parent, uint8_t start, uint8_t end, bool *cycle) {
    visited[node] = true;
    List temp = adj[node];
    while (!isEmptyList(temp)) {
		/*---- Parcours ---------------------------------------------------------------------------*/
        if (!visited[element(temp)]) {
            dfs(adj, visited, element(temp), node, start, end, cycle);
        }
		/*---- Encerclement -----------------------------------------------------------------------*/
		/* normalement il faut regarder que le parent mais                                         */
		/* je rajoute le reste pour qu'il n'y ai pas d'encerclement en 3 coups                     */
		/* (en partant du principe que le dernier coup qui donne l'encerclement est joué au bord)  */
		/* pour que ca marche tout le temps il aurait fallu un algo BFS qui donne la taille        */
		/*-----------------------------------------------------------------------------------------*/
        else if (element(temp) != parent && element(temp) != start && element(temp) != end) {
            *cycle = true;
        }
        temp = next(temp);
    }
}
bool win(List **adj, uint8_t nbrNodes, uint8_t player, uint8_t start, uint8_t end) {
	bool bridg = false; 	  // parcour d'un tableau
	bool cycle = false; 	// encerclement
    bool * visited = malloc(sizeof(bool)*nbrNodes);
    for (uint8_t i = 0; i < nbrNodes; i++)
    {
        visited[i] = false;
    }

	/*  On parcour en partant du debut                         */
	dfs(adj[player], visited, start, start, start, end, &cycle);
	/*  On regarde si on est arrive a la fin                   */
	bridg = (visited[end]) ? true : false;
	
	/*  Le cas echeant on regarde si y a pas un encerclement   */
	if (!bridg && !cycle) {
    	for (uint8_t i = 1; i < nbrNodes; i++)
    	{
    	    if (!visited[i] && !isEmptyList(adj[player][i]))
    	    {
    	        dfs(adj[player], visited, i, i, start, end, &cycle);
    	    }
    	}
	}
    
    free(visited);
    return (bridg || cycle);
}
/*=========================================================================*/

 
/*==== Interactif ==================================================*/
/*------------------------------------------------------------------*/
/*  On desactive le mode CANONIQUE (pour ne plus avoir de buffer)   */
/*  Ainsi que l'ECHO                                                */
/*  on renitialise le terminal si on fait CTRL+C                    */
/*------------------------------------------------------------------*/
void setBufferedInput(bool enable) {
	// au premier appelle on est forcément en mode canonique
	static bool canon = true;
	// au premier appelle on récupere la configuration du terminal qu'on va restore
	static struct termios restore;
	// le terminal qu'on va modifier et appliquer
	struct termios term;

	// si on veut setBufferedInput et que on est pas en mode canonique
	if (enable && !canon) {
		// restore les paramètres de base
		tcsetattr(STDIN_FILENO, TCSANOW, &restore);
		// stock l'état actuel
		canon = true;
	// si on veut enlever le BufferedInput et que on est en mode canonique
	} else if (!enable && canon) {
		// récupere les paramètres actuel du terminal
		tcgetattr(STDIN_FILENO, &term);
		// on fait une sauvegarde pour pouvoir restaurer le terminal plus tard
		restore = term;
		// désactive le mode canonique(buffered i/o) et l'écho local
		term.c_lflag &= ~(ICANON | ECHO);
		// applique les nouveau paramètres immédiatemet
		tcsetattr(STDIN_FILENO,TCSANOW, &term);
		// stock l'état actuel
		canon = false;
	}
}
void renitialize() {
    setBufferedInput(true);       // on remet le mode canonique
    printf("\x1b[?25h\x1b[0m"); // cursor visible et 0m reset all modes (style + colors)
}
void handle_interupt(int signum) {
	renitialize();
	putchar('\n');
	printf("TERMINATED\n");
	exit (signum);
}
void handle_winch() {
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
		printError("size");
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
}
/*==================================================================*/


/*==== Graphisme ===================================================*/
char * getColor(int player) {
    switch (theme) {
	case 0:
		if (player == P1) return RED;
        else if (player == P2) return BLUE;
		else if (player == ACTIVE) return WHITE;
		else if (player == INACTIVE) return CLEAR;
		break;
	case 1:
		if (player == P1) return MINT;
        else if (player == P2) return LPINK;
		else if (player == ACTIVE) return GOLD;
		else if (player == INACTIVE) return CLEAR;
		break;
	case 2:
		if (player == P1) return GRAY;
        else if (player == P2) return SAND;
		else if (player == ACTIVE) return PWHITE;
		else if (player == INACTIVE) return CLEAR;
		break;
	case 3:
		if (player == P1) return GREEN;
        else if (player == P2) return MAUVE;
		else if (player == ACTIVE) return NAVY;
		else if (player == INACTIVE) return CLEAR;
		break;	
	case 4:
		if (player == P1) return COLOR1;
        else if (player == P2) return COLOR2;
		else if (player == ACTIVE) return COLOR3;
		else if (player == INACTIVE) return CLEAR;
		break;
	}
	return CLEAR;
}

/*---- Plateau ----------------------------------*/
void firstLine(uint8_t size) {
	printf("\x1b[%s;%sm%*s", BG, getColor(ACTIVE), LENGTH, "");
	printf("\x1b[%s;%sm%*s", BG, getColor(P1), LENGTH * (2*size + 3), ""); // taille carre * nbr de carre
	printf("\x1b[%s;%sm%*s", BG, getColor(ACTIVE), LENGTH, "");
	putchar('\n');
}
void secondLine(uint8_t size) {
	printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
	printf("\x1b[%dD", LENGTH);
	for (uint8_t i = 0; i < ((size - 1) / 2 + 1); i++)
	{
		printf("\x1b[%dC", 3*LENGTH);
		printf("\x1b[%s;%sm%*s", BG, getColor(P1), LENGTH, "");
	}
	printf("\x1b[%dC", 2*LENGTH);
	printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
	putchar('\n');
}
void basicLine(uint8_t size) {
	printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
	printf("\x1b[%dC", LENGTH*(2*size + 3));
	printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
	putchar('\n');
}
void horiBridg(uint8_t player) {
	printf("\x1b[%dD", LENGTH);
	printf("\x1b[%s;%sm%*s", BG, getColor(player), 3*LENGTH, "");
	printf("\x1b[%dD", LENGTH);
}
void vertiBridg(uint8_t player) {
	printf("\x1b[s");
	printf("\x1b[1A");
	printf("\x1b[%s;%sm%*s", BG, getColor(player), LENGTH, "");
	printf("\x1b[u");
	printf("\x1b[2A");
	printf("\x1b[%s;%sm%*s", BG, getColor(player), LENGTH, "");
	printf("\x1b[u");
	printf("\x1b[1B");
	printf("\x1b[%s;%sm%*s", BG, getColor(player), LENGTH, "");
	printf("\x1b[u");
	printf("\x1b[2B");
	printf("\x1b[%s;%sm%*s", BG, getColor(player), LENGTH, "");
	printf("\x1b[u");
	printf("\x1b[%s;%sm%*s", BG, getColor(player), LENGTH, "");
}
void drawBoard(uint8_t **board, uint8_t size, uint8_t player, bool diagonal) {
	uint8_t x, y;
	uint8_t center;
	printf("\x1b[H");

	firstLine(size);
	secondLine(size);
	
	for (y = 0; y < size; y++)
	{
		basicLine(size);
		// la partie qui compte pas dans le jeu
		printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
		if (IS_ODD(y)) {
			printf("\x1b[%dC", LENGTH);
		} else {
			printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
		}
		printf("\x1b[%dC", LENGTH);
		// debut du jeu
		for (x = 0; x < size; x++) {
			// verifie si c'est une base
			if ((!IS_ODD(y) && IS_ODD(x)) || (IS_ODD(y) && !IS_ODD(x))) {
				printf("\x1b[%s;%sm%*s", BG, getColor(board[x][y]), LENGTH, "");
			}
			else if (!IS_ODD(y)) {
				if (board[x][y] == P1) {
					vertiBridg(P1);
				} else if (board[x][y] == P2) {
					horiBridg(P2);
				} else if (board[x][y] == ACTIVE) {
					printf("\x1b[5m");
					printf("\x1b[%s;%sm %d ", BG, getColor(board[x][y]), 1+player);
				} else {
					printf("\x1b[%s;%sm%*s", BG, getColor(board[x][y]), LENGTH, "");
				}
			} else if (IS_ODD(y)) {
				if (board[x][y] == P1) {
					horiBridg(P1);
				} else if (board[x][y] == P2) {
					vertiBridg(P2);
				}else if (board[x][y] == ACTIVE) {
					printf("\x1b[5m");
					printf("\x1b[%s;%sm %d ", BG, getColor(board[x][y]), 1+player);
				} else {
					printf("\x1b[%s;%sm%*s", BG, getColor(board[x][y]), LENGTH, "");
				}
			}
			printf("\x1b[%dC", LENGTH);
		}
		// partie hors du jeu
		if (IS_ODD(y)) {
			printf("\x1b[%dC", LENGTH);
		} else {
			printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
		}
		printf("\x1b[%s;%sm%*s", BG, getColor(P2), LENGTH, "");
		putchar('\n');
	}
	basicLine(size);

	secondLine(size);
	firstLine(size);
	printf("\x1b[%sm", CLEAR);
	center = LENGTH*(2*size + 5) - 14;
	printf("%*s%s%*s\n", center/2, "", "←,↑,→,↓,x or q", center/2, "");
	if (diagonal) centerPrintf("diagonal enabled", LENGTH*(2*size + 5));
	else centerPrintf("diagonal disabled", LENGTH*(2*size + 5));
	
	printf("\033[A");
}
/*-----------------------------------------------*/

/*---- Menu -------------------------------------*/
void startScreen() {
	
	wchar_t biomed[6][48] = {
		L" ██████╗ ██╗ ██████╗ ███╗   ███╗███████╗██████╗ ",
		L" ██╔══██╗██║██╔═══██╗████╗ ████║██╔════╝██╔══██╗",
		L" ██████╔╝██║██║   ██║██╔████╔██║█████╗  ██║  ██║",
		L" ██╔══██╗██║██║   ██║██║╚██╔╝██║██╔══╝  ██║  ██║",
		L" ██████╔╝██║╚██████╔╝██║ ╚═╝ ██║███████╗██████╔╝",
		L" ╚═════╝ ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝╚═════╝ "
	};
	wchar_t khana[6][48] = {
		L"    ██╗  ██╗██╗  ██╗ █████╗ ███╗   ██╗ █████╗  ",
		L"    ██║ ██╔╝██║  ██║██╔══██╗████╗  ██║██╔══██╗ ",
		L"    █████╔╝ ███████║███████║██╔██╗ ██║███████║ ",
		L"    ██╔═██╗ ██╔══██║██╔══██║██║╚██╗██║██╔══██║ ",
		L"    ██║  ██╗██║  ██║██║  ██║██║ ╚████║██║  ██║ ",
		L"    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ "
	};
	
	
	printf("\x1b[H");
	for (uint8_t i = 0; i < 6; i++)
	{
		for (uint8_t j = 0; j < 47; j++)
		{
			if (biomed[i][j] != 9608)
			{
				printf("\x1b[5;38;5;%sm", getColor(P1));
			}
			else {
				printf("\x1b[0m");
			}
			printf("%lc", biomed[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for (uint8_t i = 0; i < 6; i++)
	{
		for (uint8_t j = 0; j < 47; j++)
		{
			if (khana[i][j] != 9608)
			{
				printf("\x1b[5;38;5;%sm", getColor(P2));
			}
			else {
				printf("\x1b[0m");
			}
			printf("%lc", khana[i][j]);
		}
		printf("\n");
	}
	//dev/stdout ou /proc/self/fd/1
	// fp = freopen("/proc/self/fd/1", "w+", stdout);

	printf("\x1b[0m");
}

void selectVerticalMenu(int8_t * selected, bool * validate, int8_t lim) {
    char c;
    c = getchar();
    switch(c) {
            case UP:
                (*selected)--;
                break;
            case DOWN:
                (*selected)++;
                break;
            case ENTER:
            case SPACE:
                *validate = true;
                break;
    }
    if (*selected > lim) {
            *selected = 0;
    } else if (*selected < 0) {
            *selected = lim;
    }
}
void selectHorizontalMenu(int8_t * selected, bool * validate, int8_t lim) {
    char c;
    c = getchar();
    switch(c) {
            case LEFT:
                (*selected)--;
                break;
            case RIGHT:
                (*selected)++;
                break;
            case ENTER:
            case SPACE:
                *validate = true;
                break;
    }
    if (*selected > lim) {
            *selected = 0;
    } else if (*selected < 0) {
            *selected = lim;
    }
}
void printVerticalMenu(int8_t selected, const char menu[][BUFFER], uint8_t n) {
    
    for (uint8_t i = 0; i < n; i++) {
            if (i == selected) {
                printf("\x1b[38;5;214m>");
            }
            printf("%s \n", menu[i]);
            printf("\x1b[0m");
    }
}
void printHorizontalMenu(int8_t selected, const char menu[][BUFFER], uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
            if (i == selected) {
                printf("\x1b[38;5;214m>");
            }
            printf("%s ", menu[i]);
            printf("\x1b[0m");
    }
}

int8_t Menu() {
    int8_t selected = 0;
    bool validate = false;
    const char menu[5][BUFFER] = {
        "VS Player",
        "VS IA",
        "Tutorial",
        "Settings",
        "Quit"
    };

    while(!validate) {
        printf("\x1b[s");
        printHorizontalMenu(selected, menu, 5);
        selectHorizontalMenu(&selected, &validate, 4);
        printf("\x1b[u");
	}
    return selected;
}
void Settings(uint8_t *size, int8_t *iSettings) {
    int8_t selected = 0;
	bool validate = false;
    int8_t iMin[3] = {(MINSIZE-(*size))/2, 0, 0};
    int8_t iMax[3] = {(MAXSIZE-(*size))/2, 1, 4};
    char c;
    int8_t temp = *size;
    const char settings[3][BUFFER] = {
        "Size",
        "Difficulty",
        "Theme",
    };
    const char difficulty[2][BUFFER] = {
        "EASY",
        "MEDIUM"
    };
    
    while(!validate) {
		printf("\x1b[H");
        printf("\x1b[2J");
        printf("\x1b[4mSettings :\n");
        printf("\x1b[0m");
		theme = iSettings[2];
        for (uint8_t i = 0; i < 3; i++)
        {
            printf("\t");
            if (i == selected) {
                printf("\x1b[38;5;214m>");
            }
            printf("%s : ", settings[i]);
            switch (i)
            {
            case 0:
                printf("%d\n", temp);
                break;
            case 1:
                printf("%s\n", difficulty[iSettings[i]]);
                break;
            case 2:
				printf("\x1b[%s;%sm P1", FG, getColor(P1));
				printf("\x1b[%s;%sm P2", FG, getColor(P2));
				printf("\x1b[%s;%sm WON", FG, getColor(ACTIVE));
                break;
            }
            printf("\x1b[0m");
        }

        c = getchar();
        switch(c) {
                case LEFT:
                    iSettings[selected]--;
                    break;
                case RIGHT:
                    iSettings[selected]++;
                    break;
                case UP:
                    selected--;
                    break;
                case DOWN:
                    selected++;
                    break;
                case ENTER:
                case SPACE:
                    validate = true;
                    break;
        }
        if (selected > 3) {
            selected = 0;
        } else if (selected < 0) {
            selected = 3;
        }

        if (iSettings[selected] > iMax[selected]) {
            iSettings[selected] = iMin[selected];
        } else if (iSettings[selected] < iMin[selected]) {
            iSettings[selected] = iMax[selected];
        }
        temp = *size + iSettings[0]*2;
    }
    *size = temp;
}
int8_t Tutorial() {
    int8_t selected = 0;
    bool validate = false;
    const char tuto[2][BUFFER] = {
        "Bridge",
        "Encirclement"
    };
	printf("\x1b[H");
	printf("\x1b[2J");
    while(!validate) {
        printf("\x1b[s");
        printVerticalMenu(selected, tuto, 2);
        selectVerticalMenu(&selected, &validate, 1);
        printf("\x1b[u");
	}
    return selected;
}
/*-----------------------------------------------*/
/*==================================================================*/


/*==== Jeu ==================================================*/
/*---- Miscellaneous ----------------------------*/
uint8_t **createMatrix(uint8_t nrows, uint8_t ncolumns) {
	uint8_t **matrix = malloc(nrows * sizeof(uint8_t *));
	if(matrix == NULL) {
			printError("out of memory");
		}
	for(uint8_t i = 0; i < nrows; i++) {
		matrix[i] = malloc(ncolumns * sizeof(uint8_t));
		if(matrix[i] == NULL)
			{
				printError("out of memory");
			}
		}
	return matrix;
}
void freeMatrix(uint8_t **matrix, uint8_t nrows) {
	for (uint8_t i = 0; i < nrows; i++)
	{
		free(matrix[i]);
	}
	free(matrix);
}
void initBoard(uint8_t **board, uint8_t size) {
	uint8_t x, y;
	for (x = 0; x < size; ++x) {
		for (y = 0; y < size; ++y) {
			board[x][y] = INACTIVE;
		}
	}

	// on met les "bases"
    for (x = 1; x < size; x = x + 2) {
		for (y = 0; y < size; y += 2) {
			board[x][y] = P2;
		}
	}
    for (x = 0; x < size; x += 2) {
		for (y = 1; y < size; y += 2) {
			board[x][y] = P1;
		}
	}

	// par defaut la case active est celle en haut a gauche
	board[0][0] = ACTIVE;
}
/*-----------------------------------------------*/

/*---- Mouvement --------------------------------*/
uint8_t moveUp(uint8_t **board, uint8_t size, Position * active, bool diagonal, bool first, int8_t x, int8_t y) {
	uint8_t action = ERR;

	if (diagonal) {
		while (y >= 0 && x < size) {
			if (board[x][y] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[x][y] = ACTIVE;
				active->line = y;
				active->col = x;
				return action = MOVE;
			}
			y--, x++;
		}

		// si on verifie que x ca marchera que dans la deuxieme moitié du tableau
		if (first) {
			if (x == size || y == -1) {
				return action = moveUp(board, size, active, diagonal, SECOND, ++y, --x);
			}
		}

	}
	else {
		while (y >= 0) {	
			if (board[active->col][y] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[active->col][y] = ACTIVE;
				active->line = y;
				return action = MOVE;
			}
			y -= 2;
		}
		if (first) {
			if (y == -2) {
				return action = moveUp(board, size, active, diagonal, SECOND, active->col, size - 1);
			} else if (y == -1) {
				return action = moveUp(board, size, active, diagonal, SECOND, active->col, size - 2);
			}
		}
	}

	return action;
}
uint8_t moveDown(uint8_t **board, uint8_t size, Position * active, bool diagonal, bool first, int8_t x, int8_t y) {
	uint8_t action = ERR;

	if (diagonal) {
		while (y < size && x >= 0) {
			if (board[x][y] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[x][y] = ACTIVE;
				active->line = y;
				active->col = x;
				return action = MOVE;
			}
			y++, x--;
		}

		if (first) {
			if (y == size || x == -1) {
				return action = moveDown(board, size, active, diagonal, SECOND, --y, ++x);
			}
		}

	}
	else {
		while (y < size) {	
			if (board[active->col][y] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[active->col][y] = ACTIVE;
				active->line = y;
				return action = MOVE;
			}
			y += 2;
		}

		if (first) {
			if (y == size + 1) {
				return action = moveDown(board, size, active, diagonal, SECOND, active->col, 0);
			} else if (y == size) {
				return action = moveDown(board, size, active, diagonal, SECOND, active->col, 1);
			}
		}
	}

	return action;
}
uint8_t moveLeft(uint8_t **board, uint8_t size, Position * active, bool diagonal, bool first, int8_t x, int8_t y) {
	uint8_t action = ERR;

	if (diagonal) {
		while (y >= 0 && x >= 0) {
			if (board[x][y] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[x][y] = ACTIVE;
				active->line = y;
				active->col = x;
				return action = MOVE;
			}
			y--, x--;
		}

		if (first) {
			if (y == -1 || x == -1) {
				return action = moveLeft(board, size, active, diagonal, SECOND, size - 2 - y, size - 2 - x);
			}
		}

	}
	else {
		while (x >= 0) {	
			if (board[x][active->line] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[x][active->line] = ACTIVE;
				active->col = x;
				return action = MOVE;
			}
			x -= 2;
		}

		if (first) {
			if (x == -2) {
				return action = moveLeft(board, size, active, diagonal, SECOND, size - 1, active->line);
			} else if (x == -1) {
				return action = moveLeft(board, size, active, diagonal, SECOND, size - 2, active->line);
			}
		}
	}

	return action;
}
uint8_t moveRight(uint8_t **board, uint8_t size, Position * active, bool diagonal, bool first, int8_t x, int8_t y) {
	uint8_t action = ERR;

	if (diagonal) {
		while (y < size && x < size) {
			if (board[x][y] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[x][y] = ACTIVE;
				active->line = y;
				active->col = x;
				return action = MOVE;
			}
			y++, x++;
		}

		if (first) {
			if (y == size || x == size) {
				return action = moveRight(board, size, active, diagonal, SECOND, size - y, size - x);
			}
		}

	}
	else {
		while (x < size) {	
			if (board[x][active->line] == INACTIVE) {
				board[active->col][active->line] = INACTIVE;
				board[x][active->line] = ACTIVE;
				active->col = x;
				return action = MOVE;
			}
			x += 2;
		}

		if (first) {
			if (x == size + 1) {
				return action = moveRight(board, size, active, diagonal, SECOND, 0, active->line);
			} else if (x == size) {
				return action = moveRight(board, size, active, diagonal, SECOND, 1, active->line);
			}
		}
	}

	return action;
}
/*-----------------------------------------------*/

/*---- Joue -------------------------------------*/
bool newActive(uint8_t **board, uint8_t size, Position * active) {
	 // si jamais faux alors la grille est terminer
	for (uint8_t x = 0; x < size; x++)
	{
		for (uint8_t y = 0; y < size; y++)
		{
			if (board[x][y] == INACTIVE)
			{
				active->col = x;
				active->line = y;
				board[x][y] = ACTIVE;
				return true;
			}
		}
	}
	return false;
}
bool bridge(uint8_t **board, uint8_t size, Position * active, uint8_t * player) {
	board[active->col][active->line] = *player;

	return newActive(board, size, active);
}
uint8_t kbInput(char *c, bool *diagonal, uint8_t **board, uint8_t size, Position * active) {
	*c = getchar();
	if (*c < 0){
		printError("\nError! Cannot read keyboard input!");
		exit (EXIT_FAILURE);
	}
	switch(*c) {
		case LEFT:
			return moveLeft(board, size, active, *diagonal, FIRST, active->col, active->line);
		case RIGHT:
			return moveRight(board, size, active, *diagonal, FIRST, active->col, active->line);
		case UP:
			return moveUp(board, size, active, *diagonal, FIRST, active->col, active->line);
		case DOWN:
			return moveDown(board, size, active, *diagonal, FIRST, active->col, active->line);
		case ENTER:
		case SPACE:
			return PLAY;
		case 'x' :
		case 'X' :
			*diagonal = !(*diagonal);
			return MOVE;
		default:
			/*  Obligatoire car quand on fait une action il y a plusieurs   */
			/*  charactere envoyé (par exemple un mouvement c'est ESC puis [ puis A) */
			/*  Donc si on redefinie pas action sur erreur on va garder l'action precedent   */
			/*  jusq'a ce que le cache ce vide   */
			return ERR;
	}
}
/*-----------------------------------------------*/

/*---- Jeu --------------------------------------*/
void aiPlay(uint8_t **board, Position * active, List **adj, uint8_t size, uint8_t n, uint8_t start, uint8_t end, uint8_t nbrNodes) {
	board[active->col][active->line] = INACTIVE;
	uint8_t col = 99;
	uint8_t lin = 99;
	
	// sert juste a faire jolie
	for (uint8_t y = 0; y < size; y++)
	{
		for (uint8_t x = 0; x < size; x++)
		{
			if (board[x][y] == INACTIVE)
			{
				board[x][y] = ACTIVE;
				drawBoard(board, size, P2, false);
				board[x][y] = INACTIVE;
				nanosleep(&request, &remaining);
			}
		}
	}

	// regarde si un joueur est a un coup de gagner
	// on s'arrete que si l'ordi gagne direct
	// si c'est le joueur il faut quand meme continuer
	for (uint8_t y = 0; y < size; y++)
	{
		for (uint8_t x = 0; x < size; x++)
		{
			if (board[x][y] == INACTIVE)
			{
				addBridg(adj, P1, size, n, x, y, !IS_ODD(y), start, end);
				addBridg(adj, P2, size, n, y, x, !IS_ODD(y), start, end);
				if (win(adj, nbrNodes, P1, start, end) || win(adj, nbrNodes, P2, start, end)) {
					col = x;
					lin = y;
					if (win(adj, nbrNodes, P2, start, end)) {
						remBridg(adj, P1, size, n, x, y, !IS_ODD(y), start, end);
						remBridg(adj, P2, size, n, y, x, !IS_ODD(y), start, end);
						active->col = col;
						active->line = lin;
						return;
					}
				}
				remBridg(adj, P1, size, n, x, y, !IS_ODD(y), start, end);
				remBridg(adj, P2, size, n, y, x, !IS_ODD(y), start, end);
			}
		}
	}
	
	// le cas échéant prendre un coup au hasard
	// on ne joue pas aux bords pour eviter de faire de l'antijeux
	if (col != 99) {
		active->col = col;
		active->line = lin;
	}
	else {
		do {
			active->col = rand() % (size - 2) + 1;
			active->line = rand() % size;
		} while(board[active->col][active->line] != INACTIVE);
	}
}

void Game(uint8_t size, bool ai) {
	uint8_t action;
	uint8_t player = P1;
	Position active = { 0, 0 };
	uint8_t **board = createMatrix(size, size);
	bool hasWon = false;
	bool diagonal = false;
	uint8_t n = (size - 1) / 2;
    uint8_t nbrNodes = (n+1)*n + 2;
    const uint8_t start = 0;
    uint8_t end = nbrNodes - 1;
	char c;
	uint8_t center;
	srand(time(NULL));

	List ** adj = malloc(NBRPLAYER*sizeof(List*));
    for (uint8_t i = 0; i < NBRPLAYER; i++)
    {
        adj[i] = malloc(nbrNodes*sizeof(List));
    }
	initAdj(adj, nbrNodes);
	initBoard(board, size);
	
	printf("\x1b[2J");
	drawBoard(board, size, player, diagonal);

	while (!hasWon) {

		/*  On regarde si c'est L'ia ou le joueur 2   */
		if (ai && player == P2) {
			aiPlay(board, &active, adj, size, n, start, end, nbrNodes);
			action = PLAY;
		}
		else {
			action = kbInput(&c, &diagonal, board, size, &active);
		}
        
		/*  On interprete l'action    */
        if (action == MOVE) {
			drawBoard(board, size, player, diagonal);
		}
		if (action == PLAY) {
			board[active.col][active.line] = player;
			if (player == P1) {
				addBridg(adj, player, size, n, active.col, active.line, !IS_ODD(active.line), start, end);
			} else {
				addBridg(adj, player, size, n, active.line, active.col, !IS_ODD(active.line), start, end);
			}
			if (win(adj, nbrNodes, player, start, end)) {
				drawBoard(board, size, player, diagonal);
				printf("\x1b[A");
				center = LENGTH*(2*size + 5) - 17;
				printf("%*s", center/2, "");
				printf("\x1b[%s;%smPlayer %d", FG, getColor(player), 1+player);
				printf("\x1b[0m has ");
				printf("\x1b[5;%s;%smWON!", FG, getColor(ACTIVE));
				printf("\x1b[0m");
				hasWon = true;
			} else {
				player ^= (uint8_t)1; // change de joueur
				newActive(board, size, &active);
				drawBoard(board, size, player, diagonal);
			}
		}
		else if (c == 'q' || c == 'Q') {
			centerPrintf("QUIT? (y/n)", LENGTH*(2*size + 5));
			c = getchar();
			if (c == 'y' || c == 'Y') {
				break;
			}
			drawBoard(board, size, player, diagonal);
		}
		/* else if (c == 'r' || c == 'R') {
			printf("       RESTART? (y/n)       \n");
			c = getchar();
			if (c == 'y' || c == 'Y') {
				freeAdj(adj, nbrNodes);
				List ** adj = initAdj(nbrNodes);
				initBoard(board, size);
				player = P1;
				active.col = active.line = 0;
				printf("\x1b[2J");
			}
			drawBoard(board, size, player, diagonal);
		} */
	}
	putchar('\n');
	centerPrintf("Press any key", LENGTH*(2*size + 5));
	getchar();
	freeMatrix(board, size);
	freeAdj(adj, nbrNodes);
}

void playTutorial(uint8_t tuto) {
	uint8_t action;
	uint8_t size = 5;
	uint8_t player = P1;
	Position active = { 0, 0 };
	uint8_t **board = createMatrix(size, size);
	bool diagonal = false;
	uint8_t n = (size - 1) / 2;
    uint8_t nbrNodes = (n+1)*n + 2;
    const uint8_t start = 0;
    uint8_t end = nbrNodes - 1;
	char c;
	uint8_t center;
	srand(time(NULL));

	List ** adj = malloc(NBRPLAYER*sizeof(List*));
    for (uint8_t i = 0; i < NBRPLAYER; i++)
    {
        adj[i] = malloc(nbrNodes*sizeof(List));
    }
	initAdj(adj, nbrNodes);
	initBoard(board, size);
	switch (tuto) {
	case 0:
		for (uint8_t x = 2, y = 0; y < size; y += 4) {
			board[x][y] = P1;
			addBridg(adj, player, size, n, x, y, !IS_ODD(y), start, end);
		}
		break;
	case 1:
		for (uint8_t x = 0, y = 2; x < size-1; x += 2) {
			board[x][y] = P1;
			addBridg(adj, player, size, n, x, y, !IS_ODD(y), start, end);
		}
		board[1][1] = P1;
		addBridg(adj, player, size, n, 1, 1, !IS_ODD(1), start, end);
		break;
	default:
		break;
	}
	
	printf("\x1b[2J");
	drawBoard(board, size, player, diagonal);

	while (action != PLAY) {
		action = kbInput(&c, &diagonal, board, size, &active);
        
        if (action == MOVE) {
			drawBoard(board, size, player, diagonal);
		}
		if (action == PLAY) {
			board[active.col][active.line] = player;
			addBridg(adj, player, size, n, active.col, active.line, !IS_ODD(active.line), start, end);

			if (win(adj, nbrNodes, player, start, end)) {
				drawBoard(board, size, player, diagonal);
				printf("\x1b[A");
				center = LENGTH*(2*size + 5) - 16;
				printf("%*s", center/2, "");
				printf("\x1b[%s;%smPlayer %d", FG, getColor(player), 1+player);
				printf("\x1b[0m has ");
				printf("\x1b[5;%s;%smWON", FG, getColor(ACTIVE));
				printf("\x1b[0m");
				putchar('\n');
				centerPrintf("It's time to move on to a real challenge!", LENGTH*(2*size + 5));
			} else {
				drawBoard(board, size, player, diagonal);
				putchar('\n');
				centerPrintf("Look's like you need a bit more training...", LENGTH*(2*size + 5));
			}
		}
		else if (c == 'q' || c == 'Q') {
			centerPrintf("QUIT? (y/n)", LENGTH*(2*size + 5));
			c = getchar();
			if (c == 'y' || c == 'Y') {
				break;
			}
			drawBoard(board, size, player, diagonal);
		}
	}
	putchar('\n');
	centerPrintf("Press any key...", LENGTH*(2*size + 5));
	getchar();
	freeMatrix(board, size);
	freeAdj(adj, nbrNodes);
}
/*-----------------------------------------------*/
/*==========================================================*/


int main(int argc, char *argv[]) {
	uint8_t size = 9;
    int8_t iSettings[4] = {0};
    bool quit = false;

	if(argc > 2) {
		printError("number of args invalid");
	}
	if (argc == 2 ) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			printHelp(argv[0]);
		}
	}

    printf("\x1b[?25l\x1b[2J"); // curseur invisible puis efface l'ecran entier
	if (setlocale(LC_CTYPE, "") == NULL)
    {
        printError("setlocale");

    }
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0 || w.ws_row < 15 || w.ws_col < 47) {
		printError("size");
	}

	signal(SIGWINCH, handle_winch);
    signal(SIGINT, handle_interupt);
	setBufferedInput(false);
    
	while(!quit) {
        startScreen();
        switch (Menu()) {
        case 0:
            Game(size, !AI);
			printf("\x1b[2J");
            break;
         case 1:
            Game(size, AI);
			printf("\x1b[2J");
            break;
         case 2:
            playTutorial(Tutorial());
			printf("\x1b[2J");
            break;
        case 3:
            Settings(&size, iSettings);
			printf("\x1b[2J");
            break;
        case 4:
            quit = true;
            break;
        }
    }
	putchar('\n');
	
    renitialize();
    return EXIT_SUCCESS;
}