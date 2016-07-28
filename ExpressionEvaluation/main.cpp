#include <iostream>

#include "Expression.h"

// Lambda expression begins
//2+(3-4)

void main()
{
   auto expression = AddExpression( std::make_shared< Literal >( 2 ), std::make_shared< SubExpression >(
      std::make_shared< Literal >( 3 ), std::make_shared< Literal >( 4 ) )).Evaluate();
}