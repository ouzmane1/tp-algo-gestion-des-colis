#include <stdio.h>
#include "colis.h"

int main(void)
{
    Collection c;
    if (collection_init(&c) != 0) return 1;

    collection_ajouter(&c, 101,  8, 20, 2, "Paris");
    collection_ajouter(&c, 102,  3, 10, 5, "Lille");
    collection_ajouter(&c, 103, 12, 40, 1, "Lyon");
    collection_ajouter(&c, 104,  5, 25, 4, "Paris");
    collection_ajouter(&c, 105,  2, 15, 3, "Lille");
    collection_afficher(&c);

    printf("\nDoublon 103      -> %d (attendu %d)\n",
           collection_ajouter(&c, 103, 1, 1, 1, "X"), AJOUT_DOUBLON);
    printf("Poids negatif    -> %d (attendu %d)\n",
           collection_ajouter(&c, 200, -4, 1, 1, "X"), AJOUT_INVALIDE);
    printf("Priorite 9       -> %d (attendu %d)\n",
           collection_ajouter(&c, 201, 4, 1, 9, "X"), AJOUT_INVALIDE);

    const Colis *p = collection_chercher_id(&c, 104);
    printf("\nRecherche 104 : ");
    if (p) colis_afficher(p); else printf("introuvable\n");
    printf("Recherche 999 : %s\n",
           collection_chercher_id(&c, 999) ? "trouve (ERREUR)" : "introuvable (ok)");

    for (int i = 1000; i < 21000; i++)
        collection_ajouter(&c, i, 1 + i % 20, i % 100, 1 + i % 5, "Paris");
    int ok = 1;
    for (int i = 1000; i < 21000; i++) {
        const Colis *a = collection_chercher_id(&c, i);
        const Colis *b = collection_chercher_lineaire(&c, i % 50 == 0 ? i : 1000);
        if (!a || a->id != i || !b) ok = 0;
    }
    printf("\n%zu colis, %zu buckets, coherence hash = %s\n",
           c.taille, c.nb_buckets, ok ? "OK" : "ERREUR");

    collection_free(&c);
    return 0;
}