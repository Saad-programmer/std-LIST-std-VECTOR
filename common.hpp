// common.hpp — outils partagés par problem.cpp et fixed.cpp
#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <list>
#include <numeric>
#include <random>
#include <vector>

// Un « objet » du scénario : petit struct, comme dans un vrai programme.
struct Item {
    int    id;
    double value;
};

// Règle de suppression : on supprime un objet sur `g_mod` (donc m ≈ n / g_mod suppressions).
inline int g_mod = 100;
inline bool should_delete(const Item& it) { return it.id % g_mod == 0; }
inline void set_deletions(std::size_t n, std::size_t m) { g_mod = static_cast<int>(n / std::max<std::size_t>(m, 1)); }

// Données de départ : n objets, identifiants 0..n-1.
inline std::vector<Item> make_items(std::size_t n) {
    std::vector<Item> v(n);
    for (std::size_t i = 0; i < n; ++i) v[i] = Item{static_cast<int>(i), 0.0};
    return v;
}

// Liste « compacte » : nœuds alloués dans l'ordre -> souvent voisins en mémoire.
inline std::list<Item> make_list_compact(const std::vector<Item>& src) {
    return std::list<Item>(src.begin(), src.end());
}

// Liste « fragmentée » : nœuds alloués dans un ordre aléatoire, puis triés par id.
// L'ordre LOGIQUE est le même, mais les nœuds sont dispersés en mémoire,
// comme dans un programme long qui a beaucoup inséré/supprimé (cas réaliste).
inline std::list<Item> make_list_scattered(const std::vector<Item>& src) {
    std::vector<Item> shuffled = src;
    std::mt19937 rng(42);
    std::shuffle(shuffled.begin(), shuffled.end(), rng);
    std::list<Item> l(shuffled.begin(), shuffled.end());
    l.sort([](const Item& a, const Item& b) { return a.id < b.id; });  // relie les nœuds, ne les déplace pas
    return l;
}

// Empreinte du résultat : sert à vérifier que toutes les versions font le même travail.
struct Result { std::size_t count; double sum; };
template <class C> Result checksum(const C& c) {
    Result r{0, 0.0};
    for (const Item& it : c) { ++r.count; r.sum += it.value; }
    return r;
}
inline bool same(const Result& a, const Result& b) { return a.count == b.count && a.sum == b.sum; }

// Chronomètre : meilleur temps (ms) sur `reps` exécutions. La copie des données
// est faite HORS chronomètre.
template <class Make, class Run>
double bench(int reps, Make make, Run run, Result* out = nullptr) {
    double best = 1e300;
    for (int r = 0; r < reps; ++r) {
        auto data = make();
        auto t0 = std::chrono::steady_clock::now();
        run(data);
        auto t1 = std::chrono::steady_clock::now();
        best = std::min(best, std::chrono::duration<double, std::milli>(t1 - t0).count());
        if (out) *out = checksum(data);
    }
    return best;
}
