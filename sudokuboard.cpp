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

#include "stdafx.h"
#include "sudokuboard.h"
#include "cell.h"

SudokuBoard::SudokuBoard() {
    Init();
}

bool SudokuBoard::Init() {
    // Reset any existing state
    // ------------------------------------------
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++){
            m_board[row][col].Reset();
        }
    }

    for (int i = 0; i < 9; i++) {
        m_rows[i].Reset();
        m_cols[i].Reset();
        m_squares[i].Reset();
    }
    // ------------------------------------------

    // start of initialization
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            m_board[row][col]._cellIndex = row * 9 + col;
        }
    }

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            Cell *cell = &m_board[row][col];
            CellSet *colset = &m_cols[col];
            CellSet *rowset = &m_rows[row];
            int squareindex = 3*(row/3) + (col / 3);
            int squarecellindex = 3*(row%3) + (col %3);

            CellSet *square = &m_squares[squareindex];

            square->_set[squarecellindex] = cell;
            rowset->_set[col] = cell;
            colset->_set[row] = cell;

            cell->_row = rowset;
            cell->_column = colset;
            cell->_square = square;

            cell->_rowIndex = row;
            cell->_colIndex = col;
            cell->_squareIndex = squareindex;
            cell->_squareCellIndex = squarecellindex;
        }
    }

    return true;
}

bool SudokuBoard::LoadFromFile(const std::string& filename) {
    std::ifstream infile(filename);

    char c;
    int row = 0;
    int col = 0;
    int value = 0;

    while (infile.is_open() && infile.good()) {
        infile >> c;

        value = 0;
        if ((c >= '0') && (c <= '9')) {
            value = c - '0';
        }

        SetCellValue(row, col, value, true);

        col++;
        if (col == 9) {
            col = 0;
            row++;
        }
        if (row == 9) {
            break;
        }
    }

    if (row != 9) {
        Log("Error processing file!");
        return false;
    }

    return true;
}

bool SudokuBoard::Solve() {
    bool fSolved = false;

    if (IsSolved())
        return true;

    int scancount = 0;
    std::string oldstate, state;
    state = GetBoardState();

    Log("\n");
    Log("Starting scans number - %d", scancount);
    CombinedDump();
    Log("\n");

    while(true) {
        oldstate = state;
        ScanForSolution();
        scancount++;
        state = GetBoardState();

        Log("\n");
        Log("Starting scans number - %d", scancount);
        // CombinedDump();
        Log("\n");

        if (state == oldstate)
            break;


        fSolved = IsSolved();
        if (fSolved) {
            break;
        }
    }

    Log("Number of scans - %d", scancount);
    if (fSolved) {
        Log("Board has been solved");
    }
    else {
        Log("Board has not been solved");
        Log("");
    }

    bool fValid = IsValid();
    Log("%sBoard is%s valid", fValid?"":"WARNING - ", fValid?"":" NOT");

    return fSolved;
}

bool SudokuBoard::IsSolved() {
    // it's solved if all 81 squares have a non-zero value and the board is valid

    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (m_board[r][c]._value == 0)
                return false;
        }
    }

    return true;
}

bool SudokuBoard::IsValid() {
    // check for validity
    int squarecount[9][10] = {0};
    int rowcount[9][10] = {0};
    int colcount[9][10] = {0};

    for (int index = 0; index < 9; index++) {
        CellSet *square = &m_squares[index];
        CellSet *row = &m_rows[index];
        CellSet *col = &m_cols[index];

        for (int cellindex = 0; cellindex < 9; cellindex++) {
            int value;

            value = square->_set[cellindex]->_value;
            squarecount[index][value]++;

            value = row->_set[cellindex]->_value;
            rowcount[index][value]++;

            value = col->_set[cellindex]->_value;
            colcount[index][value]++;
        }
    }

    for (int index = 0; index < 9; index++) {
        for (int valueindex = 1; valueindex <= 9; valueindex++) {
            if (squarecount[index][valueindex] > 1)
                return false;

            if (rowcount[index][valueindex] > 1)
                return false;

            if (colcount[index][valueindex] > 1)
                return false;
        }
    }

    return true;
}

std::string SudokuBoard::GetBoardState() {
    std::stringstream ss;

    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            ss << m_board[r][c]._value;
            ss << '.';
            ss << m_board[r][c]._bitmask;
            ss << '.';
        }
    }

    return ss.str();
}

void SudokuBoard::SetCellValue(int row, int col, int value, bool fPerm) {
    assert(row >=0);
    assert(row < 9);
    assert(col >=0);
    assert(col < 9);
    assert(value >= 0);
    assert(value <= 9);

    Cell *cell = &m_board[row][col];
    SetCellValue(cell, value, fPerm);
}

void SudokuBoard::SetCellValue(Cell *cell, int value, bool fPerm) {
    cell->SetValue(value);
    cell->_isPermanent = fPerm && (value != 0);

    // clear out the value bit from all the cells in the same row,col, and square
    CellSet* rowset = cell->_row;
    CellSet* square = cell->_square;
    CellSet* colset = cell->_column;

    for (int index = 0; index < 9; index++) {
        if (cell != rowset->_set[index]) {
            rowset->_set[index]->ClearValueFromMask(value);
        }

        if (cell != colset->_set[index]) {
            colset->_set[index]->ClearValueFromMask(value);
        }

        if (cell != square->_set[index]) {
            square->_set[index]->ClearValueFromMask(value);
        }
    }

    return;

}


void SudokuBoard::ScanForSolution() {
    // this function is the main loop that looks for a solution
    int value = 0;

    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            Cell *cell = &m_board[r][c];

            if (cell->_value != 0) {  // cell is already solved
                continue;
            }

            value = SingleCandidate(cell, cell->_square);
            if (value == 0) {
                value = SingleCandidate(cell, cell->_row);
            }
            if (value == 0) {
                value = SingleCandidate(cell, cell->_column);
            }

            if (value != 0) {
                // cell has been solved, don't use advanced techniques
                continue;
            }

            NakedPair(cell, cell->_square);
            NakedPair(cell, cell->_row);
            NakedPair(cell, cell->_column);

            NakedTriple(cell, cell->_square);
            NakedTriple(cell, cell->_row);
            NakedTriple(cell, cell->_column);

        }
    }


    for (int index = 0; index < 9; index++) {
        BoxLineReduction(&m_rows[index]);
        BoxLineReduction(&m_cols[index]);
    }


    for (int index = 0; index < 9; index++) {
        DoNumberClaiming(&m_squares[index]);
    }

    // CombinedDump();
    DoXWingSets(m_cols);
    DoXWingSets(m_rows);
}

// void SudokuBoard::ScanForSolution() {
//     // this function is the main loop that looks for a solution
//     int value = 0;

//     for (int r = 0; r < 9; r++) {
//         for (int c = 0; c < 9; c++) {
//             Cell *cell = &m_board[r][c];

//             if (cell->_value != 0) {  // cell is already solved
//                 continue;
//             }

//             value = SingleCandidate(cell, cell->_square);
//             if (value == 0) {
//                 value = SingleCandidate(cell, cell->_row);
//             }
//             if (value == 0) {
//                 value = SingleCandidate(cell, cell->_column);
//             }

//             if (value != 0) {
//                 // cell has been solved, don't use advanced techniques
//                 continue;
//             }

//             NakedPair(cell, cell->_square);
//             NakedPair(cell, cell->_row);
//             NakedPair(cell, cell->_column);

//             NakedTriple(cell, cell->_square);
//             NakedTriple(cell, cell->_row);
//             NakedTriple(cell, cell->_column);

//         }
//     }


//     for (int index = 0; index < 9; index++) {
//         BoxLineReduction(&m_rows[index]);
//         BoxLineReduction(&m_cols[index]);
//     }


//     for (int index = 0; index < 9; index++) {
//         DoNumberClaiming(&m_squares[index]);
//     }

//     // CombinedDump();
//     DoXWingSets(m_cols);
//     DoXWingSets(m_rows);
// }
