#include <stdio.h>
#include <stdlib.h>
#include "camion.h"

static long long brute_score(const Collection *c, int cap, int base)
{
    long long best = 0;
    for (unsigned m = 0; m < (1u << c->taille); m++) {
        long long p = 0, s = 0;
        for (size_t i = 0; i < c->taille; i++)
            if (m >> i & 1) { p += c->data[i].poids; s += score_colis(&c->data[i], base); }
        if (p <= cap && s > best) best = s;
    }
    return best;
}

static double prio_moyenne(const Collection *c, const Chargement *ch)
{
    if (!ch->nb) return 0;
    double s = 0;
    for (size_t j = 0; j < ch->nb; j++) s += c->data[ch->indices[j]].priorite;
    return s / (double)ch->nb;
}

int main(void)
{
    Collection c;
    collection_init(&c);
    Chargement a, b;

    collection_ajouter(&c, 1, 8, 20, 5, "A");
    collection_ajouter(&c, 2, 6, 25, 1, "B");
    collection_ajouter(&c, 3, 5, 18, 4, "C");
    collection_ajouter(&c, 4, 4, 14, 3, "D");

    printf("=== Sans priorite (valeur brute), capacite 10 kg ===\n");
    camion_dp(&c, 10, &a);
    chargement_afficher(&c, &a, 10);

    printf("\n=== Avec priorite (base = %d), capacite 10 kg ===\n", BASE_PRIORITE_DEFAUT);
    camion_dp_priorite(&c, 10, BASE_PRIORITE_DEFAUT, &b);
    chargement_afficher(&c, &b, 10);
    printf("score total optimise : %lld\n", b.score_total);

    int different = (a.nb != b.nb) || (a.valeur_totale != b.valeur_totale);
    chargement_free(&a);
    chargement_free(&b);

    printf("\n=== Effet du parametre base (capacite 10 kg) ===\n");
    int bases[] = {1, 2, 4, 10, 1000};
    for (int k = 0; k < 5; k++) {
        camion_dp_priorite(&c, 10, bases[k], &b);
        printf("base %4d : valeur reelle %2lld, priorite moyenne %.2f, colis :",
               bases[k], b.valeur_totale, prio_moyenne(&c, &b));
        for (size_t j = 0; j < b.nb; j++) printf(" %s", c.data[b.indices[j]].destination);
        printf("\n");
        chargement_free(&b);
    }
    collection_free(&c);

    srand(11);
    int ok = 1, essais = 300;
    double somme_prio_plain = 0, somme_prio_pri = 0;
    long long val_plain = 0, val_pri = 0;
    for (int t = 0; t < essais; t++) {
        collection_init(&c);
        int n = 1 + rand() % 12;
        for (int i = 0; i < n; i++)
            collection_ajouter(&c, i + 1, 1 + rand() % 10, rand() % 40, 1 + rand() % 5, "X");
        int cap = 5 + rand() % 40;
        int base = 1 + rand() % 8;

        camion_dp(&c, cap, &a);
        camion_dp_priorite(&c, cap, base, &b);

        if (b.score_total != brute_score(&c, cap, base)) ok = 0;
        if (b.poids_total > cap) ok = 0;
        if (b.valeur_totale > a.valeur_totale) ok = 0;
        somme_prio_plain += prio_moyenne(&c, &a);
        somme_prio_pri += prio_moyenne(&c, &b);
        val_plain += a.valeur_totale;
        val_pri += b.valeur_totale;

        chargement_free(&a);
        chargement_free(&b);
        collection_free(&c);
    }
    printf("\n=== %d tests aleatoires ===\n", essais);
    printf("Optimal pour le score (vs force brute), capacite respectee : %s\n", ok ? "OK" : "ERREUR");
    printf("Priorite moyenne des chargements : %.2f sans priorite -> %.2f avec priorite\n",
           somme_prio_plain / essais, somme_prio_pri / essais);
    printf("Valeur totale cumulee           : %lld sans priorite -> %lld avec priorite (%.1f %%)\n",
           val_plain, val_pri, 100.0 * (double)val_pri / (double)val_plain - 100.0);

    collection_init(&c);
    collection_ajouter(&c, 1, 5, 10, 3, "X");
    printf("\nbase 0    -> code %d (attendu %d)\n", camion_dp_priorite(&c, 10, 0, &b), CAMION_INVALIDE);
    printf("base 2000 -> code %d (attendu %d)\n", camion_dp_priorite(&c, 10, 2000, &b), CAMION_INVALIDE);
    collection_free(&c);

    return ok && different ? 0 : 1;
}