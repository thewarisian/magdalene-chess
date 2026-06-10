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
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 8
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 7
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 6
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 5
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 4
    chessboard::row{'r','_','_','_','N','K','_','_'}, // rank 3
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 2 - white bishop e2
    chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 1 - white king e1
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
    bb epawns          = b.copyPieceBitboard(enemy, PieceType::PAWN);
    bb eknights        = b.copyPieceBitboard(enemy, PieceType::KNIGHT);
    bb ebishops        = b.copyPieceBitboard(enemy, PieceType::BISHOP);
    bb erooks          = b.copyPieceBitboard(enemy, PieceType::ROOK);
    bb equeens         = b.copyPieceBitboard(enemy, PieceType::QUEEN);
    bb eking           = b.copyPieceBitboard(enemy, PieceType::KING);
    bb enemyOccupied  = b.copyAllPiecesBitboard(enemy);
    bb enPassant      = b.getEnPassantAttackSquare();

    bitboard::display(
       movegen::calculatePinMasks(col, occupied, pawns, knights, bishops, rooks, queens, king, epawns, eknights, ebishops, erooks, equeens)[19]
    );

    return 0;
}