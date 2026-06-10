#include "move/movegen.h"
#include <iostream>

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

    bb calculatePawnAttacks(Color col, bb pawns) {
        //Calculates required bitboards using colour passed
        bb notLeftFile, notRightFile; int shiftDir;

        if(col==Color::WHITE) {
            notLeftFile = ~bitboard::FILE[0];
            notRightFile = ~bitboard::FILE[7];
            shiftDir = 1;
        } else {
            notLeftFile = ~bitboard::FILE[7];
            notRightFile = ~bitboard::FILE[0];
            shiftDir = -1;
        }

        //rightward attack (including en passant)
        bb pawnAttacksBitboard = utils::signedShift(pawns, 7*shiftDir) & notLeftFile;
        //leftward attack (including en passant)
        pawnAttacksBitboard |= utils::signedShift(pawns, 9*shiftDir) & notRightFile;

        return pawnAttacksBitboard;
    }

    bb hypbQuint(bb slider, bb occupied, bb visionMask) {
        //Formula: (((o&m)-2s)^((o&m)'-2s')')&m
        // a' is reverse of a. E.g. (100)' = 001
        
        //Extract only occupied bits along mask
        bitboard::bitmap maskedBits = occupied&visionMask;
        //Get attacks on higher side
        bitboard::bitmap highAttacks = maskedBits-2*slider;
        //Get attacks on lower side
        bitboard::bitmap lowAttacks = bitboard::reverseBitmap(bitboard::reverseBitmap(maskedBits)-2*bitboard::reverseBitmap(slider));

        return (lowAttacks ^ highAttacks) & visionMask;
    }

    bb calculateRookAttacks(bb rook, bb occupied) {
        //Get attack tiles along horizontal and vertical lines
        bb fileAttacks = hypbQuint(rook, occupied, bitboard::getFileMask(rook));
        bb rankAttacks = hypbQuint(rook, occupied, bitboard::getRankMask(rook));

        //Merge and return
        return (fileAttacks | rankAttacks);
    }

    bb calculateBishopAttacks(bb bishop, bb occupied) {
        //Get attack tiles along diagonals
        bb diagAttacks = hypbQuint(bishop, occupied, bitboard::getDiagonalMask(bishop));
        bb antiAttacks = hypbQuint(bishop, occupied, bitboard::getAntiDiagonalMask(bishop));

        //Merge and display
        return (diagAttacks | antiAttacks);
    }

    bb calculateQueenAttacks(bb queen, bb occupied) {
        return calculateBishopAttacks(queen, occupied) | calculateRookAttacks(queen, occupied);
    }

    bb calculateKnightAttacks(bb knight) {
        return bitboard::getKnightAttackMask(knight);
    }

    bb calculateKingAttacks(bb king) {
        return bitboard::getKingAttackMask(king);
    }

    bb calculateAttacksOfType(bb piece, bb occupied, PieceType pt) {
        switch(pt) {
            case PieceType::KNIGHT: return calculateKnightAttacks(piece);
            case PieceType::BISHOP: return calculateBishopAttacks(piece, occupied);
            case PieceType::ROOK: return calculateRookAttacks(piece, occupied);
            case PieceType::KING: return calculateKingAttacks(piece);
            case PieceType::QUEEN: return calculateQueenAttacks(piece, occupied);
            default: return 0; //HANDLE ERROR FOR PAWN AND NONE
        }
    }

    //Add note to use separate optimised function for pawns, or use that function in this fucntion
    bb calculateAllPieceMovesOfType(bb pieces, bb occupied, PieceType pt) {
        bb attacks = 0ULL;

        //Apply calculation to one piece at a time
        while(pieces) {
            bb bit = bitboard::popLSB(pieces);
            attacks |= calculateAttacksOfType(bit, occupied, pt);
        }

        return attacks;
    }
    
    bb calculatePseudoLegalMoves(Color col, bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, bb occupied) {
        bb pawnAttacks = calculatePawnAttacks(col, pawns),
        knightAttacks = calculateAllPieceMovesOfType(knights, occupied, PieceType::KNIGHT),
        //Combine bishop and queen as they both have diagonal attacks
        bishopQueenAttacks = calculateAllPieceMovesOfType(bishops|queens, occupied, PieceType::BISHOP),
        //Combine rook and queen as they both have linear attacks
        rookQueenAttacks = calculateAllPieceMovesOfType(rooks|queens, occupied, PieceType::ROOK),
        kingAttacks = calculateAllPieceMovesOfType(king, occupied, PieceType::KING);

        return pawnAttacks | knightAttacks | bishopQueenAttacks | rookQueenAttacks | kingAttacks;
    }

    bb calculateRay(bb from, bb to) {
        bb mask;

        //Same file
        if(bitboard::getFileMask(from) == bitboard::getFileMask(to)) mask = bitboard::getFileMask(from);
        //Same rank
        else if(bitboard::getRankMask(from) == bitboard::getRankMask(to)) mask = bitboard::getRankMask(from);
        //Same diagonal
        else if(bitboard::getDiagonalMask(from) == bitboard::getDiagonalMask(to)) mask = bitboard::getDiagonalMask(from);
        //Same file
        else if(bitboard::getAntiDiagonalMask(from) == bitboard::getAntiDiagonalMask(to)) mask = bitboard::getAntiDiagonalMask(from);
        //Invalid
        else return 0ULL;

        return hypbQuint(from, to, mask)&hypbQuint(to, from, mask);
    }

    //Tiles the pieces other than the king are restricted to move to
    bb calculateCheckMask(Color col, bb king, bb enemyPawns, bb enemyKnights, bb enemyBishops, bb enemyRooks, bb enemyQueens, bb occupied) {
        //Treats king as a combination of all other piece types to see what it would hypothetically attack.
        //By symmetry, what it could attack is a piece attacking it
        bb pawnCheckers = calculatePawnAttacks(col, king) & enemyPawns,
        knightCheckers = calculateKnightAttacks(king) & enemyKnights,
        bishopCheckers = calculateBishopAttacks(king, occupied) & enemyBishops,
        rookCheckers = calculateRookAttacks(king, occupied) & enemyRooks,
        queenCheckers = calculateQueenAttacks(king, occupied) & enemyQueens;

        bb checkers = pawnCheckers | knightCheckers | bishopCheckers | rookCheckers | queenCheckers;
        int numCheckers = __builtin_popcountll(checkers);

        //Different behaviour based on number of checkers
        //No checkers means no restriction applied
        if(numCheckers == 0) return ~0ULL;
        //2+ checkers means only legal move is king moving out of the way
        else if(numCheckers >= 2) return 0ULL;

        //Single checker can be captured or blocked
        if(checkers & (bishopCheckers|rookCheckers|queenCheckers)) return checkers|calculateRay(king, checkers);
        return checkers;
    }
}