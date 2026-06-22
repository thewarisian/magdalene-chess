#include "move/move.h"

namespace chessmove {
    void makeMove(Color col, move m, 
        bb& whitePawns, bb& whiteKnights, bb& whiteBishops, bb& whiteRooks, bb& whiteQueens, bb& whiteKing,
        bb& blackPawns, bb& blackKnights, bb& blackBishops, bb& blackRooks, bb& blackQueens, bb& blackKing,
        bb& enPassantSquare, castling_flags& cr
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
        else if(queens & fromBit)   queens ^= moveToggler;

        else if(rooks & fromBit) {
            rooks ^= moveToggler;

            //Disable castling rights according to rook moved
            if(col==Color::WHITE && fromBit == castling::WHITE_KS_ROOK) {
                castling::disableCastling(Color::WHITE, cr);
            } else if(col==Color::WHITE && fromBit == castling::WHITE_QS_ROOK) {
                castling::disableCastling(Color::WHITE, cr, true);
            } else if(col==Color::BLACK && fromBit == castling::BLACK_KS_ROOK) {
                castling::disableCastling(Color::BLACK, cr);
            } else if(col==Color::BLACK && fromBit == castling::BLACK_QS_ROOK) {
                castling::disableCastling(Color::BLACK, cr, true);
            }
        }
        else if(king & fromBit) {
            king ^= moveToggler;

            //Disable all castling rights as king as moved
            if(col==Color::WHITE) {
                castling::disableCastling(Color::WHITE, cr);
                castling::disableCastling(Color::WHITE, cr, true);
            } else {
                castling::disableCastling(Color::BLACK, cr);
                castling::disableCastling(Color::BLACK, cr, true);
            }
        }

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
            else if(eQueens & toBit)   eQueens ^= toBit;
            else if(eRooks & toBit) {
                eRooks ^= toBit;

                //Disable castling of enemy if rook captured from starting position
                if(col==Color::BLACK && toBit == castling::WHITE_KS_ROOK) {
                    castling::disableCastling(Color::WHITE, cr);
                } else if(col==Color::BLACK && toBit == castling::WHITE_QS_ROOK) {
                    castling::disableCastling(Color::WHITE, cr, true);
                } else if(col==Color::WHITE && toBit == castling::BLACK_KS_ROOK) {
                    castling::disableCastling(Color::BLACK, cr);
                } else if(col==Color::WHITE && toBit == castling::BLACK_QS_ROOK) {
                    castling::disableCastling(Color::BLACK, cr, true);
                }
            }
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
                case MoveType::PromotionKnight: knights ^= toBit; break;
                case MoveType::PromotionBishop: bishops ^= toBit; break;
                case MoveType::PromotionRook: rooks ^= toBit; break;
                case MoveType::PromotionQueen: queens ^= toBit; break;
                default:;
            }
        }

        //Castling King side (only rook movement is extra)
        if(mt == MoveType::KingSideCastle) {
            rooks ^= (col==Color::WHITE)? castling::WHITE_KS_ROOK_TOGGLE : castling::BLACK_KS_ROOK_TOGGLE;
        }

        //Castling Queen side (only rook movement is extra)
        if(mt == MoveType::QueenSideCastle) {
            rooks ^= (col==Color::WHITE)? castling::WHITE_QS_ROOK_TOGGLE : castling::BLACK_QS_ROOK_TOGGLE;
        }

        //Disable castling if already done (NOT NEEDED AS KING MOVEMENT ALREADY ACCOUNTS)
        // if(mt == MoveType::KingSideCastle || mt == MoveType::QueenSideCastle) {
        //     castling::disableCastling(col, cr);
        //     castling::disableCastling(col, cr, true);
        // }
    }

    void undoMove(Color col, move m, moveHistory& hist,
        bb& whitePawns, bb& whiteKnights, bb& whiteBishops, bb& whiteRooks, bb& whiteQueens, bb& whiteKing,
        bb& blackPawns, bb& blackKnights, bb& blackBishops, bb& blackRooks, bb& blackQueens, bb& blackKing,
        bb& enPassantSquare, castling_flags& cr
    ) {
        bb fromBit = bitboard::singleBit(getFromSquare(m));
        bb toBit = bitboard::singleBit(getToSquare(m));
        MoveType mt = getMoveType(m);
        PieceType captured = hist.capturedType;

        //Restore castling rights and en passant bitboard
        cr = hist.castlingRights;
        enPassantSquare = hist.enPassantSquare;

        //Decide bitboards to change based on color of move maker passed
        bb& pawns   = (col == Color::WHITE) ? whitePawns   : blackPawns;
        bb& knights = (col == Color::WHITE) ? whiteKnights : blackKnights;
        bb& bishops = (col == Color::WHITE) ? whiteBishops : blackBishops;
        bb& rooks   = (col == Color::WHITE) ? whiteRooks   : blackRooks;
        bb& queens  = (col == Color::WHITE) ? whiteQueens  : blackQueens;
        bb& king    = (col == Color::WHITE) ? whiteKing    : blackKing;

        // Undo promotion first otherwise promoted piece is moved back before pawn
        if(mt >= MoveType::PromotionKnight && mt <= MoveType::PromotionQueen) {
            //Remove promoted piece
            switch(mt) {
                case MoveType::PromotionKnight: knights ^= toBit; break;
                case MoveType::PromotionBishop: bishops ^= toBit; break;
                case MoveType::PromotionRook: rooks ^= toBit; break;
                case MoveType::PromotionQueen: queens ^= toBit; break;
                default:;
            }

            //Add pawn back instead of promoted piece
            pawns ^= toBit;
        }

        //Handle basic piece movement for general moves
        bb moveToggler = fromBit | toBit;
        if     (pawns & toBit)    pawns ^= moveToggler;
        else if(knights & toBit)  knights ^= moveToggler;
        else if(bishops & toBit)  bishops ^= moveToggler;
        else if(queens & toBit)   queens ^= moveToggler;
        else if(rooks & toBit)    rooks ^= moveToggler;
        else if(king & toBit)     king ^= moveToggler;

        // Remove captured piece
        if(mt == MoveType::Capture) {
            //Determining enemy piece bitboards
            bb& ePawns   = (col == Color::WHITE) ? blackPawns   : whitePawns;
            bb& eKnights = (col == Color::WHITE) ? blackKnights : whiteKnights;
            bb& eBishops = (col == Color::WHITE) ? blackBishops : whiteBishops;
            bb& eRooks   = (col == Color::WHITE) ? blackRooks   : whiteRooks;
            bb& eQueens  = (col == Color::WHITE) ? blackQueens  : whiteQueens;

            if     (captured == PieceType::PAWN)    ePawns ^= toBit;
            else if(captured == PieceType::KNIGHT)  eKnights ^= toBit;
            else if(captured == PieceType::BISHOP)  eBishops ^= toBit;
            else if(captured == PieceType::QUEEN)   eQueens ^= toBit;
            else if(captured == PieceType::ROOK)    eRooks ^= toBit;
        }

        // Add pawn captured by en pasant back
        if(mt == MoveType::EnPassant) {
            bb& ePawns   = (col == Color::WHITE) ? blackPawns   : whitePawns;
            int shift = (col==Color::WHITE)?-8:8;
            ePawns ^= utils::signedShift(toBit, shift);
        }

        //Castling King side (only rook movement is extra)
        if(mt == MoveType::KingSideCastle) {
            rooks ^= (col==Color::WHITE)? castling::WHITE_KS_ROOK_TOGGLE : castling::BLACK_KS_ROOK_TOGGLE;
        }

        //Castling Queen side (only rook movement is extra)
        if(mt == MoveType::QueenSideCastle) {
            rooks ^= (col==Color::WHITE)? castling::WHITE_QS_ROOK_TOGGLE : castling::BLACK_QS_ROOK_TOGGLE;
        }
    }
}