# Vecteurs et listes en C++ : mesurer avant de choisir

> Une opération plus rapide sur le papier ne garantit pas un programme plus rapide. La disposition des données en mémoire et le travail réellement effectué comptent aussi.

Ce projet compare la suppression d'éléments pendant un parcours de `std::vector` et de `std::list`. Il montre pourquoi une liste, malgré la suppression en temps constant à partir d'un itérateur, peut perdre face à un vecteur, puis présente trois façons de supprimer efficacement tout en conservant la mémoire contiguë.

Le dépôt accompagne une étude pédagogique. Les mesures dépendent du processeur, du compilateur, de sa version, des options de compilation et de la charge de la machine ; elles ne constituent pas des résultats universels.

## En bref

- `problem` reproduit une approche naïve avec `vector::erase` dans la boucle et la compare à une suppression équivalente dans une liste.
- `fixed` compare cette approche à trois variantes sur vecteur : compaction en une passe, `std::erase_if` et swap-and-pop.
- Les deux exécutables vérifient des résultats avec un checksum et affichent le meilleur temps de trois exécutions.
- La conclusion pratique est de choisir le conteneur selon les opérations réellement nécessaires, puis de mesurer. Pour un parcours suivi de suppressions, une compaction sur vecteur est souvent une meilleure première piste qu'un changement automatique vers une liste.

## Pourquoi comparer ces conteneurs ?

Un `std::vector` stocke ses éléments dans une zone contiguë. Le parcours peut donc accéder à la mémoire de façon séquentielle, ce qui favorise la localité spatiale et l'utilisation des lignes de cache du processeur. En revanche, retirer un élément au milieu avec `erase` décale les éléments suivants.

Un `std::list` est constitué de nœuds reliés entre eux. Une fois l'itérateur obtenu, supprimer un nœud ne demande pas de décaler tous les éléments. Toutefois, parcourir la liste implique de suivre des pointeurs ; les nœuds peuvent se trouver à des endroits éloignés en mémoire, ce qui réduit l'efficacité du cache et ajoute le coût des allocations par nœud.

| Opération ou propriété | `std::vector` | `std::list` |
| --- | --- | --- |
| Parcours complet | Linéaire, généralement favorable à la mémoire séquentielle | Linéaire, avec accès indirect de nœud en nœud |
| Suppression au milieu | Décalage des éléments suivants | Suppression en temps constant si l'itérateur est déjà connu |
| Accès par indice | Temps constant | Pas d'accès par indice en temps constant |
| Stockage | Contigu | Nœuds séparés, reliés par pointeurs |

Les complexités seules ne racontent donc qu'une partie de l'histoire. Dans le scénario de ce projet, il faut parcourir tous les éléments pour décider lesquels supprimer. La liste évite les décalages, mais son parcours peut coûter davantage ; la compaction du vecteur évite, elle, de répéter les décalages tout en gardant un parcours séquentiel.

## Contenu du dépôt

| Fichier | Rôle |
| --- | --- |
| `problem.cpp` | Benchmark de référence : vecteur avec `erase` dans la boucle, liste compacte et liste dispersée. |
| `fixed.cpp` | Comparaison de la référence avec trois stratégies de suppression sur vecteur. |
| `common.hpp` | Type `Item`, génération des données, fabrication des listes, règle de suppression, checksum et chronomètre partagés. |
| `Makefile` | Compilation des deux programmes et cible pour les exécuter. |
| `Vecteurs_vs_Listes_CPP.docx` | Document d'accompagnement et synthèse théorique. |

## Prérequis

- Un compilateur C++ compatible avec C++17 et C++20, par exemple `g++`.
- `make` pour utiliser les commandes regroupées ci-dessous.
- Un système d'exploitation où les programmes peuvent allouer plusieurs millions d'objets en mémoire.

`problem` est compilé en C++17. `fixed` utilise `std::erase_if`, disponible à partir de C++20. Les options `-O2 -Wall -Wextra` sont déjà définies dans le `Makefile`.

## Compiler et exécuter

Depuis le répertoire du projet :

```sh
make
```

Cette commande construit `problem` et `fixed`. Pour lancer les deux benchmarks à la suite :

```sh
make run
```

Les exécutables peuvent aussi être lancés séparément :

```sh
./problem
./fixed
```

Pour supprimer manuellement les exécutables construits :

```sh
rm -f problem fixed
```

Compilation manuelle, sans `make` :

```sh
g++ -O2 -std=c++17 -Wall -Wextra problem.cpp -o problem
g++ -O2 -std=c++20 -Wall -Wextra fixed.cpp -o fixed
```

## Scénario mesuré

Chaque élément est un `Item` comprenant un identifiant entier et une valeur flottante. Les données de départ sont initialisées dans l'ordre avec les identifiants `0` à `n - 1`. Pendant chaque parcours, la valeur de chaque élément examiné est incrémentée ; un élément est supprimé lorsque son identifiant est divisible par un pas calculé à partir de `n` et du nombre de suppressions demandé.

Les programmes exécutent deux séries d'expériences :

1. `n` reste fixé à 500 000 et le nombre demandé de suppressions varie entre 5 et 5 000.
2. Le nombre demandé de suppressions reste proche de 10 et `n` varie entre 100 000 et 8 000 000.

Dans `problem`, la liste compacte est construite en insérant les éléments dans l'ordre. La liste dispersée est construite en insérant les éléments dans un ordre pseudo-aléatoire déterministe, puis en triant les nœuds par identifiant. Le tri rétablit l'ordre logique sans déplacer les nœuds en mémoire. Cette variante illustre une liste dont les éléments sont moins proches physiquement.

La préparation et la copie des données sont hors de la zone chronométrée. Chaque mesure est répétée trois fois et le meilleur temps est affiché en millisecondes. Cette méthode réduit l'effet de certaines variations ponctuelles, mais le meilleur de trois n'est pas une mesure statistique complète : pour une étude rigoureuse, répétez les expériences dans des conditions contrôlées et comparez aussi la médiane ou la distribution des temps.

## Stratégies comparées

### Référence : `vector::erase` dans la boucle

À chaque suppression, `vector::erase` décale les éléments qui suivent. Si `m` éléments sont retirés d'un vecteur de `n` éléments, le coût peut atteindre $O(n + mn)$ dans ce scénario. Les déplacements sont toutefois contigus et peuvent être efficaces pour un petit nombre de suppressions.

### Liste : suppression pendant le parcours

La suppression d'un nœud est en $O(1)$ lorsque son itérateur est connu. Le parcours reste en $O(n)$, pour un coût total théorique de $O(n + m)$. Cette complexité ne tient pas compte du coût pratique des accès dispersés, des pointeurs ni des allocations des nœuds.

### FIX 1 : compaction en une passe

Un indice lit chaque élément et un autre écrit les éléments conservés à la suite. Chaque élément est examiné une fois et les survivants gardent leur ordre. Le coût total est $O(n)$, suivi d'une suppression du suffixe inutilisé.

### FIX 2 : `std::erase_if`

Cette forme idiomatique de C++20 incrémente les valeurs, puis retire les éléments correspondants avec `std::erase_if`. Le parcours et la compaction restent linéaires, soit $O(n)$ au total, et l'ordre des éléments conservés est préservé. En C++17, le remplacement classique est `erase-remove` avec `std::remove_if`.

### FIX 3 : swap-and-pop

L'élément à supprimer est remplacé par le dernier, puis le vecteur est raccourci. Chaque élément est traité au plus une fois, pour un coût total de $O(n)$ dans cette implémentation. Cette stratégie ne conserve pas l'ordre des éléments et ne convient que si cet ordre n'a pas de sens pour l'application.

| Variante | Complexité du scénario | Ordre conservé | Standard utilisé |
| --- | --- | --- | --- |
| Vecteur avec `erase` répété | Jusqu'à $O(n + mn)$ | Oui | C++17 |
| Liste avec `erase` pendant le parcours | $O(n + m)$ | Oui | C++17 |
| FIX 1 : compaction | $O(n)$ | Oui | C++17 |
| FIX 2 : `std::erase_if` | $O(n)$ | Oui | C++20 |
| FIX 3 : swap-and-pop | $O(n)$ | Non | C++17 |

Les complexités décrivent le travail algorithmique de ces implémentations ; elles ne prédisent pas à elles seules le temps observé. En particulier, elles ne prennent pas en compte les effets du cache, l'allocateur, la taille réelle des objets et les caractéristiques de la machine.

## Lire les résultats

`problem` affiche les temps du vecteur, de la liste compacte et de la liste dispersée. `fixed` affiche les temps du vecteur avec suppression répétée, de la liste dispersée, puis des trois corrections. Les programmes affichent une vérification (`ok` dans `fixed`) lorsque le nombre d'éléments et la somme des valeurs concordent entre variantes.

Cette vérification est un checksum léger, pas une preuve d'équivalence élément par élément. FIX 3 ne préserve notamment pas l'ordre. Les temps et le classement peuvent changer selon la machine, l'état du cache, la charge système et le compilateur. Relancez les programmes dans des conditions comparables avant de tirer une conclusion.

L'expérience illustre surtout deux idées : les opérations répétées de décalage peuvent être évitées en repensant l'algorithme, et les accès contigus peuvent rendre un vecteur très performant pour des parcours complets. Elle ne démontre pas que `std::vector` est toujours préférable à `std::list` : une liste reste pertinente lorsque ses propriétés correspondent réellement aux opérations et aux contraintes du programme.

## Limites

- Le benchmark utilise un seul type d'élément simple et une règle de suppression déterministe.
- Le nombre effectif de suppressions dépend du pas entier calculé ; il est voisin de la valeur demandée, sans être nécessairement identique pour tous les couples `n` et `m`.
- La liste dispersée simule une disposition défavorable par un ordre d'allocation mélangé ; elle ne reproduit pas toutes les formes de fragmentation d'un programme réel.
- Les résultats incluent l'opération de mise à jour de chaque élément, mais excluent la construction et la copie des conteneurs.
- Les mesures utilisent le meilleur temps de trois répétitions et ne pinent pas le processus sur un cœur.
- Aucun résultat numérique fixe n'est fourni ici, car il dépend de l'environnement d'exécution.

## Références

- B. Horenberger, [« Contiguous memory matters: vectors vs lists in C++ »](https://horenbergerb.github.io/2024/05/09/vector-vs-list.html), 9 mai 2024.
- [Documentation C++ : `std::vector`](https://en.cppreference.com/w/cpp/container/vector)
- [Documentation C++ : `std::list`](https://en.cppreference.com/w/cpp/container/list)
- [Documentation C++ : `std::erase_if`](https://en.cppreference.com/w/cpp/container/vector/erase2)
