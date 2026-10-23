#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "camion.h"
#include "colis.h"
#include "tri.h"

#define LIMITE_DEFAUT 30
#define MAX_DEST      256

typedef struct {
    Collection col;
    Chargement charge;
    int charge_valide;
    int charge_capacite;
    char charge_mode[64];
} Etat;

static Etat g;

static void nettoyer(void)
{
    chargement_free(&g.charge);
    collection_free(&g.col);
}

static void fin_entree(void)
{
    printf("\n(fin de l'entree : arret du programme)\n");
    nettoyer();
    exit(0);
}

static int lire_ligne(char *buf, size_t taille)
{
    if (!fgets(buf, (int)taille, stdin)) return 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[--len] = '\0';
    else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    while (len > 0 && (buf[len - 1] == '\r' || isspace((unsigned char)buf[len - 1])))
        buf[--len] = '\0';
    return 1;
}

static long lire_entier(const char *invite, long min, long max, int has_def, long def)
{
    char buf[128];
    for (;;) {
        printf("%s", invite);
        fflush(stdout);
        if (!lire_ligne(buf, sizeof buf)) fin_entree();
        if (buf[0] == '\0' && has_def) return def;
        char *fin;
        errno = 0;
        long v = strtol(buf, &fin, 10);
        if (fin == buf || *fin != '\0' || errno == ERANGE) {
            printf("  Entree invalide : tapez un nombre entier.\n");
            continue;
        }
        if (v < min || v > max) {
            printf("  Valeur hors limites (%ld a %ld).\n", min, max);
            continue;
        }
        return v;
    }
}

static void lire_texte(const char *invite, char *buf, size_t taille)
{
    for (;;) {
        printf("%s", invite);
        fflush(stdout);
        if (!lire_ligne(buf, taille)) fin_entree();
        if (buf[0] != '\0') return;
        printf("  Le texte ne peut pas etre vide.\n");
    }
}

static size_t demander_limite(size_t n)
{
    if (n <= LIMITE_DEFAUT) return 0;
    char invite[128];
    snprintf(invite, sizeof invite,
             "  %zu lignes : combien afficher ? (Entree = %d, 0 = toutes) : ", n, LIMITE_DEFAUT);
    return (size_t)lire_entier(invite, 0, LONG_MAX, 1, LIMITE_DEFAUT);
}

static void action_afficher(void)
{
    if (g.col.taille == 0) {
        printf("Aucun colis. Utilisez l'option 2 (ajouter) ou 8 (jeu de test).\n");
        return;
    }
    collection_afficher_limite(&g.col, demander_limite(g.col.taille));
}

static const char *message_ajout(int code)
{
    switch (code) {
        case AJOUT_OK:       return "Colis ajoute.";
        case AJOUT_DOUBLON:  return "Refuse : cet identifiant existe deja.";
        case AJOUT_INVALIDE: return "Refuse : donnees invalides.";
        default:             return "Erreur : memoire insuffisante.";
    }
}

static void action_ajouter(void)
{
    char dest[DEST_MAX];
    long id = lire_entier("  Identifiant (entier >= 1) : ", 1, INT_MAX, 0, 0);
    long poids = lire_entier("  Poids en kg (entier >= 1) : ", 1, 1000000, 0, 0);
    long valeur = lire_entier("  Valeur (entier >= 0) : ", 0, 1000000000, 0, 0);
    long prio = lire_entier("  Priorite (1 a 5, 5 = la plus urgente) : ",
                            PRIORITE_MIN, PRIORITE_MAX, 0, 0);
    lire_texte("  Destination : ", dest, sizeof dest);
    int r = collection_ajouter(&g.col, (int)id, (int)poids, (int)valeur, (int)prio, dest);
    printf("%s\n", message_ajout(r));
}

static void action_rechercher(void)
{
    if (g.col.taille == 0) {
        printf("Aucun colis.\n");
        return;
    }
    long id = lire_entier("  Identifiant a rechercher : ", INT_MIN, INT_MAX, 0, 0);
    const Colis *p = collection_chercher_id(&g.col, (int)id);
    if (!p) {
        printf("Colis %ld introuvable.\n", id);
        return;
    }
    colis_afficher_entete();
    colis_afficher(p);
    colis_afficher_pied();
}

static void action_organiser(void)
{
    if (g.col.taille == 0) {
        printf("Aucun colis.\n");
        return;
    }
    printf("  Trier selon :\n    1. le poids\n    2. la valeur\n    3. la priorite\n");
    long choix = lire_entier("  Votre choix : ", 1, 3, 0, 0);
    Critere crit = choix == 1 ? CRIT_POIDS : (choix == 2 ? CRIT_VALEUR : CRIT_PRIORITE);
    long sens_defaut = (crit == CRIT_PRIORITE) ? 2 : 1;
    printf("  Sens :\n    1. croissant\n    2. decroissant%s\n",
           crit == CRIT_PRIORITE ? " (les plus urgents d'abord)" : "");
    long sens = lire_entier("  Votre choix (Entree = defaut) : ", 1, 2, 1, sens_defaut);
    size_t limite = demander_limite(g.col.taille);

    clock_t t0 = clock();
    int r = collection_afficher_trie_limite(&g.col, crit, sens == 2, limite);
    double ms = 1000.0 * (double)(clock() - t0) / CLOCKS_PER_SEC;
    if (r != 0)
        printf("Erreur : memoire insuffisante pour trier.\n");
    else
        printf("(tri fusion, stable, O(n log n) : %.2f ms, affichage compris)\n", ms);
}

static void erreur_camion(int code)
{
    switch (code) {
        case CAMION_INVALIDE:
            printf("Erreur : parametres invalides.\n"); break;
        case CAMION_TROP_GRAND:
            printf("Refuse : capacite trop grande (la table de calcul depasserait la limite de memoire).\n"
                   "         Essayez une capacite plus petite.\n"); break;
        default:
            printf("Erreur : memoire insuffisante.\n");
    }
}

static double prio_moyenne(const Collection *c, const Chargement *ch)
{
    if (ch->nb == 0) return 0.0;
    double s = 0;
    for (size_t j = 0; j < ch->nb; j++) s += c->data[ch->indices[j]].priorite;
    return s / (double)ch->nb;
}

static const char *nom_mode(int mode)
{
    switch (mode) {
        case 1:  return "Valeur maximale (programmation dynamique)";
        case 2:  return "Valeur ponderee par la priorite (programmation dynamique)";
        default: return "Glouton valeur/poids (rapide, non optimal)";
    }
}

static int calculer(int mode, int cap, int base, Chargement *out, double *ms)
{
    clock_t t0 = clock();
    int r;
    switch (mode) {
        case 1:  r = camion_dp(&g.col, cap, out); break;
        case 2:  r = camion_dp_priorite(&g.col, cap, base, out); break;
        default: r = camion_glouton(&g.col, cap, out); break;
    }
    *ms = 1000.0 * (double)(clock() - t0) / CLOCKS_PER_SEC;
    return r;
}

static void action_camion(void)
{
    if (g.col.taille == 0) {
        printf("Aucun colis a charger.\n");
        return;
    }
    long cap = lire_entier("  Capacite maximale du camion en kg : ", 0, INT_MAX, 0, 0);
    printf("  Methode :\n"
           "    1. Valeur maximale (programmation dynamique, optimale)\n"
           "    2. Avec priorites (programmation dynamique, score = valeur x coefficient)\n"
           "    3. Glouton valeur/poids (rapide, pas toujours optimal)\n"
           "    4. Comparer les trois methodes\n");
    long mode = lire_entier("  Votre choix : ", 1, 4, 0, 0);

    int base = BASE_PRIORITE_DEFAUT;
    if (mode == 2 || mode == 4) {
        char invite[160];
        snprintf(invite, sizeof invite,
                 "  Parametre base de la priorite (1 = priorite tres forte ... %d = negligeable, Entree = %d) : ",
                 BASE_PRIORITE_MAX, BASE_PRIORITE_DEFAUT);
        base = (int)lire_entier(invite, 1, BASE_PRIORITE_MAX, 1, BASE_PRIORITE_DEFAUT);
    }

    if (mode == 4) {
        printf("\n  %-34s %6s %9s %10s %9s %9s\n", "Methode", "colis", "poids", "valeur", "prio moy", "temps ms");
        for (int m = 1; m <= 3; m++) {
            Chargement ch;
            double ms;
            int r = calculer(m, (int)cap, base, &ch, &ms);
            if (r != CAMION_OK) {
                printf("  %-34s ", nom_mode(m));
                erreur_camion(r);
                continue;
            }
            const char *court = m == 1 ? "1. Valeur maximale (DP)"
                              : m == 2 ? "2. Avec priorites (DP)"
                                       : "3. Glouton valeur/poids";
            printf("  %-34s %6zu %9lld %10lld %9.2f %9.2f\n", court, ch.nb,
                   ch.poids_total, ch.valeur_totale, prio_moyenne(&g.col, &ch), ms);
            chargement_free(&ch);
        }
        printf("\n  (La comparaison n'enregistre pas de chargement : relancez avec une methode 1, 2 ou 3.)\n");
        return;
    }

    Chargement ch;
    double ms;
    int r = calculer((int)mode, (int)cap, base, &ch, &ms);
    if (r != CAMION_OK) {
        erreur_camion(r);
        return;
    }
    chargement_free(&g.charge);
    g.charge = ch;
    g.charge_valide = 1;
    g.charge_capacite = (int)cap;
    snprintf(g.charge_mode, sizeof g.charge_mode, "%s", nom_mode((int)mode));

    printf("\nMethode : %s\n", g.charge_mode);
    chargement_afficher_limite(&g.col, &g.charge, g.charge_capacite,
                               demander_limite(g.charge.nb));
    printf("(calcule en %.2f ms)\n", ms);
}

typedef struct {
    char nom[DEST_MAX];
    size_t nb;
    long long poids, valeur;
} DestStat;

static void cumuler_destination(DestStat *tab, size_t *nd, const Colis *p)
{
    size_t i;
    for (i = 0; i < *nd; i++)
        if (strcmp(tab[i].nom, p->destination) == 0) break;
    if (i == *nd) {
        if (*nd == MAX_DEST) i = MAX_DEST - 1;
        else {
            snprintf(tab[i].nom, DEST_MAX, "%s", p->destination);
            tab[i].nb = 0; tab[i].poids = 0; tab[i].valeur = 0;
            (*nd)++;
        }
    }
    tab[i].nb++;
    tab[i].poids += p->poids;
    tab[i].valeur += p->valeur;
}

static void afficher_destinations(const DestStat *tab, size_t nd)
{
    printf("  %-14s %8s %12s %12s\n", "Destination", "colis", "poids", "valeur");
    for (size_t i = 0; i < nd; i++)
        printf("  %-14s %8zu %12lld %12lld\n", tab[i].nom, tab[i].nb, tab[i].poids, tab[i].valeur);
}

static void action_chargement(void)
{
    if (!g.charge_valide) {
        printf("Aucun chargement calcule. Utilisez d'abord l'option 5.\n");
        return;
    }
    printf("Methode : %s\n", g.charge_mode);
    chargement_afficher_limite(&g.col, &g.charge, g.charge_capacite,
                               demander_limite(g.charge.nb));

    static DestStat dest[MAX_DEST];
    size_t nd = 0;
    for (size_t j = 0; j < g.charge.nb; j++)
        cumuler_destination(dest, &nd, &g.col.data[g.charge.indices[j]]);
    if (nd > 0) {
        printf("\nRepartition du chargement par destination :\n");
        afficher_destinations(dest, nd);
    }
    printf("Taux de remplissage : %.1f %%\n",
           g.charge_capacite > 0 ? 100.0 * (double)g.charge.poids_total / g.charge_capacite : 0.0);
}

static void action_stats(void)
{
    const Collection *c = &g.col;
    if (c->taille == 0) {
        printf("Aucun colis.\n");
        return;
    }
    long long pt = 0, vt = 0;
    size_t par_prio[PRIORITE_MAX + 1] = {0};
    static DestStat dest[MAX_DEST];
    size_t nd = 0;
    const Colis *lourd = &c->data[0], *precieux = &c->data[0];

    for (size_t i = 0; i < c->taille; i++) {
        const Colis *p = &c->data[i];
        pt += p->poids;
        vt += p->valeur;
        par_prio[p->priorite]++;
        if (p->poids > lourd->poids) lourd = p;
        if (p->valeur > precieux->valeur) precieux = p;
        cumuler_destination(dest, &nd, p);
    }
    double n = (double)c->taille;

    printf("===== STATISTIQUES =====\n");
    printf("Nombre de colis   : %zu\n", c->taille);
    printf("Poids             : total %lld kg, moyen %.2f kg\n", pt, (double)pt / n);
    printf("Valeur            : totale %lld, moyenne %.2f\n", vt, (double)vt / n);
    printf("Colis le plus lourd    : id %d (%d kg, destination %s)\n",
           lourd->id, lourd->poids, lourd->destination);
    printf("Colis le plus precieux : id %d (valeur %d, destination %s)\n",
           precieux->id, precieux->valeur, precieux->destination);

    printf("\nRepartition par priorite :\n");
    for (int p = PRIORITE_MAX; p >= PRIORITE_MIN; p--)
        printf("  priorite %d : %8zu colis (%5.1f %%)\n", p, par_prio[p], 100.0 * (double)par_prio[p] / n);

    printf("\nRepartition par destination :\n");
    afficher_destinations(dest, nd);

    size_t plus_longue = 0, occupees = 0;
    for (size_t b = 0; b < c->nb_buckets; b++) {
        size_t len = 0;
        for (const Noeud *x = c->buckets[b]; x; x = x->suivant) len++;
        if (len) occupees++;
        if (len > plus_longue) plus_longue = len;
    }
    double octets = (double)c->capacite * sizeof(Colis) + n * sizeof(Noeud)
                  + (double)c->nb_buckets * sizeof(Noeud *);
    printf("\nStructures internes :\n");
    printf("  tableau de colis : %zu / %zu cases utilisees\n", c->taille, c->capacite);
    printf("  table de hachage : %zu cases (%zu occupees), facteur de charge %.2f, chaine la plus longue : %zu\n",
           c->nb_buckets, occupees, n / (double)c->nb_buckets, plus_longue);
    printf("  memoire estimee  : %.2f Mo\n", octets / 1e6);

    if (g.charge_valide) {
        printf("\nDernier chargement (%s) :\n", g.charge_mode);
        printf("  %zu colis, %lld / %d kg, valeur %lld, priorite moyenne %.2f\n",
               g.charge.nb, g.charge.poids_total, g.charge_capacite,
               g.charge.valeur_totale, prio_moyenne(c, &g.charge));
    }
}

static void vider_collection(void)
{
    chargement_free(&g.charge);
    g.charge_valide = 0;
    collection_free(&g.col);
    if (collection_init(&g.col) != 0) {
        printf("Erreur fatale : memoire insuffisante.\n");
        exit(1);
    }
}

static void charger_exemple(void)
{
    static const struct { int id, poids, valeur, prio; const char *dest; } ex[] = {
        {101, 8, 20, 2, "Paris"}, {102, 3, 10, 5, "Lille"}, {103, 12, 40, 1, "Lyon"},
        {104, 5, 25, 4, "Paris"}, {105, 2, 15, 3, "Lille"},
    };
    int ajoutes = 0;
    for (size_t i = 0; i < sizeof ex / sizeof ex[0]; i++)
        if (collection_ajouter(&g.col, ex[i].id, ex[i].poids, ex[i].valeur, ex[i].prio, ex[i].dest) == AJOUT_OK)
            ajoutes++;
    printf("%d colis de l'enonce ajoutes (ids 101 a 105)%s.\n", ajoutes,
           ajoutes < 5 ? " ; les autres existaient deja" : "");
}

static void charger_aleatoire(void)
{
    static const char *villes[] = {"Paris", "Lyon", "Lille", "Marseille", "Bordeaux", "Nantes"};
    long n = lire_entier("  Nombre de colis a generer (1 a 1000000) : ", 1, 1000000, 0, 0);
    int id = 1;
    for (size_t i = 0; i < g.col.taille; i++)
        if (g.col.data[i].id >= id && g.col.data[i].id < INT_MAX) id = g.col.data[i].id + 1;
    srand((unsigned)time(NULL));
    long ajoutes = 0;
    for (long i = 0; i < n; i++, id++)
        if (collection_ajouter(&g.col, id, 1 + rand() % 50, 1 + rand() % 1000,
                               1 + rand() % 5, villes[rand() % 6]) == AJOUT_OK)
            ajoutes++;
    printf("%ld colis aleatoires ajoutes (poids 1-50, valeur 1-1000, priorite 1-5). Total : %zu colis.\n",
           ajoutes, g.col.taille);
}

static void importer_csv(void)
{
    char nom[256], ligne[512], dest[DEST_MAX];
    lire_texte("  Nom du fichier CSV (id;poids;valeur;priorite;destination) : ", nom, sizeof nom);
    FILE *f = fopen(nom, "r");
    if (!f) {
        printf("Impossible d'ouvrir le fichier '%s'.\n", nom);
        return;
    }
    long ajoutes = 0, ignores = 0;
    while (fgets(ligne, sizeof ligne, f)) {
        int id, poids, valeur, prio;
        if (sscanf(ligne, " %d ; %d ; %d ; %d ; %31[^\r\n]", &id, &poids, &valeur, &prio, dest) == 5 &&
            collection_ajouter(&g.col, id, poids, valeur, prio, dest) == AJOUT_OK)
            ajoutes++;
        else
            ignores++;
    }
    fclose(f);
    printf("%ld colis importes, %ld lignes ignorees (en-tete, format invalide, doublon ou valeur hors limites).\n",
           ajoutes, ignores);
}

static void action_jeu_test(void)
{
    printf("  1. Ajouter les 5 colis de l'enonce\n"
           "  2. Generer des colis aleatoires\n"
           "  3. Importer un fichier CSV\n"
           "  4. Vider la collection\n");
    switch (lire_entier("  Votre choix : ", 1, 4, 0, 0)) {
        case 1:  charger_exemple(); break;
        case 2:  charger_aleatoire(); break;
        case 3:  importer_csv(); break;
        default: vider_collection(); printf("Collection videe.\n");
    }
}

int main(void)
{
    if (collection_init(&g.col) != 0) {
        printf("Erreur fatale : memoire insuffisante.\n");
        return 1;
    }

    for (;;) {
        printf("\n===== GESTION DES LIVRAISONS =====  (%zu colis)\n"
               "1. Afficher les colis\n"
               "2. Ajouter un colis\n"
               "3. Rechercher un colis\n"
               "4. Organiser les colis\n"
               "5. Preparer un camion\n"
               "6. Afficher le chargement\n"
               "7. Afficher les statistiques\n"
               "8. Jeu de test / import / vider\n"
               "0. Quitter\n",
               g.col.taille);
        long choix = lire_entier("Choix : ", 0, 8, 0, 0);
        printf("\n");
        switch (choix) {
            case 1: action_afficher(); break;
            case 2: action_ajouter(); break;
            case 3: action_rechercher(); break;
            case 4: action_organiser(); break;
            case 5: action_camion(); break;
            case 6: action_chargement(); break;
            case 7: action_stats(); break;
            case 8: action_jeu_test(); break;
            default:
                printf("Au revoir.\n");
                nettoyer();
                return 0;
        }
    }
}
