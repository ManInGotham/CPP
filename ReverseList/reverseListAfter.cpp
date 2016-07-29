#include <stack>
#include <iostream>

struct Node
{
   int value;
   Node* next;
};

Node* reverseAfter( int after, Node* list )
{
   Node* start = list;
   Node* current = list;
   Node* end = nullptr;

   bool found = false;
   while( current )
   {
      if( current->value == after )
      {
         found = true;
         break;
      }
      end = current;
      current = current->next;
   }

   if( !found )
      return start;

   std::stack<Node*> s;

   // push items to the stack
   while( current )
   {
      s.push( current );
      current = current->next;
   }

   // append to the end
   while( !s.empty() )
   {
      Node* node = s.top();
      s.pop();
      end->next = node;
      end = node;
   }

   end->next = nullptr;

   return start;
}

void print( Node* n )
{
   while( n )
   {
      std::cout << n->value << "->";
      n = n->next;
   }

   std::cout << "null" << std::endl;
}

void main()
{
   Node f { 1, nullptr }, e { 2, &f }, d { 3, &e }, c { 4, &d }, b { 5, &c }, a { 6, &b };
   print( &a );
   print( reverseAfter( 3, &a ) );
}

