#include <iostream>

using namespace std;

struct node
{
   node(int v, node* l = nullptr, node* r = nullptr) :
      v {v}, l {l}, r {r}
   { }

   int v;
   node* l;
   node* r;
};


bool search( node* n, int value )
{
   if(!n) 
      return false;

   if(n->v == value)
      return true;

   cout << "visiting: " << n->v << endl;

   bool found = false;
   if(n->l)
      found = found || search(n->l, value);

   if(!found && n->r)
      found = found || search(n->r, value);

   return found;
}

auto main() -> void {
   node n(1, 
      &node(2, 
         &node(3),
         &node(4)
      ),
      &node(5)
   );

   cout << "found? " << search(&n, 5) << endl;
   int i;
   cin >> i;
}