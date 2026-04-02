#include <iostream>

#include "board/chessboard.h"
#include "move/movegen.h"

int main() {
    chessboard::matrix board = {
        chessboard::row{'r','n','b','q','k','b','n','r'}, // rank 8
        chessboard::row{'p','p','p','p','p','p','p','p'}, // rank 7
        chessboard::row{'_','_','_','N','_','_','_','_'}, // rank 6
        chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 5
        chessboard::row{'_','_','_','N','_','_','_','_'}, // rank 4
        chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 3
        chessboard::row{'P','P','P','P','P','P','P','P'}, // rank 2
        chessboard::row{'R','N','B','Q','K','B','N','R'}, // rank 1
    };

    //chessboard::GameBoard b;

    chessboard::GameBoard b(
        board,
        false,
        true, true, true, true,
        -1,
        0,
        1
    ); // Default initial position

    // chessmove::Move m1 = {55, 39, 'p', 'E'}, m2 = {52, 36, 'p', 'E'}, m3 = {0,0,'P','E'};
    // b.makeMove(m1);
    //bitboard::display(b.getEnPassantAttackSquare());

   bitboard::display(movegen::calculateBlackPawnMoves(b));

    //std::vector<std::string> v = movegen::getMovesList(movegen::calculateWhitePawnMoves(b));
    //for(std::string& move : v) { std::cout << move << "\n"; }
    return 0;
}