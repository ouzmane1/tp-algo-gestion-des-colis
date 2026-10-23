#include <stdio.h>
#include <stdlib.h>
#include "tri.h"

static int verifier(const Collection *c, Critere crit, int decr)
{
    const Colis **t = collection_trier(c, crit, decr);
    if (!t) return 0;
    int ok = 1;
    for (size_t i = 1; i < c->taille && ok; i++) {
        int a, b;
        switch (crit) {
            case CRIT_POIDS:  a = t[i-1]->poids;    b = t[i]->poids;    break;
            case CRIT_VALEUR: a = t[i-1]->valeur;   b = t[i]->valeur;   break;
            default:          a = t[i-1]->priorite; b = t[i]->priorite; break;
        }
        if (decr ? a < b : a > b) ok = 0;
        if (a == b && t[i-1] > t[i]) ok = 0;
    }
    free(t);
    return ok;
}

int main(void)
{
    Collection c;
    if (collection_init(&c) != 0) return 1;

    collection_ajouter(&c, 101,  8, 20, 2, "Paris");
    collection_ajouter(&c, 102,  3, 10, 5, "Lille");
    collection_ajouter(&c, 103, 12, 40, 1, "Lyon");
    collection_ajouter(&c, 104,  5, 25, 4, "Paris");
    collection_ajouter(&c, 105,  2, 15, 3, "Lille");

    printf("--- Par poids (croissant) ---\n");
    collection_afficher_trie(&c, CRIT_POIDS, 0);
    printf("\n--- Par valeur (decroissant) ---\n");
    collection_afficher_trie(&c, CRIT_VALEUR, 1);
    printf("\n--- Par priorite (decroissant : 5 = la plus urgente) ---\n");
    collection_afficher_trie(&c, CRIT_PRIORITE, 1);

    srand(42);
    for (int i = 0; i < 100000; i++)
        collection_ajouter(&c, 1000 + i, 1 + rand() % 50, rand() % 500,
                           1 + rand() % 5, "Paris");

    int ok = 1;
    for (int crit = CRIT_POIDS; crit <= CRIT_PRIORITE; crit++)
        for (int decr = 0; decr <= 1; decr++)
            ok &= verifier(&c, (Critere)crit, decr);
    printf("\n%zu colis : 3 criteres x 2 sens, tri correct et stable = %s\n",
           c.taille, ok ? "OK" : "ERREUR");

    Collection vide;
    collection_init(&vide);
    printf("Collection vide : %s\n",
           collection_afficher_trie(&vide, CRIT_POIDS, 0) == 0 ? "ok" : "ERREUR");
    collection_free(&vide);

    collection_free(&c);
    return ok ? 0 : 1;
}
