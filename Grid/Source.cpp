// Author: Gleb Karapish
// Date  : 10/21/2014
//
//
// Filename: Source.cpp
//
// Implements an algorithm to find circumscribed rectangle around a blob where every '1' cell 
// has at least one adjoining cell with (right, left, top, bottom; NOT diagonal) '1'.
// 
// To find blob's
//   -top    -- scans from left to right in each column 
//   -left   -- scans from top to bottom in each row
//   -bottom -- scans from right to left in each column
//   -right  -- scans from bottom to top in each row
//
//  Every time it hits occupied cell we check if it has occupied neighbour.
//

#include <iostream>
#include <thread>

using namespace std;

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

namespace Blob
{
	// Wrapping two-dim array into struct to allow to pass to method
	struct Grid
	{
		static const unsigned NUM_ROWS = 10;
		static const unsigned NUM_COLS = 10;

		bool array[Grid::NUM_ROWS][Grid::NUM_COLS];
	};

	// Bounding rect for a blob
	struct Rect
	{
		unsigned Left;
		unsigned Right;
		unsigned Top;
		unsigned Bottom;
	};

	// Class which computes the bounding rectangle
	class BlobRect
	{
	private:
		// Input array
		Grid grid;

		// Bounding rectangle for a blob
		Rect rect;

		// Determines if cell[i,j] have at least one adjoining cell to the right, left, top, or bottom that is also occupied.
		//
		bool DoesCellBelongToBlob(int i, int j)
		{
			if (i - 1 >= 0)
				if (grid.array[i - 1][j])
					return true;

			if (i + 1 < Grid::NUM_ROWS)
				if (grid.array[i + 1][j])
					return true;

			if (j - 1 >= 0)
				if (grid.array[i][j - 1])
					return true;

			if (j + 1 < Grid::NUM_COLS)
				if (grid.array[i][j + 1])
					return true;

			return false;
		}

		// Determines the bounding rectangle within the range of rows and columns.
		// This allows to parallelize the computation
		//
		// Complexity is O(n) = 4*N = N
		//
		void FindBlobRect(Rect* rect, unsigned char rowStart, unsigned char rowEnd, unsigned char colStart, unsigned char colEnd)
		{
			// Find top
			//
			bool foundTop = false;
			for (unsigned char row = rowStart; row < rowEnd; ++row)
			{
				for (unsigned char column = colStart; column < colEnd; ++column)
				{
					if (grid.array[row][column] && DoesCellBelongToBlob(row, column))
					{
						rect->Top = row;
						foundTop = true;
						break;
					}
				}

				if (foundTop)
					break;
			}

			// Find left
			//
			bool foundLeft = false;
			for (unsigned char column = colStart; column < colEnd; ++column)
			{
				for (unsigned char row = rowStart; row < rowEnd; ++row)
				{
					if (grid.array[row][column] && DoesCellBelongToBlob(row, column))
					{
						rect->Left = column;
						foundLeft = true;
						break;
					}
				}

				if (foundLeft)
					break;
			}

			// Find right
			//
			bool foundRight = false;
			for (unsigned char column = colEnd - 1; column >= colStart; --column)
			{
				for (unsigned char row = rowStart; row < rowEnd; ++row)
				{
					if (grid.array[row][column] && DoesCellBelongToBlob(row, column))
					{
						rect->Right = column;
						foundRight = true;
						break;
					}
				}

				if (foundRight)
					break;
			}

			// Find bottom
			//
			bool foundBottom = false;
			for (unsigned char row = rowEnd - 1; row >= rowStart; --row)
			{
				for (unsigned char column = colStart; column < colEnd; ++column)
				{
					if (grid.array[row][column] && DoesCellBelongToBlob(row, column))
					{
						rect->Bottom = row;
						foundBottom = true;
						break;
					}
				}

				if (foundBottom)
					break;
			}
		}

	public:
		BlobRect(const Grid &inputGrid)
			: grid(inputGrid)
		{}

		void Detect(bool multiThreaded = false)
		{
			if (!multiThreaded)
			{
				FindBlobRect(&rect, /*start row*/ 0, Grid::NUM_ROWS, /*start column*/ 0, Grid::NUM_COLS);
			}
			else
			{
				// In multi-threaded case spin 4 threads assigning each quadrant on a dedicated thread
				// then combine the result.
				//
				Rect leftTopRect, rightTopRect, leftBottomRect, rightBottomRect;

				std::thread t1(&BlobRect::FindBlobRect, this, &leftTopRect, 0, 5, 0, 5);
				std::thread t2(&BlobRect::FindBlobRect, this, &rightTopRect, 6, Grid::NUM_ROWS, 0, 5);
				std::thread t3(&BlobRect::FindBlobRect, this, &leftBottomRect, 0, 5, 6, Grid::NUM_COLS);
				std::thread t4(&BlobRect::FindBlobRect, this, &rightBottomRect, 6, Grid::NUM_ROWS, 6, Grid::NUM_COLS);

				t1.join(); t2.join(); t3.join(); t4.join();

				rect.Left = min(leftTopRect.Left, leftBottomRect.Left);
				rect.Right = max(rightTopRect.Right, rightBottomRect.Right);
				rect.Top = min(leftTopRect.Top, rightTopRect.Top);
				rect.Bottom = max(leftBottomRect.Bottom, rightBottomRect.Bottom);
			}
		}

		void Print()
		{
			cout << "Top: " << rect.Top << endl;
			cout << "Left: " << rect.Left << endl;
			cout << "Bottom: " << rect.Bottom << endl;
			cout << "Right: " << rect.Right << endl;
		}
	};
}

using namespace Blob;

int main()
{
	Grid grid = 
	{
		{
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
			{ 0, 0, 1, 1, 1, 1, 1, 0, 0, 0 },
			{ 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
			{ 0, 0, 1, 1, 1, 1, 1, 0, 0, 1 },
			{ 0, 0, 0, 0, 1, 0, 1, 0, 0, 0 },
			{ 0, 0, 0, 0, 1, 0, 1, 0, 0, 0 },
			{ 0, 0, 0, 0, 1, 1, 1, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 1, 0, 0, 1, 0, 1, 0, 0, 0, 1 }
		}
	};
	
	BlobRect blobRect(grid);
	blobRect.Detect();
	blobRect.Print();

	// Uncomment to run
	//
	//cout << "Multi-threaded version" << endl;
	//blobRect.Detect(/*multithreaded?*/ true);
	//blobRect.Print();
}
