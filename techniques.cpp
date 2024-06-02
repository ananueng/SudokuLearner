#include "stdafx.h"
#include "sudokuboard.h"
#include "cell.h"

int SudokuBoard::LastCandidate(Cell *cell) {
    // only called when cell value == 0 (unsolved)
    assert(cell->_value == 0);
    int value = 0;
    // if (cell->_value != 0) {
    //     return 0;
    // }

    if (Cell::BitCount(cell->_bitmask) == 1) {
        // all other values have been eliminated for this cell, it must be "the one"
        value = Cell::GetCellValueFromBitmask(cell->_bitmask);
        SetCellValue(cell->_rowIndex, cell->_colIndex, value);
        LogWithoutLineBreak("D1: LastCandidate - setting value of %d at (r=%d c=%d)\n", value, cell->_rowIndex, cell->_colIndex);
        CombinedDump();
        return 1;
    }

    // no progress
    return 0;
}

// SingleCandidate looks at the non-eliminated values at "cell" and compares it to all the
// other non-eliminated values from the {square,row,column} set.  If a non-eliminated value appears only once,
// then it will set set that value and update the candidates in the other sets
int SudokuBoard::SingleCandidate(Cell *cell, CellSet *set) {
    assert(cell->_value == 0);
    uint16_t wOtherMask = 0;
    int value = 0;

    // if (cell->_value != 0) {
    //     return 1;
    // }

    // if (Cell::BitCount(cell->_bitmask) == 1) {
    //     // all other values have been eliminated for this cell, it must be "the one"
    //     value = Cell::GetCellValueFromBitmask(cell->_bitmask);
    //     SetCellValue(cell->_rowIndex, cell->_colIndex, value);
    //     LogWithoutLineBreak("LastCandidate - setting value of %d at (r=%d c=%d)\n", value, cell->_rowIndex, cell->_colIndex);
    //     CombinedDump();
    //     return value;
    // }


    for (int index = 0; index < 9; index++) {
        Cell *othercell = set->_set[index];

        if (othercell != cell) {
            wOtherMask |= othercell->_bitmask;
        }
    }

    // subtract wOtherMask from cell->m_bitmask
    // if the result is something that has only 1 bit set, then it follows that there is exactly one value for that cell

    uint16_t wResult = (cell->_bitmask & ~wOtherMask);

    if (Cell::BitCount(wResult) == 1) {
        value = Cell::GetCellValueFromBitmask(wResult);

        SetCellValue(cell, value);

        CELL_RELATIONSHIP relate = cell->GetRelationship(set);

        const char *psz = g_relationship_name[relate];

        Log("D1: SingleCandidate - setting value of %d at (r=%d c=%d) [%s elimination]", value, cell->_rowIndex, cell->_colIndex, psz);
        CombinedDump();
        return 2;
    }

    // no progress
    return 0;
}

int SudokuBoard::BoxLineReduction(CellSet *set) {

    bool placed[10] = {0};
    Cell *cell = NULL;
    Cell *comparecell = NULL;
    bool fCanDoBoxLineReduction;
    int reducecount = 0;
    
    for (int cellindex = 0; cellindex < 9; cellindex++) {
        int value = set->_set[cellindex]->_value;
        placed[value] = true;  // value[0] is irrelevant
    }

    for (int valueindex = 1; valueindex <= 9; valueindex++) {
        fCanDoBoxLineReduction = true;
        comparecell = NULL;

        if (placed[valueindex] == true) {
            continue;
        }

        // look at all the cells in this set that have index in the candidate list.   If all the cells of this set appear
        // in the same box, we can do boxline reduction

        for (uint16_t cellindex = 0; cellindex < 9; cellindex++) {
            cell = set->_set[cellindex];

            if (cell->_value != 0) {
                continue;
            }

            if (cell->IsOkToSetValue(valueindex)) {
                // we found a cell that contains a missing value for this set it it's candidate list
                if (comparecell == NULL) {
                    comparecell = cell;
                }
                else {
                    if (comparecell->_square != cell->_square) {
                        // "index" appears in m_bitmask in cells across different boxes - time to skip
                        fCanDoBoxLineReduction = false;
                        break;
                    }
                }
            }
        }

        cell = NULL;

        if (comparecell && fCanDoBoxLineReduction) {
            CellSet *square = comparecell->_square;

            // iterate over all the cells in the square that don't belong to "set" (row/column)
            for (uint16_t cellindex = 0; cellindex < 9; cellindex++) {
                cell = square->_set[cellindex];
                if  ((cell->_row == set) || (cell->_column == set)) {
                    continue;
                }

                assert(cell != comparecell);

                assert(cell->_value != valueindex);

                // another cell in the same square that doesn't belong to "set", remove valueindex from the bitmask
                if (cell->ClearValueFromMask(valueindex)) {
                    Log("D2: BoxLineReduced %d from cell(r=%d c=%d)", valueindex, cell->_rowIndex, cell->_colIndex);
                    reducecount++;
                    CombinedDump();
                }
            }
        }
    }

    return reducecount;
}

int SudokuBoard::ClaimNumbers(uint16_t mask, CellSet *square, CellSet *set) {
    // Remove all the values of "mask" from "set" that are not in "square"

    int value;
    int count = 0;
    
    value = Cell::GetCellValueFromBitmaskAndClear(mask);
    while (value != 0) {
        for (int index = 0; index < 9; index++) {
            Cell *cell = set->_set[index];
            if ((cell->_square != square) && (cell->_value == 0)) {
                if (cell->IsOkToSetValue(value)) {
                    Log("D2: Number Claiming - removing %d from candidate list of cell at (r=%d c=%d)", value, cell->_rowIndex, cell->_colIndex);
                }

                cell->ClearValueFromMask(value);
                count++;
                CombinedDump();
                // TODO: check this
            }
        }

        value = Cell::GetCellValueFromBitmaskAndClear(mask);
    }

    return count;
}

int SudokuBoard::DoNumberClaiming(CellSet *square) {
    int count  = 0;
    uint16_t maskPerRow[3] ={0};
    uint16_t maskPerCol[3] = {0};
    uint16_t wMaskRow;
    uint16_t wMaskCol;

    for (int r = 0; r < 3; r++) {
        wMaskRow = 0;
        for (int c = 0; c < 3; c++) {
            Cell *cell = square->_set[r*3+c];
            if (cell->_value == 0) {
                wMaskRow |= cell->_bitmask;
            }
        }
        maskPerRow[r] = wMaskRow;
    }

    for (int c = 0; c < 3; c++) {
        wMaskCol = 0;
        for (int r = 0; r < 3; r++) {
            Cell *cell = square->_set[r*3+c];
            if (cell->_value == 0) {
                wMaskCol |= cell->_bitmask;
            }
        }
        maskPerCol[c] = wMaskCol;
    }

    // for each row/col, look to see if there are any bits in the mask that are not in the other two row/cols

    for (int r = 0; r < 3; r++) {
        uint16_t wMaskedOut;
        int rx[3][3] = {{0,1,2}, {1,0,2}, {2,0,1}};
        wMaskedOut = (maskPerRow[rx[r][0]] & ~(maskPerRow[rx[r][1]] | maskPerRow[rx[r][2]]));

        if (wMaskedOut != 0) {
            Cell *refcell = square->_set[r*3];
            CellSet *row = refcell->_row;
            count += ClaimNumbers(wMaskedOut, square, row);
        }
    }

    for (int c = 0; c < 3; c++) {
        uint16_t wMaskedOut;
        int cx[3][3] = {{0,1,2}, {1,0,2}, {2,0,1}};
        wMaskedOut = (maskPerCol[cx[c][0]] & ~(maskPerCol[cx[c][1]] | maskPerCol[cx[c][2]]));

        if (wMaskedOut != 0) {
            Cell *refcell = square->_set[c];
            CellSet *col = refcell->_column;
            count += ClaimNumbers(wMaskedOut, square, col);
        }
    }

    return count;
}

// Naked Pair
int SudokuBoard::NakedPair(Cell *cell, CellSet *set) {
    // scan the entire set.  If a pair of cells are found whereby both have the same bitmask of two
    // candidate values, then those candidate values can be erased from the rest of the cells in the set

    Cell *matchcell = NULL;
    bool progress = false;

    if (cell->_value != 0)
        return 0;

    if (Cell::BitCount(cell->_bitmask) != 2) {
        return 0;
    }

    // scan the set looking for another cell with the same bitmask
    for (int cellindex = 0; cellindex < 9; cellindex++) {
        Cell *othercell = set->_set[cellindex];

        if (othercell == cell) {
            continue;
        }

        if (othercell->_value != 0) {
            continue;
        }

        if (othercell->_bitmask == cell->_bitmask) {
            matchcell = othercell;
            break;
        }
    }

    if (matchcell == NULL)
        return 0;

    uint16_t wBitmask = matchcell->_bitmask;

    int values[2];
    values[0] = Cell::GetCellValueFromBitmaskAndClear(wBitmask);
    values[1] = Cell::GetCellValueFromBitmaskAndClear(wBitmask);

    assert(values[0] != 0);
    assert(values[1] != 0);
    assert(wBitmask == 0);

    // remove the bitmask from all other cells in the set
    for (int cellindex = 0; cellindex < 9; cellindex++) {
        Cell *othercell = set->_set[cellindex];

        if ((othercell == matchcell) || (othercell == cell))
            continue;

        for (int x = 0; x < 2; x++) {
            if (othercell->IsOkToSetValue(values[x])) {
                // Log("NakedPair - %d removed from cell at (r=%d c=%d)", values[x], othercell->_rowIndex, othercell->_colIndex);
                Log("D2: NakedPair - (r=%d c=%d): {%s}, (r=%d c=%d): {%s}. Removed %d from cell at (r=%d c=%d)",
                        cell->_rowIndex, cell->_colIndex, cell->BitmaskToString().c_str(),
                        matchcell->_rowIndex, matchcell->_colIndex, matchcell->BitmaskToString().c_str(),
                        values[x], othercell->_rowIndex, othercell->_colIndex);
                othercell->ClearValueFromMask(values[x]);
                progress = true;
                CombinedDump();
            }
        }
    }

    return progress ? 1 : 0;
}

int SudokuBoard::NakedTriple(Cell *cell, CellSet *set) {
    uint16_t wUnion;
    int value;
    bool progress = false;

    if(cell->_value != 0) {
        return 0;
    }

    int bitcount = Cell::BitCount(cell->_bitmask);

    if ((bitcount < 2) || (bitcount > 3)) {
        return 0;                
    }

    // look for two other cells that have an intersection in bitmasks with the cell passed in
    Cell *matches[2] = {0};
    int matchindex = 0;

    for (int index = 0; index < 9; index++) {
        Cell *othercell = set->_set[index];
        if (othercell == cell)
            continue;

        if (othercell->_value != 0)
            continue;

        if ((othercell->_bitmask & cell->_bitmask) == 0) {
            // no intersection
            continue;
        }

        wUnion = othercell->_bitmask | cell->_bitmask;

        if (Cell::BitCount(wUnion) > 3) {
            continue;
        }

        matches[matchindex] = othercell;
        matchindex++;
        if (matchindex == 2)
            break;
    }

    if (!(matches[0] && matches[1])) {
        return 0;
    }

    wUnion = matches[0]->_bitmask | matches[1]->_bitmask | cell->_bitmask;

    // something would be "off" if we had three cells only having a union of two or less bits
    // If the union bitmask is 4 or more, then my assertion that the loop above shouldn't do that is wrong
    //assert(Cell::BitCount(wUnion) == 3);

    if (Cell::BitCount(wUnion) != 3)
        return 0;

    // we have our three cells, let's pull these bits out of the other cells that aren't set

    for (int index = 0; index < 9; index++) {
        Cell *othercell = set->_set[index];
        if (othercell == cell)
            continue;

        if (othercell == matches[0])
            continue;

        if (othercell == matches[1])
            continue;

        if (othercell->_value != 0)
            continue;

        if (othercell->_bitmask & wUnion) {
            uint16_t wMask = wUnion;

            while (wMask) {
                value = Cell::GetCellValueFromBitmaskAndClear(wMask);
                if (othercell->IsOkToSetValue(value)) {
                    othercell->ClearValueFromMask(value);
                    // Log("NakedTriple - %d removed from cell at (r=%d c=%d)", value, othercell->_rowIndex, othercell->_colIndex);
                    Log("D2.5: NakedTriple - (r=%d c=%d): {%s}, (r=%d c=%d): {%s}, (r=%d c=%d): {%s}. Removed %d from cell at (r=%d c=%d)",
                        cell->_rowIndex, cell->_colIndex, cell->BitmaskToString().c_str(),
                        matches[0]->_rowIndex, matches[0]->_colIndex, matches[0]->BitmaskToString().c_str(),
                        matches[1]->_rowIndex, matches[1]->_colIndex, matches[1]->BitmaskToString().c_str(),
                        value, othercell->_rowIndex, othercell->_colIndex);
                    CombinedDump();
                    progress = true;
                }
            }
        }
    }

    return progress ? 1 : 0;
}


int SudokuBoard::DoXWingSets(CellSet *sets) {
    // look at every row where there are exactly two candidate cells for a particular value
    // If there is another row exactly two candidate cells for the same value, then it can be removed from the columns in the other rows

    int value = 0;
    uint16_t wBitMask = 0;
    int valuecounts[10][10] = {0}; // [row][value]
    int col1, col2;
    int changecount = 0;
    bool fRet;

    for  (int rowindex = 0; rowindex < 9; rowindex++) {
        CellSet *row = &sets[rowindex];

        for (int cellindex = 0; cellindex < 9; cellindex++) {
            Cell *cell = row->_set[cellindex];
            if (cell->_value != 0)
                continue;

            wBitMask = cell->_bitmask;

            while (wBitMask) {
                value = Cell::GetCellValueFromBitmaskAndClear(wBitMask);
                valuecounts[rowindex][value] = valuecounts[rowindex][value] + 1;
            }
        }
    }

    // for each row, look for instances where there are only two possible cells for a given instance

    for (int rowindex = 0; rowindex < 9; rowindex++) {
        for (int valueindex = 1; valueindex <= 9; valueindex++) {
            if (valuecounts[rowindex][valueindex] == 2) {

                fRet = XWing_FindColumnIndices(&sets[rowindex], valueindex, col1, col2);
                if (fRet == false)
                    continue;
              

                // find the matching row
                CellSet *matchrow = NULL;
                for (int nextrowindex = rowindex+1; nextrowindex < 9; nextrowindex++) {
                    if (valuecounts[nextrowindex][valueindex] == 2) {
                        matchrow = &sets[nextrowindex];

                        Cell *c1 = matchrow->_set[col1];
                        Cell *c2 = matchrow->_set[col2];

                        if (!(c1->IsOkToSetValue(valueindex) && c2->IsOkToSetValue(valueindex))) {
                            matchrow = NULL;
                        }
                        else {
                            break;
                        }
                    }
                }


                if (matchrow != NULL) {
                    changecount = XWing_DoFilter(sets, &sets[rowindex], matchrow, valueindex, col1, col2);
                }
            }
        }
    }

    return changecount;
}

// 
bool SudokuBoard::XWing_FindColumnIndices(CellSet *row, int value, int &col1, int &col2) {
    col1 = -1;
    col2 = -1;

    for (int cellindex = 0; cellindex < 9; cellindex++) {
        Cell *cell = row->_set[cellindex];

        if (cell->IsOkToSetValue(value)) {
            if (col1 == -1)
                col1 = cellindex;
            else if (col2 == -1)
                col2 = cellindex;
            else
                assert(false);  // a third instance found in this row???
        }
    }

    if ((col1 == -1) || (col2 == -1)) {
        // this can happen if a previous loop in DoXWingSets removed one of the values
        return false;
    }
    return true;
}

// Remove values that are identified from x-wing
int SudokuBoard::XWing_DoFilter(CellSet *sets, CellSet *firstrow, CellSet *matchrow, int value, int col1, int col2) {
    int changecount = 0;

    bool fUsingColumns = (firstrow->_set[0]->_rowIndex == matchrow->_set[0]->_rowIndex);

    for (int rowindex = 0; rowindex < 9; rowindex++) {
        CellSet  *row = &sets[rowindex];

        if ((row == firstrow) || (row == matchrow)) {
            continue;
        }

        // now look at col1 and col2 to see if value appears in the candidate list for these cells

        Cell *cells[2];
        
        cells[0] = row->_set[col1];
        cells[1] = row->_set[col2];

        for (int x = 0; x < 2; x++) {
            if (cells[x]->IsOkToSetValue(value)) {
                Log("D3: XWing - removing %d from cell at (r=%d c=%d)", value, cells[x]->_rowIndex, cells[x]->_colIndex);

                if (fUsingColumns == false) {
                    Log("   XWing cells are at (r=%d c=%d) (r=%d c=%d) (r=%d c=%d) (r=%d c=%d)", firstrow->_set[0]->_rowIndex, col1, firstrow->_set[0]->_rowIndex, col2, matchrow->_set[0]->_rowIndex, col1, matchrow->_set[0]->_rowIndex, col2);
                }
                else {
                    Log("   XWing cells are at (r=%d c=%d) (r=%d c=%d) (r=%d c=%d) (r=%d c=%d)", col1, firstrow->_set[0]->_colIndex, col1, matchrow->_set[0]->_colIndex, col2, firstrow->_set[0]->_colIndex, col2, matchrow->_set[0]->_colIndex);
                }


                cells[x]->ClearValueFromMask(value);
                changecount++;
                CombinedDump();
            }
        }
    }

    return changecount;
}