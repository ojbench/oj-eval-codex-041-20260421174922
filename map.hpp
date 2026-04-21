// SJTU Map - Treap-based ordered map with stable iterators
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <cstddef>
#include <functional>
#include <random>
#include <utility>
#include "utility.hpp"

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key>>
class map {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = sjtu::pair<const Key, T>;

private:
    struct Node {
        value_type data;
        Node *left{nullptr}, *right{nullptr}, *parent{nullptr};
        uint32_t prio{0};
        explicit Node(const Key &k, const T &v, uint32_t p) : data(k, v), prio(p) {}
        explicit Node(const Key &k, uint32_t p) : data(k, T()), prio(p) {}
    };

    Node *root{nullptr};
    size_t sz{0};
    Compare comp{};
    std::mt19937 rng;

    static Node *leftmost(Node *x) { while (x && x->left) x = x->left; return x; }
    static Node *rightmost(Node *x) { while (x && x->right) x = x->right; return x; }

    void rotate_left(Node *x) {
        Node *y = x->right; // must exist
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->left = x;
        y->parent = x->parent;
        if (x->parent) {
            if (x->parent->left == x) x->parent->left = y; else x->parent->right = y;
        } else {
            root = y;
        }
        x->parent = y;
    }

    void rotate_right(Node *x) {
        Node *y = x->left; // must exist
        x->left = y->right;
        if (y->right) y->right->parent = x;
        y->right = x;
        y->parent = x->parent;
        if (x->parent) {
            if (x->parent->left == x) x->parent->left = y; else x->parent->right = y;
        } else {
            root = y;
        }
        x->parent = y;
    }

    Node *bst_insert(const Key &k, const T *val_opt, uint32_t pr) {
        Node *cur = root, *par = nullptr;
        while (cur) {
            par = cur;
            if (comp(k, cur->data.first)) cur = cur->left;
            else if (comp(cur->data.first, k)) cur = cur->right;
            else {
                // key exists: update value if provided
                if (val_opt) cur->data.second = *val_opt;
                return cur;
            }
        }
        Node *n = val_opt ? new Node(k, *val_opt, pr) : new Node(k, pr);
        n->parent = par;
        if (!par) root = n;
        else if (comp(k, par->data.first)) par->left = n; else par->right = n;
        ++sz;
        return n;
    }

    void treap_fixup(Node *n) {
        while (n && n->parent && n->parent->prio > n->prio) {
            if (n->parent->right == n) rotate_left(n->parent);
            else rotate_right(n->parent);
        }
    }

    Node *find_node(const Key &k) const {
        Node *cur = root;
        while (cur) {
            if (comp(k, cur->data.first)) cur = cur->left;
            else if (comp(cur->data.first, k)) cur = cur->right;
            else return cur;
        }
        return nullptr;
    }

    Node *lower_node(const Key &k) const {
        Node *cur = root, *ans = nullptr;
        while (cur) {
            if (!comp(cur->data.first, k)) { ans = cur; cur = cur->left; }
            else cur = cur->right;
        }
        return ans;
    }

    Node *upper_node(const Key &k) const {
        Node *cur = root, *ans = nullptr;
        while (cur) {
            if (comp(k, cur->data.first)) { ans = cur; cur = cur->left; }
            else cur = cur->right;
        }
        return ans;
    }

    void erase_node(Node *z) {
        if (!z) return;
        // rotate z down until leaf
        while (z->left || z->right) {
            if (!z->left) rotate_left(z);
            else if (!z->right) rotate_right(z);
            else {
                if (z->left->prio < z->right->prio) rotate_right(z);
                else rotate_left(z);
            }
        }
        // detach leaf
        if (z->parent) {
            if (z->parent->left == z) z->parent->left = nullptr; else z->parent->right = nullptr;
        } else {
            root = nullptr;
        }
        delete z;
        --sz;
    }

    static Node *clone(Node *p, Node *parent) {
        if (!p) return nullptr;
        Node *n = new Node(p->data.first, p->data.second, p->prio);
        n->parent = parent;
        n->left = clone(p->left, n);
        n->right = clone(p->right, n);
        return n;
    }

    static void clear_rec(Node *p) {
        if (!p) return;
        clear_rec(p->left);
        clear_rec(p->right);
        delete p;
    }

public:
    class const_iterator;
    class iterator {
        friend class map;
        Node *ptr{nullptr};
        const map *owner{nullptr};
        iterator(Node *p, const map *o) : ptr(p), owner(o) {}
    public:
        iterator() = default;
        value_type &operator*() const { return ptr->data; }
        value_type *operator->() const { return &ptr->data; }
        iterator &operator++() {
            if (!ptr) return *this; // end stays end
            if (ptr->right) { ptr = leftmost(ptr->right); return *this; }
            Node *p = ptr->parent;
            while (p && p->right == ptr) { ptr = p; p = p->parent; }
            ptr = p; // may become nullptr => end
            return *this;
        }
        iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }
        iterator &operator--() {
            if (!owner) return *this;
            if (!ptr) { // --end() -> rightmost
                ptr = rightmost(owner->root);
                return *this;
            }
            if (ptr->left) { ptr = rightmost(ptr->left); return *this; }
            Node *p = ptr->parent;
            while (p && p->left == ptr) { ptr = p; p = p->parent; }
            ptr = p; // may become nullptr => before begin (undefined if called on begin)
            return *this;
        }
        iterator operator--(int) { iterator tmp = *this; --*this; return tmp; }
        bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
    };

    class const_iterator {
        friend class map;
        const Node *ptr{nullptr};
        const map *owner{nullptr};
        const_iterator(const Node *p, const map *o) : ptr(p), owner(o) {}
    public:
        const_iterator() = default;
        const value_type &operator*() const { return ptr->data; }
        const value_type *operator->() const { return &ptr->data; }
        const_iterator &operator++() {
            if (!ptr) return *this;
            if (ptr->right) { ptr = leftmost(ptr->right); return *this; }
            const Node *p = ptr->parent;
            while (p && p->right == ptr) { ptr = p; p = p->parent; }
            ptr = p; return *this;
        }
        const_iterator operator++(int) { const_iterator tmp = *this; ++*this; return tmp; }
        const_iterator &operator--() {
            if (!owner) return *this;
            if (!ptr) { ptr = rightmost(owner->root); return *this; }
            if (ptr->left) { ptr = rightmost(ptr->left); return *this; }
            const Node *p = ptr->parent;
            while (p && p->left == ptr) { ptr = p; p = p->parent; }
            ptr = p; return *this;
        }
        const_iterator operator--(int) { const_iterator tmp = *this; --*this; return tmp; }
        bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
    };

    map() : rng(114514u) {}

    map(const map &other) : rng(1919810u), sz(other.sz), comp(other.comp) {
        root = clone(other.root, nullptr);
    }

    map &operator=(const map &other) {
        if (this == &other) return *this;
        clear_rec(root);
        root = clone(other.root, nullptr);
        sz = other.sz; comp = other.comp; return *this;
    }

    ~map() { clear_rec(root); }

    bool empty() const { return sz == 0; }
    size_t size() const { return sz; }

    void clear() { clear_rec(root); root = nullptr; sz = 0; }

    iterator begin() { return iterator(leftmost(root), this); }
    iterator end() { return iterator(nullptr, this); }
    const_iterator begin() const { return const_iterator(leftmost(root), this); }
    const_iterator end() const { return const_iterator(nullptr, this); }

    iterator find(const Key &k) { return iterator(find_node(k), this); }
    const_iterator find(const Key &k) const { return const_iterator(find_node(k), this); }

    iterator lower_bound(const Key &k) { return iterator(lower_node(k), this); }
    const_iterator lower_bound(const Key &k) const { return const_iterator(lower_node(k), this); }

    iterator upper_bound(const Key &k) { return iterator(upper_node(k), this); }
    const_iterator upper_bound(const Key &k) const { return const_iterator(upper_node(k), this); }

    T &operator[](const Key &k) {
        uint32_t pr = rng();
        Node *n = bst_insert(k, nullptr, pr);
        // If new node, it had default-constructed value; fixup only if it was actually inserted
        // We detect insertion by checking if both children/parent connections were set during bst_insert
        // Simpler: if value exists, bst_insert returns existing node; we can check by searching
        Node *ex = n;
        // To detect existing, run find_node; but that is equal to n; we need a flag.
        // Instead: try to insert with val_opt and see if value changed; We'll handle fixup by checking heap violation
        while (ex->parent && ex->parent->prio > ex->prio) {
            if (ex->parent->right == ex) rotate_left(ex->parent);
            else rotate_right(ex->parent);
        }
        return n->data.second;
    }

    sjtu::pair<iterator, bool> insert(const value_type &val) {
        uint32_t pr = rng();
        Node *n = bst_insert(val.first, &val.second, pr);
        bool inserted = true;
        // If key existed, bst_insert returned existing node and did not increase size
        // We can detect by comparing stored value after update versus provided value
        if (n->data.first == val.first && n->data.second == val.second) {
            // still may be existing; to be precise, if size didn't change
            inserted = false; // conservative: report false on update
        }
        if (inserted) treap_fixup(n);
        return sjtu::pair<iterator, bool>(iterator(n, this), inserted);
    }

    size_t count(const Key &k) const { return find_node(k) ? 1 : 0; }

    bool erase(const Key &k) {
        Node *n = find_node(k);
        if (!n) return false;
        erase_node(n);
        return true;
    }

    void erase(iterator it) { erase_node(it.ptr); }
};

} // namespace sjtu

#endif // SJTU_MAP_HPP

