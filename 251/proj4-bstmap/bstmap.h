#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;

template <typename KeyT, typename ValT>
class BSTMap {
 private:
  struct BSTNode {
    const KeyT key;
    ValT value;
    BSTNode* parent;
    BSTNode* left;
    BSTNode* right;

    BSTNode(KeyT key, ValT value, BSTNode* parent)
        : key(key),
          value(value),
          parent(parent),
          left(nullptr),
          right(nullptr) {
    }
  };

  BSTNode* root;
  size_t sz;

  BSTNode* curr;

  static void setParent(BSTNode* child, BSTNode* p) {
    if (child) child->parent = p;
  }

  void replaceChild(BSTNode* p, BSTNode* oldC, BSTNode* newC) {
  if (!p) {
    root = newC;
  } else if (p->left == oldC) {
    p->left = newC;
  } else {
    p->right = newC;
  }
    setParent(newC, p);
  }

  BSTNode* findMin(BSTNode* n) const {
    if (!n) return nullptr;
    while (n->left) n = n->left;
    return n;
  }

  static BSTNode* endSentinel() {
    return reinterpret_cast<BSTNode*>(-1); 
  }

  BSTNode* successor(BSTNode* n) const {
    if (!n) return nullptr;
    if (n->right) return findMin(n->right);
    while (n->parent && n == n->parent->right) n = n->parent;
    return n->parent;
  }

  BSTNode* findNode(const KeyT& key) const {
    BSTNode* cur = root;
    while (cur) {
      if (key == cur->key) return cur;
      cur = (key < cur->key) ? cur->left : cur->right;
    }
    return nullptr;
  }

  static BSTNode* cloneSubtree(BSTNode* otherNode, BSTNode* parent) {
    if (!otherNode) return nullptr;
    BSTNode* me = new BSTNode(otherNode->key, otherNode->value, parent);
    me->left = cloneSubtree(otherNode->left, me);
    me->right = cloneSubtree(otherNode->right, me);
    return me;
  }

  static void deleteSubtree(BSTNode* n) {
    if (!n) return;
    deleteSubtree(n->left);
    deleteSubtree(n->right);
    delete n;
  }

  template <class F>
  bool inorderAll(BSTNode* n, F f) const {
    if (!n) return true;
    if (!inorderAll(n->left, f)) return false;
    if (!f(n)) return false;
    return inorderAll(n->right, f);
  }

  void inorderWrite(BSTNode* n, std::ostringstream& out) const {
    if (!n) return;
    inorderWrite(n->left, out);
    out << n->key << ": " << n->value << '\n';
    inorderWrite(n->right, out);
  }

 public:
  BSTMap() : root(nullptr), sz(0), curr(nullptr) {}

  bool empty() const {
    return sz == 0;
  }

  size_t size() const {
    return sz;
  }

  void insert(KeyT key, ValT value) {
    if (!root) {
      root = new BSTNode(key, value, nullptr);
      ++sz;
      return;
    }
    BSTNode* p = root;
    while (true) {
      if (key == p->key) {
        return;
      } else if (key < p->key) {
      if (!p->left) {
        p->left = new BSTNode(key, value, p);
        ++sz;
        return;
      }
        p = p->left;
      } else {
        if (!p->right) {
          p->right = new BSTNode(key, value, p);
          ++sz;
          return;
        }
        p = p->right;
      }
    }
  }

  ValT& at(const KeyT& key) const {
    BSTNode* n = findNode(key);
    if (!n) throw std::out_of_range("key not found");
    return const_cast<ValT&>(n->value);
  }

  bool contains(const KeyT& key) const {
    return findNode(key) != nullptr;
  }

  void clear() {
    deleteSubtree(root);
    root = nullptr;
    sz = 0;
    curr = nullptr;
  }

  ~BSTMap() {
    clear();
  }

  string to_string() const {
    std::ostringstream out;
    inorderWrite(root, out);
    return out.str();
  }

  BSTMap(const BSTMap& other) : root(nullptr), sz(0), curr(nullptr) {
    root = cloneSubtree(other.root, nullptr);
    sz   = other.sz;
  }
  BSTMap& operator=(const BSTMap& other) {
    if (this == &other) return *this;
    clear();
    root = cloneSubtree(other.root, nullptr);
    sz   = other.sz;
    curr = nullptr;
    return *this;
  }


  pair<KeyT, ValT> remove_min() {
    if (!root) throw std::runtime_error("remove_min on empty map");
    BSTNode* m = findMin(root);
    BSTNode* r = m->right;
    BSTNode* p = m->parent;


    pair<KeyT, ValT> out(m->key, m->value);
    replaceChild(p, m, r);
    delete m;
    --sz;
    curr = nullptr;
    return out;
  }

  bool operator==(const BSTMap& other) const {
    if (sz != other.sz) return false;
    bool ok = true;
    inorderAll(root, [&](BSTNode* n) -> bool {
      if (!other.contains(n->key)) return ok = false;
      try {
        const ValT& ov = other.at(n->key);
        if (!(ov == n->value)) return ok = false;
      } catch (const std::out_of_range&) {
        return ok = false;
      }
      return ok;
    });
    return ok;
  }

  void begin() {
    curr = nullptr;
  }

  bool next(KeyT& key, ValT& val) {
    if (curr == endSentinel())
      return false;

    BSTNode* n = (curr == nullptr) ? findMin(root) : successor(curr);

    if (!n) {
      curr = endSentinel(); 
      return false;
    }

    curr = n;
    key = curr->key;
    val  = curr->value;
    return true;
  }

  ValT erase(const KeyT& key) {
    BSTNode* n = findNode(key);
    if (!n) throw std::out_of_range("key not found");

    ValT ret = n->value;

    if (!n->left || !n->right) {
      BSTNode* child = n->left ? n->left : n->right;
      replaceChild(n->parent, n, child);
      delete n;
    } else {
      BSTNode* s = findMin(n->right);
      if (s->parent != n) {
        replaceChild(s->parent, s, s->right);
        s->right = n->right;
        setParent(s->right, s);
      }
      s->left = n->left;
      setParent(s->left, s);
      replaceChild(n->parent, n, s);
      delete n;
    }

    --sz;
    curr = nullptr;
    return ret;
  }

  void* getRoot() const {
    return this->root;
  }
};
