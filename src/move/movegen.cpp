#include "move/movegen.h"

namespace movegen {
    std::vector<std::string> getMovesList(bitboard::bitmap movesBitboard) {
        std::vector<std::string> destinationSquares;

        while(movesBitboard) {
            //Get bit index of least significant set bit (rightmost 1-bit)
            int nextBitIdx = bitboard::popLSBIndex(movesBitboard);
            //Convert to tile name and add to vector
            destinationSquares.push_back(chessboard::bitIndexToTileString(nextBitIdx));
            //Remove least significant 1 bit
            movesBitboard &= (movesBitboard-1);
        }

        return destinationSquares;
    }

    bitboard::bitmap calculateWhitePawnMoves(const chessboard::GameBoard& b) {
        bitboard::bitmap whitePawns = b.getPieceBitboard('P');
        bitboard::bitmap blackCapturables = b.getAllPiecesBitboard('B') | b.getEnPassantAttackSquare();
        bitboard::bitmap emptySquares = b.getAllPiecesBitboard('N');

        //rightward attack (including en passant)
        bitboard::bitmap pawnMovesBitboard = (whitePawns<<7) & blackCapturables & ~bitboard::FILE[0];
        //leftward attack (including en passant)
        pawnMovesBitboard |= (whitePawns<<9) & blackCapturables & ~bitboard::FILE[7];
        //single pawn push
        pawnMovesBitboard |= (whitePawns<<8) & emptySquares;
        //double pawn push
        pawnMovesBitboard |= (whitePawns<<16) & emptySquares & (emptySquares << 8) & bitboard::RANK[3];

        return pawnMovesBitboard;
    }

    bitboard::bitmap calculateBlackPawnMoves(const chessboard::GameBoard& b) {
        bitboard::bitmap blackPawns = b.getPieceBitboard('p');
        bitboard::bitmap whiteCapturables = b.getAllPiecesBitboard('W') | b.getEnPassantAttackSquare();
        bitboard::bitmap emptySquares = b.getAllPiecesBitboard('N');

        //rightward attack (including en passant)
        bitboard::bitmap pawnMovesBitboard = (blackPawns>>7) & whiteCapturables & ~bitboard::FILE[7];
        //leftward attack (including en passant)
        pawnMovesBitboard |= (blackPawns>>9) & whiteCapturables & ~bitboard::FILE[0];
        //single pawn push
        pawnMovesBitboard |= (blackPawns>>8) & emptySquares;
        //double pawn push
        pawnMovesBitboard |= (blackPawns>>16) & emptySquares & (emptySquares >> 8) & bitboard::RANK[4];

        return pawnMovesBitboard;
    }
}