#include "move/movegen.h"

namespace movegen {
    void addSingleMoves(bb moves, int shiftAmt, MoveType mt, std::vector<move>& possibleMoves) {
        while(moves) {
            int toIdx = __builtin_ctzll(moves);

            Square from = utils::intToSquare(toIdx - shiftAmt);
            Square to = utils::intToSquare(toIdx);

            possibleMoves.push_back(chessmove::packMoveInfo(from, to, mt));

            //Eliminate last bit
            moves &= (moves-1);
        }
    }

    void addPromotionMoves(bb moves, int shiftAmt, std::vector<move>& possibleMoves) {
        addSingleMoves(moves, shiftAmt, MoveType::PromotionKnight, possibleMoves);
        addSingleMoves(moves, shiftAmt, MoveType::PromotionBishop, possibleMoves);
        addSingleMoves(moves, shiftAmt, MoveType::PromotionRook, possibleMoves);
        addSingleMoves(moves, shiftAmt, MoveType::PromotionQueen, possibleMoves);
    }

    void addPossiblePawnMoves(Color col, bb pawns, bb empty, bb enemyPieces, bb enPassantSquare, std::vector<move>& possibleMoves) {
        bb notLeftFile, notRightFile, doublePushDestination, promotionRank; int shiftDir;

        if(col==Color::WHITE) {
            notLeftFile = ~bitboard::FILE[0]; notRightFile = ~bitboard::FILE[7];
            doublePushDestination = bitboard::RANK[3]; promotionRank = bitboard::RANK[7];
            shiftDir = 1;
        } else {
            notLeftFile = ~bitboard::FILE[7]; notRightFile = ~bitboard::FILE[0];
            doublePushDestination = bitboard::RANK[4]; promotionRank = bitboard::RANK[0];
            shiftDir = -1;
        }
        
        //rightward attack
        int shiftAmt = 7*shiftDir;
        bb moves = utils::signedShift(pawns, shiftAmt) & enemyPieces & notLeftFile;

        //Non promotion
        addSingleMoves(moves & ~promotionRank, shiftAmt, MoveType::Capture, possibleMoves);
        //Promotion
        addPromotionMoves(moves & promotionRank, shiftAmt, possibleMoves);
        //En Passant
        addSingleMoves(moves & enPassantSquare, shiftAmt, MoveType::EnPassant, possibleMoves);

        //================================================================================================

        //leftward attack
        shiftAmt = 9*shiftDir;
        moves = utils::signedShift(pawns, shiftAmt) & enemyPieces & notRightFile;
        
        //Non promotion
        addSingleMoves(moves & ~promotionRank, shiftAmt, MoveType::Capture, possibleMoves);
        //Promotion
        addPromotionMoves(moves & promotionRank, shiftAmt, possibleMoves);
        //En Passant
        addSingleMoves(moves & enPassantSquare, shiftAmt, MoveType::EnPassant, possibleMoves);

        //================================================================================================

        //single pushes
        shiftAmt = 8*shiftDir;
        moves = utils::signedShift(pawns, shiftAmt) & empty;

        //non promotion
        addSingleMoves(moves & ~promotionRank, shiftAmt, MoveType::Quiet, possibleMoves);
        //promotion
        addPromotionMoves(moves & promotionRank, shiftAmt, possibleMoves);

        //double pushes
        moves = utils::signedShift(moves, shiftAmt) & empty & doublePushDestination;
        addSingleMoves(moves, 2*shiftAmt, MoveType::DoublePawnPush, possibleMoves);
    }





    bb hypbQuint(bb slider, bb occupied, bb visionMask) {
        //Formula: (((o&m)-2s)^((o&m)'-2s')')&m
        // a' is reverse of a. E.g. (100)' = 001
        
        //Extract only occupied bits along mask
        bb maskedBits = occupied&visionMask;
        //Get attacks on higher side
        bb highAttacks = maskedBits-2*slider;
        //Get attacks on lower side
        bb lowAttacks = bitboard::reverseBitmap(bitboard::reverseBitmap(maskedBits)-2*bitboard::reverseBitmap(slider));

        return (lowAttacks ^ highAttacks) & visionMask;
    }

    bb calculateKnightAttacks(bb knight) {
        return bitboard::getKnightAttackMask(knight);
    }

    bb calculateBishopAttacks(bb bishop, bb occupied) {
        //Get attack tiles along diagonals
        bb diagAttacks = hypbQuint(bishop, occupied, bitboard::getDiagonalMask(bishop));
        bb antiAttacks = hypbQuint(bishop, occupied, bitboard::getAntiDiagonalMask(bishop));

        //Merge and display
        return (diagAttacks | antiAttacks);
    }

    bb calculateRookAttacks(bb rook, bb occupied) {
        //Get attack tiles along horizontal and vertical lines
        bb fileAttacks = hypbQuint(rook, occupied, bitboard::getFileMask(rook));
        bb rankAttacks = hypbQuint(rook, occupied, bitboard::getRankMask(rook));

        //Merge and return
        return (fileAttacks | rankAttacks);
    }

    bb calculateQueenAttacks(bb queen, bb occupied) {
        return calculateBishopAttacks(queen, occupied) | calculateRookAttacks(queen, occupied);
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
            default: return 0;
        }
    }

    bb calculateMajorMinorAttackScope(bb pieces, bb occupied, bb sameColPieces, bb enemy, PieceType pt) {
        bb attacks = 0ull;

        //Apply calculation to one piece at a time
        while(pieces) {
            bb piece = bitboard::popLSB(pieces);
            attacks |= calculateAttacksOfType(piece, occupied, pt);
        }

        return attacks;
    }



    void addPossibleNonPawnMovesOfType(bb pieces, bb occupied, bb sameColPieces, bb enemy, PieceType pt, std::vector<move>& possibleMoves) {
        //Apply calculation to one piece at a time
        while(pieces) {
            Square from = bitboard::popLSBSquare(pieces);
            bb piece = bitboard::singleBit(from);
            
            bb moves = calculateAttacksOfType(piece, occupied, pt) & ~sameColPieces;
            while(moves) {
                Square to = bitboard::popLSBSquare(moves);
                MoveType mt = (bitboard::singleBit(to) & enemy)? MoveType::Capture : MoveType::Quiet;
                possibleMoves.push_back(chessmove::packMoveInfo(from, to, mt));
            }
        }
    }

    bb captureScope(Color col, bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, bb occupied, bb sameColPieces, bb enemyPieces) {
        //PAWNS
        bb notLeftFile, notRightFile; int shiftDir;
        if(col==Color::WHITE) {
            notLeftFile = ~bitboard::FILE[0]; notRightFile = ~bitboard::FILE[7];
            shiftDir = 1;
        } else {
            notLeftFile = ~bitboard::FILE[7]; notRightFile = ~bitboard::FILE[0];
            shiftDir = -1;
        }

        int shiftAmt = 7*shiftDir;
        bb attacks = utils::signedShift(pawns, shiftAmt) & notLeftFile;
        //leftward attack
        shiftAmt = 9*shiftDir;
        attacks |= utils::signedShift(pawns, shiftAmt) & notRightFile;

        //KNIGHTS
        attacks |= calculateMajorMinorAttackScope(knights, occupied, sameColPieces, enemyPieces, PieceType::KNIGHT);
        //BISHOP & QUEEN DIAGONALS
        attacks |= calculateMajorMinorAttackScope(bishops|queens, occupied, sameColPieces, enemyPieces, PieceType::BISHOP);
        //ROOK & QUEEN ORTHOGONALS
        attacks |= calculateMajorMinorAttackScope(rooks|queens, occupied, sameColPieces, enemyPieces, PieceType::ROOK);
        //KING
        attacks |= calculateMajorMinorAttackScope(king, occupied, sameColPieces, enemyPieces, PieceType::KING);

        return attacks;
    }

    void addPossibleCastling(Color col, bb king, bb occupied, bb enemyAttackScope, bool castleKing, bool castleQueen, std::vector<move>& possibleMoves) {

        //King cannot castle out of check
        if(!(enemyAttackScope & king)) {
            bb OO_MASK, OOO_MASK, OOO_ATTACK_MASK;
            move KingSideCastle, QueenSideCastle;

            if(col==Color::WHITE) {
                OO_MASK = bitboard::WHITE_OO_MASK;
                OOO_MASK = bitboard::WHITE_OOO_MASK;
                OOO_ATTACK_MASK = bitboard::WHITE_OOO_ATTACK_MASK;
                KingSideCastle = castling::WHITE_KING_SIDE;
                QueenSideCastle = castling::WHITE_QUEEN_SIDE;
            } else {
                OO_MASK = bitboard::BLACK_OO_MASK;
                OOO_MASK = bitboard::BLACK_OOO_MASK;
                OOO_ATTACK_MASK = bitboard::BLACK_OOO_ATTACK_MASK;
                KingSideCastle = castling::BLACK_KING_SIDE;
                QueenSideCastle = castling::BLACK_QUEEN_SIDE;
            }

            //Path for castling not occupied, and king does not walk through enemy attacks
            if(castleKing && !(occupied & OO_MASK) && !(enemyAttackScope & OO_MASK))  
                possibleMoves.push_back(KingSideCastle);

            if(castleQueen && !(occupied & OOO_MASK) && !(enemyAttackScope & OOO_ATTACK_MASK))
                possibleMoves.push_back(QueenSideCastle);
        }
    }





    std::vector<move> getAllMoves(Color col, 
        bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, 
        bb empty, bb occupied, bb sameColPieces, bb enemyPieces, bb enemyAttackScope, bb enPassantSquare,
        bool castleKing, bool castleQueen) {
        std::vector<move> allMoves;

        addPossiblePawnMoves(col, pawns, empty, enemyPieces, enPassantSquare, allMoves);
        addPossibleNonPawnMovesOfType(knights, occupied, sameColPieces, enemyPieces, PieceType::KNIGHT, allMoves);
        addPossibleNonPawnMovesOfType(bishops, occupied, sameColPieces, enemyPieces, PieceType::BISHOP, allMoves);
        addPossibleNonPawnMovesOfType(rooks, occupied, sameColPieces, enemyPieces, PieceType::ROOK, allMoves);
        addPossibleNonPawnMovesOfType(queens, occupied, sameColPieces, enemyPieces, PieceType::QUEEN, allMoves);
        addPossibleNonPawnMovesOfType(king, occupied, sameColPieces, enemyPieces, PieceType::KING, allMoves);
        addPossibleCastling(col, king, occupied, enemyAttackScope, castleKing, castleQueen, allMoves);

        return allMoves;
    }
}