#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

template<class T = int>
class RedBlackTree { 
public:
    RedBlackTree() {
        nil = new Node<T>;
        nil->color = Node<T>::Color::BLACK;
        root = nil;
    }

    RedBlackTree(T x) {
        root = new Node<T>(x);
        nil = new Node<T>;
        root->left = root->right = root->parent = nil;
        root->color = nil->color = Node<T>::Color::BLACK;
    }

    ~RedBlackTree() {
        root = empty(root);
        delete nil;
    }

    void insert(T x) { root = insert(root, root, x); }

    void remove(T x);

    void print() { print_inorder(root); }

//private:

    template<class U = int>
    struct Node {
        Node() {
            left = right = parent = nullptr;
            key = -9999;
            color = Color::RED;
        }

        Node(U x) {
            left = right = parent = nullptr;
            key = x;
            color = Color::RED;
        }

        Node<U> *left, *right, *parent;
        U key;

        enum class Color { RED, BLACK } color;
    };

    Node<T> *root, *nil;

    Node<T> *empty(Node<T> *t) {
        if (t == nil) return t;

        empty(t->left);
        empty(t->right);
        delete t;

        return nullptr;
    }

    void print_inorder(Node<T> *t) {
        if (t == nil) return;

        print_inorder(t->left);
        
        std::cout << '(' << ((t->color == Node<T>::Color::BLACK) ? "BLACK" : "RED") << ")" 
                  << t->key << " -> parent: " << t->parent->key << ", " 
                  << "left: " << t->left->key << ", right: " << t->right->key
                  << std::endl;

        print_inorder(t->right);
    }

    Node<T> *find_min(Node<T> *t) {
        if (t == nil) return nullptr;
        else if (!t->left) return t;
        else return find_min(t->left);
    }

    Node<T> *find_max(Node<T> *t) {
        if (t == nil) return nullptr;
        else if (!t->right) return t;
        else return find_min(t->right);
    }

    Node<T> *find(Node<T> *t, T x) {
        if(t == nil) return nullptr;
        else if(x < t->key) return find(t->left, x);
        else if(x > t->key) return find(t->right, x);
        else return t;
    }

    Node<T> *insert(Node<T> *t, Node<T> *p, T x) {
        if (t == nil) {
            t = new Node<T>(x);
            t->left = t->right = nil;
            t->parent = p;
        } 
        else if (x < t->key) t->left = insert(t->left, t, x);
        else if (x > t->key) t->right = insert (t->right, t, x);
        return t;
    }


    void left_rotate(Node<T> *x) {
        Node<T> *y = x->right;
        x->right = y->left;

        if (y->left != nil)
            y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == nil)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;

        y->left = x;
        x->parent = y;
    }

    void right_rotate(Node<T> *y) {
        Node<T> *x = y->left;
        y->left = x->right;

        if (x->right != nil)
            x->right->parent = y;

        x->parent = y->parent;

        if (y->parent == nil)
            root = x;
        else if (y == y->parent->right)
            y->parent->right = x;
        else
            y->parent->left = x;

        y->parent = x;
        x->right = y;
    }
};

#endif