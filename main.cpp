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

    // ============ BIT OPERATION HELPERS ================

    /**
     * @brief Returns a bitboard with a single bit set at the given index.
     *
     * @param bitIdx Square index (0 = a1, 63 = h8)
     * @return Bitboard with only the specified bit set
     */
    inline bitmap singleBit(int bitIdx) {
        return 1ULL << bitIdx;
    }

    /**
     * @brief Checks whether a given square is occupied in a bitboard.
     *
     * @param b Bitboard to query
     * @param i Square index (0 = a1, 63 = h8)
     * @return True if the bit at index i is set, false otherwise
     */
    inline bool occupiedAt(bitmap b, int i) {
        return (b >> i) & 1ULL;
    }

    /**
     * @brief Sets the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     *
     * @note Performs a bitwise OR with a single-bit mask.
     */
    inline void placeBitAt(bitmap& b, int i) {
        b |= singleBit(i);
    }

    /**
     * @brief Clears (resets) the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     *
     * @note Performs a bitwise AND with the inverted single-bit mask.
     */
    inline void removeBitAt(bitmap& b, int i) {
        b &= ~singleBit(i);
    }

    /**
     * @brief Toggles the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     *
     * @note Flips the bit using XOR. Applying twice restores original state.
     */
    inline void toggleBitAt(bitmap& b, int i) {
        b ^= singleBit(i);
    }

    /**
     * @brief Returns the index of the least significant set bit (LSB) in a bitboard.
     *
     * Computes the position of the lowest 1-bit by counting the number of
     * trailing zeros in the binary representation of the bitboard.
     *
     * @param b Bitboard to inspect
     * @return Index (0–63) of the least significant set bit.
     *         Returns 64 if the bitboard is empty (b == 0).
     *
     * @note
     * - Uses GCC/Clang builtin `__builtin_ctzll`, which counts trailing zeros
     *   in a 64-bit integer.
     * - `__builtin_ctzll(0)` is undefined behavior, so the zero case is handled explicitly.
     *
     * @warning
     * - Assumes a 64-bit bitboard representation.
     * - Returned index depends on the engine’s square mapping
     *   (e.g., h1 = 0, a8 = 63 in this implementation).
     *
     * @complexity O(1) — typically compiles to a single CPU instruction.
     */
    int getLeastSignifOneBitIndex(bitmap b) {
        //Count number of leading zeros. Hard set b to 64 as method is undefined for parameter 0
        return b==0? 64 : __builtin_ctzll(b);
    }

    /**
     * @brief Displays a bitboard as an 8×8 grid with customizable symbols.
     *
     * Iterates through all 64 squares in reverse index order (63 → 0)
     * and prints a character for each square based on occupancy:
     * - `occupiedSymbol` if the bit is set
     * - `emptySymbol` if the bit is unset
     *
     * Output Format:
     * - The board is printed rank by rank from top (rank 8) to bottom (rank 1)
     * - Each rank contains 8 squares
     * - A newline is inserted after every rank
     *
     * Bit Index Mapping:
     * - Index 0 corresponds to square h1
     * - Index 63 corresponds to square a8
     * - Reverse iteration ensures correct visual orientation
     *
     * @param b Bitboard to display
     * @param occupiedSymbol Character used for occupied squares (default: '1')
     * @param emptySymbol Character used for empty squares (default: '.')
     *
     * @note
     * - Intended for debugging and visualization.
     * - Uses `occupiedAt()` to determine square occupancy.
     */
    void display(bitmap b, char occupiedSymbol = '1', char emptySymbol='.') {
        for(int i = chessmeta::NUM_TILES-1; i >= 0; i--) {
            // Print symbol based on occupancy
            std::cout << (occupiedAt(b, i) ? occupiedSymbol : emptySymbol);
            if(i % 8 == 0) { std::cout << "\n"; }
        }
    }

    // File bitboards (columns)
    // Each entry represents one file (column) from FILE_A to FILE_H
    const std::array<bitmap, 8> FILE = {
        0x8080808080808080ULL, // FILE_A
        0x4040404040404040ULL, // FILE_B
        0x2020202020202020ULL, // FILE_C
        0x1010101010101010ULL, // FILE_D
        0x0808080808080808ULL, // FILE_E
        0x0404040404040404ULL, // FILE_F
        0x0202020202020202ULL, // FILE_G
        0x0101010101010101ULL  // FILE_H
    };

    // Rank bitboards (rows)
    // Each entry represents one rank (row) from RANK_1 to RANK_8
    const std::array<bitmap, 8> RANK = {
        0x00000000000000FFULL, // RANK_1
        0x000000000000FF00ULL, // RANK_2
        0x0000000000FF0000ULL, // RANK_3
        0x00000000FF000000ULL, // RANK_4
        0x000000FF00000000ULL, // RANK_5
        0x0000FF0000000000ULL, // RANK_6
        0x00FF000000000000ULL, // RANK_7
        0xFF00000000000000ULL  // RANK_8
    };

    // Used later for fast move generation
    const bitmap KNIGHT_ATTACKS[64] = {};
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
     * @brief Represents a chess move using bitboard square indices and piece metadata.
     *
     * A move consists of:
     * - `fromBitIdx`: Source square index
     * - `toBitIdx`: Destination square index
     * - `attackPieceType`: Piece being moved
     * - `capturedPieceType`: Piece being captured (or 'E' if none)
     *
     * Indexing:
     * - Range: 0–63
     * - Mapping: a1 = 0, h8 = 63
     *
     * Piece Encoding:
     * - White: 'P','N','B','R','Q','K'
     * - Black: 'p','n','b','r','q','k'
     * - Empty: 'E'
     *
     * Design Notes:
     * - Stores enough information to apply moves without querying the board
     * - Simplifies make/unmake move logic
     * - Slightly larger than minimal representations but reduces recomputation
     *
     * @note
     * This structure can be extended to include:
     * - Promotion piece (if any)
     * - Special move flags (castling, en passant, double pawn push)
     * - Move flags (bitmask for fast checks)
     * - Move ordering score (for search optimizations)
     *
     * @warning
     * - Assumes consistency with board state (no validation performed)
     * - Incorrect piece types may corrupt board state during move execution
     */
    struct Move {
        int fromBitIdx;
        int toBitIdx;

        char attackPieceType;
        char capturedPieceType;
    };

    bool isDoubleWhitePawnPush(Move m) {
        if(m.attackPieceType != 'P') { return false ;}

        bool pawnOnRank2 = m.fromBitIdx / 8 == 1;
        bool movedToRank4 = (m.toBitIdx - m.fromBitIdx) == 16; // Also ensures pawn move up same file

        return pawnOnRank2 && movedToRank4;
    }
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

    // ======================= BIT INDEX AND CHESS TILE CONVERTERS

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
        int tileStringToBitIndex(const std::string& tile) {
            int rankIdx = tile[1] - '1';                // ranks 0..7
            int fileIdx = (chessmeta::NUM_COLS - 1) - (tile[0] - 'a'); // files reversed for bitboard indexing

            int idx = chessmeta::NUM_COLS * rankIdx + fileIdx;

            if(idx < 0 || idx >= chessmeta::NUM_TILES) {
                throw std::out_of_range("Tile index exceeds chessboard bounds");
            }
            return idx;
        }

        /**
         * @brief Converts a bitboard index to algebraic tile notation (e.g., "e4").
         *
         * Translates a 0–63 bit index into standard chess coordinate format:
         * - File ('a'–'h')
         * - Rank ('1'–'8')
         *
         * @param bitIdx Bit index in the range 0–63
         * @return String representing the corresponding tile (e.g., "e4")
         *
         * @note
         * - Assumes a custom bitboard mapping where:
         *   - Bit 0 corresponds to h1
         *   - Bit indices increase right-to-left across files and bottom-to-top across ranks
         * - The file is computed by reversing the column index to match algebraic notation
         *   (i.e., converting internal right-to-left indexing into standard left-to-right files).
         * - The rank is derived directly from integer division of the bit index.
         *
         * @warning
         * - No bounds checking is performed; passing values outside 0–63 results in undefined behavior.
         *
         * @complexity O(1)
         */
        std::string bitIndexToTileString(int bitIdx) {
            //Reverse/Complement to mirror column ordering to match order of files, and add to 'a' char
            char file = (chessmeta::NUM_COLS-1 - (bitIdx%8)) + 'a';
            //Add '1' to translate accordingly to char
            char rank = bitIdx/8 + '1';
            return std::string(1, file) + rank;
        }

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
        bitboard::bitmap whitePieces, blackPieces, occupiedSquares, emptySquares;

        // ===================== BOARD STATE FLAGS =====================
        bool whiteToMove;
        bool whiteCanCastleKingSide, whiteCanCastleQueenSide;
        bool blackCanCastleKingSide, blackCanCastleQueenSide;

        int halfMove;      // halfmove clock for 50-move rule
        int fullMove;      // fullmove number
        int enPassantIdx;  // bit index of en passant target (-1 if none)

        // ===================== UPDATE METHOD FOR BOARD STATE WHEN CHANGED ================================

        /**
         * @brief Updates aggregate bitboards representing overall board occupancy.
         *
         * Recomputes combined bitboards for:
         * - All white pieces
         * - All black pieces
         * - All occupied squares
         * - All empty squares
         *
         * This function consolidates individual piece bitboards into higher-level
         * representations used for move generation and board evaluation.
         *
         * Computed Bitboards:
         * - `whitePieces`: Union of all white piece bitboards
         * - `blackPieces`: Union of all black piece bitboards
         * - `occupiedSquares`: All squares occupied by any piece
         * - `emptySquares`: Complement of occupied squares
         *
         * @note
         * - Must be called after any change to individual piece bitboards.
         * - `emptySquares` includes all bits not occupied (within 64-bit space).
         */
        void updateBitboards() {
            //All white pieces on board
            whitePieces = 
                whitePawns | whiteKnights | whiteBishops |
                whiteRooks | whiteQueens  | whiteKing;

            //All black pieces on board
            blackPieces = 
                blackPawns | blackKnights | blackBishops |
                blackRooks | blackQueens  | blackKing;

            //Squares with some piece
            occupiedSquares = whitePieces | blackPieces;
            //Squares with no piece
            emptySquares = ~occupiedSquares;
        }

        /**
         * @brief Updates board state after a move is applied.
         *
         * Currently delegates to `updateBoard()` to recompute all aggregate bitboards.
         *
         * Intended as a semantic wrapper to clearly indicate when the update
         * is triggered by a move operation, allowing future extension such as:
         * - Incremental updates instead of full recomputation
         * - Updating additional state (e.g., castling rights, en passant, clocks)
         * - Triggering evaluation or hashing logic
         *
         * @note
         * - Functionally equivalent to `updateBoard()` at present.
         * - Exists for clarity and future scalability.
         */
        void updateBoardAfterMove(chessmove::Move m) {
            //Update en passant
            if(chessmove::isDoubleWhitePawnPush(m)) { enPassantIdx = m.toBitIdx - 8; }
            //Since en passant can only be made immediately after double push
            else { enPassantIdx = -1; }

            //Reverse player;
            whiteToMove = ~whiteToMove;
            //Change all bitboards
            updateBitboards();
            //TODO add other updates
        }
        
        // ===================== INTERNAL METHODS (board initialisation / manipulation) =====================
        
        /**
         * @brief Interprets a FEN character and updates bitboards accordingly.
         *
         * Processes a single character from the FEN piece-placement string and
         * updates the corresponding piece bitboard. Also returns how much the
         * square index (`bitIdx`) should advance.
         *
         * Indexing Model:
         * - Bit indices range from 0–63
         * - Mapping: a1 = 0, h8 = 63 (bottom-up, left-to-right)
         * - `bitIdx` is expected to increase sequentially across the board
         *
         * Behavior:
         * - Piece characters ('P', 'n', etc.):
         *      → Places the piece at the current square
         *      → Advances index by 1
         *
         * - Digits ('1'–'8'):
         *      → Represent consecutive empty squares
         *      → Advances index by the digit value
         *
         * - '/' (rank separator):
         *      → Ignored in this indexing scheme
         *      → No index adjustment needed since progression is linear
         *
         * - Other characters:
         *      → Treated as a single-square advancement (failsafe behavior)
         *
         * @param ch      FEN character to interpret
         * @param bitIdx  Current square index (0–63)
         *
         * @return Number of squares to advance `bitIdx` after processing
         *
         * @note
         * This function assumes that the caller iterates through the FEN string
         * and updates `bitIdx` as:
         *
         *     bitIdx += placePiece(ch, bitIdx);
         *
         * @warning
         * - Assumes valid FEN input; minimal validation is performed
         * - Incorrect characters may silently advance the index
         * - Board consistency is not verified
         */
        int placePiece(char ch, int bitIdx) {
            switch(ch) {
                // ===== WHITE PIECES =====
                case 'P': bitboard::placeBitAt(whitePawns, bitIdx); break;
                case 'N': bitboard::placeBitAt(whiteKnights, bitIdx); break;
                case 'B': bitboard::placeBitAt(whiteBishops, bitIdx); break;
                case 'R': bitboard::placeBitAt(whiteRooks, bitIdx); break;
                case 'Q': bitboard::placeBitAt(whiteQueens, bitIdx); break;
                case 'K': bitboard::placeBitAt(whiteKing, bitIdx); break;

                // ===== BLACK PIECES =====
                case 'p': bitboard::placeBitAt(blackPawns, bitIdx); break;
                case 'n': bitboard::placeBitAt(blackKnights, bitIdx); break;
                case 'b': bitboard::placeBitAt(blackBishops, bitIdx); break;
                case 'r': bitboard::placeBitAt(blackRooks, bitIdx); break;
                case 'q': bitboard::placeBitAt(blackQueens, bitIdx); break;
                case 'k': bitboard::placeBitAt(blackKing, bitIdx); break;

                // ===== EMPTY SQUARES =====
                // Digit represents number of consecutive empty squares
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8':
                    return (ch - '0');

                // ===== RANK SEPARATOR =====
                // Ignored due to linear bottom-up indexing
                case '/':
                    return 0;

                // ===== FALLBACK =====
                // Unknown character → advance one square (safe default)
                default:
                    return 1;
            }

            // All piece placements advance by 1 square
            return 1;
        }

        /**
         * @brief Removes a piece from a given square in the bitboards.
         *
         * Clears the bit corresponding to the given square (`bitIdx`) from the
         * appropriate piece bitboard based on the provided piece character.
         *
         * @param ch Piece identifier:
         *        - 'P','N','B','R','Q','K' for white pieces
         *        - 'p','n','b','r','q','k' for black pieces
         *        - 'E' or any other value → no operation
         *
         * @param bitIdx Bit index (0–63) representing the square to clear.
         *
         * @note This function assumes the piece actually exists on the given square.
         *       If not, the operation is still safe (bitwise clear is idempotent).
         *
         * @warning Does not validate board consistency. Passing incorrect piece types
         *          may silently corrupt board state.
         *
         * @see placePiece()
         */
        void removePiece(char ch, int bitIdx) {
            switch(ch) {
                // White pieces
                case 'P': bitboard::removeBitAt(whitePawns, bitIdx); break;
                case 'N': bitboard::removeBitAt(whiteKnights, bitIdx); break;
                case 'B': bitboard::removeBitAt(whiteBishops, bitIdx); break;
                case 'R': bitboard::removeBitAt(whiteRooks, bitIdx); break;
                case 'Q': bitboard::removeBitAt(whiteQueens, bitIdx); break;
                case 'K': bitboard::removeBitAt(whiteKing, bitIdx); break;

                // Black pieces
                case 'p': bitboard::removeBitAt(blackPawns, bitIdx); break;
                case 'n': bitboard::removeBitAt(blackKnights, bitIdx); break;
                case 'b': bitboard::removeBitAt(blackBishops, bitIdx); break;
                case 'r': bitboard::removeBitAt(blackRooks, bitIdx); break;
                case 'q': bitboard::removeBitAt(blackQueens, bitIdx); break;
                case 'k': bitboard::removeBitAt(blackKing, bitIdx); break;

                //Empty target square
                case 'E' : 
                // unknown char (could warn)
                default: break;
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
                pos += placePiece(ch, pos);
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
        void applyBoardMatrix(const std::array<std::array<char, 8>, 8>& boardMatrix) {
            //Setting other parameters arbitrarily
            //TODO: pass/calculate below parameters
            whiteToMove = whiteCanCastleKingSide = whiteCanCastleQueenSide = blackCanCastleKingSide = blackCanCastleQueenSide = true;
            enPassantIdx = -1, halfMove = 0, fullMove = 1;

            //Places pieces from matrix onto board
            for(int i = chessmeta::NUM_ROWS-1; i >= 0; i--) {
                for(int j = chessmeta::NUM_COLS-1; j >= 0; j--) {
                    int bitIdx = (chessmeta::NUM_TILES-1) - (8*i + j);
                    bitIdx += placePiece(boardMatrix[i][j], bitIdx);
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
            updateBitboards();
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
            updateBitboards();
        }

        // ===================== PUBLIC GETTER METHODS =====================

        /**
         * @brief Generates a human-readable string representation of the board.
         *
         * Iterates through all 64 squares and constructs a formatted string
         * showing the current board state using piece symbols.
         *
         * Output format:
         * - Uppercase letters represent white pieces (P, N, B, R, Q, K)
         * - Lowercase letters represent black pieces (p, n, b, r, q, k)
         * - '_' represents an empty square
         * - Squares are separated by two spaces for readability
         * - Each rank is separated by a blank line
         *
         * @note
         * - Iteration proceeds from bit index 0 to 63, but the string is built
         *   in reverse to display the board from rank 8 (top) to rank 1 (bottom).
         * - This method is intended for debugging and visualization, not performance-critical use.
         *
         * @return A formatted string representing the current board position
         */
        std::string toString() const {
            std::string rep = "";
            for(int i = 0; i < chessmeta::NUM_TILES; i++) {
                if(i != 0 && i % 8 == 0) rep = "\n\n" + rep;

                if(bitboard::occupiedAt(whitePawns, i))        { rep = "P  " + rep; }
                else if(bitboard::occupiedAt(whiteKnights, i)) { rep = "N  " + rep; }
                else if(bitboard::occupiedAt(whiteBishops, i)) { rep = "B  " + rep; }
                else if(bitboard::occupiedAt(whiteRooks, i))   { rep = "R  " + rep; }
                else if(bitboard::occupiedAt(whiteQueens, i))  { rep = "Q  " + rep; }
                else if(bitboard::occupiedAt(whiteKing, i))    { rep = "K  " + rep; }
                else if(bitboard::occupiedAt(blackPawns, i))   { rep = "p  " + rep; }
                else if(bitboard::occupiedAt(blackKnights,i))  { rep = "n  " + rep; }
                else if(bitboard::occupiedAt(blackBishops,i))  { rep = "b  " + rep; }
                else if(bitboard::occupiedAt(blackRooks, i))   { rep = "r  " + rep; }
                else if(bitboard::occupiedAt(blackQueens,i))   { rep = "q  " + rep; }
                else if(bitboard::occupiedAt(blackKing,i))     { rep = "k  " + rep; }
                
                else { rep = "_  " + rep; }
            }
            return "\n" + rep + "\n";
        }
        
        /**
         * @brief Retrieves the bitboard corresponding to a specific piece type.
         *
         * Maps a character representing a chess piece to its associated bitboard.
         *
         * @param type Character representing the piece:
         *             - White: 'P', 'N', 'B', 'R', 'Q', 'K'
         *             - Black: 'p', 'n', 'b', 'r', 'q', 'k'
         *
         * @return Bitboard containing all squares occupied by the specified piece type
         *
         * @throws std::invalid_argument if the piece type is invalid
         *
         * @note
         * - This function provides a uniform interface for accessing individual
         *   piece bitboards and is commonly used in move generation.
         */
        bitboard::bitmap getPieceBitboard(char type) const {
            switch(type) {
                // White pieces
                case 'P': return whitePawns;
                case 'N': return whiteKnights;
                case 'B': return whiteBishops;
                case 'R': return whiteRooks;
                case 'Q': return whiteQueens;
                case 'K': return whiteKing;

                // Black pieces
                case 'p': return blackPawns;
                case 'n': return blackKnights;
                case 'b': return blackBishops;
                case 'r': return blackRooks;
                case 'q': return blackQueens;
                case 'k': return blackKing;

                default: throw std::invalid_argument("Invalid piece type");
            }
        }

        /**
         * @brief Retrieves a combined bitboard based on a color or occupancy filter.
         *
         * Returns aggregated bitboards representing subsets of the board:
         *
         * @param colourFilter Specifies which set to return:
         *        - 'W' : All white pieces
         *        - 'B' : All black pieces
         *        - 'A' : All occupied squares (white | black)
         *        - 'N' : All empty squares (~occupied)
         *
         * @return Bitboard corresponding to the selected filter
         *
         * @throws std::invalid_argument if the filter is invalid
         *
         * @note
         * - This function simplifies access to commonly used board masks during
         *   move generation and evaluation.
         */
        bitboard::bitmap getAllPiecesBitboard(char colourFilter = 'A') const {
            switch(colourFilter) {
                case 'W': return whitePieces;
                case 'B': return blackPieces;
                case 'A': return occupiedSquares;
                case 'N': return emptySquares;

                default: throw std::invalid_argument("Invalid piece type");
            }
        }
        
        // ========================= MUTATOR METHODS =======================

        /**
         * @brief Executes a move on the board.
         *
         * Applies the given move by:
         * 1. Removing any captured piece from the destination square.
         * 2. Removing the attacking piece from its source square.
         * 3. Placing the attacking piece on the destination square.
         * 4. Updating aggregate board state (e.g., occupancies, side to move, etc.).
         *
         * @param move Reference to a Move struct containing:
         *        - fromBitIdx: Source square index
         *        - toBitIdx: Destination square index
         *        - attackPieceType: Type of the moving piece
         *        - capturedPieceType: Type of captured piece (if any)
         *
         * @note Assumes the move is pseudo-legal or legal.
         * @note Does NOT handle special moves explicitly (castling, en passant, promotion)
         *       unless encoded within the Move object and handled inside helper functions.
         *
         * @warning Does not perform legality checks (e.g., king safety).
         *          Caller is responsible for ensuring move validity.
         */
        void makeMove(chessmove::Move& move) {
            removePiece(move.capturedPieceType, move.toBitIdx);
            removePiece(move.attackPieceType, move.fromBitIdx);
            placePiece(move.attackPieceType, move.toBitIdx);

            updateBoardAfterMove(move);
        } 
    };
}

/**
 * @namespace movegen
 * @brief Contains functions responsible for generating pseudo-legal moves.
 *
 * This namespace operates on the current board state and produces move
 * bitboards for individual piece types using efficient bitboard operations.
 */
namespace movegen {
    /**
     * @brief Generates pseudo-legal moves for all white pawns.
     *
     * Computes a bitboard representing all possible moves for white pawns,
     * including:
     * - Diagonal captures (left and right)
     * - Single forward pushes
     * - Double forward pushes from the starting rank
     *
     * @param b Reference to the current GameBoard state
     *
     * @return Bitboard containing all destination squares reachable by white pawns
     *
     * @note
     * - This function does NOT handle:
     *   - Promotions
     *   - En passant captures
     *   - Move legality (e.g., checks)
     *
     * - File masking prevents wraparound during diagonal shifts.
     * - Double push requires both intermediate and destination squares to be empty.
     * 
     * Double pawn pushes are computed by ensuring:
     * - The destination square is empty
     * - The intermediate square (one step ahead) is also empty
     * - The pawn lands on rank 4 (i.e., originated from rank 2)
     *
     * @warning
     * - Assumes correct alignment of FILE[] and RANK[] masks with the board's
     *   bit indexing scheme.
     * - Incorrect mask orientation will lead to invalid move generation.
     */
    bitboard::bitmap calculateWhitePawnMoves(const chessboard::GameBoard& b) {
        bitboard::bitmap whitePawns = b.getPieceBitboard('P');
        bitboard::bitmap blackPieces = b.getAllPiecesBitboard('B');
        bitboard::bitmap emptySquares = b.getAllPiecesBitboard('N');


        //rightward attack
        bitboard::bitmap pawnMovesBitboard = (whitePawns<<7) & blackPieces & ~bitboard::FILE[0];
        //leftward attack
        pawnMovesBitboard |= (whitePawns<<9) & blackPieces & ~bitboard::FILE[7];
        //single pawn push
        pawnMovesBitboard |= (whitePawns<<8) & emptySquares;
        //double pawn push
        pawnMovesBitboard |= (whitePawns<<16) & emptySquares & (emptySquares << 8) & bitboard::RANK[3];

        return pawnMovesBitboard;
    }

    /**
     * @brief Converts a bitboard of moves into a list of destination square strings.
     *
     * Iterates through all set bits in the given bitboard and converts each
     * bit index into standard algebraic notation (e.g., "e4").
     *
     * @param movesBitboard Bitboard where each set bit represents a valid destination square
     * @return Vector of strings representing destination squares
     *
     * @note
     * - Uses an efficient bit iteration technique:
     *   - `__builtin_ctzll` (via getLeastSignifOneBitIndex) to find the index of the
     *     least significant set bit (LSB)
     *   - `b &= (b - 1)` to remove the LSB after processing
     * - Iteration order is from least significant bit to most significant bit,
     *   which depends on the internal bitboard mapping.
     *
     * @warning
     * - The order of moves in the returned vector is not sorted in chessboard order;
     *   it follows the bit order of the underlying representation.
     *
     * @complexity O(k), where k is the number of set bits in the bitboard
     */
    std::vector<std::string> getMovesList(bitboard::bitmap movesBitboard) {
        std::vector<std::string> destinationSquares;

        while(movesBitboard) {
            //Get bit index of least significant set bit (rightmost 1-bit)
            int nextBitIdx = bitboard::getLeastSignifOneBitIndex(movesBitboard);
            //Convert to tile name and add to vector
            destinationSquares.push_back(chessboard::bitIndexToTileString(nextBitIdx));
            //Remove least significant 1 bit
            movesBitboard &= (movesBitboard-1);
        }

        return destinationSquares;
    }
}

// ===================== TEST MAIN ======================================================================

int main() {
    chessboard::matrix board = {
        chessboard::row{'r','n','b','q','k','b','n','r'}, // rank 8
        chessboard::row{'p','p','p','p','p','p','p','p'}, // rank 7
        chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 6
        chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 5
        chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 4
        chessboard::row{'_','_','_','_','_','_','_','_'}, // rank 3
        chessboard::row{'P','P','P','P','P','P','P','P'}, // rank 2
        chessboard::row{'R','N','B','Q','K','B','N','R'}, // rank 1
    };

    chessboard::GameBoard b; // Default initial position
    chessmove::Move m = {12, 28, 'N', 'E'};
    std::cout << chessmove::isDoubleWhitePawnPush(m) << "\n";
    return 0;
}