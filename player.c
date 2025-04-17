#include <stdbool.h> // bool, true, false
#include <stdlib.h>  // rand
#include <stdio.h>   // printf
#include "lode_runner.h" // types and prototypes used to code the game

// global declarations used by the game engine
extern const char BOMB;  // ascii used for the bomb
extern const char BONUS;  // ascii used for the bonuses
extern const char CABLE;  // ascii used for the cable
extern const char ENEMY;  // ascii used for the ennemies
extern const char EXIT;   // ascii used for the exit
extern const char FLOOR;  // ascii used for the floor
extern const char LADDER; // ascii used for the ladder
extern const char PATH;   // ascii used for the pathes
extern const char RUNNER; // ascii used for runner
extern const char WALL;   // ascii used for the walls

extern const int BOMB_TTL; // time to live for bombs

extern bool DEBUG; // true if and only if the game runs in debug mode

const char *students = "LE BRONNEC Elias"; // replace Radom with the student names here

// local prototypes (add your own prototypes below)

struct list
{
  int* tab;
  int taille;
  int debut;
  int fin;
};

typedef struct list file;

void print_action(action);

//Primitives de file
file* init_file(int);
void print_file(file*);
bool est_vide(file*);
void liberer_file(file*);
void enfiler(file*, int);
void defiler(file*, int*);
int file_len(file*);

//Gestion de coordonnées
int from_coord_to_int(levelinfo, int, int);
int get_x(levelinfo, int);
int get_y(levelinfo, int);
action vect_to_dir(int, int);
int distance_m(int, int, int, int);

void print_int_tab(int*, int);
void print_int_matrice(int**, int, int);
void show_prec(levelinfo, int**);
void show_explored(int**, int, int, char, int, char, char, const int);
void show_path(levelinfo, int**, int, int, int);
void print_tab_mobs(levelinfo, int*, int);

//Gestion des entitées
character get_player(character_list);
bool est_hostile(levelinfo, int*, int*, int, int, int);
bool est_bomb(bomb_list, int, int);
int nb_mobs(character_list);
int* get_tab_mobs(levelinfo, character_list);
int nearest_enemy(levelinfo, int*, int*, int, int, int);

//Traitement des cases
bool est_correct(char);
bool est_bonus(bonus_list, int, int);
int nb_bonus(bonus_list);
bool explorable(levelinfo, int*, int*, int, int**, int, int);
int defendable(levelinfo, bomb_list, character_list, int*, int*, int, int, int);
bool free_fall(levelinfo, bomb_list, int*, int*, int, int, int);

int search_next_case(levelinfo, int**, int, int, int);
int fall(levelinfo, bomb_list, int*, int*, int, int, int);

int process_enemy(levelinfo, int, int, int);
action bomb_dir(levelinfo, bomb_list, character_list, int, int);
action fuite_verticale(levelinfo, bomb_list, int*, int, int, int);
action get_dir(levelinfo, character_list, bonus_list, bomb_list);

//Permet de créer une file
file* init_file(int taille){
  /*
  La file créée est vide donc le début et la fin se confondent. Et donc pas besoin d'initialiser les valeurs du tableau après l'avoir alloué.
  */
  file* f = malloc(sizeof(file));
  f->tab = malloc(sizeof(int)*taille);
  f->taille = taille; // capacité maximale du tableau
  f->debut = 0;
  f->fin = 0;
  return f;
}

//Permet d'afficher le contenu de la file
void print_file(file* f){
  /*
  On utilise un tableau circulaire, donc on peut se trouver dans deux situations. Soit les valeurs affectées sont entre le debut et la fin.
  Ou soit la fin est après le debut donc les valeurs de la file sont du debut jusqu'à la capacité maximale du tableau puis du début du tableau jusqu'à la fin.
  */
  printf("[");
  if(f->debut <= f->fin){//Premier cas
    for(int i = f->debut; i < f->fin; i++){
      printf(" %d,", f->tab[i]);
    }
  }else{//Second cas
    for(int i = f->debut; i < f->taille; i++){//Debut de la file jusqu'à la capacité maximale
      printf(" %d,", f->tab[i]);
    }
    for(int i = 0; i < f->fin; i++){//Debut tu tableau jusqu'à la fin de la file
      printf(" %d,", f->tab[i]);
    }
  }
  printf("]\n");
}

//Vérifie si la file est vide
bool est_vide(file* file){
  //Le seul cas de file vide avec un tableau circulaire est le suivant:
  return file->debut == file->fin;
}

//Libère la file
void liberer_list(file* f){
  /*
  On libère le tableau alloué pour la file puis le pointeur vers la file
  */
  if(f != NULL){
    if(f->tab != NULL){
      free(f->tab);
    }
    free(f);
  }
}

//Enfile un élément à la file
void enfiler(file* f, int n){
  /*
  La prochaine case vide disponible est la case de position fin puis on incrémente fin.
  */
  f->tab[f->fin] = n;
  f->fin = (f->fin + 1) % f->taille;//Il y a un modulo car on est dans un tableau circulaire.
}

//Defile un élément de la file
void defiler(file* f, int* val){
  /*
  L'élément en tête de file est à la position debut. Puis on incrémente cet indice de position.
  */
  if(!est_vide(f)){
    *val = f->tab[f->debut];//La valeur défilé est attribué au paramètre passé par adresse
    f->debut = (f->debut + 1) % f->taille;//Incrémentation dans un tableau circulaire
  }
}

//Attribue un unique entier à chaque couple de coordonnées possible
int from_coord_to_int(levelinfo level, int x, int y){
  /*
  Cet unique entier dépend de la hauteur du niveau, donc on a besoin de level.
  */
  return x * level.ysize + y;
}

//Obtient l'abscisse d'une case à partir de son numero
int get_x(levelinfo level, int n){
  return n / level.ysize;//Ici c'est une division entière
}

//Obtient l'ordonnée d'une case à partir de son numero
int get_y(levelinfo level, int n){
  return n % level.ysize;
}

//Affiche le contenu d'un tableau d'entiers de taille size
void print_int_tab(int* tab, int size){
  printf("[");
  for(int i = 0; i < size; i++){//On parcours le tableau sans dépasser sa taille
    printf(" %i,", tab[i]);
  }
  printf("]\n");
}

//Affiche une matrice nxm d'entiers
void print_int_matrice(int** mat, int n, int m){
  printf("n = %i / m = %i\n", n, m);
  for(int i = 0; i < m; i++){
    print_int_tab(mat[i], n);//Chaque ligne est un tableau d'entier de taille m
  }
}

//Affiche la matrice d'entiers des prédecesseurs
void show_prec(levelinfo level, int** prec){
  /*
  Pour chaque case de la matrice on affiche les coordonnées correspondant au numéro de la case du prédecesseurs s'il existe
  */
  int x,y,n;
  printf("PREC");
  for(int i = 0; i < level.ysize; i++){
    printf("\n%i\n", i);
    for(int j = 0; j < level.xsize; j++){
      n = prec[i][j];
      if(n >= 0){
        x = get_x(level, prec[i][j]);
        y = get_y(level, prec[i][j]);
        printf("(%i %i)", x, y);
      }else if(n == -2){
        printf("(départ)");//Le départ n'a pas de prédecesseur
      }else{
        printf("(- -)");//D'autres cases que le départ n'ont pas non plus de prédecesseur
      }

    }
  }
  printf("\n");
}

//Vérifie si le type de la case permet au runner d'y être
bool est_correct(char block){
  /*
  Il ne faut pas oublier d'autoriser la sortie en type de case sinon on s'interdit de gagner
  */
  return block == (char) PATH || block == (char) LADDER || block == (char) CABLE || block == (char) EXIT;
}

//Affiche la matrice des cases déjà explorée
void show_explored(int** mat, int n, int m, char common, int n_common, char explored, char start, const int n_start){
  /*
  On prend en entrée une matrice quelconque et au lieu d'afficher le contenu de chacune de ses cases on le remplace par des caractères spéciaux.
  */
  for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      if(mat[i][j] == n_common){
        printf("%c", common);
      }else if(mat[i][j] == n_start){
        printf("%c", start);
      }else{
        printf("%c", explored);
      }
    }
    printf("\n");
  }
}

//Affiche le chemin construit par le parcours en largeur
void show_path(levelinfo level, int** prec, int xmax, int ymax, int m){
  /*
  D'abord on construit une matrice mettant en évidence le chemin à l'aide du tableau des prédecesseurs puis on applique la fonction précédente à la matrice nouvellement construite.
  */
  int** mat = malloc(sizeof(int*)*ymax);
  for(int i = 0; i < ymax; i++){
    mat[i] = malloc(sizeof(int)*xmax);
    for(int j = 0; j < xmax; j++){
      mat[i][j] = 0;
    }
  }
  int a, b, last_a, last_b;
  a = get_x(level, m);
  b = get_y(level, m);
  mat[b][a] = 2;
  while(prec[get_y(level, m)][get_x(level, m)] != -2){
    m = prec[b][a];
    last_a = a;
    last_b = b;
    a = get_x(level, m);
    b = get_y(level, m);
    mat[b][a] = 1;
  }
  a = last_a;
  b = last_b;
  show_explored(mat, xmax, ymax, '.', 0, '*', 'x', 2);
  for(int i = 0; i < ymax; i++){
    free(mat[i]);
  }
  free(mat);
}

//Renvoie la longueur de la file
int file_len(file* f){
  /*
  On distingue deux situations liées aux tableaux circulaires.
  */
  if(f->fin > f->debut){//Cas 1
    return f->fin - f->debut;
  }
  return f->fin + f->taille - f->debut;//Cas 2
}

//Renvoie l'objet character lié au runner
character get_player(character_list characterl){
  /*
  On effectue un parcours de liste chainée sur characterl jusqu'à obtenir l'objet character lié au runner.
  */
  character_list cl = characterl;
  while(cl != NULL){
    if(cl->c.item == RUNNER){
      return cl->c;
    }
    cl = cl->next;
  }
  character c = {RUNNER, -1, -1, NONE};//Cette ligne évite un warning et ne sera jamais atteinte.
  return c;
}

//Vérifie si une case comporte un ennemi
bool est_hostile(levelinfo level, int* mobs, int* last_mobs, int n_mobs, int x, int y){
  /*
  On vérifie si les coordonnées en paramètre correspondent à une case comportant un ennemi.
  On regarde dans le tableau de la position des ennemis mobs, puis dans le tableau last_mobs de leur deuxième position probable.
  */
  int a,b,c,d;
  for(int i = 0; i < n_mobs; i++){
    a = get_x(level, mobs[i]);
    b = get_y(level, mobs[i]);
    c = get_x(level, last_mobs[i]);
    d = get_y(level, last_mobs[i]);
    if((a ==x && b == y) || (c == x && d == y)){
      return true;
    }
  }
  return false;
}

//Vérifie si une case comporte une bombe
bool est_bomb(bomb_list bl, int x, int y){
  /*
  On effectue un parcours de liste chainée sur bl, si on obtient un objet bombe dont les coordonnées correspondent à celles en entrée on renvoie true.
  Si on n'obtient jamais cet objet bombe, on renvoie false.
  */
  bomb_list bltmp = bl;
  while(bltmp != NULL){
    if(bltmp->x == x && bltmp->y == y){
      return true;
    }
    bltmp = bltmp->next;
  }
  return false;
}

//Vérifie si une case comporte un bonus
bool est_bonus(bonus_list bonusl, int x, int y){
  /*
  On effectue un parcous de liste chainée sur bonusl, si on obtient un objet bonus dont les coordonnées correspondent à celles en entrée on renvoie true.
  Si on n'obtient jamais cet objet bonus, on renvoie false.
  */
  bonus_list p = bonusl;
  while(p != NULL){
    if(p->b.x == x && p->b.y == y){
      return true;
    }
    p = p->next;
  }
  return false;
}

//Renvoie le nombre de bonus restants
int nb_bonus(bonus_list bonusl){
  /*
  On effectue un parcous de liste chainée sur bonusl, et à chaque nouvel objet bonus on incrémente le compteur n initialement à 0.
  Puis on renvoie ce compteur.
  */
  bonus_list p = bonusl;
  int n = 0;
  while(p != NULL){
    n++;
    p = p->next;
  }
  return n;
}

//Renvoie l'action correspondant à un vecteur de déplacement
action vect_to_dir(int x, int y){
  /*
  Pour les mouvement left et right, on ne regarde pas la valeur de y car un mouvement latéral peut entraîner une chute libre.
  */
  if(x==1){
    return RIGHT;
  }
  if(x==-1){
    return LEFT;
  }
  if(y==-1){
    return UP;
  }
  if(x==0 && y>=1){
    return DOWN;
  }
  return NONE;
}

//Renvoie le nombre d'ennemis
int nb_mobs(character_list charl){
  /*
  On effectue un parcous de liste chainée sur charl, et à chaque nouvel objet character on incrémente le compteur n initialement à 0.
  Puis on renvoie ce compteur.
  */
  int n = 0;
  character_list p = charl;
  while(p != NULL){
    if(p->c.item == ENEMY){
      n++;
    }
    p = p->next;
  }
  return n;
}

//Renvoie la distance de manhattan à l'ennemi le plus proche
int nearest_enemy(levelinfo level, int* mobs, int* last_mobs, int n_mobs, int x, int y){
  /*
  On initialise une distance de manhattan minimum courante m avec sa valeur maximale qu'elle peut prendre. Puis on parcours les deux tableaux des ennemis.
  A chaque ennemi on calcul sa distance de manhattan à la position en entrée et si elle est plus petite que m, on met à jour m.
  Puis on renvoie m.
  */
  int m = level.xsize + level.ysize;
  int a, b, c, d;
  for(int i = 0; i < n_mobs; i++){
    a = get_x(level, mobs[i]);
    b = get_y(level, mobs[i]);
    c = get_x(level, last_mobs[i]);
    d = get_y(level, last_mobs[i]);
    if(distance_m(x, a, y, b) < m){
      m = distance_m(x, a, y, b);
    }
    if(distance_m(x, c, y, d) < m){
      m = distance_m(x, c, y, d);
    }
  }
  return m;
}

//Convertit la liste chainée des entités en tableau d'ennemis
int* get_tab_mobs(levelinfo level, character_list charl){
  /*
  On effectue le parcours de la liste chainée charl et à chaque ennemi parcouru, on l'ajoute dans le tableau des ennemis puis on le renvoie.
  */
  int len = nb_mobs(charl);
  int* mobs = malloc(sizeof(int)*len);
  int i = 0;
  character_list p = charl;
  while(p != NULL){
    if(p->c.item == ENEMY){
      mobs[i] = from_coord_to_int(level, p->c.x, p->c.y);
      i++;
    }
    p = p->next;
  }
  return mobs;
}

//Vérifie si une case est visitable par un parcours en largeur
bool explorable(levelinfo level, int* mobs, int* last_mobs, int n_mobs, int** prec, int x, int y){
  /*
  On vérifie d'abord que le runner peut se positionner dans cette case, puis que cette case n'ait jamais été visité et enfin qu'aucun ennemi ne s'y trouve.
  */
  return est_correct(level.map[y][x]) && prec[y][x] == -1 && !est_hostile(level, mobs, last_mobs, n_mobs, x, y);
}

//Effectue la régression du tableau des prédecesseurs afin de trouver la case suivant la position de départ d'un parcours en largeur.
int search_next_case(levelinfo level, int** prec, int x, int y, int terminal){
  /*
  On parcours récursivement le tableau des prédecesseurs jusqu'à ce que l'ancêtre de l'ancêtre de la case courante soit l'entier terminal.
  */
  int n = from_coord_to_int(level, x, y);
  int last_x = x;
  int last_y = y;
  while(prec[get_y(level, n)][get_x(level, n)] != terminal){
    n = prec[y][x];
    last_x = x;
    last_y = y;
    x = get_x(level, n);
    y = get_y(level, n);
  }
  return from_coord_to_int(level, last_x, last_y);
}

//Simule le comportement des ennemis et renvoie la position suivant la position courante
int process_enemy(levelinfo level, int case_enemy, int player_x, int player_y){
  /*
  On effectue un parcours en largeur du niveau avec comme position de départ celle de l'ennemi simulé et comme position d'arrivée celle du joueur.
  On s'autorise tous les déplacement qu'un ennemi peut faire, c'est-à-dire tous les déplacements du joueur sauf la chute libre.
  Pour tester si une case adjacente est visitable, on regarde si le type de la case permet à l'ennemi de s'y trouver,
  si la case n'a pas déjà été visité et enfin si la nature de la case courante permet ce déplacement.
  */
  int x, y, n;
  x = get_x(level, case_enemy);
  y = get_y(level, case_enemy);
  char left, right, up, down, current;
  file* file = init_file(level.xsize * level.ysize * 4);//file des cases à visiter
  int** prec = malloc(sizeof(int*)*level.ysize);
  for(int i = 0; i < level.ysize; i++){
    prec[i] = malloc(sizeof(int)*level.xsize);
    for(int j = 0; j < level.xsize; j++){
      prec[i][j] = -1;
    }
  }
  prec[y][x] = -2;//La première case n'a pas de prédecesseur
  enfiler(file, from_coord_to_int(level, x, y));//On veut visiter la première case
  while(!est_vide(file)){
    defiler(file, &n);
    x = get_x(level, n);//abscisse courante de la case visité
    y = get_y(level, n);//ordonnee courante de la case visité
    if(x == player_x && y == player_y){//La case visité est celle du joueur
      break;//On arrête le parcours
    }
    //La case visité et les 4 cases adjacentes:
    current = level.map[y][x];
    left = level.map[y][x-1];
    right = level.map[y][x+1];
    up = level.map[y-1][x];
    down = level.map[y+1][x];
    //Gauche
    if(est_correct(left) && prec[y][x-1] == -1 && (down == FLOOR || current == LADDER || up == CABLE || down == LADDER)){//Test de visitabilité à gauche
      enfiler(file, from_coord_to_int(level, x-1, y));
      prec[y][x-1] = n;
    }
    //Droite
    if(est_correct(right) && prec[y][x+1] == -1 && (down == FLOOR || current == LADDER || up == CABLE || down == LADDER)){//Test de visitabilité à droite
      enfiler(file, from_coord_to_int(level, x+1, y));
      prec[y][x+1] = n;
    }
    //Haut
    if(est_correct(up) && prec[y-1][x] == -1 && (current == LADDER)){//Test de visitabilité en haut
      enfiler(file, from_coord_to_int(level, x, y-1));
      prec[y-1][x] = n;
    }
    //Bas
    if(est_correct(down) && prec[y+1][x] == -1 && down != PATH){//Test de visitabilité en bas
      enfiler(file, from_coord_to_int(level, x, y+1));
      prec[y+1][x] = n;
    }
  }
  //On applique une régression du tableau des prédecesseurs
  int m = n;
  int a = x;
  int b = y;
  int last_a, last_b;
  while(prec[get_y(level, m)][get_x(level, m)] != -2){
    m = prec[b][a];
    last_a = a;
    last_b = b;
    a = get_x(level, m);
    b = get_y(level, m);
  }
  liberer_list(file);
  for(int i = 0; i < level.ysize; i++){
    free(prec[i]);
  }
  free(prec);
  return from_coord_to_int(level, last_a, last_b);//On renvoie l'action permettant d'aller à la case suivante
}

//Renvoie l'action permettant de fuire verticalement lorsque nécessaire et si c'est possible.
action fuite_verticale(levelinfo level, bomb_list bombl, int* mobs, int n_mobs, int player_x, int player_y){
  /*
  On vérifie si un ennemi est adjacent au joueur. Si c'est le cas, on vérifié s'il est possible d'aller en haut et on s'y rend.
  Puis si c'est possible d'aller en bas puis on s'y rend.
  Sinon on reste sur place.
  */
  if(nearest_enemy(level, mobs, mobs, n_mobs, player_x, player_y) == 1){
    if(!est_hostile(level, mobs, mobs, n_mobs, player_x, player_y-1) && level.map[player_y][player_x] == LADDER){
      return UP;
    }
    if(!est_hostile(level, mobs, mobs, n_mobs, player_x, player_y+1) && est_correct(level.map[player_y+1][player_x])){
      return DOWN;
    }
  }
  return NONE;
}

//Renvoie l'action permettant de se défendre et souvent avec une bombe.
action bomb_dir(levelinfo level, bomb_list bl, character_list characterl, int player_x, int player_y){
  /*
  D'abord on regarde à l'altitude du joueur, la distance à gauche et à droite avec l'ennemi le plus proche.
  Ensuite à l'aide de fuite_verticale on fuit verticalement si un ennemi nous est adjacent et s'il est possible de fuire.
  Sinon on fuit latéralement dans la direction opposé à l'ennemi adjacent s'il existe.
  Sinon on pose une bombe en direction de l'ennemi le plus proche s'il est suffisamment proche.
  Soit on pose une bombe car une case nous sépare d'un ennemi.
  Soit on pose une bombe car deux cases nous sépare d'un ennemi et que le trou permet de créer un chemin.
  */
  int l = level.xsize;
  int r = level.xsize;
  int* mobs = get_tab_mobs(level, characterl);
  int n_mobs = nb_mobs(characterl);
  character_list cl = characterl;
  action a;
  while(cl != NULL){
    if(cl->c.item == ENEMY){
      if(cl->c.y == player_y){
        if(cl->c.x < player_x && player_x - cl->c.x < l){
          l = player_x - cl->c.x;
        }
        if(cl->c.x > player_x && cl->c.x - player_x < r){
          r = cl->c.x - player_x;
        }
      }
    }
    cl = cl->next;
  }
  a = fuite_verticale(level, bl, mobs, n_mobs, player_x, player_y);
  free(mobs);
  if(r == 1){//fuite latérale gauche
    return LEFT;
  }
  if(l == 1){//fuite latérale droite
    return RIGHT;
  }
  if(a != NONE){
    return a;
  }
  if(r <= 2 || (r == 3 && est_correct(level.map[player_y+2][player_x+1]))){//Défense par bombe droite
    if(!est_bomb(bl, player_x+1, player_y+1) && level.map[player_y+1][player_x+1] == FLOOR && level.map[player_y][player_x+1 == PATH]){
      return BOMB_RIGHT;
    }
  }
  if(l <= 2 || (l == 3 && est_correct(level.map[player_y+2][player_x-1]))){//défense par bombe gauche
    if(!est_bomb(bl, player_x-1, player_y+1) && level.map[player_y+1][player_x-1] == FLOOR && level.map[player_y][player_x-1 == PATH]){
      return BOMB_LEFT;
    }
  }
  return NONE;
}

//Renvoie le maximum de deux entiers
int max(int a, int b){
  if(a > b){
    return a;
  }else{
    return b;
  }
}

//Calcul le score de défendabilité d'une case
int defendable(levelinfo level, bomb_list bombl, character_list characterl, int* mobs, int* last_mobs, int n_mobs, int x, int y){
  /*
  On regarde à gauche et à droite sur combien de cases s'étend une plateforme n'ayant que des cases de type FLOOR au sol et des cases de type PATH à la hauteur du joueur.
  Le tout sans ennemi ni bombe. On conserve ces valeurs dans r et l.
  Si cette plateforme n'existe pas, alors le score est nulle, car non défendable.
  Si l'un des entier est égale à 1, disons sans perte de généralité r.
  Alors ça veut dire qu'il n'y a qu'une case qui sépare le joueur d'un ennemi à droite, ce qui est idéal pour poser une bombe.
  Et la plateforme s'étend de l cases à gauche ce qui permet de reculer après avoir posé une bombe à droite.
  Dans cette situation on renvoie une valeur élevé et croissante à la valeur supérieur à 1. Renvoyer 2*l est raisonnable.
  Dans le dernier cas, on renvoie la plus grande valeur entre r et l.
  Dans cette dernière situation, le runner est en sécurité mais ne peut pas défendre sa position.
  */
  int l = 1;
  int r = 1;
  bool bombable = true;
  while(bombable){
    if(level.map[y][x+r] == PATH && level.map[y+1][x+r] == FLOOR && !est_bomb(bombl, x+r, y+1) && !est_hostile(level, mobs, last_mobs, n_mobs, x+r, y)){
      r++;
    }else{
      bombable = false;
    }
  }
  r--;
  bombable = true;
  while(bombable){
    if(level.map[y][x-l] == PATH && level.map[y+1][x-l] == FLOOR && !est_bomb(bombl, x-l, y+1) && !est_hostile(level, mobs, last_mobs, n_mobs, x-l, y)){
      l++;
    }else{
      bombable = false;
    }
  }
  l--;
  if(r==0 || l ==0){//Cas critique
    return 0;
  }
  if(est_hostile(level, mobs, last_mobs, n_mobs, x-2, y) && l == 1){//Deuxième cas
    return 2*r;
  }
  if(est_hostile(level, mobs, last_mobs, n_mobs, x+2, y) && r == 1){//idem
    return 2*l;
  }
  return max(r, l);//dernier cas
}

//Vérifie si le joueur est dans une situation de chute libre
bool free_fall(levelinfo level, bomb_list bombl, int* mobs, int* last_mobs, int n_mobs, int player_x, int player_y){
  /*
  On vérifie toutes les conditions sur les case adjacentes permettant de dire si le joueur est en chute libre
  */
  //On récupère les casesa adjacentes
  char current = level.map[player_y][player_x];
  char down = level.map[player_y+1][player_x];
  char up = level.map[player_y-1][player_x];
  return up != CABLE && current != LADDER && (down == PATH || (down == FLOOR && est_bomb(bombl, player_x, player_y+1) && !est_hostile(level, mobs, last_mobs, n_mobs, player_x, player_y+1)));
}

//Renvoie en cas de chute libre la nouvelle position du runner après la chute
int fall(levelinfo level, bomb_list bombl, int* mobs, int* last_mobs, int n_mobs, int player_x, int player_y){
  /*
  Tant que la case courante met le runner dans un état de chute libre, on prend la case du dessous.
  Dès qu'on obtient une case qui ne met pas le runner dans un état de chute libre, on renvoie son ordonnée.
  Si parmis les cases parcourus il y a une case comportant un ennemi, on renvoie -1, ce qui écarte cette chute
  libre des déplacements possibles.
  */
  int y = player_y;
  while(free_fall(level, bombl, mobs, last_mobs, n_mobs, player_x, y)){
    if(est_hostile(level, mobs, last_mobs, n_mobs, player_x, y+1)){
      return -1;
    }
    y++;
  }
  return y;
}

//Affiche les coordonnées des ennemis
void print_tab_mobs(levelinfo level, int* mobs, int n_mobs){
  /*
  On parcours les numéro correspondant aux cases des ennemis et on obtient les coordonnées qui correspondent puis on les affiche.
  */
  int x,y;
  printf("[");
  for(int i = 0; i < n_mobs; i++){
    x = get_x(level, mobs[i]);
    y = get_y(level, mobs[i]);
    printf("(%i | %i), ", x, y);
  }
  printf("]\n");
}

int distance_m(int xa, int xb, int ya, int yb){
  return abs(xa - xb) + abs(ya - yb);
}

//Renvoie la prochaine action que le runner doit faire pour trouver un bonus, la porte ou pour se protéger
action get_dir(levelinfo level, character_list characterl, bonus_list bonusl, bomb_list bombl){
  /*
  Cette fonction est un parcous en largeur qui ressemble au parcours en largeur des ennemis.
  A la différence qu'ici on détermine aussi la case la plus facile à défendre, que les objectifs cherchés ne sont pas les mêmes,
  qu'on simule les ennemis et qu'on s'autorise les chutes libres.

  Pour déterminer la case la plus facile à défendre, à chaque fois qu'on visite une nouvelle case, on calcul sont score de défendabilité
  et s'il dépasse le score de défendabilité courrant maximum defense_score, on met à jour ce maximum et on conserve le numéro de case
  correspondante dans n_defend.

  Ici on arrête la boucle while dès lors que la case visité correspond à un bonus ou à une porte. Et on assigne la valeur true à trouve pour indiquer que la
  boucle s'est arrêté car un objectif a été trouvé.

  Pour simuler les ennemis on a d'abord un entier profondeur qui indique la profondeur du parcours en largeur et un entier largeur qui indique le nombre de
  cases qu'il reste à visiter dans la file avant d'atteindre la profondeur suivante. On simule les ennemis lors d'une incrémentation de profondeur sur deux.
  Pour ça on a un booléen qui détermine si les ennemis jouent à cette profondeur. Ne connaissant pas la valeur de ce booléen en profondeur 0, on le considère
  initialement à la valeur true et à mobs on attribue la valeur des positions des ennemis selon cette hypothèse. Et à last_mobs on attribue la valeur de la
  position des ennemis selon l'hypothèse opposé, c'est-à-dire si le booléen indiquant le tour des ennemis était sur false.

  A chaque fois qu'on visite une case, après avoir regardé si le déplacement vers une case adjacente était licite, on regarde si ce déplacement entraine une
  chute libre, si ce n'est pas le cas on traite la case comme dans la simulation du comportement des ennemis, et si c'est le cas on regarde la trajectoire de
  la chute libre et s'il n'y a pas d'ennemis sur cette trajectoire ou d'ennemis adjacent à la case de fin de chute, on ajoute cette case à la file, sinon
  on ne la considère pas.
  */
  int n, x, y;
  int ea = 1;
  int n_defend = -1;
  int defense_score = -1;
  bool trouve = false;
  char up, down, current;
  int profondeur = 0;
  int largeur = 1;
  int* mobs = get_tab_mobs(level, characterl);
  int n_mobs = nb_mobs(characterl);
  int* last_mobs = malloc(sizeof(int)*n_mobs);
  file* file = init_file(level.xsize * level.ysize * 4);

  int** prec = malloc(sizeof(int*)*level.ysize);
  for(int i = 0; i < level.ysize; i++){
    prec[i] = malloc(sizeof(int)*level.xsize);
    for(int j = 0; j < level.xsize; j++){
      prec[i][j] = -1;
    }
  }

  character player = get_player(characterl);
  prec[player.y][player.x] = -2;
  enfiler(file, from_coord_to_int(level, player.x, player.y));
  int d;
  while(!est_vide(file)){
    defiler(file, &n);
    largeur--;
    x = get_x(level, n);
    y = get_y(level, n);
    d = defendable(level, bombl, characterl, mobs, last_mobs, n_mobs, x, y);
    if(d > defense_score){
      defense_score = d;
      n_defend = n;
    }
    if(est_bonus(bonusl, x, y) || (nb_bonus(bonusl) == 0 && x == level.xexit && y == level.yexit)){
      trouve = true;
      break;
    }
    current = level.map[y][x];
    up = level.map[y-1][x];
    down = level.map[y+1][x];

    if(largeur==0){
      if(ea && !free_fall(level, bombl, mobs, last_mobs, n_mobs, x, y)){
        for(int i = 0; i < n_mobs; i++){
          last_mobs[i] = mobs[i];
          mobs[i] = process_enemy(level, mobs[i], x, y);
        }
      }
      ea = (ea + 1)%2;
      largeur = file_len(file);
      profondeur++;
    }

    //Gauche
    int fy;
    if(explorable(level, mobs, last_mobs, n_mobs, prec, x-1, y) && ((down == FLOOR && (!est_bomb(bombl, x-1, y+1) || est_hostile(level, mobs, last_mobs, n_mobs, x-1, y+1))) || current == LADDER || up == CABLE || down == LADDER)){
      if(free_fall(level, bombl, mobs, last_mobs, n_mobs, x-1, y)){//La case mène à une chute libre
        fy = fall(level, bombl, mobs, last_mobs, n_mobs, x-1, y);
        if(fy != -1 && explorable(level, mobs, last_mobs, n_mobs, prec, x-1, fy) && nearest_enemy(level, mobs, last_mobs, n_mobs, x-1, fy) > 1){//Cette chute libre n'est pas dangereuse
          enfiler(file, from_coord_to_int(level, x-1, fy));
          prec[fy][x-1] = n;
        }
      }else{//Il n'y a pas de chute libre, on traite la case normalement
        enfiler(file, from_coord_to_int(level, x-1, y));
        prec[y][x-1] = n;
      }
    }
    //Droite
    if(explorable(level, mobs, last_mobs, n_mobs, prec, x+1, y) && ((down == FLOOR && (!est_bomb(bombl, x+1, y+1) || est_hostile(level, mobs, last_mobs, n_mobs, x+1, y+1))) || current == LADDER || up == CABLE || down == LADDER)){
      if(free_fall(level, bombl, mobs, last_mobs, n_mobs, x+1, y)){//La case mène à une chute libre
        fy = fall(level, bombl, mobs, last_mobs, n_mobs, x+1, y);
        if(fy != -1 && explorable(level, mobs, last_mobs, n_mobs, prec, x+1, fy) && nearest_enemy(level, mobs, last_mobs, n_mobs, x+1, fy) > 1){//Cette chute libre n'est pas dangereuse
          enfiler(file, from_coord_to_int(level, x+1, fy));
          prec[fy][x+1] = n;
        }
      }else{//Il n'y a pas de chute libre, on traite la case normalement
        enfiler(file, from_coord_to_int(level, x+1, y));
        prec[y][x+1] = n;
      }
      
    }
    //Haut
    //Un mouvement vers le haut ne peut pas entrainer une chute libre
    if(explorable(level, mobs, last_mobs, n_mobs, prec, x, y-1) && (current == LADDER)){
      enfiler(file, from_coord_to_int(level, x, y-1));
      prec[y-1][x] = n;
    }
    //Bas
    if(explorable(level, mobs, last_mobs, n_mobs, prec, x, y+1)){
      if(free_fall(level, bombl, mobs, last_mobs, n_mobs, x, y+1)){//La case mène à une chute libre
        fy = fall(level, bombl, mobs, last_mobs, n_mobs, x, y+1);
        if(fy != -1 && explorable(level, mobs, last_mobs, n_mobs, prec, x, fy) && nearest_enemy(level, mobs, last_mobs, n_mobs, x, fy) > 1){//Cette chute libre n'est pas dangereuse
          enfiler(file, from_coord_to_int(level, x, fy));
          prec[fy][x] = n;
        }
      }else{//Il n'y a pas de chute libre, on traite la case normalement
        enfiler(file, from_coord_to_int(level, x, y+1));
        prec[y+1][x] = n;
      }
    }
  }
  /*
  Si un chemin vers un objectif a été trouvé, on renvoie l'action menant à la première case de ce chemin.
  Sinon on renvoie la première action menant à la case accessible la plus facile à défendre.
  Si cette case n'existe pas on ne renvoie rien.
  */
  action def = NONE;
  if(n_defend != -1){
    int next_case = search_next_case(level, prec, get_x(level, n_defend), get_y(level, n_defend), -2);
    def = vect_to_dir(get_x(level, next_case) - player.x, get_y(level, next_case) - player.y);
  }
  int a,b;
  if(trouve){
    int m = search_next_case(level, prec, x, y, -2);
    a = get_x(level, m);
    b = get_y(level, m);
  }
  for(int i = 0; i < level.ysize; i++){
    free(prec[i]);
  }
  free(prec);
  liberer_list(file);
  free(mobs);
  free(last_mobs);
  if(trouve){
    return vect_to_dir(a - player.x, b - player.y);
  }else{
    return def;
  }
}
/* 
  function to code: it may use as many modules (functions and procedures) as needed
  Input (see lode_runner.h for the type descriptions): 
    - level provides the information for the game level
    - characterl is the linked list of all the characters (runner and enemies)
    - bonusl is the linked list of all the bonuses that have not been collected yet
    - bombl is the linked list of all the bombs that are still active
  Output
    - the action to perform
*/


//Renvoie l'action que le runner doit faire en fonction de l'état actuel du jeu
action lode_runner(levelinfo level, character_list characterl, bonus_list bonusl, bomb_list bombl){
  /*
  On regarde s'il existe un chemin vers un bonus, une porte ou une position facile à défendre en appelant get_dir.
  S'il n'y en a pas, alors on se défend en appelant bomb_dir, ce qui va soit poser une bombe ou fuire.
  Si aucune des actions précédentes n'est possible alors on ne fait rien.
  */
  character player = get_player(characterl);
  action a = get_dir(level, characterl, bonusl, bombl);
  if(a == NONE){
    a = bomb_dir(level, bombl, characterl, player.x, player.y);
    if(a == NONE){
      return NONE;
    }
  }
  return a;
}

action lode_runner_random(
  levelinfo level,
  character_list characterl,
  bonus_list bonusl,
  bomb_list bombl
  )
{
  action a; // action to choose and then return
  bool ok; // boolean to control the do while loops
  
  int x; // runner's x position
  int y; // runner's y position

  character_list pchar=characterl; // iterator on the character list

  // looking for the runner ; we know s.he is in the list
  ok=false; // ok will become true when the runner will be found
  do
  { 
    if(pchar->c.item==RUNNER) // runner found
    {
      x=pchar->c.x; 
      y=pchar->c.y;
      ok=true;
    }
    else // otherwise move on next character
      pchar=pchar->next;
  } while(!ok);

  ok=false; // ok will become true when a valid action will be guessed
  do
  {
    a = rand() % 7; // randomly guess a integer between 0 and 6 as we have 7 possible actions
    switch (a)
    {
    case NONE:
      ok = true; // it's always possible, though often useless, to do nothing ;-)
      break;
    case UP:
      if (level.map[y][x] == LADDER)
        ok = true; // it's possible to go up if on a ladder
      break;
    case DOWN:
      if (level.map[y + 1][x] == LADDER || level.map[y + 1][x] == PATH)
        ok = true; // it's possible to go down if there is a ladder or nothing (jump) below 
      break;
    case LEFT:
      if (level.map[y][x - 1] != WALL && level.map[y][x - 1] != FLOOR)
        ok = true; // it's possible to go left if there is no wall or floor
      break;
    case RIGHT:
      if (level.map[y][x + 1] != WALL && level.map[y][x + 1] != FLOOR)
        ok = true; // it's possible to go right if there is no wall or floor
      break;
    case BOMB_LEFT: 
      if (level.map[y + 1][x - 1] == FLOOR && level.map[y][x - 1] == PATH)
        ok = true; // it's possible to bomb left if there is some floor that can be destroyed
      break;
    case BOMB_RIGHT:
      if (level.map[y + 1][x + 1] == FLOOR && level.map[y][x + 1] == PATH)
        ok = true; // it's possible to bomb right if there is some floor that can be destroyed
      break;
    }
    if(DEBUG) // only when the game is in debug mode
    {
      printf("[Player] Candidate action ");
      print_action(a);
      if(ok) 
        printf(" is valid"); 
      else 
        printf(" not valid");
      printf(".\n");
    }
  } while (!ok);

  return a; // action to perform
}

/*
  Procedure that print the action name based on its enum type value
  Input:
    - the action a to print
*/
void print_action(action a)
{
  switch (a)
  {
  case NONE:
    printf("NONE");
    break;
  case UP:
    printf("UP");
    break;
  case DOWN:
    printf("DOWN");
    break;
  case LEFT:
    printf("LEFT");
    break;
  case RIGHT:
    printf("RIGHT");
    break;
  case BOMB_LEFT:
    printf("BOMB_LEFT");
    break;
  case BOMB_RIGHT:
    printf("BOMB_RIGHT");
    break;
  }
}