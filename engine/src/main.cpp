#include <iostream>

#include "board/chessboard.h"
#include "move/movegen.h"

// TERMINAL COMMAND TO RUN: g++ src/**/*.cpp -Iinclude -std=c++17 -o chess && ./chess
int main(int argc, char* argv[]) {
    
    if(argc < 2) {
        chessboard::GameBoard b;
        std::cout << b.toString();
    }
    else {
        std::string fen = argv[1];
        chessboard::GameBoard b(fen);
        std::cout << b.toString();
    }

    return 0;

    // chessboard::matrix board = {
    //     chessboard::row{'r','n','b','q','k','b','n','r'}, // rank 8
    //     chessboard::row{'p','p','p','p','p','p','p','p'}, // rank 7
    //     chessboard::row{'_','_','_','N','_','_','_','_'}, // rank 6
    //     chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 5
    //     chessboard::row{'_','_','_','N','_','_','_','_'}, // rank 4
    //     chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 3
    //     chessboard::row{'P','P','P','P','P','P','P','P'}, // rank 2
    //     chessboard::row{'R','N','B','Q','K','B','N','R'}, // rank 1
    // };

    // chessboard::GameBoard b(
    //     board,
    //     false,
    //     true, true, true, true,
    //     Square::None,
    //     0,
    //     1
    // ); // Default initial position

    // chessmove::Move m1 = {Square::E2, Square::E4, MoveType::DoublePawnPush};
    // chessmove::Move m2 = {Square::A7, Square::A5, MoveType::DoublePawnPush};
    // b.makeMove(m1, Color::WHITE, PieceType::PAWN, Color::None, PieceType::None);
    // b.makeMove(m2, Color::BLACK, PieceType::PAWN, Color::None, PieceType::None);
    // bitboard::display(b.getEnPassantAttackSquare());

    //std::cout << b.toString();
   //bitboard::display(movegen::calculateWhitePawnMoves(b));

    //std::vector<std::string> v = movegen::getMovesList(movegen::calculateWhitePawnMoves(b));
    //for(std::string& move : v) { std::cout << move << "\n"; }
}