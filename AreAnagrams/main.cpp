//#include <operations.h>

#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;

bool AreAnagrams( string s1, string s2 )
{
   unordered_map< char, size_t > map;

   for( auto c : s1 )
      map[ c ]++;

   for( auto c : s2 )
   {
      try
      {
         map.at( c )--;
      }
      catch( out_of_range& )
      {
         return false;
      }
   }

   for( auto item : map )
      if( item.second != 0 )
         return false;

   return true;
}

int main()
{
   string s1 = "integral";
   string s2 = "triangle";
   cout << "Are " << s1 << " and " << s2 << " anagrams? " << AreAnagrams( s1, s2 ) << endl;

   s1 = s1.substr( 0, s1.length() - 2 );
   cout << "Are " << s1 << " and " << s2 << " anagrams? " << AreAnagrams( s1, s2 ) << endl;

   return 0;
}