/*
   Author: Gleb Karapish

   This file contains implementation of the trading metrics computations along with tests and CSV file I/O.
*/

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Describes Trade data structure
typedef unsigned long long int VERYLONG;
class Trade
{
public:
   Trade() :
      Trade( 0, "", 0, 0 )
   { }

   Trade( VERYLONG timestamp, std::string symbol, VERYLONG quantity, unsigned int price )
   {
      SetPrice( price );
      SetQuantity( quantity );
      SetSymbol( symbol );
      SetTimestamp( timestamp );
   }

   const std::string& GetSymbol() const
   { return m_symbol; }

   VERYLONG GetTimestamp() const
   { return m_timestamp; }

   VERYLONG GetQuantity() const
   { return m_quantity; }

   unsigned int GetPrice() const
   { return m_price; }

   void SetSymbol( std::string symbol )
   { m_symbol = symbol; }

   void SetTimestamp( VERYLONG timestamp )
   { m_timestamp = timestamp; }

   void SetQuantity( VERYLONG quantity )
   { m_quantity = quantity; }

   void SetPrice( unsigned int price )
   { m_price = price; }

private:
   VERYLONG m_timestamp;
   std::string m_symbol;
   VERYLONG m_quantity;
   unsigned int m_price;
};

// Implements computation of the metrics for a symbol
class SymbolMetrics
{
public:
   SymbolMetrics( VERYLONG timegap, VERYLONG volume, VERYLONG totalVolumeValue, unsigned int maxPrice ) :
      m_lastTimeOccurence { 0 },
      m_maxTimeGap { timegap },
      m_totalVolume { volume },
      m_maxPrice { maxPrice },
      m_totalVolumeValue { totalVolumeValue },
	  m_totalTrades { 0 }
   { }

   SymbolMetrics() :
      SymbolMetrics( 0, 0, 0, 0 )
   { }
   
   // First time this trading symbol occurs
   SymbolMetrics( const Trade& trade ) :
      SymbolMetrics()
   {
      m_lastTimeOccurence = trade.GetTimestamp();
      Intake( trade );
   }
   
   // TODO: Implement comparison operators
   //bool operator==( const Statistics& rhs )
   //{
   //   if( &rhs == this )
   //      return true;
   //
   //}

   void Intake( const Trade& trade )
   {
      // 1. Compute maximum time gap
      auto gapToConsider = trade.GetTimestamp() - m_lastTimeOccurence;
      if( gapToConsider > m_maxTimeGap )
      {
         m_maxTimeGap = gapToConsider;
         m_lastTimeOccurence = trade.GetTimestamp();
      }

      // 2. Total volume traded (sum of the quantity for all trades in a symbol)
      m_totalVolume += trade.GetQuantity();

      // 3. Max trade price
      if( trade.GetPrice() > m_maxPrice )
         m_maxPrice = trade.GetPrice();

      // 4. Weighted average price
      // Divide total value on total volume
      m_totalVolumeValue += trade.GetPrice() * trade.GetQuantity();
	  
	  // 5. Compute number of trades
	  m_totalTrades++;
   }

   VERYLONG GetAveragePrice() const
   { return m_totalVolume == 0 ? 0 : m_totalVolumeValue / m_totalVolume; }

private:
   VERYLONG m_lastTimeOccurence;
   VERYLONG m_maxTimeGap;

   VERYLONG m_totalVolume;
   VERYLONG m_totalVolumeValue;

   unsigned int m_maxPrice;

   VERYLONG m_totalTrades;
   
   friend class Aggregator;
};

// Aggregates metrics for all symbols
typedef std::map< std::string, SymbolMetrics > METRICS;
class Aggregator
{
public:
   Aggregator()
   {
      Reset();
   }

   void Reset()
   {
      m_metrics.clear();
   }

   void Intake( const std::vector< Trade >& trades )
   {
      for( const auto& trade : trades )
         Intake( trade );
   }

   void Intake( const Trade& trade )
   {
      try
      {
         SymbolMetrics& stat = m_metrics.at( trade.GetSymbol() );
         stat.Intake( trade );
      }
      catch ( std::out_of_range& )
      {
         // adding a new symbol to the storage
         m_metrics[ trade.GetSymbol() ] = SymbolMetrics( trade );
      }
   }

   const METRICS& GetUnderlyingMetrics() const
   {
      return m_metrics;
   }

   std::ostringstream GetUnderlyingMericsAsText() const
   {
      std::ostringstream result;
      for( const auto& item : m_metrics )
      {
         result << item.first << "," << item.second.m_maxTimeGap << "," << item.second.m_totalVolume << ","
                << item.second.GetAveragePrice() << "," << item.second.m_maxPrice << "," << item.second.m_totalTrades << std::endl;
      }

      return result;
   }

private:
   METRICS m_metrics;

};

#pragma region Helper printing functions
static void PrintToConsole( const Aggregator& aggregator )
{
   std::cout << aggregator.GetUnderlyingMericsAsText().str();
}

static void PrintToCSV( const Aggregator& aggregator, const std::string& filename )
{
   std::ofstream file;
   file.open( filename );
   file << aggregator.GetUnderlyingMericsAsText().str();
   file.close();
}
#pragma endregion

#pragma region Tests
static void RunTest( Aggregator& aggregator, const std::vector< Trade >& datafeed, const METRICS& result )
{
   std::cout << "Running test" << std::endl;

   aggregator.Intake( datafeed );
   PrintToConsole( aggregator );

   // TODO: Implement strict comparison

   aggregator.Reset();
   std::cout << std::endl << std::endl;
}

// Input:
// <symbol>,<MaxTimeGap>,<Volume>,<WeightedAveragePrice>,<MaxPrice>

// Output:
// <TimeStamp>, <Symbol>, <Quantity>, <Price>

static void RunTest1( Aggregator& aggregator )
{
   auto test = {
      Trade( 52924702, "aaa", 13, 1136 ),
      Trade( 52924702, "aac", 20, 477 ),
      Trade( 52925641, "aab", 31, 907 ),
      Trade( 52927350, "aab", 29, 724 ),
      Trade( 52927783, "aac", 21, 638 ),
      Trade( 52930489, "aaa", 18, 1222 ),
      Trade( 52931654, "aaa", 9, 1077 ),
      Trade( 52933453, "aab", 9, 756 )
   };

   METRICS expected = {
      std::make_pair( "aaa", SymbolMetrics( 5787, 40, 1161, 1222 ) ),
      std::make_pair( "aab", SymbolMetrics( 6103, 69, 810, 907 ) ),
      std::make_pair( "aac", SymbolMetrics( 3081, 41, 559, 638 ) )
   };

   RunTest( aggregator, test, expected );
}

void RunTest2( Aggregator& aggregator )
{
   auto test = {
      Trade( 1, "aaa", 1, 100 ),
      Trade( 2, "bbb", 2, 100 ),
      Trade( 3, "ccc", 3, 100 ),
      Trade( 4, "ccc", 3, 100 )
   };

   METRICS expected = {
      std::make_pair( "aaa", SymbolMetrics( 0, 1, 1, 100 ) ),
      std::make_pair( "bbb", SymbolMetrics( 0, 2, 1, 100 ) ),
      std::make_pair( "ccc", SymbolMetrics( 1, 6, 100, 100 ) )
   };

   RunTest( aggregator, test, expected );
}

// Test order when printing out
void RunTest3( Aggregator& aggregator )
{
   auto test = {
      Trade( 1, "bbb", 1, 100 ),
      Trade( 2, "aaa", 1, 100 ),
   };

   METRICS expected = {
      std::make_pair( "aaa", SymbolMetrics( 0, 1, 100, 100 ) ),
      std::make_pair( "bbb", SymbolMetrics( 0, 1, 100, 100 ) )
   };

   RunTest( aggregator, test, expected );
}
#pragma endregion

// Real datafeed
void ProcessCSV( Aggregator& aggregator )
{
   std::ifstream datafeed( "C:\\Users\\GLEBKA\\Desktop\\input.csv" );
   std::string line;
   while( std::getline( datafeed, line ) )
   {
      std::stringstream  lineStream( line );
      std::string        cell;
      int position = 0;
      Trade trade;
      while( std::getline( lineStream, cell, ',' ) )
      {
         switch( position )
         {
            case 0:
               trade.SetTimestamp( std::stoll( cell ) );
               break;

            case 1:
               trade.SetSymbol( cell );
               break;

            case 2:
               trade.SetQuantity( std::stoll( cell ) );
               break;

            case 3:
               trade.SetPrice( std::stol( cell ) );
               break;
         }

         position++;
      }

      aggregator.Intake( trade );
   }

   PrintToConsole( aggregator );
   PrintToCSV( aggregator, "C:\\Users\\GLEBKA\\Desktop\\output.csv" );
}

void main()
{
   Aggregator aggregator;
   RunTest1( aggregator );
   RunTest2( aggregator );
   RunTest3( aggregator );
   ProcessCSV( aggregator );
}