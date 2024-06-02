#include "stdafx.h"
#include "sudokuboard.h"
#include "cell.h"

void SudokuBoard::LogWithoutLineBreak(const char *pwszFormat, ...) {
    va_list args;
    va_start(args, pwszFormat);
    char szMsg[1024];

    vsnprintf(szMsg, 1024, pwszFormat, args);

    std::cout << szMsg;

    va_end(args);
}

void SudokuBoard::Log(const char *pwszFormat, ...) {
    va_list args;
    va_start(args, pwszFormat);
    char szMsg[1024];

    vsnprintf(szMsg, 1024, pwszFormat, args);

    std::cout << szMsg << std::endl;

    va_end(args);
}

void SudokuBoard::CombinedDump() {
    int value;
    int count;

    for (int row = 0; row < 9; row++) {
        // Print the candidates
        for (int col = 0; col < 9; col++) {
            value = m_board[row][col]._value;
            if (value != 0) {
                // already filled
                LogWithoutLineBreak("%-9c", value + '0');  // 9 spaces (8 after the value)
            }
            else {
                // candidates
                LogWithoutLineBreak("{");
                uint16_t wMask = m_board[row][col]._bitmask;
                count = 0;
                while (wMask) {
                    value = Cell::GetCellValueFromBitmaskAndClear(wMask);
                    LogWithoutLineBreak("%c", value + '0');
                    count++;
                }

                LogWithoutLineBreak("}");
                int remainingSpaces = 9 - count - 2; // 9 total characters minus the count and braces
                while (remainingSpaces > 0) {
                    LogWithoutLineBreak(" ");
                    remainingSpaces--;
                }
            }

            if ((col % 3 == 2) && (col != 8))
                LogWithoutLineBreak("| ");
        }

        // Newline after each row
        Log("");

        // Print separator lines after each 3rd row
        if (row % 3 == 2) {
            Log("-----------------------------------------------------------------------------------");
        }
    }
}



// void SudokuBoard::CombinedDump() {
//     int value;
//     int count;

//     for (int row = 0; row < 9; row++) {
//         // Print the standard board on the left
//         for (int col = 0; col < 9; col++) {
//             LogWithoutLineBreak("%c ", m_board[row][col]._value ? (m_board[row][col]._value + '0') : '?');

//             if ((col % 3 == 2) && (col != 8))
//                 LogWithoutLineBreak("| ");
//         }

//         // Add separator between the two boards
//         LogWithoutLineBreak("\t||\t");

//         // Print the candidates on the right
//         for (int col = 0; col < 9; col++) {
//             value = m_board[row][col]._value;
//             if (value != 0) {
//                 // already filled
//                 LogWithoutLineBreak("%-9c", value + '0');  // 9 spaces (8 after the value)
//             }
//             else {
//                 // candidates
//                 LogWithoutLineBreak("{");
//                 uint16_t wMask = m_board[row][col]._bitmask;
//                 count = 0;
//                 while (wMask) {
//                     value = Cell::GetCellValueFromBitmaskAndClear(wMask);
//                     LogWithoutLineBreak("%c", value + '0');
//                     count++;
//                 }

//                 LogWithoutLineBreak("}");
//                 int remainingSpaces = 9 - count - 2; // 9 total characters minus the count and braces
//                 while (remainingSpaces > 0) {
//                     LogWithoutLineBreak(" ");
//                     remainingSpaces--;
//                 }
//             }

//             if ((col % 3 == 2) && (col != 8))
//                 LogWithoutLineBreak("| ");
//         }

//         // Newline after each row
//         Log("");

//         // Print separator lines after each 3rd row
//         if (row % 3 == 2) {
//             Log("---------------------\t||\t------------------------------ ------------------------------ ------------------------------");
//         }
//     }
// }




// void SudokuBoard::Dump() {
//     for (int row = 0; row < 9; row++) {
//         for (int col = 0; col < 9; col++) {
//             LogWithoutLineBreak("%c ", m_board[row][col]._value ? (m_board[row][col]._value + '0') : '?');

//             if ((col % 3 == 2) && (col != 8))
//                 LogWithoutLineBreak("| ");
//         }
//         Log("");

//         if (row % 3 == 2) {
//             Log("---------------------");
//         }
           
//     }
//     // TODO: add prints for candidates
// }

// void SudokuBoard::FullDump() {
//     int value;
//     int count;

//     for (int row = 0; row < 9; row++) {
//         for (int col = 0; col < 9; col++) {
//             value = m_board[row][col]._value;
//             if (value != 0) {
//                 LogWithoutLineBreak("%c      ", value + '0');
//             }
//             else {
//                 LogWithoutLineBreak("{");
//                 uint16_t wMask = m_board[row][col]._bitmask;
//                 count = 0;
//                 while (wMask) {
//                     value = Cell::GetCellValueFromBitmaskAndClear(wMask);
//                     LogWithoutLineBreak("%c", value + '0');
//                     count++;
//                 }

//                 int remainingspaces = 5 - count;
//                 LogWithoutLineBreak("}");
//                 while (remainingspaces > 0) {
//                     LogWithoutLineBreak(" ");
//                     remainingspaces--;
//                 }


//             }

//             if ((col % 3 == 2) && (col != 8))
//                 LogWithoutLineBreak("| ");
//         }

//         Log("");
//         if (row % 3 == 2) {
//             Log("------------------------------------------------------------------");
//         }

//     }
// }