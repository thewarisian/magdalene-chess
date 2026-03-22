#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>

// ================== DATA CACHE ============================================================================

/**
 * @namespace chessmeta
 * @brief Contains high-level metadata describing the chessboard and game configuration.
 *
 * This namespace defines logical properties of the chessboard that are independent
 * of internal representations such as bitboards. It provides:
 * - The standard initial position in Forsyth–Edwards Notation (FEN)
 * - Board dimensions and derived constants
 *
 * Responsibilities:
 * - Acts as a central source of truth for board size and layout
 * - Provides default game state initialization via FEN
 *
 * @note
 * - FEN encodes the board rank-by-rank from rank 8 (top) to rank 1 (bottom).
 * - These values are immutable and intended to be shared across the engine.
 */
namespace chessmeta {
    // Initial position in FEN notation (full board + basic state)
    // FEN encodes the board row by row from rank 8 (top) to rank 1 (bottom)
    const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Board dimensions
    const int NUM_ROWS = 8;
    const int NUM_COLS = 8;
    const int NUM_TILES = NUM_ROWS * NUM_COLS;
}

/**
 * @namespace bitboard
 * @brief Provides low-level utilities and precomputed data for bitboard-based operations.
 *
 * This namespace contains performance-critical structures used for efficient chess
 * computations. It includes:
 * - The `bitmap` type (64-bit integer) representing board states
 * - Predefined masks for ranks and files
 * - Precomputed attack tables (to be populated) for fast move generation
 *
 * Design Goals:
 * - Enable constant-time board queries and transformations
 * - Avoid recomputation of frequently used patterns
 * - Support scalable move generation logic
 *
 * Bit Indexing Convention:
 * - Bit index 0 corresponds to square a1
 * - Bit index 63 corresponds to square h8
 * - Indices increase left-to-right within a rank, then bottom-to-top across ranks
 *
 * @note
 * - Attack tables are placeholders and should be initialized during program startup
 *   or via compile-time generation.
 * - This namespace should remain free of game-state logic (no turn, castling, etc.).
 */
namespace bitboard {
    using bitmap = uint64_t;

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

    // TODO: Precompute attack bitboards
    // Used later for fast move generation
    const bitmap PAWN_ATTACKS[2][64] = {}; // White (0) and Black (1)
    const bitmap KNIGHT_ATTACKS[64] = {};
    const bitmap BISHOP_ATTACKS[64] = {};
    const bitmap ROOK_ATTACKS[64] = {};
    const bitmap QUEEN_ATTACKS[64] = {};
    const bitmap KING_ATTACKS[64] = {};
}

/**
 * @namespace chessmove
 * @brief Defines data structures used to represent and manipulate chess moves.
 *
 * This namespace isolates move-related logic from board representation and
 * bitboard utilities, improving modularity and maintainability.
 *
 * Responsibilities:
 * - Provide compact and efficient representations of moves
 * - Serve as the interface between move generation and higher-level logic
 *
 * Future Extensions:
 * - Move flags (capture, promotion, castling, en passant)
 * - Piece type information
 * - Encoded move formats (e.g., bit-packed integers for performance)
 */
namespace chessmove {
        /**
     * @struct Move
     * @brief Represents a chess move using bitboard square indices.
     *
     * A move consists of:
     * - `fromBitIdx`: Source square index
     * - `toBitIdx`: Destination square index
     *
     * Indexing:
     * - Range: 0–63
     * - Mapping: a1 = 0, h8 = 63
     *
     * Design Notes:
     * - This is a minimal representation optimized for clarity and simplicity
     * - Suitable for early-stage move generation and debugging
     *
     * @note
     * This structure can be extended to include:
     * - Captured piece type
     * - Promotion piece (if any)
     * - Special move flags (castling, en passant)
     * - Move scoring (for search algorithms)
     */
    struct Move {
        int fromBitIdx;
        int toBitIdx;
    };
}

/**
 * @namespace chessboard
 * @brief Contains core board representations and logic for managing chess positions.
 *
 * This namespace encapsulates:
 * - Human-readable board structures (`row`, `matrix`)
 * - The main `GameBoard` class, which represents a chess position using bitboards
 *
 * Design Overview:
 * - Provides both a high-level (2D matrix) and low-level (bitboard) representation
 * - Separates board state management from move generation and evaluation logic
 * - Ensures modularity and avoids global namespace pollution
 *
 * Representation Details:
 * - Matrix representation:
 *     - Used for input, debugging, and visualization
 *     - Indexed as matrix[rank][file] (rank 0 = 8th rank, file 0 = 'a')
 *
 * - Bitboard representation:
 *     - Used internally for efficient computation
 *     - Each piece type and color has its own 64-bit bitmap
 *     - Bit index 0 corresponds to square a1, index 63 to h8
 *
 * Usage:
 * - Create a `GameBoard` using either:
 *     - A FEN string (standard initialization)
 *     - A 2D matrix (custom/debug setups)
 *
 * @note
 * This namespace focuses solely on board state representation.
 * Move generation, evaluation, and search should be implemented
 * in separate namespaces/modules for better separation of concerns.
 */
namespace chessboard {
    /**
     * @typedef row
     * @brief Represents a single rank (row) of the chessboard.
     *
     * Each row contains exactly 8 characters corresponding to files 'a' through 'h'.
     * Characters follow standard piece notation:
     * - Uppercase letters for white pieces (P, N, B, R, Q, K)
     * - Lowercase letters for black pieces (p, n, b, r, q, k)
     * - '_' for empty squares
     */
    using row = std::array<char, 8>;

    /**
     * @typedef matrix
     * @brief Represents the full 8×8 chessboard as a 2D array.
     *
     * The board is indexed as matrix[rank][file], where:
     * - rank 0 corresponds to rank 8 (top of the board)
     * - rank 7 corresponds to rank 1 (bottom of the board)
     * - file 0 corresponds to file 'a'
     * - file 7 corresponds to file 'h'
     *
     * This representation is human-readable and primarily used for input/output
     * and debugging, while internal computations use bitboards.
     */
    using matrix = std::array<row, 8>;

    // ======================= CHESSBOARD CLASS ======================================================================

    /**
     * @class GameBoard
     * @brief Represents a chess position using bitboards and auxiliary state.
     *
     * This class stores the complete state of a chess game using:
     * - Separate 64-bit bitboards for each piece type and color
     * - Game state flags (side to move, castling rights, en passant)
     * - Move counters for rule enforcement
     *
     * Supported initialization methods:
     * - From a full FEN string (standard format)
     * - From a 2D board matrix (for testing and debugging)
     *
     * Internal Representation:
     * - Squares are indexed from 0 to 63
     * - Index 0 corresponds to square a1
     * - Index 63 corresponds to square h8
     *
     * The class prioritizes efficient move generation and compact storage.
     */
    class GameBoard {
    private:
        // ===================== PIECE BITBOARDS =====================

        bitboard::bitmap whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
        bitboard::bitmap blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

        // ===================== BOARD STATE FLAGS =====================
        bool whiteToMove;
        bool whiteCanCastleKingSide, whiteCanCastleQueenSide;
        bool blackCanCastleKingSide, blackCanCastleQueenSide;

        int halfMove;      // halfmove clock for 50-move rule
        int fullMove;      // fullmove number
        int enPassantIdx;  // bit index of en passant target (-1 if none)

        //========================= HELPER METHODS ====================

        /**
         * @brief Checks whether a given square is occupied in a bitboard.
         *
         * @param b Bitboard to query
         * @param i Square index (0 = a1, 63 = h8)
         * @return True if the bit at index i is set, false otherwise
         */
        static bool bitmapOccupiedAt(const bitboard::bitmap& b, int i) {
            return ((b >> i) & 1);
        }

        /**
         * @brief Sets the bit corresponding to a square in a bitboard.
         *
         * @param b Bitboard to modify
         * @param i Square index (0 = a1, 63 = h8)
         *
         * @note This performs a bitwise OR with a mask containing a single set bit.
         */
        static void placeOnBitmapAt(bitboard::bitmap& b, int i) {
            b |= (1ULL << i);
        }

        /**
         * @brief Converts algebraic notation (e.g., "e4") to a bitboard index.
         *
         * @param tile Square string in the format "a1" to "h8"
         * @return Integer index in range [0, 63]
         *
         * @throws std::out_of_range if the computed index is invalid
         *
         * @note
         * - Rank is mapped from '1'–'8' → 0–7
         * - File is reversed to match internal bitboard orientation
         * - Final mapping ensures:
         *     a1 → 0, h8 → 63
         */
        static int tileStringToBitIndex(const std::string& tile) {
            int rankIdx = tile[1] - '1';                // ranks 0..7
            int fileIdx = (chessmeta::NUM_COLS - 1) - (tile[0] - 'a'); // files reversed for bitboard indexing

            int idx = chessmeta::NUM_COLS * rankIdx + fileIdx;

            if(idx < 0 || idx >= chessmeta::NUM_TILES) {
                throw std::out_of_range("Tile index exceeds chessboard bounds");
            }
            return idx;
        }

        // ===================== INTERNAL METHODS =====================
        
        /**
         * @brief Interprets a FEN character and updates bitboards accordingly.
         *
         * Processes a single character from the FEN piece-placement string:
         * - Places a piece on the appropriate bitboard
         * - Advances the bit index based on the number of squares represented
         *
         * @param ch      FEN character
         * @param bitIdx  Current square index (0–63), passed by reference
         *
         * Behavior:
         * - Piece characters ('P', 'n', etc.) → place piece and increment index
         * - Digits ('1'–'8') → skip that many empty squares
         * - '/' → rank separator (ignored)
         * - Other characters → treated as single-square advancement
         *
         * @note Assumes bitIdx progresses in increasing order (a1 → h8).
         */
        void placePiece(char ch, int& bitIdx) {
            switch(ch) {
                // White pieces
                case 'P': placeOnBitmapAt(whitePawns, bitIdx++); break;
                case 'N': placeOnBitmapAt(whiteKnights, bitIdx++); break;
                case 'B': placeOnBitmapAt(whiteBishops, bitIdx++); break;
                case 'R': placeOnBitmapAt(whiteRooks, bitIdx++); break;
                case 'Q': placeOnBitmapAt(whiteQueens, bitIdx++); break;
                case 'K': placeOnBitmapAt(whiteKing, bitIdx++); break;

                // Black pieces
                case 'p': placeOnBitmapAt(blackPawns, bitIdx++); break;
                case 'n': placeOnBitmapAt(blackKnights, bitIdx++); break;
                case 'b': placeOnBitmapAt(blackBishops, bitIdx++); break;
                case 'r': placeOnBitmapAt(blackRooks, bitIdx++); break;
                case 'q': placeOnBitmapAt(blackQueens, bitIdx++); break;
                case 'k': placeOnBitmapAt(blackKing, bitIdx++); break;

                // ===== FEN STRING CHARACTERS ======
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8':
                    bitIdx += (ch - '0'); // skip empty squares
                    break;
                case '/': break; // rank separator ignored

                default: bitIdx++; break; // unknown char (could warn)
            }
        }

        /**
         * @brief Initializes piece bitboards from FEN piece-placement data.
         *
         * Iterates through the FEN string in reverse order to align with
         * the internal bitboard indexing scheme.
         *
         * @param reducedFen FEN substring containing only piece placement
         *
         * @note
         * - FEN lists ranks from 8 → 1, while bit indices go from a1 → h8
         * - Reverse traversal ensures correct mapping without extra transformations
         * - Only piece positions are set; other state must be handled separately
         */
        void setBoard(const std::string& reducedFen) {
            int pos = 0;
            for(int j = reducedFen.size() - 1; j >= 0; j--) {
                const char& ch = reducedFen[j];
                placePiece(ch, pos);
            }
        }

        /**
         * @brief Parses and applies a full FEN string to the board state.
         *
         * Extracts and initializes:
         * - Piece placement
         * - Side to move
         * - Castling rights
         * - En passant target square
         * - Halfmove and fullmove counters
         *
         * @param fen Full FEN string
         *
         * @throws std::invalid_argument if the FEN string is malformed
         */
        void applyFenString(const std::string& fen) {
            std::istringstream iss(fen);
            std::string reducedFen, toMove, castling, enpass;

            if(!(iss >> reducedFen >> toMove >> castling >> enpass >> halfMove >> fullMove)) {
                throw std::invalid_argument("Malformed FEN");
            }

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

        /**
         * @brief Initializes the board from a 2D matrix representation.
         *
         * Converts a human-readable 8×8 board into internal bitboards.
         *
         * @param boardMatrix 2D array representing the board
         *
         * @note
         * - Assumes matrix[0] is rank 8 and matrix[7] is rank 1
         * - Converts each square into the corresponding bit index
         * - Does NOT infer game state (castling, en passant, etc.)
         * - Default values are assigned to state variables
         *
         * @warning
         * Current implementation assigns placeholder values for:
         * - Side to move
         * - Castling rights
         * - Move counters
         * These should be explicitly set in a complete engine.
         */
        void applyBoardMatrix(const std::array<std::array<char, 8>, 8> boardMatrix) {
            //Setting other parameters arbitrarily
            //TODO: pass/calculate below parameters
            whiteToMove = whiteCanCastleKingSide = whiteCanCastleQueenSide = blackCanCastleKingSide = blackCanCastleQueenSide = true;
            enPassantIdx = -1, halfMove = 0, fullMove = 1;

            //Places pieces from matrix onto board
            for(int i = chessmeta::NUM_ROWS-1; i >= 0; i--) {
                for(int j = chessmeta::NUM_COLS-1; j >= 0; j--) {
                    int bitIdx = chessmeta::NUM_TILES-1 - (8*i + j);
                    placePiece(boardMatrix[i][j], bitIdx);
                }
            }
        }

    public:
        // ===================== CONSTRUCTORS =====================

        /**
         * @brief Constructs a GameBoard from a FEN string.
         *
         * Initializes all bitboards and game state using the provided FEN.
         *
         * @param fen FEN string (defaults to standard starting position)
         */
        GameBoard(const std::string& fen = chessmeta::INITIAL_FEN) {
            // Reset all bitboards
            whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
            blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;

            applyFenString(fen);
        }

        /**
         * @brief Constructs a GameBoard from a 2D matrix.
         *
         * Useful for testing and debugging custom board setups.
         *
         * @param boardMatrix 8×8 character matrix representing the board
         */
        GameBoard(const std::array<std::array<char, 8>, 8> boardMatrix) {
            // Reset all bitboards
            whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
            blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;

            applyBoardMatrix(boardMatrix);
        }

        // ===================== PUBLIC METHODS =====================

        /**
         * @brief Generates a human-readable string representation of the board.
         *
         * Output format:
         * - Uppercase letters for white pieces
         * - Lowercase letters for black pieces
         * - '_' for empty squares
         * - Each rank separated by a blank line for readability
         *
         * @return Formatted board string
         */
        std::string toString() {
            std::string rep = "";
            for(int i = 0; i < chessmeta::NUM_TILES; i++) {
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
}

// ===================== TEST MAIN ======================================================================

int main() {
    chessboard::matrix board = {
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
        chessboard::row{'_','_','_','_','_','_','_','_'},
    };

    chessboard::GameBoard b(board); // Default initial position
    std::cout << b.toString() << "\n";
    return 0;
}