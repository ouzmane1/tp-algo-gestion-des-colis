#ifndef COLIS_H
#define COLIS_H

#include <stddef.h>

#define DEST_MAX 32
#define PRIORITE_MIN 1
#define PRIORITE_MAX 5

typedef struct {
    int id;
    int poids;
    int valeur;
    int priorite;
    char destination[DEST_MAX];
} Colis;

typedef struct Noeud {
    size_t index;
    struct Noeud *suivant;
} Noeud;

typedef struct {
    Colis *data;
    size_t taille;
    size_t capacite;
    Noeud **buckets;
    size_t nb_buckets;
} Collection;

#define AJOUT_OK         0
#define AJOUT_DOUBLON   -1
#define AJOUT_INVALIDE  -2
#define AJOUT_MEMOIRE   -3

int  collection_init(Collection *c);
void collection_free(Collection *c);

int  collection_ajouter(Collection *c, int id, int poids, int valeur,
                        int priorite, const char *destination);

const Colis *collection_chercher_id(const Collection *c, int id);

const Colis *collection_chercher_lineaire(const Collection *c, int id);

void colis_afficher_entete(void);
void colis_afficher_pied(void);
void colis_afficher(const Colis *p);
void collection_afficher(const Collection *c);
void collection_afficher_limite(const Collection *c, size_t limite);

#endif