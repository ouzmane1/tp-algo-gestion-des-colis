#include "camion.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tri.h"

#define LIMITE_BITS_OCTETS ((size_t)512 * 1024 * 1024)

static void ch_init(Chargement *ch)
{
    ch->indices = NULL;
    ch->nb = 0;
    ch->poids_total = 0;
    ch->valeur_totale = 0;
    ch->score_total = 0;
}

void chargement_free(Chargement *ch)
{
    free(ch->indices);
    ch_init(ch);
}

static int bit_lire(const unsigned char *b, size_t pos)
{
    return (b[pos >> 3] >> (pos & 7)) & 1;
}

static void bit_poser(unsigned char *b, size_t pos)
{
    b[pos >> 3] |= (unsigned char)(1u << (pos & 7));
}

static int knapsack_dp(const Collection *c, int capacite,
                       const long long *vals, Chargement *res)
{
    ch_init(res);
    if (capacite < 0) return CAMION_INVALIDE;

    size_t n = c->taille;

    long long somme = 0;
    for (size_t i = 0; i < n; i++) somme += c->data[i].poids;
    size_t W = (size_t)capacite;
    if ((long long)W > somme) W = (size_t)somme;
    size_t largeur = W + 1;

    if (n > 0 && largeur > SIZE_MAX / n) return CAMION_TROP_GRAND;
    size_t nbits = n * largeur;
    size_t nbytes = (nbits + 7) / 8;
    if (nbytes > LIMITE_BITS_OCTETS) return CAMION_TROP_GRAND;

    long long *dp = calloc(largeur, sizeof *dp);
    unsigned char *keep = calloc(nbytes ? nbytes : 1, 1);
    size_t *sel = malloc((n ? n : 1) * sizeof *sel);
    if (!dp || !keep || !sel) {
        free(dp); free(keep); free(sel);
        return CAMION_MEMOIRE;
    }

    for (size_t i = 0; i < n; i++) {
        size_t p = (size_t)c->data[i].poids;
        long long v = vals ? vals[i] : c->data[i].valeur;
        for (size_t w = W; w >= p; w--) {
            long long candidat = dp[w - p] + v;
            if (candidat > dp[w]) {
                dp[w] = candidat;
                bit_poser(keep, i * largeur + w);
            }
        }
    }

    size_t k = 0;
    size_t w = W;
    for (size_t i = n; i-- > 0; ) {
        if (bit_lire(keep, i * largeur + w)) {
            sel[k++] = i;
            w -= (size_t)c->data[i].poids;
        }
    }
    for (size_t a = 0, b = k; a + 1 < b; a++, b--) {
        size_t t = sel[a]; sel[a] = sel[b - 1]; sel[b - 1] = t;
    }

    res->indices = sel;
    res->nb = k;
    for (size_t j = 0; j < k; j++) {
        res->poids_total += c->data[sel[j]].poids;
        res->valeur_totale += c->data[sel[j]].valeur;
        res->score_total += vals ? vals[sel[j]] : c->data[sel[j]].valeur;
    }

    free(dp);
    free(keep);
    return CAMION_OK;
}

int camion_dp(const Collection *c, int capacite, Chargement *res)
{
    return knapsack_dp(c, capacite, NULL, res);
}

long long score_colis(const Colis *p, int base)
{
    return (long long)p->valeur * (base + p->priorite - 1);
}

int camion_dp_priorite(const Collection *c, int capacite, int base, Chargement *res)
{
    ch_init(res);
    if (base < 1 || base > BASE_PRIORITE_MAX) return CAMION_INVALIDE;

    long long *scores = malloc((c->taille ? c->taille : 1) * sizeof *scores);
    if (!scores) return CAMION_MEMOIRE;
    for (size_t i = 0; i < c->taille; i++)
        scores[i] = score_colis(&c->data[i], base);

    int r = knapsack_dp(c, capacite, scores, res);
    free(scores);
    return r;
}

int camion_glouton(const Collection *c, int capacite, Chargement *res)
{
    ch_init(res);
    if (capacite < 0) return CAMION_INVALIDE;

    const Colis **t = collection_trier(c, CRIT_RATIO, 1);
    size_t *sel = malloc((c->taille ? c->taille : 1) * sizeof *sel);
    if (!t || !sel) {
        free(t); free(sel);
        return CAMION_MEMOIRE;
    }

    for (size_t i = 0; i < c->taille; i++) {
        if (res->poids_total + t[i]->poids <= capacite) {
            sel[res->nb++] = (size_t)(t[i] - c->data);
            res->poids_total += t[i]->poids;
            res->valeur_totale += t[i]->valeur;
        }
    }
    res->indices = sel;
    free(t);
    return CAMION_OK;
}

void chargement_afficher(const Collection *c, const Chargement *ch, int capacite)
{
    chargement_afficher_limite(c, ch, capacite, 0);
}

void chargement_afficher_limite(const Collection *c, const Chargement *ch,
                                int capacite, size_t limite)
{
    size_t m = (limite > 0 && limite < ch->nb) ? limite : ch->nb;
    colis_afficher_entete();
    for (size_t j = 0; j < m; j++)
        colis_afficher(&c->data[ch->indices[j]]);
    colis_afficher_pied();
    if (m < ch->nb)
        printf("(%zu colis listes sur %zu)\n", m, ch->nb);
    printf("%zu colis charges | poids %lld / %d kg | valeur totale %lld\n",
           ch->nb, ch->poids_total, capacite, ch->valeur_totale);
    if (ch->nb > 0) {
        double somme = 0;
        for (size_t j = 0; j < ch->nb; j++)
            somme += c->data[ch->indices[j]].priorite;
        printf("priorite moyenne du chargement : %.2f\n", somme / (double)ch->nb);
    }
}