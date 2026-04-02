#pragma once

#include <string>
#include <sstream>

#include "meta/metadata.h"
#include "bitboard/bitboard.h"
#include "move/move.h"

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

    // ======================= BIT INDEX AND CHESS TILE CONVERTERS ==========================

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
        inline int tileStringToBitIndex(const std::string& tile) {
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
        inline std::string bitIndexToTileString(int bitIdx) {
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
        void updateBitboards();

        /**
         * @brief Updates board state after a move is applied.
         *
         * Handles:
         * - En passant target square
         * - Side to move toggle
         * - Recalculation of aggregate bitboards
         *
         * @param m Move that was executed
         *
         * @note
         * - En passant square is set only for double pawn pushes
         * - Cleared otherwise
         *
         * @warning
         * - Does NOT update:
         *     - Castling rights
         *     - Halfmove clock (50-move rule)
         *     - Fullmove counter
         *     - Special move effects (promotion, castling, en passant capture)
         */
        void updateBoardAfterMove(chessmove::Move m);
        
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
        int placePiece(char ch, int bitIdx);

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
        void removePiece(char ch, int bitIdx);

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
        void setBoard(const std::string& reducedFen);

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
        void applyFenString(const std::string& fen);

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
         *
         * @note
         * - Assumes each matrix entry represents exactly one square
         * - Unlike FEN parsing, no multi-square compression is used
         *
         * @warning
         * - Relies on placePiece() returning 1 for all inputs
         * - Reusing placePiece() for other formats may break this assumption
         */
        void applyBoardMatrix(const std::array<std::array<char, 8>, 8>& boardMatrix);

    public:
        // ===================== CONSTRUCTORS =====================

        /**
         * @brief Constructs a GameBoard from a FEN string.
         *
         * Initializes all bitboards and game state using the provided FEN.
         *
         * @param fen FEN string (defaults to standard starting position)
         */
        GameBoard(const std::string& fen = chessmeta::INITIAL_FEN);

        /**
         * @brief Constructs a GameBoard from a 2D matrix.
         *
         * Useful for testing and debugging custom board setups.
         *
         * @param boardMatrix 8×8 character matrix representing the board
         */
        GameBoard(const std::array<std::array<char, 8>, 8> boardMatrix, bool wToMove, 
        bool wCanCastKing, bool wCanCastQueen, bool bCanCastKing, bool bCanCastQueen,
        int enPassIdx, int numHalf, int numFull);

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
        std::string toString() const;
        
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
        bitboard::bitmap getPieceBitboard(char type) const;

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
        bitboard::bitmap getAllPiecesBitboard(char colourFilter = 'A') const;
        
        /**
     * @brief Returns the en passant target square as a bitboard.
     *
     * If an en passant capture is currently available, this function
     * returns a bitboard with a single bit set at the en passant square.
     * Otherwise, returns an empty bitboard (0).
     *
     * @return Bitboard with en passant square set, or 0 if none exists
     *
     * @note
     * - En passant square is valid only immediately after a double pawn push
     * - Does not check which side can capture; only encodes the square
     */
        inline bitboard::bitmap getEnPassantAttackSquare() const {
            if(enPassantIdx == -1) { return 0ULL; }
            return bitboard::singleBit(enPassantIdx);
        }

        /**
         * @brief Indicates which player's turn it is.
         *
         * @return True if it is White's turn to move, false if Black's
         *
         * @note
         * - Updated after every move in updateBoardAfterMove()
         */
        inline bool isWhiteTurn() const {
            return whiteToMove;
        }

        // ========================= MUTATOR METHODS =======================

        /**
         * @brief Executes a move on the board.
         *
         * Applies the given move by:
         * 1. Removing any captured piece from the destination square
         * 2. Removing the moving piece from the source square
         * 3. Placing the moving piece on the destination square
         * 4. Updating board state (turn, en passant, occupancy)
         *
         * @param move Move to execute
         *
         * @note
         * - Assumes the move is pseudo-legal or legal
         * - Relies entirely on Move metadata (no board validation)
         *
         * @warning
         * - Does NOT handle:
         *     - Castling (rook movement not applied)
         *     - En passant capture (captured pawn not removed)
         *     - Promotion (no piece replacement)
         * - Incorrect Move data may corrupt board state
         */
        void makeMove(chessmove::Move& move);
    };
}
