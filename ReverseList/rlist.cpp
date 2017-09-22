#include <iostream>
#include <vector>

using namespace std;
// Reversing the linked list

template <typename T, size_t S=10>
class stack
{
    size_t i;
    vector<T> v;

public:
    stack() :
        i{0}
    {
    }

    T peek() const {
       return i > 0 ? v.at(i-1) : nullptr;;
    }

    size_t size() const {
       return v.size();
    }
    
    bool empty() const {
        return size() == 0;
    }

    T pop() {
       if(i > 0) {
          auto r = v.back();
          v.pop_back();
          --i;
          return r;
       }
       return nullptr;
    }

    stack& push(T t) {
       if(size() < S) {
          v.push_back(t);
          ++i;
       }

       return *this;
    }
};

struct node {
    int v;
    node* n;
    
    node(int v = 0, node* n = nullptr) :
        v{v}, n{n} 
    {
    }
};


int main() {
    // 5 - 4 - 3 - 2 - 1
    auto n1 = node(1),
         n2 = node(2, &n1),
         n3 = node(3, &n2),
         n4 = node(4, &n3),
         n5 = node(5, &n4);
    
    auto* b = &n5;
    stack<node*> s;
    while(b) {
        s.push(b);
        cout << b->v << endl;
        b = b->n;
    };
 
    auto* rll = s.peek();
    
    b = rll;
    while(!s.empty()) {
        b->v = s.pop()->v;
        b->n = s.peek();
        b = b->n;
    };

    b = rll;
    while(b) {
        cout << b->v << " ";
        b = b->n;
    }
    cout << endl;
    
    return 0;
}