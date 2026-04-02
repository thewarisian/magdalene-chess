#pragma once

#include <vector>

#include "bitboard/bitboard.h"
#include "board/chessboard.h"

/**
 * @namespace movegen
 * @brief Contains functions responsible for generating pseudo-legal moves.
 *
 * This namespace operates on the current board state and produces move
 * bitboards for individual piece types using efficient bitboard operations.
 */
namespace movegen {
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
     *
     * @note
     * - Only destination squares are returned
     * - Does NOT include:
     *     - Source square
     *     - Piece type
     *     - Special move flags (capture, promotion, etc.)
     *
     * - Intended for debugging or visualization, not full move representation
     */
    std::vector<std::string> getMovesList(bitboard::bitmap movesBitboard);

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
     *
     * @note
     * - Double pawn push uses a bitwise alignment trick:
     *     (emptySquares << 8) ensures the intermediate square is empty
     *     by aligning it with the pawn's destination square
     *
     * - This avoids explicitly computing intermediate positions
     *   but relies on consistent bitboard orientation
     */
    bitboard::bitmap calculateWhitePawnMoves(const chessboard::GameBoard& b);

    /**
     * @brief Calculates all pseudo-legal moves for black pawns.
     *
     * Generates a bitboard representing all possible destination squares
     * for black pawn moves, including:
     * - Single forward pushes
     * - Double pushes from initial rank
     * - Diagonal captures
     * - En passant captures
     *
     * @param b Constant reference to the current GameBoard
     * @return Bitboard with bits set at all valid destination squares
     *
     * @details
     * Move generation is performed using bitwise operations on bitboards:
     *
     * 1. Single Push:
     *    - Black pawns move one square down (>> 8)
     *    - Only allowed if the destination square is empty
     *
     * 2. Double Push:
     *    - Black pawns on rank 7 can move two squares down (>> 16)
     *    - Requires both:
     *        - Destination square empty
     *        - Intermediate square empty
     *    - Intermediate square is checked using:
     *        (emptySquares >> 8)
     *      which aligns the square in front of the pawn with the destination
     *
     * 3. Captures:
     *    - Right capture: (>> 7)
     *    - Left capture:  (>> 9)
     *    - Captures include:
     *        - White pieces
     *        - En passant target square
     *
     * 4. File Masking:
     *    - Prevents wraparound across board edges:
     *        - ~FILE[7] blocks h-file overflow (for >> 7)
     *        - ~FILE[0] blocks a-file overflow (for >> 9)
     *
     * @note
     * - Bitboard indexing assumes:
     *     - bit 0 = a1, bit 63 = h8
     *     - Right shift (>>) moves pieces toward rank 1
     *
     * - En passant square is treated as a capturable square
     *   and must be handled separately during move execution
     *
     * @warning
     * - Generates pseudo-legal moves only:
     *     - Does NOT check for king safety
     *     - Does NOT handle pins
     *
     * - Does NOT handle:
     *     - Pawn promotion (rank 1)
     *     - En passant capture removal (only includes square)
     *
     * - Assumes:
     *     - GameBoard correctly maintains:
     *         - Occupancy bitboards
     *         - En passant square validity
     *
     * @see calculateWhitePawnMoves for white pawn equivalent
     */
    bitboard::bitmap calculateBlackPawnMoves(const chessboard::GameBoard& b);
}