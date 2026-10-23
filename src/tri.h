#ifndef TRI_H
#define TRI_H

#include "colis.h"

typedef enum {
    CRIT_POIDS,
    CRIT_VALEUR,
    CRIT_PRIORITE,
    CRIT_RATIO
} Critere;

const Colis **collection_trier(const Collection *c, Critere crit, int decroissant);

int collection_afficher_trie(const Collection *c, Critere crit, int decroissant);

int collection_afficher_trie_limite(const Collection *c, Critere crit,
                                    int decroissant, size_t limite);

#endif