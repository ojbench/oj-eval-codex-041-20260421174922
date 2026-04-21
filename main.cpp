#include <bits/stdc++.h>
#include "map.hpp"
using namespace std;

/*
  Placeholder solution for Problem 041 (map extra).
  The actual assignment expects an implementation of a custom map with
  stable iterators and benchmark behavior. Since the precise I/O is not
  disclosed here, we provide a pass-through harness that mirrors std::map
  functionality for a common subset: operations by commands from stdin.

  We support commands:
    insert k v    -> insert/update key k with value v
    erase k       -> erase key k
    find k        -> print value or NOTFOUND
    lower k       -> print lower_bound key:value or END
    upper k       -> print upper_bound key:value or END
    prev          -> print last key:value or EMPTY
    size          -> print size
    clear         -> clear all

  This matcher allows local testing and ensures the build produces `code`.
*/

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    sjtu::map<long long, long long> mp;
    string op;
    if (!cin.good()) return 0;
    while (cin >> op) {
        if (op == "insert") {
            long long k, v; if (!(cin >> k >> v)) break; mp[k] = v;
        } else if (op == "erase") {
            long long k; if (!(cin >> k)) break; mp.erase(k);
        } else if (op == "find") {
            long long k; if (!(cin >> k)) break; auto it = mp.find(k);
            if (it == mp.end()) cout << "NOTFOUND\n";
            else cout << it->second << "\n";
        } else if (op == "lower") {
            long long k; if (!(cin >> k)) break; auto it = mp.lower_bound(k);
            if (it == mp.end()) cout << "END\n";
            else cout << it->first << ":" << it->second << "\n";
        } else if (op == "upper") {
            long long k; if (!(cin >> k)) break; auto it = mp.upper_bound(k);
            if (it == mp.end()) cout << "END\n";
            else cout << it->first << ":" << it->second << "\n";
        } else if (op == "prev") {
            if (mp.empty()) cout << "EMPTY\n";
            else { auto it = mp.end(); --it; cout << it->first << ":" << it->second << "\n"; }
        } else if (op == "size") {
            cout << mp.size() << "\n";
        } else if (op == "clear") {
            mp.clear();
        } else {
            // Unknown op: attempt to interpret as k v pair lines then store
            // Fallback: ignore rest of the line
            string rest; getline(cin, rest);
        }
    }
    return 0;
}
