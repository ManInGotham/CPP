#pragma once

#include <forward_list>
#include <vector>


template <typename T>
std::forward_list<T> getTopN( const size_t N, const std::vector<T> values )
{
   std::forward_list<T> result;

   for( auto value : values )
   {
      bool fEmplaced = false;
      std::forward_list<T>::iterator previousItem;
      for( auto topValue = result.begin(); topValue != result.end(); ++topValue )
      {
         if( value >= *topValue )
         {
            result.emplace_after( previousItem, value );
            fEmplaced = true;
            break;
         }
         previousItem = topValue;
      }

      if( !fEmplaced )
         result.push_front( value );

      if( std::distance( std::begin( result ), std::end( result ) ) > N )
      {
         result.pop_front();
      }
   }

   return result;
}