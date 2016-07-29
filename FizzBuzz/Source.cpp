#include <iostream>

using namespace std;

int main() {
	/* Enter your code here. Read input from STDIN. Print output to STDOUT */

	int inputNumber = 1;
	cin >> inputNumber;

	for (int i = 1; i <= inputNumber; ++i)
	{
		bool processed = false;

		if (i % 3 == 0)
		{
			cout << "Fizz";
			processed = true;
		}

		if (i % 5 == 0)
		{
			cout << "Buzz";
			processed = true;
		}

		if (!processed)
		{
			cout << i;
		}

		cout << endl;
	}

	return 0;
}