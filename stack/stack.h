// Example program
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T, size_t S>
class stack
{
    size_t i;
    vector<T> v;

public:
    stack() :
            i{0}
    {
       cout << "S " << S << endl;
    }

    T peek() const {
       return i > 0 ? v.at(i-1) : throw T();
    }

    size_t size() const {
       return v.size();
    }

    T pop() {
       if(i > 0) {
          auto r = v.back();
          v.pop_back();
          --i;
          return r;
       }
       throw T();
    }

    stack& push(T t) {
       cout << "size " << size() << endl;
       if(size() < S) {
          v.push_back(t);
          ++i;
       }

       return *this;
    }
};


int main()
{
   stack<int, 3> s;
   cout << s.size() << endl;
   cout << s.push(1).peek() << endl;
   cout << s.peek() << endl;
   cout << s.pop() << endl;
   cout << s.push(1).push(2).peek() << endl;
   cout << s.pop() << endl;
   cout << s.pop() << endl;
   cout << s.size() << endl;
   cout << s.push(1).push(2).push(3).push(4).peek() << endl;
   cout << s.size() << endl;
}