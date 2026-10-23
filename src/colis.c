#include "colis.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITE_INITIALE 16
#define BUCKETS_INITIAUX  16

static size_t hash_id(int id, size_t nb_buckets)
{
    uint32_t h = (uint32_t)id * 2654435761u;
    return (size_t)h % nb_buckets;
}

static void liberer_chaines(Noeud **buckets, size_t nb)
{
    for (size_t i = 0; i < nb; i++) {
        Noeud *n = buckets[i];
        while (n) {
            Noeud *suiv = n->suivant;
            free(n);
            n = suiv;
        }
        buckets[i] = NULL;
    }
}

static int table_inserer(Noeud **buckets, size_t nb, int id, size_t index)
{
    Noeud *n = malloc(sizeof *n);
    if (!n) return -1;
    size_t h = hash_id(id, nb);
    n->index = index;
    n->suivant = buckets[h];
    buckets[h] = n;
    return 0;
}

static int table_agrandir(Collection *c)
{
    size_t nb = c->nb_buckets * 2;
    Noeud **nouveau = calloc(nb, sizeof *nouveau);
    if (!nouveau) return -1;

    for (size_t i = 0; i < c->taille; i++) {
        if (table_inserer(nouveau, nb, c->data[i].id, i) != 0) {
            liberer_chaines(nouveau, nb);
            free(nouveau);
            return -1;
        }
    }
    liberer_chaines(c->buckets, c->nb_buckets);
    free(c->buckets);
    c->buckets = nouveau;
    c->nb_buckets = nb;
    return 0;
}

int collection_init(Collection *c)
{
    c->taille = 0;
    c->capacite = CAPACITE_INITIALE;
    c->nb_buckets = BUCKETS_INITIAUX;
    c->data = malloc(c->capacite * sizeof *c->data);
    c->buckets = calloc(c->nb_buckets, sizeof *c->buckets);
    if (!c->data || !c->buckets) {
        free(c->data);
        free(c->buckets);
        c->data = NULL;
        c->buckets = NULL;
        return -1;
    }
    return 0;
}

void collection_free(Collection *c)
{
    if (c->buckets) {
        liberer_chaines(c->buckets, c->nb_buckets);
        free(c->buckets);
    }
    free(c->data);
    c->data = NULL;
    c->buckets = NULL;
    c->taille = c->capacite = c->nb_buckets = 0;
}

int collection_ajouter(Collection *c, int id, int poids, int valeur,
                       int priorite, const char *destination)
{
    if (poids <= 0 || valeur < 0 ||
        priorite < PRIORITE_MIN || priorite > PRIORITE_MAX || !destination)
        return AJOUT_INVALIDE;

    if (collection_chercher_id(c, id))
        return AJOUT_DOUBLON;

    if (c->taille == c->capacite) {
        size_t nouvelle = c->capacite * 2;
        Colis *tmp = realloc(c->data, nouvelle * sizeof *tmp);
        if (!tmp) return AJOUT_MEMOIRE;
        c->data = tmp;
        c->capacite = nouvelle;
    }

    if ((c->taille + 1) * 4 > c->nb_buckets * 3) {
        if (table_agrandir(c) != 0) return AJOUT_MEMOIRE;
    }

    Colis *p = &c->data[c->taille];
    p->id = id;
    p->poids = poids;
    p->valeur = valeur;
    p->priorite = priorite;
    strncpy(p->destination, destination, DEST_MAX - 1);
    p->destination[DEST_MAX - 1] = '\0';

    if (table_inserer(c->buckets, c->nb_buckets, id, c->taille) != 0)
        return AJOUT_MEMOIRE;
    c->taille++;
    return AJOUT_OK;
}

const Colis *collection_chercher_id(const Collection *c, int id)
{
    for (const Noeud *n = c->buckets[hash_id(id, c->nb_buckets)]; n; n = n->suivant)
        if (c->data[n->index].id == id)
            return &c->data[n->index];
    return NULL;
}

const Colis *collection_chercher_lineaire(const Collection *c, int id)
{
    for (size_t i = 0; i < c->taille; i++)
        if (c->data[i].id == id)
            return &c->data[i];
    return NULL;
}

void colis_afficher(const Colis *p)
{
    printf("| %-8d | %6d | %7d | %8d | %-12s |\n",
           p->id, p->poids, p->valeur, p->priorite, p->destination);
}

void colis_afficher_entete(void)
{
    printf("+----------+--------+---------+----------+--------------+\n");
    printf("| %-8s | %-6s | %-7s | %-8s | %-12s |\n",
           "Id", "Poids", "Valeur", "Priorite", "Destination");
    printf("+----------+--------+---------+----------+--------------+\n");
}

void colis_afficher_pied(void)
{
    printf("+----------+--------+---------+----------+--------------+\n");
}

void collection_afficher_limite(const Collection *c, size_t limite)
{
    size_t m = (limite > 0 && limite < c->taille) ? limite : c->taille;
    colis_afficher_entete();
    for (size_t i = 0; i < m; i++)
        colis_afficher(&c->data[i]);
    colis_afficher_pied();
    if (m < c->taille)
        printf("%zu colis affiches sur %zu\n", m, c->taille);
    else
        printf("%zu colis\n", c->taille);
}

void collection_afficher(const Collection *c)
{
    collection_afficher_limite(c, 0);
}