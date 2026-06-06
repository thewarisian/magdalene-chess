#include <iostream>

#include "board/chessboard.h"
#include "move/movegen.h"

using bb = bitboard::bitmap;

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
    chessboard::row{'_','_','_','_','k','_','_','_'}, // rank 8
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 7
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 6
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 5
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 4
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 3
    chessboard::row{'_','_','_','_','B','_','_','_'}, // rank 2 - white bishop e2
    chessboard::row{'_','_','_','_','K','_','_','_'}, // rank 1 - white king e1
};

    chessboard::GameBoard b
    (
        board,
        false,
        true, true, true, true,
        Square::None,
        0,
        1
    )
    ;

    Color col = Color::WHITE;
    bb pawns          = b.copyPieceBitboard(col, PieceType::PAWN);
    bb knights        = b.copyPieceBitboard(col, PieceType::KNIGHT);
    bb bishops        = b.copyPieceBitboard(col, PieceType::BISHOP);
    bb rooks          = b.copyPieceBitboard(col, PieceType::ROOK);
    bb queens         = b.copyPieceBitboard(col, PieceType::QUEEN);
    bb king           = b.copyPieceBitboard(col, PieceType::KING);
    bb friendOccupied = b.copyAllPiecesBitboard(col);
    bb occupied       = b.copyAllPiecesBitboard();
    bb empty          = ~occupied;
    Color enemy       = (col == Color::WHITE) ? Color::BLACK : Color::WHITE;
    bb enemyOccupied  = b.copyAllPiecesBitboard(enemy);
    bb enPassant      = b.getEnPassantAttackSquare();

    bitboard::display(
        //movegen::calculatePawnMoves(col, pawns, enemyCapturables, empty)
        //movegen::calculateBishopTypeMoves(occupied, friendOccupied, Square::F1)
        movegen::calculatePlayerAttacks(col, pawns, knights, bishops, rooks, queens, king, enemyOccupied, empty, occupied, friendOccupied)
    );

    return 0;
}