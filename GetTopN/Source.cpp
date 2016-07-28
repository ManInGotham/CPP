#include "Header.h"

#include <iostream>

void main()
{
   for( auto i : getTopN<int>( 2, { 1, 2, 3, 3 } ) )
      std::cout << i << std::endl;
}