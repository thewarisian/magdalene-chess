#include <cstdint>
#include <string>
using bitmap = uint64_t;

// ================== DATA CACHE ============================================================================

// Initial position in FEN notation
const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// File bitboards (columns)
// Each entry represents one file (column) on the board, from FILE_A to FILE_H
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
/// Each entry represents one rank (row) on the board, from RANK_1 to RANK_8
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

// Precomputed attack bitboards for each piece type and square
/// Currently empty; placeholders for fast move generation
const bitmap PAWN_ATTACKS[2][64] = {}; // Pawn attacks for white (0) and black (1)
const bitmap KNIGHT_ATTACKS[64] = {};  // Knight attacks from each square
const bitmap BISHOP_ATTACKS[64] = {};  // Bishop attacks from each square
const bitmap ROOK_ATTACKS[64] = {};    // Rook attacks from each square
const bitmap QUEEN_ATTACKS[64] = {};   // Queen attacks from each square
const bitmap KING_ATTACKS[64] = {};    // King attacks from each square

// ======================= CHESSBOARD CLASS ======================================================================

/**
 * @class ChessBoard
 * @brief Represents a chessboard using 64-bit bitboards.
 *
 * Stores the state of a chess game using separate bitmaps for each piece type and color.
 * Supports initialization via a FEN string and provides a simple text-based display.
 */
class ChessBoard {
private:
    // ===================== PIECE BITBOARDS =====================

    // Bitboards for white pieces
    bitmap whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
    // Bitboards for black pieces
    bitmap blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

    // ===================== HELPER METHODS ======================

    /**
     * @brief Checks if a specific square is occupied in a given bitboard.
     * @param b The bitboard to check.
     * @param i The square index (0 = a1, 63 = h8).
     * @return true if the square is occupied, false otherwise.
     */
    bool bitmapOccupiedAt(bitmap b, int i) {
        return ((b >> i) & 1) == 1;
    }

    // ===================== INTERNAL METHODS =====================

    /**
     * @brief Initializes all piece bitboards from a FEN string.
     * 
     * Currently sets all bitboards to 0. FEN parsing logic should populate the bitboards.
     * @param fen A valid Forsyth-Edwards Notation string.
     */
    void initializeBoard(const std::string& fen) {
        // TODO: implement FEN parsing
        whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
        blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;
    }

public:
    // ===================== CONSTRUCTORS =====================

    /**
     * @brief Constructs a ChessBoard object.
     * 
     * If no FEN string is provided, initializes to the standard chess starting position.
     * @param fen Optional FEN string to initialize board state.
     */
    ChessBoard(const std::string& fen = INITIAL_FEN) {
        initializeBoard(fen);
    }

    // ===================== PUBLIC METHODS =====================

    /**
     * @brief Generates a simple text-based representation of the board.
     * 
     * Each square is represented by a character: 
     * Uppercase for white pieces, lowercase for black, underscore '_' for empty.
     * The board is returned as a string with ranks separated by newline characters.
     * @return std::string A human-readable board representation.
     */
    std::string displayBoard() {
        std::string rep = "";

        for(int i = 0; i < 64; i++) {
            // Insert newline at the start of a new rank
            if(i != 0 && i % 8 == 0) rep = '\n' + rep;

            //  White piece display
            if(bitmapOccupiedAt(whitePawns, i))        { rep = "P" + rep; }
            else if(bitmapOccupiedAt(whiteKnights, i)){ rep = "N" + rep; }
            else if(bitmapOccupiedAt(whiteBishops, i)){ rep = "B" + rep; }
            else if(bitmapOccupiedAt(whiteRooks, i))  { rep = "R" + rep; }
            else if(bitmapOccupiedAt(whiteQueens, i)) { rep = "Q" + rep; }
            else if(bitmapOccupiedAt(whiteKing, i))   { rep = "K" + rep; }

            // Black Piece display
            else if(bitmapOccupiedAt(blackPawns, i))        { rep = "p" + rep; }
            else if(bitmapOccupiedAt(blackKnights, i))      { rep = "n" + rep; }
            else if(bitmapOccupiedAt(blackBishops, i))      { rep = "b" + rep; }
            else if(bitmapOccupiedAt(blackRooks, i))        { rep = "r" + rep; }
            else if(bitmapOccupiedAt(blackQueens, i))       { rep = "q" + rep; }
            else if(bitmapOccupiedAt(blackKing, i))         { rep = "k" + rep; }

            // Empty square
            else { rep = '_' + rep; }
        }

        return rep;
    }
};

#include <iostream>

int main() {
    ChessBoard b;
    std::cout << b.displayBoard() << "\n";
    return 0;
}
