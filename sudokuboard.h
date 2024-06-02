/*
    Copyright 2017 John Selbie
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef SUDODKU_BOARD_H
#define SUDODKU_BOARD_H


#include "cell.h"

class SudokuBoard
{
public:
    SudokuBoard();

    bool Init();

    bool LoadFromFile(const std::string& filename);

    std::string GetBoardState();

    bool Solve();

    bool IsSolved();
    bool IsValid();

    void Dump();
    void FullDump();
    void CombinedDump();
    std::string BitmaskToString(uint16_t bitmask);




protected:
    Cell m_board[9][9];  // row, column
    CellSet m_squares[9];
    CellSet m_rows[9];
    CellSet m_cols[9];

    // ScanForSolution will do one full pass on the on the board
    // It will attempt to assign values to cells and eliminate values from the candidate list of each cell
    void ScanForSolution();

    // SetCellValue will set the value at the specified cell.  It will also clear out the value from other cells that are
    // in the same area (row, column, square) as this cell
    void SetCellValue(int row, int col, int value, bool fPerm=false);
    void SetCellValue(Cell *cell, int value, bool fPerm=false);


    // the following are individual algorithms for solving

    int LastCandidate(Cell *cell);

    // SingleCandidate will look at the sell within the specified set.  If only one bit is set in the candidate list of the cell
    // or contains a candidate value that doesn't appear anywhere else in the set, then that value gets assigned to the cell
    int SingleCandidate(Cell *cell, CellSet *set);

    // NakedPair will evaluate "cell" if it only has two candidate values.  If it does, it will try to find another cell
    // in "set" that has the same pair of candidate values.  If it finds a match, the seven other cells in "set" have the two candidate
    // values removed
    int NakedPair(Cell *cell, CellSet *set);


    // NakedTriple works just like NakedPair except it looks for 3 candidate values across three sells in the same set
    int NakedTriple(Cell *cell, CellSet *set);


    // BoxLineReduction takes a row or column set as a parameter
    // This function will determine what values are missing from this row or column.
    // For each missing value in "set", it will detemine if all the candidate cells for this value lie in the same box.
    // If so, it will remove the value from the candidate list of cells in this SQUARE that do not belong to this set (row/column)
    int BoxLineReduction(CellSet *set); // set must not be a square - rows or columns only


    // DoNumberClaiming
    // if a candidate value appears in only 1 row/column of a square, then it can be eliminated from the candidate
    // lists of the cells in the same row/column outside the square
    int DoNumberClaiming(CellSet *set);
    int ClaimNumbers(uint16_t mask, CellSet *square, CellSet *set);

    // XWing pairs are tricky.  If a value appears in the candidate lists of exactly two cells of a given row and there is
    // another row with the two cells holding the candidate value in the same two columns, then other columns on other rows have this value eliminated
    // from the candidate list.
    int DoXWingSets(CellSet *sets); // all the rows or all the columns
    bool XWing_FindColumnIndices(CellSet *row, int value, int &col1, int &col2);
    int XWing_DoFilter(CellSet *sets, CellSet *firstrow, CellSet *matchrow, int value, int col1, int col2);

    void LogWithoutLineBreak(const char *pszFormat, ...);
    void Log(const char *pszFormat, ...);

private:
    // disable assignment, comparsion, and copy constructor
    SudokuBoard(const SudokuBoard& other) {};
    SudokuBoard& operator=(const SudokuBoard& other) { return *this; };
    bool operator==(const SudokuBoard& other) { return false; }

};

#endif
