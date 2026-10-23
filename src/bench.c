#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "camion.h"
#include "colis.h"
#include "tri.h"

static const size_t TAILLES[] = {100, 1000, 10000, 100000};
#define NB_TAILLES 4

static volatile long long puits;

static double maintenant(void) { return (double)clock() / CLOCKS_PER_SEC; }

static void remplir(Collection *c, size_t n)
{
    srand(12345);
    collection_init(c);
    for (size_t i = 0; i < n; i++)
        collection_ajouter(c, (int)(1 + i * 3), 1 + rand() % 50, rand() % 1000,
                           1 + rand() % 5, "Paris");
}

static const Colis **tri_insertion_poids(const Collection *c)
{
    size_t n = c->taille;
    const Colis **t = malloc((n ? n : 1) * sizeof *t);
    if (!t) return NULL;
    for (size_t i = 0; i < n; i++) t[i] = &c->data[i];
    for (size_t i = 1; i < n; i++) {
        const Colis *x = t[i];
        size_t j = i;
        while (j > 0 && t[j - 1]->poids > x->poids) {
            t[j] = t[j - 1];
            j--;
        }
        t[j] = x;
    }
    return t;
}

int main(void)
{
    printf("# Mesures (clock(), -O2, meilleur temps sur plusieurs repetitions)\n\n");

    printf("## 1. Ajout de n colis (tableau dynamique + table de hachage)\n\n");
    printf("| n | temps total (ms) | par ajout (ns) |\n|---|---|---|\n");
    for (int k = 0; k < NB_TAILLES; k++) {
        size_t n = TAILLES[k];
        int reps = n <= 1000 ? 200 : (n <= 10000 ? 20 : 5);
        double best = 1e9;
        for (int r = 0; r < reps; r++) {
            Collection c;
            double t0 = maintenant();
            remplir(&c, n);
            double dt = maintenant() - t0;
            if (dt < best) best = dt;
            collection_free(&c);
        }
        printf("| %zu | %.3f | %.0f |\n", n, best * 1e3, best / (double)n * 1e9);
    }

    printf("\n## 2. Recherche d'un colis par id : hachage O(1) contre lineaire O(n)\n\n");
    printf("| n | hachage (ns/recherche) | lineaire (ns/recherche) | rapport lineaire/hachage |\n|---|---|---|---|\n");
    for (int k = 0; k < NB_TAILLES; k++) {
        size_t n = TAILLES[k];
        Collection c;
        remplir(&c, n);

        size_t q_hash = 200000;
        size_t q_lin = 50000000 / n;
        if (q_lin > 200000) q_lin = 200000;
        if (q_lin < 50) q_lin = 50;
        int *cibles = malloc(q_hash * sizeof *cibles);
        for (size_t i = 0; i < q_hash; i++)
            cibles[i] = (int)(1 + ((size_t)rand() % n) * 3);

        double best_h = 1e9, best_l = 1e9;
        for (int r = 0; r < 5; r++) {
            long long acc = 0;
            double t0 = maintenant();
            for (size_t i = 0; i < q_hash; i++)
                acc += collection_chercher_id(&c, cibles[i])->id;
            double dt = (maintenant() - t0) / (double)q_hash;
            puits = acc;
            if (dt < best_h) best_h = dt;

            acc = 0;
            t0 = maintenant();
            for (size_t i = 0; i < q_lin; i++)
                acc += collection_chercher_lineaire(&c, cibles[i])->id;
            dt = (maintenant() - t0) / (double)q_lin;
            puits = acc;
            if (dt < best_l) best_l = dt;
        }
        printf("| %zu | %.0f | %.0f | x%.0f |\n", n, best_h * 1e9, best_l * 1e9, best_l / best_h);
        free(cibles);
        collection_free(&c);
    }

    printf("\n## 3. Tri par poids : tri fusion O(n log n) contre tri par insertion O(n^2)\n\n");
    printf("| n | tri fusion (ms) | tri insertion (ms) | rapport insertion/fusion |\n|---|---|---|---|\n");
    for (int k = 0; k < NB_TAILLES; k++) {
        size_t n = TAILLES[k];
        Collection c;
        remplir(&c, n);
        int reps = n <= 1000 ? 100 : (n <= 10000 ? 5 : 2);
        double best_f = 1e9, best_i = 1e9;
        for (int r = 0; r < reps; r++) {
            double t0 = maintenant();
            const Colis **t = collection_trier(&c, CRIT_POIDS, 0);
            double dt = maintenant() - t0;
            puits = t[0]->id;
            free(t);
            if (dt < best_f) best_f = dt;

            t0 = maintenant();
            t = tri_insertion_poids(&c);
            dt = maintenant() - t0;
            puits = t[0]->id;
            free(t);
            if (dt < best_i) best_i = dt;
        }
        printf("| %zu | %.3f | %.3f | x%.0f |\n", n, best_f * 1e3, best_i * 1e3, best_i / best_f);
        collection_free(&c);
    }

    printf("\n## 4a. Sac a dos (programmation dynamique) : evolution avec n, capacite fixe W = 500\n\n");
    printf("| n | temps (ms) | table de reconstruction (Mo) |\n|---|---|---|\n");
    for (int k = 0; k < NB_TAILLES; k++) {
        size_t n = TAILLES[k];
        Collection c;
        remplir(&c, n);
        int reps = n <= 1000 ? 20 : (n <= 10000 ? 5 : 3);
        double best = 1e9;
        for (int r = 0; r < reps; r++) {
            Chargement ch;
            double t0 = maintenant();
            camion_dp(&c, 500, &ch);
            double dt = maintenant() - t0;
            puits = ch.valeur_totale;
            chargement_free(&ch);
            if (dt < best) best = dt;
        }
        printf("| %zu | %.2f | %.2f |\n", n, best * 1e3, (double)n * 501 / 8 / 1e6);
        collection_free(&c);
    }

    printf("\n## 4b. Sac a dos : evolution avec la capacite W, n = 10 000 colis\n\n");
    printf("| W | temps (ms) | table de reconstruction (Mo) |\n|---|---|---|\n");
    {
        Collection c;
        remplir(&c, 10000);
        int caps[] = {100, 1000, 10000, 100000};
        for (int k = 0; k < 4; k++) {
            double best = 1e9;
            int reps = caps[k] <= 1000 ? 5 : 2;
            int code = 0;
            for (int r = 0; r < reps; r++) {
                Chargement ch;
                double t0 = maintenant();
                code = camion_dp(&c, caps[k], &ch);
                double dt = maintenant() - t0;
                puits = ch.valeur_totale;
                chargement_free(&ch);
                if (dt < best) best = dt;
            }
            if (code == CAMION_OK)
                printf("| %d | %.2f | %.2f |\n", caps[k], best * 1e3,
                       10000.0 * (caps[k] + 1) / 8 / 1e6);
            else
                printf("| %d | refuse (code %d) | > limite |\n", caps[k], code);
        }
        collection_free(&c);
    }

    printf("\nsizeof(Colis) = %zu octets, sizeof(Noeud) = %zu octets\n",
           sizeof(Colis), sizeof(Noeud));
    return 0;
}
