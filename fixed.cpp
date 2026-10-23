// fixed.cpp — le même travail (parcourir + supprimer), corrigé.
//
// Idée : garder la mémoire CONTIGUË (donc le cache CPU content) ET supprimer sans
// décaler les éléments m fois. Trois corrections, de la plus simple à la plus rapide :
//
//   FIX 1  compaction en une passe (« write index »)   -> O(n), ordre conservé
//   FIX 2  erase-remove / std::erase_if (C++20)        -> O(n), ordre conservé, idiomatique
//   FIX 3  swap-and-pop                                -> O(n + m), ordre NON conservé
//
// Compilation : g++ -O2 -std=c++20 fixed.cpp -o fixed
#include "common.hpp"

// ---- Références (code du problème) ---------------------------------------------
void run_vector_erase(std::vector<Item>& v) {            // O(n + m·n)
    for (auto it = v.begin(); it != v.end();) {
        it->value += 1.0;
        if (should_delete(*it)) it = v.erase(it); else ++it;
    }
}
void run_list_erase(std::list<Item>& l) {                // O(n + m) mais cache-hostile
    for (auto it = l.begin(); it != l.end();) {
        it->value += 1.0;
        if (should_delete(*it)) it = l.erase(it); else ++it;
    }
}

// ---- FIX 1 : compaction en une seule passe -------------------------------------
// On lit avec l'indice r, on écrit les survivants avec l'indice w (w <= r).
// Chaque élément est déplacé AU PLUS UNE FOIS, au lieu d'être décalé à chaque
// suppression. Un seul parcours séquentiel de mémoire contiguë.
void run_fix1_compaction(std::vector<Item>& v) {
    std::size_t w = 0;
    for (std::size_t r = 0; r < v.size(); ++r) {
        v[r].value += 1.0;                       // le même travail
        if (!should_delete(v[r])) {
            if (w != r) v[w] = v[r];
            ++w;
        }
    }
    v.erase(v.begin() + w, v.end());             // un SEUL erase final : O(1) pour des types triviaux
}

// ---- FIX 2 : idiome erase-remove (C++20 : std::erase_if) -----------------------
// Deux passes, toutes deux séquentielles sur de la mémoire contiguë.
void run_fix2_erase_if(std::vector<Item>& v) {
    for (Item& it : v) it.value += 1.0;
    std::erase_if(v, should_delete);             // avant C++20 : v.erase(std::remove_if(...), v.end())
}

// ---- FIX 3 : swap-and-pop (si l'ordre n'a pas d'importance) --------------------
// On remplace l'élément supprimé par le dernier, puis on réduit la taille : O(1).
void run_fix3_swap_pop(std::vector<Item>& v) {
    std::size_t i = 0;
    while (i < v.size()) {
        v[i].value += 1.0;
        if (should_delete(v[i])) {
            v[i] = v.back();
            v.pop_back();                        // on n'avance PAS i : le dernier élément (non encore traité)
                                                 // vient d'arriver en v[i] et sera traité au prochain tour
        } else {
            ++i;
        }
    }
}

static void row(std::size_t n, std::size_t m) {
    set_deletions(n, m);
    const auto base = make_items(n);
    Result r0, r1, r2, r3, r4;
    double t0 = bench(3, [&] { return base; },                      run_vector_erase,     &r0);
    double t1 = bench(3, [&] { return make_list_scattered(base); }, run_list_erase,       &r1);
    double t2 = bench(3, [&] { return base; },                      run_fix1_compaction,  &r2);
    double t3 = bench(3, [&] { return base; },                      run_fix2_erase_if,    &r3);
    double t4 = bench(3, [&] { return base; },                      run_fix3_swap_pop,    &r4);
    bool ok = same(r0, r1) && same(r0, r2) && same(r0, r3) && same(r0, r4);
    std::printf("%9zu | %6zu | %9.2f | %10.2f | %9.2f | %9.2f | %9.2f | %s\n",
                n, n - r0.count, t0, t1, t2, t3, t4, ok ? "ok" : "DIFFERENT!");
}

int main() {
    const char* head = "        n |      m | vec erase | list disp. | FIX1 comp | FIX2 e_if | FIX3 s&p  | verif\n"
                       "----------+--------+-----------+------------+-----------+-----------+-----------+------\n";
    std::printf("Temps en ms (meilleur de 3)\n\n");
    std::printf("Experience 1 : n = 500 000, m variable\n%s", head);
    for (std::size_t m : {5u, 50u, 500u, 5000u}) row(500'000, m);
    std::printf("\nExperience 2 : m ~ 10, n variable\n%s", head);
    for (std::size_t n : {100'000u, 1'000'000u, 4'000'000u, 8'000'000u}) row(n, 10);
}
