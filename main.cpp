#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>

using bitmap = uint64_t;

// ================== DATA CACHE ============================================================================

// Initial position in FEN notation (full board + basic state)
// FEN encodes the board row by row from rank 8 (top) to rank 1 (bottom)
const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Board dimensions
const int NUM_ROWS = 8;
const int NUM_COLS = 8;
const int NUM_TILES = NUM_ROWS * NUM_COLS;

// File bitboards (columns)
// Each entry represents one file (column) from FILE_A to FILE_H
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
// Each entry represents one rank (row) from RANK_1 to RANK_8
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
// Used later for fast move generation
const bitmap PAWN_ATTACKS[2][64] = {}; // White (0) and Black (1)
const bitmap KNIGHT_ATTACKS[64] = {};
const bitmap BISHOP_ATTACKS[64] = {};
const bitmap ROOK_ATTACKS[64] = {};
const bitmap QUEEN_ATTACKS[64] = {};
const bitmap KING_ATTACKS[64] = {};

// ======================= CHESSBOARD CLASS ======================================================================

/**
 * @class ChessBoard
 * @brief Represents a chessboard using 64-bit bitboards for each piece type and color.
 *
 * This class maintains separate bitboards for each piece type and color.
 * Supports initialization via FEN strings, including piece placement,
 * castling rights, en passant square, and move counters.
 */
class ChessBoard {
private:
    // ===================== PIECE BITBOARDS =====================

    bitmap whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
    bitmap blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

    // ===================== BOARD STATE FLAGS =====================
    bool whiteToMove;
    bool whiteCanCastleKingSide, whiteCanCastleQueenSide;
    bool blackCanCastleKingSide, blackCanCastleQueenSide;

    int halfMove;      // halfmove clock for 50-move rule
    int fullMove;      // fullmove number
    int enPassantIdx;  // bit index of en passant target (-1 if none)

    // ===================== HELPER METHODS ======================

    /**
     * @brief Checks if a specific square is occupied in a bitboard.
     * @param b Bitboard to check
     * @param i Square index (0 = a1, 63 = h8)
     * @return True if the square contains a piece
     */
    static bool bitmapOccupiedAt(const bitmap& b, int i) {
        return ((b >> i) & 1);
    }

    /**
     * @brief Sets a bit in a bitboard to indicate a piece at a specific square.
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     */
    static void placeOnBitmapAt(bitmap& b, int i) {
        b |= (1ULL << i);
    }

    /**
     * @brief Converts a chess square in algebraic notation (e.g., "e4") to a bit index.
     * @param tile Square string in format "a1" to "h8"
     * @return Bit index in the range 0-63
     * @throws std::out_of_range if the tile is invalid
     */
    static int tileStringToBitIndex(const std::string& tile) {
        int rankIdx = tile[1] - '1';                // ranks 0..7
        int fileIdx = (NUM_COLS - 1) - (tile[0] - 'a'); // files reversed for bitboard indexing

        int idx = NUM_COLS * rankIdx + fileIdx;

        if(idx < 0 || idx >= NUM_TILES) {
            throw std::out_of_range("Tile index exceeds chessboard bounds");
        }
        return idx;
    }

    // ===================== INTERNAL METHODS =====================

    /**
     * @brief Populates the piece bitboards from the FEN board string.
     * @param reducedFen FEN string containing only the piece placement
     *
     * Loops in reverse to match the bitboard layout. Does not set
     * castling, en passant, or move counters.
     */
    void setBoard(const std::string& reducedFen) {
        int pos = 0;
        for(int j = reducedFen.size() - 1; j >= 0; j--) {
            const char& ch = reducedFen[j];
            switch(ch) {
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8':
                    pos += (ch - '0'); // skip empty squares
                    break;
                case '/': break; // rank separator ignored

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

                default: pos++; break; // unknown char (could warn)
            }
        }
    }

    /**
     * @brief Parses a full FEN string and sets board and state variables.
     * @param fen Full FEN string (piece placement, to move, castling, en passant, half/full move)
     */
    void applyFenString(const std::string& fen) {
        std::istringstream iss(fen);
        std::string reducedFen, toMove, castling, enpass;

        if(!(iss >> reducedFen >> toMove >> castling >> enpass >> halfMove >> fullMove)) {
            throw std::invalid_argument("Malformed FEN");
        }

        // Reset all bitboards
        whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
        blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;

        // Set piece positions
        setBoard(reducedFen);

        // Player to move
        whiteToMove = toMove == "w";

        // Castling rights
        whiteCanCastleKingSide = whiteCanCastleQueenSide = false;
        blackCanCastleKingSide = blackCanCastleQueenSide = false;
        for(char ch : castling) {
            switch(ch) {
                case 'K': whiteCanCastleKingSide = true; break;
                case 'Q': whiteCanCastleQueenSide = true; break;
                case 'k': blackCanCastleKingSide = true; break;
                case 'q': blackCanCastleQueenSide = true; break;
            }
        }

        // En passant target
        enPassantIdx = enpass == "-" ? -1 : tileStringToBitIndex(enpass);
    }

public:
    // ===================== CONSTRUCTORS =====================

    /**
     * @brief Constructs a ChessBoard object from FEN.
     * @param fen Optional FEN string; defaults to standard initial position.
     */
    ChessBoard(const std::string& fen = INITIAL_FEN) {
        applyFenString(fen);
    }

    // ===================== PUBLIC METHODS =====================

    /**
     * @brief Returns a simple text-based representation of the board.
     *
     * Squares are:
     * - Uppercase letters for white pieces (P, N, B, R, Q, K)
     * - Lowercase letters for black pieces (p, n, b, r, q, k)
     * - '_' for empty squares
     * Ranks are separated by double newlines.
     */
    std::string toString() {
        std::string rep = "";
        for(int i = 0; i < NUM_TILES; i++) {
            if(i != 0 && i % 8 == 0) rep = "\n\n" + rep;

            if(bitmapOccupiedAt(whitePawns, i))        { rep = "P  " + rep; }
            else if(bitmapOccupiedAt(whiteKnights, i)) { rep = "N  " + rep; }
            else if(bitmapOccupiedAt(whiteBishops, i)) { rep = "B  " + rep; }
            else if(bitmapOccupiedAt(whiteRooks, i))   { rep = "R  " + rep; }
            else if(bitmapOccupiedAt(whiteQueens, i))  { rep = "Q  " + rep; }
            else if(bitmapOccupiedAt(whiteKing, i))    { rep = "K  " + rep; }
            else if(bitmapOccupiedAt(blackPawns, i))   { rep = "p  " + rep; }
            else if(bitmapOccupiedAt(blackKnights,i))  { rep = "n  " + rep; }
            else if(bitmapOccupiedAt(blackBishops,i))  { rep = "b  " + rep; }
            else if(bitmapOccupiedAt(blackRooks, i))   { rep = "r  " + rep; }
            else if(bitmapOccupiedAt(blackQueens,i))   { rep = "q  " + rep; }
            else if(bitmapOccupiedAt(blackKing,i))     { rep = "k  " + rep; }
            
            else { rep = "_  " + rep; }
        }
        return "\n" + rep + "\n";
    }
};

// ===================== TEST MAIN ======================================================================

int main() {
    ChessBoard b; // Default initial position
    std::cout << b.toString() << "\n";
    return 0;
}