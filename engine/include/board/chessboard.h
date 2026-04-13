/**
 * @file chessboard.h
 * @brief Core board representation and state management for the chess engine.
 *
 * This file defines the GameBoard class, which encapsulates the full state
 * of a chess position using bitboards and auxiliary metadata.
 *
 * Responsibilities:
 * - Maintain piece placement using bitboards
 * - Track game state (turn, castling rights, en passant, move counters)
 * - Provide interfaces for move execution and board inspection
 *
 * @note
 * - This module does not perform move generation or legality checks
 * - It serves as the central state container for the engine
 */
#pragma once

#include <string>
#include <sstream>

#include "core/metadata.h"
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
 *     - Bit index 0 corresponds to square h1
 *     - Bit index 63 corresponds to square a8
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
        inline Square tileStringToSquare(const std::string& tile) {
            int rankIdx = tile[1] - '1';                // ranks 0..7
            int fileIdx = (chessmeta::NUM_COLS - 1) - (tile[0] - 'a'); // files reversed for bitboard indexing

            int idx = chessmeta::NUM_COLS * rankIdx + fileIdx;

            if(idx < 0 || idx >= chessmeta::NUM_TILES) {
                throw std::out_of_range("Tile index exceeds chessboard bounds");
            }
            return utils::intToSquare(idx);
        }

        /**
         * @brief Converts a square to algebraic notation (e.g., "e4").
         *
         * @param sq Square enum value
         * @return Corresponding tile string
         *
         * @note
         * - Converts internal bitboard indexing to standard file/rank notation
         */
        inline std::string squareToString(Square sq) {
            int bitIdx = static_cast<int>(sq);
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

        //Rows are black/white and columns are each piece type
        std::array<std::array<bitboard::bitmap, 6>, 2> pieceBitboard;
        bitboard::bitmap allWhitePieces, allBlackPieces, occupiedSquares, emptySquares;

        // ===================== BOARD STATE FLAGS =====================
        bool whiteToMove;
        bool whiteCanCastleKingSide, whiteCanCastleQueenSide;
        bool blackCanCastleKingSide, blackCanCastleQueenSide;

        int halfMove;      // halfmove clock for 50-move rule
        int fullMove;      // fullmove number
        Square enPassantSq;  // bit index of en passant target (-1 if none)

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
        void updateBoardAfterMove(const chessmove::Move& move,
        Color attackPieceColor, PieceType attackPieceType, 
        Color capturedPieceColor, PieceType capturedPieceType);
        
        // ===================== INTERNAL METHODS (board initialisation / manipulation) =====================

        bitboard::bitmap& getBitboardOf(Color col, PieceType piece);
        
        /**
         * @brief Places a piece on the given square.
         *
         * Updates the appropriate piece bitboard by setting the corresponding bit.
         *
         * @param col Piece color
         * @param piece Piece type
         * @param sq Target square
         *
         * @note
         * - Does not update aggregate bitboards (call updateBitboards separately)
         */
        void placePiece(Color col, PieceType piece, Square sq);

        /**
         * @brief Removes a piece from the given square.
         *
         * Clears the bit corresponding to the specified square from the
         * appropriate piece bitboard.
         *
         * @param col Color of the piece to remove
         * @param piece Type of the piece to remove
         * @param sq Square from which the piece is removed
         *
         * @note
         * - This operation is idempotent: clearing an already empty bit has no effect
         * - Does not update aggregate bitboards (call updateBitboards() afterward)
         *
         * @warning
         * - Assumes the correct piece type and color are provided
         * - Passing incorrect values may lead to inconsistent board state
         *
         * @see placePiece()
         */
        void removePiece(Color col, PieceType piece, Square sq);

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
        Square enPassSq, int numHalf, int numFull);

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
        
        bitboard::bitmap copyPieceBitboard(Color col, PieceType piece) const;

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
        bitboard::bitmap copyAllPiecesBitboard(Color colorFilter = Color::None) const;
        
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
            if(enPassantSq == Square::None) { return 0ULL; }
            return bitboard::singleBit(enPassantSq);
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
         * - Piece and capture information must be provided externally
         *   since Move does not store this data.
         *
         * @warning
         * - Does NOT handle:
         *     - Castling (rook movement not applied)
         *     - En passant capture (captured pawn not removed)
         *     - Promotion (no piece replacement)
         * - Incorrect Move data may corrupt board state
         */
        void makeMove (const chessmove::Move& move,
        Color attackPieceColor, PieceType attackPieceType, 
        Color capturedPieceColor, PieceType capturedPieceType);
    };
}
