#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "camion.h"

static long long force_brute(const Collection *c, int cap)
{
    long long best = 0;
    for (unsigned m = 0; m < (1u << c->taille); m++) {
        long long p = 0, v = 0;
        for (size_t i = 0; i < c->taille; i++)
            if (m >> i & 1) { p += c->data[i].poids; v += c->data[i].valeur; }
        if (p <= cap && v > best) best = v;
    }
    return best;
}

static int valide(const Collection *c, const Chargement *ch, int cap)
{
    long long p = 0, v = 0;
    for (size_t j = 0; j < ch->nb; j++) {
        if (ch->indices[j] >= c->taille) return 0;
        for (size_t k = 0; k < j; k++)
            if (ch->indices[k] == ch->indices[j]) return 0;
        p += c->data[ch->indices[j]].poids;
        v += c->data[ch->indices[j]].valeur;
    }
    return p <= cap && p == ch->poids_total && v == ch->valeur_totale;
}

int main(void)
{
    Collection c;
    collection_init(&c);
    Chargement dp, gl;

    collection_ajouter(&c, 1, 8, 20, 1, "A");
    collection_ajouter(&c, 2, 6, 25, 1, "B");
    collection_ajouter(&c, 3, 5, 18, 1, "C");
    collection_ajouter(&c, 4, 4, 14, 1, "D");
    printf("=== Exemple de l'enonce, capacite 10 kg ===\n");
    camion_dp(&c, 10, &dp);
    chargement_afficher(&c, &dp, 10);
    chargement_free(&dp);
    collection_free(&c);

    collection_init(&c);
    collection_ajouter(&c, 1, 10,  60, 1, "X");
    collection_ajouter(&c, 2, 20, 100, 1, "Y");
    collection_ajouter(&c, 3, 30, 120, 1, "Z");
    printf("\n=== Glouton vs DP, capacite 50 kg ===\n");
    camion_glouton(&c, 50, &gl);
    printf("Glouton (meilleur ratio d'abord) :\n");
    chargement_afficher(&c, &gl, 50);
    camion_dp(&c, 50, &dp);
    printf("Programmation dynamique :\n");
    chargement_afficher(&c, &dp, 50);
    int demo = gl.valeur_totale < dp.valeur_totale;
    printf("-> %s\n", demo ? "le glouton n'est PAS optimal ici (160 contre 220)"
                           : "ERREUR : le glouton aurait du etre sous-optimal");
    chargement_free(&gl);
    chargement_free(&dp);
    collection_free(&c);

    srand(7);
    int ok = 1, glouton_sous_optimal = 0, essais = 300;
    for (int t = 0; t < essais; t++) {
        collection_init(&c);
        int n = rand() % 13;
        for (int i = 0; i < n; i++)
            collection_ajouter(&c, i + 1, 1 + rand() % 10, rand() % 40, 1, "X");
        int cap = rand() % 45;

        camion_dp(&c, cap, &dp);
        camion_glouton(&c, cap, &gl);
        long long ref = force_brute(&c, cap);
        if (dp.valeur_totale != ref || !valide(&c, &dp, cap)) ok = 0;
        if (!valide(&c, &gl, cap) || gl.valeur_totale > dp.valeur_totale) ok = 0;
        if (gl.valeur_totale < dp.valeur_totale) glouton_sous_optimal++;
        chargement_free(&dp);
        chargement_free(&gl);
        collection_free(&c);
    }
    printf("\n=== %d tests aleatoires vs force brute ===\n", essais);
    printf("DP optimal et valide : %s\n", ok ? "OK" : "ERREUR");
    printf("Le glouton a ete sous-optimal %d fois sur %d\n", glouton_sous_optimal, essais);

    collection_init(&c);
    camion_dp(&c, 50, &dp);
    printf("\nCollection vide : %zu colis charges\n", dp.nb);
    chargement_free(&dp);
    collection_ajouter(&c, 1, 5, 10, 1, "X");
    printf("Capacite negative -> code %d (attendu %d)\n",
           camion_dp(&c, -1, &dp), CAMION_INVALIDE);
    camion_dp(&c, 0, &dp);
    printf("Capacite 0 : %zu colis charges\n", dp.nb);
    chargement_free(&dp);
    printf("Capacite gigantesque, un seul colis : ");
    camion_dp(&c, 2000000000, &dp);
    printf("%zu colis charges (ok, capacite ramenee a la somme des poids)\n", dp.nb);
    chargement_free(&dp);
    collection_free(&c);

    collection_init(&c);
    for (int i = 0; i < 100000; i++)
        collection_ajouter(&c, i + 1, 1 + rand() % 50, rand() % 1000, 1, "Paris");
    clock_t t0 = clock();
    int r = camion_dp(&c, 500, &dp);
    double ms = 1000.0 * (clock() - t0) / CLOCKS_PER_SEC;
    printf("\n100000 colis, capacite 500 : code %d, %zu colis, valeur %lld, %.0f ms\n",
           r, dp.nb, dp.valeur_totale, ms);
    ok &= (r == 0) && valide(&c, &dp, 500);
    chargement_free(&dp);
    collection_free(&c);

    return ok && demo ? 0 : 1;
}
