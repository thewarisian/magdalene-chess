#include "move/movegen.h"

namespace movegen {
    std::vector<std::string> getMovesList(bitboard::bitmap movesBitboard) {
        std::vector<std::string> destinationSquares;

        while(movesBitboard) {
            //Get bit index of least significant set bit (rightmost 1-bit)
            Square nextSq = bitboard::popLSBSquare(movesBitboard);
            //Convert to tile name and add to vector
            destinationSquares.push_back(chessboard::squareToString(nextSq));
        }

        return destinationSquares;
    }

    bitboard::bitmap calculateWhitePawnMoves(const chessboard::GameBoard& b) {
        bitboard::bitmap whitePawns = b.copyPieceBitboard(Color::WHITE, PieceType::PAWN);
        bitboard::bitmap blackCapturables = b.copyAllPiecesBitboard(Color::BLACK) | b.getEnPassantAttackSquare();
        bitboard::bitmap emptySquares = ~b.copyAllPiecesBitboard();

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
        bitboard::bitmap blackPawns = b.copyPieceBitboard(Color::BLACK, PieceType::PAWN);
        bitboard::bitmap whiteCapturables = b.copyAllPiecesBitboard(Color::WHITE) | b.getEnPassantAttackSquare();
        bitboard::bitmap emptySquares = ~b.copyAllPiecesBitboard();

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

    bitboard::bitmap hypbQuint(Square sq, bitboard::bitmap occupied, bitboard::bitmap friendOccupied, bitboard::bitmap visionMask) {
        //Find slider bitmap
        bitboard::bitmap slider = bitboard::singleBit(sq);

        //Formula: (((o&m)-2s)^((o&m)'-2s')')&m
        // a' is reverse of a. E.g. (100)' = 001
        
        //Extract only occupied bits along mask
        bitboard::bitmap maskedBits = occupied&visionMask;
        //Get attacks on higher side
        bitboard::bitmap highAttacks = maskedBits-2*slider;
        //Get attacks on lower side
        bitboard::bitmap lowAttacks = bitboard::reverseBitmap(bitboard::reverseBitmap(maskedBits)-2*bitboard::reverseBitmap(slider));

        return (lowAttacks ^ highAttacks) & visionMask & ~friendOccupied;
    }

    bitboard::bitmap calculateRookMoves(const chessboard::GameBoard& b, Square sq, Color col) {
        bitboard::bitmap friendOccupied = b.copyAllPiecesBitboard(col);
        bitboard::bitmap occupied = b.copyAllPiecesBitboard();
        
        bitboard::bitmap fileAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getFileMask(sq));
        bitboard::bitmap rankAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getRankMask(sq));

        return fileAttacks | rankAttacks;
    }

    bitboard::bitmap calculateBishopMoves(const chessboard::GameBoard& b, Square sq, Color col) {
        bitboard::bitmap friendOccupied = b.copyAllPiecesBitboard(col);
        bitboard::bitmap occupied = b.copyAllPiecesBitboard();
        
        bitboard::bitmap diagAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getDiagonalMask(sq));
        bitboard::bitmap antiAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getAntiDiagonalMask(sq));

        return diagAttacks | antiAttacks;
    }

    bitboard::bitmap calculateQueenMoves(const chessboard::GameBoard& b, Square sq, Color col) {
        return calculateBishopMoves(b, sq, col) | calculateRookMoves(b, sq, col);
    }
}