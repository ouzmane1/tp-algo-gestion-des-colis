#ifndef CAMION_H
#define CAMION_H

#include "colis.h"

#define CAMION_OK          0
#define CAMION_INVALIDE   -1
#define CAMION_MEMOIRE    -2
#define CAMION_TROP_GRAND -3

typedef struct {
    size_t *indices;
    size_t nb;
    long long poids_total;
    long long valeur_totale;
    long long score_total;
} Chargement;

#define BASE_PRIORITE_DEFAUT 4
#define BASE_PRIORITE_MAX    1000

long long score_colis(const Colis *p, int base);

void chargement_free(Chargement *ch);

int camion_dp(const Collection *c, int capacite, Chargement *res);

int camion_dp_priorite(const Collection *c, int capacite, int base, Chargement *res);

int camion_glouton(const Collection *c, int capacite, Chargement *res);

void chargement_afficher(const Collection *c, const Chargement *ch, int capacite);
void chargement_afficher_limite(const Collection *c, const Chargement *ch,
                                int capacite, size_t limite);

#endif