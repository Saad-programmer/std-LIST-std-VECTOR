// problem.cpp — reproduit le problème décrit dans l'article.
//
// Scénario : un grand conteneur d'objets ; on le PARCOURT et on SUPPRIME
// quelques objets pendant le parcours.
//   Version A : std::vector + erase() dans la boucle   (code d'origine)
//   Version B : std::list   + erase() dans la boucle   (l'« optimisation » Big O)
//
// Théorie :  A = O(n + m·n)   B = O(n + m)   -> B devrait gagner. En pratique ?
//
// Compilation : g++ -O2 -std=c++17 problem.cpp -o problem
#include "common.hpp"

// ---- Version A : vecteur, erase dans la boucle ---------------------------------
void run_vector_erase(std::vector<Item>& v) {
    for (auto it = v.begin(); it != v.end();) {
        it->value += 1.0;                 // « du travail » : on lit/écrit chaque objet
        if (should_delete(*it))
            it = v.erase(it);             // O(n) : décale tous les éléments suivants
        else
            ++it;
    }
}

// ---- Version B : liste, erase dans la boucle -----------------------------------
void run_list_erase(std::list<Item>& l) {
    for (auto it = l.begin(); it != l.end();) {
        it->value += 1.0;
        if (should_delete(*it))
            it = l.erase(it);             // O(1) : on ne change qu'un pointeur
        else
            ++it;                         // mais suivre le pointeur = saut mémoire
    }
}

static void row(std::size_t n, std::size_t m) {
    set_deletions(n, m);
    const auto base = make_items(n);
    Result ra, rb, rc;
    double ta = bench(3, [&] { return base; },                      run_vector_erase, &ra);
    double tb = bench(3, [&] { return make_list_compact(base); },   run_list_erase,   &rb);
    double tc = bench(3, [&] { return make_list_scattered(base); }, run_list_erase,   &rc);
    if (!same(ra, rb) || !same(ra, rc)) std::printf("ATTENTION : resultats differents !\n");
    std::printf("%9zu | %7zu | %11.3f | %16.3f | %18.3f\n", n, ra.count ? n - ra.count : 0, ta, tb, tc);
}

int main() {
    const char* head = "        n |       m |   A: vector | B: list compacte | B': list dispersee\n"
                       "----------+---------+-------------+------------------+-------------------\n";
    std::printf("Temps en ms (meilleur de 3). m = nombre de suppressions.\n\n");

    std::printf("Experience 1 : n fixe = 500 000, on fait varier m\n%s", head);
    for (std::size_t m : {5u, 50u, 500u, 5000u}) row(500'000, m);

    std::printf("\nExperience 2 : 'quelques' suppressions (m ~ 10), on fait varier n\n%s", head);
    for (std::size_t n : {100'000u, 1'000'000u, 4'000'000u, 8'000'000u}) row(n, 10);

    std::printf("\nLecture : A paie ~m deplacements de n/2 elements (contigus, donc rapides) ;\n"
                "B evite ce cout mais suit des pointeurs vers des noeuds disperses (defauts de cache).\n");
}
