#include <cstdint>
#include <string>
using bitmap = uint64_t;

// ================== DATA CACHE ============================================================================

// Initial position in FEN notation (board only)
// Forsyth–Edwards Notation (FEN) encodes board state row by row, top (rank 8) to bottom (rank 1)
const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

// Total number of squares on the chessboard
const int NUM_TILES = 64;

// File bitboards (columns)
// Each entry represents one file (column) from A to H
const bitmap FILE_BITS[8] = {
    0x0101010101010101ULL, // FILE_A
    0x0202020202020202ULL, // FILE_B
    0x0404040404040404ULL, // FILE_C
    0x0808080808080808ULL, // FILE_D
    0x1010101010101010ULL, // FILE_E
    0x2020202020202020ULL, // FILE_F
    0x4040404040404040ULL, // FILE_G
    0x8080808080808080ULL  // FILE_H
};

// Rank bitboards (rows)
// Each entry represents one rank (row) from 1 to 8
const bitmap RANK_BITS[8] = {
    0x00000000000000FFULL, // RANK_1
    0x000000000000FF00ULL, // RANK_2
    0x0000000000FF0000ULL, // RANK_3
    0x00000000FF000000ULL, // RANK_4
    0x000000FF00000000ULL, // RANK_5
    0x0000FF0000000000ULL, // RANK_6
    0x00FF000000000000ULL, // RANK_7
    0xFF00000000000000ULL  // RANK_8
};

// Precomputed attack bitboards (placeholders for now)
// These are used for fast move generation for each piece type and square
const bitmap PAWN_ATTACKS[2][64] = {}; // White (0) and Black (1) pawn attacks
const bitmap KNIGHT_ATTACKS[64] = {};
const bitmap BISHOP_ATTACKS[64] = {};
const bitmap ROOK_ATTACKS[64] = {};
const bitmap QUEEN_ATTACKS[64] = {};
const bitmap KING_ATTACKS[64] = {};

// ======================= CHESSBOARD CLASS ======================================================================

#include <algorithm>

/**
 * @class ChessBoard
 * @brief Represents a chessboard using 64-bit bitboards for each piece type.
 *
 * This class maintains separate bitboards for each type of piece for both colors.
 * It supports initialization via a FEN string and provides a text-based display.
 */
class ChessBoard {
private:
    // ===================== PIECE BITBOARDS =====================

    // White pieces
    bitmap whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;

    // Black pieces
    bitmap blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

    // Board state flags
    bool whiteToMove, whiteCanCastleKingSide, whiteCanCastleQueenSide;
    bool blackCanCastleKingSide, blackCanCastleQueenSide;
    int halfMove, fullMove, enPassantIdx;

    // ===================== HELPER METHODS ======================

    /**
     * @brief Checks if a specific square is occupied in a bitboard.
     * @param b Bitboard to check.
     * @param i Square index (0 = a1, 63 = h8).
     * @return True if the square is occupied.
     */
    bool bitmapOccupiedAt(const bitmap& b, int i) {
        return ((b >> i) & 1);
    }

    /**
     * @brief Places a piece on the bitboard at a specific square.
     * @param b Bitboard to modify.
     * @param i Square index (0 = a1, 63 = h8).
     */
    void placeOnBitmapAt(bitmap& b, int i) {
        b |= (1ULL << i);
    }

    // ===================== INTERNAL METHODS =====================

    /**
     * @brief Initializes all piece bitboards from a FEN string.
     *
     * Loops through the FEN string in reverse to populate bitboards correctly.
     * Currently ignores castling, en passant, and move counters.
     *
     * @param fen Forsyth–Edwards Notation string (board part only works for now).
     */
    void setBoard(const std::string& fen) {
        int pos = 0;
        for(int j = fen.size() - 1; j >= 0; j--) {
            const char& ch = fen[j];
            switch(ch) {
                // Empty squares: skip '1'-'8' tiles
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8':
                    pos += (ch - '0');
                    break;

                // Rank separator '/' is ignored
                case '/':
                    break;

                // White pieces
                case 'P': placeOnBitmapAt(whitePawns, pos++); break;
                case 'N': placeOnBitmapAt(whiteKnights, pos++); break;
                case 'B': placeOnBitmapAt(whiteBishops, pos++); break;
                case 'R': placeOnBitmapAt(whiteRooks, pos++); break;
                case 'Q': placeOnBitmapAt(whiteQueens, pos++); break;
                case 'K': placeOnBitmapAt(whiteKing, pos++); break;

                // Black pieces
                case 'p': placeOnBitmapAt(blackPawns, pos++); break;
                case 'n': placeOnBitmapAt(blackKnights, pos++); break;
                case 'b': placeOnBitmapAt(blackBishops, pos++); break;
                case 'r': placeOnBitmapAt(blackRooks, pos++); break;
                case 'q': placeOnBitmapAt(blackQueens, pos++); break;
                case 'k': placeOnBitmapAt(blackKing, pos++); break;

                // Default: increment position for any unknown character
                default: pos++; break;
            }
        }
    }

public:
    // ===================== CONSTRUCTORS =====================

    /**
     * @brief Constructs a ChessBoard object.
     * 
     * Initializes all bitboards to zero and then sets the board from the FEN string.
     * @param fen Optional FEN string; defaults to the standard starting position.
     */
    ChessBoard(const std::string& fen = INITIAL_FEN) {
        whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
        blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;
        setBoard(fen);
    }

    // ===================== PUBLIC METHODS =====================

    /**
     * @brief Returns a simple text-based representation of the board.
     *
     * Each square is represented by:
     * - Uppercase letters for white pieces (P, N, B, R, Q, K)
     * - Lowercase letters for black pieces (p, n, b, r, q, k)
     * - '_' for empty squares
     *
     * Ranks are separated by double newlines for readability.
     * @return std::string Human-readable board representation.
     */
    std::string toString() {
        std::string rep = "";

        for(int i = 0; i < NUM_TILES; i++) {
            if(i != 0 && i % 8 == 0) rep = "\n\n" + rep;

            if(bitmapOccupiedAt(whitePawns, i))       { rep = "P  " + rep; }
            else if(bitmapOccupiedAt(whiteKnights, i)){ rep = "N  " + rep; }
            else if(bitmapOccupiedAt(whiteBishops, i)){ rep = "B  " + rep; }
            else if(bitmapOccupiedAt(whiteRooks, i))  { rep = "R  " + rep; }
            else if(bitmapOccupiedAt(whiteQueens, i)) { rep = "Q  " + rep; }
            else if(bitmapOccupiedAt(whiteKing, i))   { rep = "K  " + rep; }
            else if(bitmapOccupiedAt(blackPawns, i))  { rep = "p  " + rep; }
            else if(bitmapOccupiedAt(blackKnights,i)) { rep = "n  " + rep; }
            else if(bitmapOccupiedAt(blackBishops,i)) { rep = "b  " + rep; }
            else if(bitmapOccupiedAt(blackRooks, i))  { rep = "r  " + rep; }
            else if(bitmapOccupiedAt(blackQueens,i))  { rep = "q  " + rep; }
            else if(bitmapOccupiedAt(blackKing,i))    { rep = "k  " + rep; }
            else { rep = "_  " + rep; }
        }

        return "\n" + rep + "\n";
    }
};

// ===================== TESTING MAIN ======================================================================

#include <iostream>

int main() {
    // Example FEN for testing
    ChessBoard b("rnbqkbnr/pp1p2pp/4p3/2p5/3P4/2N1P3/PPP2PPP/R1BQKBNR");
    std::cout << b.toString() << "\n";
    return 0;
}
