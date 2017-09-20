#include <iostream>
#include <vector>

using namespace std;

template <typename T, size_t S>    
class queue
{
   vector<T> v;
   
   public:
   
   queue& enqueue(const T i) {
       if(size() < S) {
           v.push_back(i);
       }
       return *this;
   }
   
   T dequeue() {
       if(size() > 0) {
           T i = v.front();
           v.erase(v.begin());
           return i;
       }
       
       throw T();
   }
   
   size_t size() const {
        return v.size();    
   }
   
   T peek() const {
       if(size() > 0) {
           return v.front();
       }
       throw T();
   }
};
    
int main()
{
    queue<int, 3> q;
    cout << q.size() << endl;
    cout << q.enqueue(3).peek() << endl;
    cout << q.enqueue(2).peek() << endl;
    cout << q.size() << endl;
    cout << q.dequeue() << endl;
    cout << q.peek() << endl;
    cout << q.size() << endl;
    cout << q.dequeue() << endl;
    return 0;
    
}