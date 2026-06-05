/**
 * @file movegen.h
 * @brief Pseudo-legal move generation using bitboard operations.
 *
 * This file contains functions responsible for generating pseudo-legal
 * moves from a given board state. Move generation is performed using
 * efficient bitwise operations on bitboards.
 *
 * Scope:
 * - Generates destination squares only (bitboards)
 * - Does NOT enforce move legality (e.g., king safety, pins)
 *
 * @note
 * - All functions assume a consistent bitboard mapping and correctly
 *   initialized GameBoard state.
 */
#pragma once

#include <vector>

#include "board/chessboard.h"
#include "bitboard/bitboard.h"

/**
 * @namespace movegen
 * @brief Contains functions responsible for generating pseudo-legal moves.
 *
 * This namespace operates on the current board state and produces move
 * bitboards for individual piece types using efficient bitboard operations.
 */
namespace movegen {
    /**
     * @brief Converts a bitboard into a list of destination squares (algebraic notation).
     *
     * Iterates over all set bits and converts each index into a square string
     * (e.g., "e4").
     *
     * @param movesBitboard Bitboard with set bits representing destination squares
     * @return Vector of destination squares in algebraic notation
     *
     * @note
     * - Iteration uses efficient LSB extraction:
     *     - Isolate LSB
     *     - Process index
     *     - Clear LSB
     *
     * - Order is dependent on bitboard layout and is NOT sorted.
     *
     * @warning
     * - Intended for debugging or visualization only
     * - Does not encode full move information
     *
     * @complexity O(k), where k = number of set bits
     */
    std::vector<std::string> getMovesList(bitboard::bitmap movesBitboard);

    /**
     * @brief Generates pseudo-legal moves for white pawns.
     *
     * Produces a bitboard of all destination squares reachable by white pawns,
     * including:
     * - Single pushes
     * - Double pushes from starting rank
     * - Diagonal captures
     *
     * @param b Current GameBoard state
     * @return Bitboard of destination squares
     *
     * @note
     * - Excludes:
     *     - Promotions
     *     - En passant
     *     - Legality checks (pins, checks)
     *
     * - Double push requires:
     *     - Both intermediate and destination squares empty
     *
     * - File masks prevent wraparound during diagonal shifts
     *
     * @warning
     * - Relies on correct alignment of rank/file masks with bitboard layout
     */
    bitboard::bitmap calculateWhitePawnMoves(const chessboard::GameBoard& b);

    /**
     * @brief Generates pseudo-legal moves for black pawns.
     *
     * Produces a bitboard of all destination squares reachable by black pawns,
     * including:
     * - Single pushes
     * - Double pushes from starting rank
     * - Diagonal captures
     * - En passant targets
     *
     * @param b Current GameBoard state
     * @return Bitboard of destination squares
     *
     * @note
     * - Movement uses right shifts (>>)
     * - File masking prevents edge wraparound
     * - En passant square is treated as a capturable target
     *
     * @warning
     * - Generates pseudo-legal moves only:
     *     - No king safety checks
     *     - No pin handling
     *
     * - Does NOT handle:
     *     - Promotions
     *     - En passant capture execution (only includes target square)
     *
     * @see calculateWhitePawnMoves
     */
    bitboard::bitmap calculateBlackPawnMoves(const chessboard::GameBoard& b);

    /**
     * @brief Computes sliding piece attacks along a single ray using the Hyperbola Quintessence algorithm.
     *
     * Generates all attacked squares for a sliding piece on a given ray (rank, file,
     * diagonal, or anti-diagonal) in both directions simultaneously, correctly
     * stopping at blockers and capturing enemy pieces.
     *
     * Algorithm:
     * Given occupancy o, slider s, and ray mask m:
     *   attacks = ((o&m) - 2s) ^ reverse(reverse(o&m) - 2*reverse(s))) & m
     *
     * - The forward subtraction propagates attacks toward higher-index squares
     * - The reversed subtraction propagates attacks toward lower-index squares
     * - XOR combines both directions
     * - Masking with m restricts results to the ray
     * - Masking with ~friendOccupied removes friendly piece destinations
     *
     * @param sq              Square the sliding piece occupies
     * @param occupied        Bitboard of all occupied squares
     * @param friendOccupied  Bitboard of squares occupied by friendly pieces
     * @param visionMask      Ray mask (file, rank, diagonal, or anti-diagonal)
     * @return Bitboard of all reachable squares along the ray
     *
     * @note
     * - Captures enemy pieces (included in result)
     * - Blocked by friendly pieces (excluded from result)
     * - Call once per ray direction; combine results for full piece mobility
     *
     * @complexity O(1) — fully branchless bitboard arithmetic
     * @see reverseBitmap, calculateRookMoves
     */
    bitboard::bitmap hypbQuint(Square sq, bitboard::bitmap occupied, bitboard::bitmap friendOccupied, bitboard::bitmap visionMask);

    /**
     * @brief Generates all pseudo-legal rook moves from a given square.
     *
     * Computes rook mobility by applying the Hyperbola Quintessence algorithm
     * independently along the rook's two movement axes (file and rank),
     * then combining the results.
     *
     * @param b   Current board state
     * @param sq  Square the rook occupies
     * @param col Color of the rook (used to exclude friendly captures)
     * @return Bitboard of all reachable squares
     *
     * @note
     * - Generates pseudo-legal moves only (does not check for king safety or pins)
     * - Friendly pieces block and are excluded; enemy pieces are included as captures
     *
     * @see hypbQuint
     */
    bitboard::bitmap calculateRookMoves(const chessboard::GameBoard& b, Square sq, Color col);
}