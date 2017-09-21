#include <iostream>
#include <queue>

using namespace std;

template <typename T=size_t>
struct Node {
    T v;
    Node* left;
    Node* right;
    
    static Node<T>* create(T v, Node<T>* l, Node<T>* r) {
        auto n = new Node;
        n->v = v;
        n->left = l;
        n->right = r;
        return n;
    }
};

int main()
{
    Node<int>* t = 
    Node<int>::create(1, 
        Node<int>::create(2, 
            Node<int>::create(3,nullptr, nullptr),
            Node<int>::create(4,nullptr, nullptr)
        ),
        Node<int>::create(5, 
            Node<int>::create(3,nullptr, nullptr),
            Node<int>::create(4,nullptr, nullptr)
        )
    );
    
    struct NodeL {
        Node<int>* node;
        size_t level;
        
        static NodeL* create(Node<int>* n, size_t l) {
            auto node = new NodeL;
            node->node = n;
            node->level = l;
            return node;
        }
    };
    
    queue<NodeL*> q;
    q.push(NodeL::create(t, 1));
    auto level = 1;
    
    while(!q.empty()) {
        auto i = q.front();
        if(i->level != level) {
            cout << endl;
            ++level;
            continue;
        }
        
        q.pop();
        cout << i->node->v << " ";
        
        if(i->node->left)
            q.push(NodeL::create(i->node->left, level+1));
        if(i->node->right)
            q.push(NodeL::create(i->node->right, level+1));
    }
    
    return 0;
}
