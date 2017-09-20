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
   const string TEN = "Ten";
   const vector<string> SINGLE_DIGIT_WORDS = {"One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"};
   const vector<string> DOUBLE_DIGIT_WORDS = {"Ten", "Twenty", "Thirty", "Fourty", "Fifty", "Sixty", "Seventy", "Eightly", "Ninthy"};
   const vector<string> FROM_11_TO_19_WORDS = {"Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen"};
   
   while(true) {
       vector<string> result;
           
       size_t i = -1;
       cout << "Enter number:";
       cin >> i;
    
       size_t s = i, order = 9;
       while(s != 0) {
          if(!result.empty()) {
                if(order == 6) {
                    result.push_back(MILLION);
                }
                if(order == 3) {
                    result.push_back(THOUSAND);
                }
            } 
           
          size_t d = static_cast<size_t>(pow(10, order-1));
          size_t p = s / d;
          string add;
          if(p > 0) {
             if(order % 3 == 0)
                add = SINGLE_DIGIT_WORDS[p-1] + HUNDRED;
             else if(order % 2 == 0)
                add = DOUBLE_DIGIT_WORDS[p-1];
             else {
                 if(!result.empty() && result.back() == TEN) {
                     // special case
                    result.pop_back();
                    add = FROM_11_TO_19_WORDS[p-1];
                 }
                 else 
                    add = SINGLE_DIGIT_WORDS[p-1];
             }
          }
          
          if(!add.empty()) {
            result.push_back(add);
          }
            
          //cout << "add " << add << ";p " << p << ";" << "result " << (result.empty() ? "" : result.back()) << ";" << "s " << s << ";" << "order " << order << endl;
          s = s - p*d;
          --order;
       }
    
       string r;
       for(auto i:result) {
           r += i;
       }
    
       cout << (r.empty() ? "Zero" : r) << endl;
   }

   return 0;
}