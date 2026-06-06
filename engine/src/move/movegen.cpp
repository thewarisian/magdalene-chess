#include "move/movegen.h"

using bb = bitboard::bitmap;

namespace movegen {
    std::vector<std::string> getMovesList(bb movesBitboard) {
        std::vector<std::string> destinationSquares;

        while(movesBitboard) {
            //Get bit index of least significant set bit (rightmost 1-bit)
            Square nextSq = bitboard::popLSBSquare(movesBitboard);
            //Convert to tile name and add to vector
            destinationSquares.push_back(chessboard::squareToString(nextSq));
        }

        return destinationSquares;
    }

    //NOTE: Enemy capturables includes all enemy pieces and the en passant square
    bb calculatePawnMoves(Color col, bb pawns, bb enemyCapturables, bb emptySquares) {
        //Calculates required bitboards using colour passed
        bb notLeftFile, notRightFile, doublePushDestination; int shiftDir;

        if(col==Color::WHITE) {
            notLeftFile = ~bitboard::FILE[0];
            notRightFile = ~bitboard::FILE[7];
            doublePushDestination = bitboard::RANK[3];
            shiftDir = 1;
        } else {
            notLeftFile = ~bitboard::FILE[7];
            notRightFile = ~bitboard::FILE[0];
            doublePushDestination = bitboard::RANK[4];
            shiftDir = -1;
        }

        //rightward attack (including en passant)
        bb pawnMovesBitboard = utils::signedShift(pawns, 7*shiftDir) & enemyCapturables & notLeftFile;
        //leftward attack (including en passant)
        pawnMovesBitboard |= utils::signedShift(pawns, 9*shiftDir) & enemyCapturables & notRightFile;
        //single pawn push
        pawnMovesBitboard |= utils::signedShift(pawns, 8*shiftDir) & emptySquares;
        //double pawn push
        pawnMovesBitboard |= utils::signedShift(pawns, 16*shiftDir) & emptySquares & utils::signedShift(emptySquares, 8*shiftDir) & doublePushDestination;

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

    bitboard::bitmap calculateRookTypeMoves(bb occupied, bb friendOccupied, Square sq) {
        //Get attack tiles along horizontal and vertical lines
        bb fileAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getFileMask(sq));
        bb rankAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getRankMask(sq));

        //Merge and return
        return fileAttacks | rankAttacks;
    }

    bitboard::bitmap calculateBishopTypeMoves(bb occupied, bb friendOccupied, Square sq) {
        //Get attack tiles along diagonals
        bb diagAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getDiagonalMask(sq));
        bb antiAttacks = hypbQuint(sq, occupied, friendOccupied, bitboard::getAntiDiagonalMask(sq));

        //Merge and display
        return diagAttacks | antiAttacks;
    }

    bitboard::bitmap calculateKnightMoves(bb friendOccupied, Square sq) {
        return bitboard::getKnightAttackMask(sq) & ~friendOccupied;
    }

    bitboard::bitmap calculateKingMoves(bb friendOccupied, Square sq) {
        return bitboard::getKingAttackMask(sq) & ~friendOccupied;
    }

    bitboard::bitmap calculateTypeMoves(bb occupied, bb friendOccupied, Square sq, PieceType pt) {
        //NOTE NOT TO BE USED FOR PAWNS!!!!!!!!!!
        switch(pt) {
            case PieceType::KNIGHT: return calculateKnightMoves(friendOccupied, sq);
            case PieceType::BISHOP: return calculateBishopTypeMoves(occupied, friendOccupied, sq);
            case PieceType::ROOK: return calculateRookTypeMoves(occupied, friendOccupied, sq);
            case PieceType::KING: return calculateKingMoves(friendOccupied, sq);
            case PieceType::PAWN: //TODO Handle Pawns!!!!!!!!!!!!!!!!!
            case PieceType::QUEEN: //TODO Handle Queens Unused input!!!!!!!!!!!!!!!!!
            default: return 0; 
        }
    }

    bitboard::bitmap calculateMajorPieceMovesOfType(bb pieces, bb occupied, bb friendOccupied, PieceType pt) {
        bb attacks = 0ULL;

        //Apply calculation to one piece at a time
        while(pieces) {
            Square sq = bitboard::popLSBSquare(pieces);
            attacks |= calculateTypeMoves(occupied, friendOccupied, sq, pt);
        }

        return attacks;
    }
    
    bitboard::bitmap calculatePlayerAttacks(Color col, 
                                            bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king,
                                            bb enemyCapturables, bb empty, bb occupied, bb friendOccupied) {
        bitboard::bitmap 
        pawnAttacks = calculatePawnMoves(col, pawns, enemyCapturables, empty),
        knightAttacks = calculateMajorPieceMovesOfType(knights, occupied, friendOccupied, PieceType::KNIGHT),
        //Combine bishop and queen as they both have diagonal attacks
        bishopQueenAttacks = calculateMajorPieceMovesOfType(bishops|queens, occupied, friendOccupied, PieceType::BISHOP),
        //Combine rook and queen as they both have linear attacks
        rookQueenAttacks = calculateMajorPieceMovesOfType(rooks|queens, occupied, friendOccupied, PieceType::ROOK),
        kingAttacks = calculateMajorPieceMovesOfType(king, occupied, friendOccupied, PieceType::KING);

        return pawnAttacks | knightAttacks | bishopQueenAttacks | rookQueenAttacks | kingAttacks;
    }

}