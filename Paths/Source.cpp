#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <bitset>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <utility>
#include <iostream>
#include <algorithm>
#include <functional>

using namespace std;

const int M = 3, N = 4;

inline void checkEndOrMove(int grid [][N], int M, int N, int i, int j, int * count) {

	// check if bottom-right cell is reached
	// make sure index is not out of bound
	//
	if (i == M - 1 && j == N - 1)
	{
		(*count)++;
		return;
	}

	// move to the right (change column index)
	//
	if (j <= N - 2 && grid[i][j + 1] == 1)
	{
		checkEndOrMove(grid, M, N, i, j + 1, count);
	}

	// check row to the bottom
	if (i <= M - 2 && grid[i + 1][j] == 1)
	{
		checkEndOrMove(grid, M, N, i + 1, j, count);
	}
}

int numberOfPaths(int a [][N], int M, int N)
{
	int count = 0;
	checkEndOrMove(a, M, N, 0, 0, &count);
	return count % 1000000007;
}

int main() {
	int a[M][N] = {
			{ 1, 1, 1, 1 },
			{ 0, 0, 0, 1 },
			{ 0, 0, 0, 1 },
	};

	printf("%d\n", numberOfPaths(a, M, N));
	return 0;
}