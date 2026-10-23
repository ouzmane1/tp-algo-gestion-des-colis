# Gestion des livraisons : chargement optimal d'un camion

TP d'algorithmique. Programme en **C** qui gère une collection de colis (identifiant, poids, valeur, priorité, destination) et prépare le chargement d'un camion de capacité limitée en maximisant la valeur transportée.

**Langage : C**, car c'est le langage du cours. Il oblige à écrire soi-même le tableau dynamique, la table de hachage et le tri, donc à en maîtriser les complexités. Les détails et les justifications sont dans `Rapport.pdf`.

## Compilation

Prérequis : `gcc` (et `make` pour la commodité).

```bash
make              # compile le programme principal : ./livraisons
make tests        # compile et lance les 4 programmes de test
make bench        # compile le programme de mesures : ./bench
make clean        # supprime les exécutables
```

Sans `make` (par exemple sous Windows avec MinGW) :

```bash
gcc -std=c11 -O2 -o livraisons src/main.c src/colis.c src/tri.c src/camion.c
```

## Utilisation

```bash
./livraisons
```

```
===== GESTION DES LIVRAISONS =====
1. Afficher les colis
2. Ajouter un colis
3. Rechercher un colis
4. Organiser les colis
5. Preparer un camion
6. Afficher le chargement
7. Afficher les statistiques
8. Jeu de test / import / vider
0. Quitter
```

Pour tester rapidement : option **8** puis **1** (les 5 colis de l'énoncé), ou **2** (colis aléatoires, jusqu'à 1 000 000), ou **3** (import de `data/colis_exemple.csv`). Ensuite, l'option **5** prépare un camion ; elle propose :

1. valeur maximale (programmation dynamique, solution optimale) ;
2. valeur pondérée par la priorité (programmation dynamique) ;
3. glouton valeur/poids (rapide, pas toujours optimal) ;
4. comparaison des trois méthodes.

Format du fichier CSV : une ligne par colis, `id;poids;valeur;priorite;destination` (la ligne d'en-tête est ignorée).

## Organisation

```
projet/
├── src/
│   ├── colis.h / colis.c      Colis, collection (tableau dynamique + table de hachage), recherche par id
│   ├── tri.h / tri.c          Tri fusion (poids, valeur, priorité, ratio valeur/poids)
│   ├── camion.h / camion.c    Sac à dos (programmation dynamique), priorités, glouton
│   ├── main.c                 Menu
│   ├── teste..4.c       Tests (un par étape du TP)
│   └── bench.c                Mesures de performance
├── Makefile
├── README.md
└── Rapport.pdf
```

## Choix algorithmiques et complexités

| Fonctionnalité | Algorithme | Temps | Espace |
|---|---|---|---|
| Stockage | Tableau dynamique (capacité doublée quand plein) | ajout O(1) amorti | O(n) |
| Recherche par id | Table de hachage, chaînage, facteur de charge ≤ 0,75 | O(1) en moyenne | O(n) |
| Tri (poids, valeur, priorité) | Tri fusion, stable | O(n log n) | O(n) |
| Camion (valeur maximale) | Programmation dynamique (sac à dos 0/1), optimale | O(n × W) | O(W + n × W / 8 octets) |
| Camion (priorités) | Même programmation dynamique sur le score `valeur × (base + priorité − 1)`, base = 4 | O(n × W) | idem |
| Comparaison | Glouton par ratio valeur/poids, pas toujours optimal | O(n log n) | O(n) |

n est le nombre de colis, W la capacité du camion en kg.

**Règle de priorité.** Le score d'un colis est `valeur × (base + priorité − 1)`. Avec `base = 4`, un colis de priorité 5 compte deux fois plus qu'un colis de priorité 1 à valeur égale. Le paramètre `base` règle l'effet de la priorité (1 : très forte, 1000 : négligeable). La priorité va de 1 (faible) à 5 (la plus urgente).

## Tests et mesures

- `make tests` vérifie : ajout et recherche, tris corrects et stables sur 100 000 colis, programmation dynamique égale à la force brute sur 300 jeux aléatoires, optimalité du score avec priorités.
- `make bench` mesure les temps pour 100, 1 000, 10 000 et 100 000 colis (durée : environ 13 secondes ; les temps absolus dépendent de la machine).
- Les programmes ont été vérifiés avec `valgrind` : aucune fuite mémoire, aucun accès invalide.

## Hypothèses et limites

- Poids entier ≥ 1, valeur entière ≥ 0, identifiant entier ≥ 1 et unique, priorité entre 1 et 5.
- La programmation dynamique dépend de la capacité W : si `n × W / 8` dépasse 512 Mo, le calcul est refusé avec un message.
- Pas de suppression ni de modification de colis (non demandées) ; les données ne sont pas sauvegardées entre deux exécutions (import CSV possible).
