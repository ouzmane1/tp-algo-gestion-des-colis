#include "tri.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*Comparateur)(const Colis *, const Colis *);

static int cmp_poids(const Colis *a, const Colis *b)
{
    return (a->poids > b->poids) - (a->poids < b->poids);
}

static int cmp_valeur(const Colis *a, const Colis *b)
{
    return (a->valeur > b->valeur) - (a->valeur < b->valeur);
}

static int cmp_priorite(const Colis *a, const Colis *b)
{
    return (a->priorite > b->priorite) - (a->priorite < b->priorite);
}

static int cmp_ratio(const Colis *a, const Colis *b)
{
    long long x = (long long)a->valeur * b->poids;
    long long y = (long long)b->valeur * a->poids;
    return (x > y) - (x < y);
}

static void tri_fusion(const Colis **t, const Colis **tmp, size_t n,
                       Comparateur cmp, int decroissant)
{
    if (n < 2) return;

    size_t m = n / 2;
    tri_fusion(t, tmp, m, cmp, decroissant);
    tri_fusion(t + m, tmp, n - m, cmp, decroissant);

    size_t i = 0, j = m, k = 0;
    while (i < m && j < n) {
        int c = cmp(t[i], t[j]);
        if (decroissant) c = -c;
        tmp[k++] = (c <= 0) ? t[i++] : t[j++];
    }
    while (i < m) tmp[k++] = t[i++];
    while (j < n) tmp[k++] = t[j++];

    memcpy(t, tmp, n * sizeof *t);
}

const Colis **collection_trier(const Collection *c, Critere crit, int decroissant)
{
    Comparateur cmp;
    switch (crit) {
        case CRIT_POIDS:    cmp = cmp_poids;    break;
        case CRIT_VALEUR:   cmp = cmp_valeur;   break;
        case CRIT_PRIORITE: cmp = cmp_priorite; break;
        case CRIT_RATIO:    cmp = cmp_ratio;    break;
        default: return NULL;
    }

    size_t n = c->taille;
    const Colis **t   = malloc((n ? n : 1) * sizeof *t);
    const Colis **tmp = malloc((n ? n : 1) * sizeof *tmp);
    if (!t || !tmp) {
        free(t);
        free(tmp);
        return NULL;
    }

    for (size_t i = 0; i < n; i++)
        t[i] = &c->data[i];

    tri_fusion(t, tmp, n, cmp, decroissant);
    free(tmp);
    return t;
}

int collection_afficher_trie_limite(const Collection *c, Critere crit,
                                    int decroissant, size_t limite)
{
    const Colis **t = collection_trier(c, crit, decroissant);
    if (!t) return -1;

    size_t m = (limite > 0 && limite < c->taille) ? limite : c->taille;
    colis_afficher_entete();
    for (size_t i = 0; i < m; i++)
        colis_afficher(t[i]);
    colis_afficher_pied();
    if (m < c->taille)
        printf("%zu colis affiches sur %zu\n", m, c->taille);
    else
        printf("%zu colis\n", c->taille);

    free(t);
    return 0;
}

int collection_afficher_trie(const Collection *c, Critere crit, int decroissant)
{
    return collection_afficher_trie_limite(c, crit, decroissant, 0);
}