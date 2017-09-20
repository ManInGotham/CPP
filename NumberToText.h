/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <cmath>
#include <vector>

using namespace std;

// Write a function that represents numbers from 0 to billion as camel case words, 
// e.g. EightHunredFIftySevenThousandTwoHundredThirtyFive.

int main()
{
   // 113013003 < 10^7
   const string MILLION = "Million";
   const string THOUSAND = "Thousand";
   const string HUNDRED = "Hundred";
   const vector<string> SINGLE_DIGIT_WORDS = {"One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"};
   const vector<string> DOUBLE_DIGIT_WORDS = {"Ten", "Twenty", "Thirty", "Fourty", "Fifty", "Sixty", "Seventy", "Eightly", "Ninthy"};

   size_t i = -1;
   cin >> i;

   size_t s = i, order = 9;
   string result = "";
   while(s != 0) {
      if(!result.empty())
         if(order == 6)
            result += MILLION;
         else if(order == 3)
            result += THOUSAND;

      auto d = pow(10, order-1);
      size_t p = s / d;
      string add = "";
      if(p > 0) {
         if(order % 3 == 0)
            add = SINGLE_DIGIT_WORDS[p-1] + HUNDRED;
         else if(order % 2 == 0)
            add = DOUBLE_DIGIT_WORDS[p-1];
         else add = SINGLE_DIGIT_WORDS[p-1];
      }
      result += add;
      s = s - p*d;
      --order;
      cout << "p " << p << ";" << "result " << result << ";" << "s " << s << ";" << "order " << order << endl;
   }

   cout<<result;

   return 0;
}