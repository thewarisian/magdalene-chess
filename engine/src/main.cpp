#include <iostream>

#include "board/chessboard.h"
#include "move/movegen.h"

// TERMINAL COMMAND TO RUN: g++ engine/src/**/*.cpp -Iengine/include -std=c++17 -o chess && ./chess
int main(int argc, char* argv[]) {
    
    //FINAL CODE TO INTEGRATE WITH GUI
    // if(argc < 2) {
    //     chessboard::GameBoard b;
    //     std::cout << b.toString();
    // }
    // else {
    //     std::string fen = argv[1];
    //     chessboard::GameBoard b(fen);
    //     std::cout << b.toString();
    // }


    // ===================================================
    chessboard::matrix board = {
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 8
    chessboard::row{'_','_','_','_','p','_','_','_'}, // rank 7 - black pawn on e7 (diagonal capture)
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 6
    chessboard::row{'_','_','_','_','Q','_','_','_'}, // rank 5 - white queen on e5
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 4
    chessboard::row{'_','_','p','_','_','_','_','_'}, // rank 3 - black pawn on c3 (diagonal capture)
    chessboard::row{'_','_','_','_','P','_','_','_'}, // rank 2 - white pawn on e2 (blocks file)
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 1
};

    chessboard::GameBoard b(
        board,
        false,
        true, true, true, true,
        Square::None,
        0,
        1
    ); // Default initial position

    // chessmove::Move m1 = {Square::E2, Square::E4, MoveType::DoublePawnPush};
    // chessmove::Move m2 = {Square::A7, Square::A5, MoveType::DoublePawnPush};
    // b.makeMove(m1, Color::WHITE, PieceType::PAWN, Color::None, PieceType::None);
    // b.makeMove(m2, Color::BLACK, PieceType::PAWN, Color::None, PieceType::None);
    // bitboard::display(b.getEnPassantAttackSquare());

    //std::cout << b.toString();
    // bitboard::display(movegen::calculateWhitePawnMoves(b));
    // std::cout << "\n";
    bitboard::display(movegen::calculateQueenMoves(b, Square::E5, Color::WHITE));

    // std::vector<std::string> v = movegen::getMovesList(movegen::calculateWhitePawnMoves(b));
    // for(std::string& move : v) { std::cout << move << "\n"; }

    // for(bitboard::bitmap diag : bitboard::ANTI_DIAGONAL) {
    //     bitboard::display(diag);
    //     std::cout << "\n";
    // }

   //bitboard::display(bitboard::getAntiDiagonalMask(Square::F4));

    return 0;
}