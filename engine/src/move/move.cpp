#include "move/move.h"

namespace chessmove {
    void makeMove(Color col, move m, 
        bb& whitePawns, bb& whiteKnights, bb& whiteBishops, bb& whiteRooks, bb& whiteQueens, bb& whiteKing,
        bb& blackPawns, bb& blackKnights, bb& blackBishops, bb& blackRooks, bb& blackQueens, bb& blackKing,
        bb& enPassantSquare
    ) {
        bb fromBit = bitboard::singleBit(getFromSquare(m));
        bb toBit = bitboard::singleBit(getToSquare(m));
        MoveType mt = getMoveType(m);

        //Decide bitboards to change based on color of move maker passed
        bb& pawns   = (col == Color::WHITE) ? whitePawns   : blackPawns;
        bb& knights = (col == Color::WHITE) ? whiteKnights : blackKnights;
        bb& bishops = (col == Color::WHITE) ? whiteBishops : blackBishops;
        bb& rooks   = (col == Color::WHITE) ? whiteRooks   : blackRooks;
        bb& queens  = (col == Color::WHITE) ? whiteQueens  : blackQueens;
        bb& king    = (col == Color::WHITE) ? whiteKing    : blackKing;

        //Handle basic piece movement for general moves
        bb moveToggler = fromBit | toBit;
        if     (pawns & fromBit)    pawns ^= moveToggler;
        else if(knights & fromBit)  knights ^= moveToggler;
        else if(bishops & fromBit)  bishops ^= moveToggler;
        else if(rooks & fromBit)    rooks ^= moveToggler;
        else if(queens & fromBit)   queens ^= moveToggler;
        else if(king & fromBit)     king ^= moveToggler;

        // Remove captured piece
        if(mt == MoveType::Capture) {
            //Determining enemy piece bitboards
            bb& ePawns   = (col == Color::WHITE) ? blackPawns   : whitePawns;
            bb& eKnights = (col == Color::WHITE) ? blackKnights : whiteKnights;
            bb& eBishops = (col == Color::WHITE) ? blackBishops : whiteBishops;
            bb& eRooks   = (col == Color::WHITE) ? blackRooks   : whiteRooks;
            bb& eQueens  = (col == Color::WHITE) ? blackQueens  : whiteQueens;

            if     (ePawns & toBit)    ePawns ^= toBit;
            else if(eKnights & toBit)  eKnights ^= toBit;
            else if(eBishops & toBit)  eBishops ^= toBit;
            else if(eRooks & toBit)    eRooks ^= toBit;
            else if(eQueens & toBit)   eQueens ^= toBit;
        }

        // Enable en passant if double pawn push
        if(mt == MoveType::DoublePawnPush) {
            int shift = (col==Color::WHITE)?-8:8;
            enPassantSquare = utils::signedShift(toBit, shift);
        } // Disiable en passant if non-double pawn push move made
        else enPassantSquare = 0ull;

        // Special removal of pawn behind destination square in en passant
        if(mt == MoveType::EnPassant) {
            //Determining enemy piece bitboards
            bb& ePawns   = (col == Color::WHITE) ? blackPawns   : whitePawns;
            int shift = (col==Color::WHITE)?-8:8;
            ePawns ^= utils::signedShift(toBit, shift);
        }

        // Pawn Promotion (Enumeration allows following check Promo to Knight, Bishop, Rook, Queen are enumerated continuously)
        if(mt >= MoveType::PromotionKnight && mt <= MoveType::PromotionQueen) {
            //Remove pawn from promotion square
            pawns ^= toBit;

            //Add piece of correct type in place of pawn
            switch(mt) {
                case MoveType::PromotionKnight: knights |= toBit; break;
                case MoveType::PromotionBishop: bishops |= toBit; break;
                case MoveType::PromotionRook: rooks |= toBit; break;
                case MoveType::PromotionQueen: queens |= toBit; break;
                default:;
            }
        }

        //Castling King side
        if(mt == MoveType::KingSideCastle) {
            rooks ^= (col==Color::WHITE)? castling_cache::WHITE_KS_ROOK_TOGGLE : castling_cache::BLACK_KS_ROOK_TOGGLE;
        }

        //Castling Queen side
        if(mt == MoveType::QueenSideCastle) {
            rooks ^= (col==Color::WHITE)? castling_cache::WHITE_QS_ROOK_TOGGLE : castling_cache::BLACK_QS_ROOK_TOGGLE;
        }
    }
}