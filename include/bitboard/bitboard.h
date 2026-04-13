/**
 * @file bitboard.h
 * @brief Core bitboard utilities and precomputed masks for fast board operations.
 *
 * This file defines low-level primitives for working with 64-bit bitboards,
 * including bit manipulation helpers, rank/file masks, and attack tables.
 *
 * Scope:
 * - Bit-level operations (set, clear, toggle, query)
 * - Precomputed board masks (files, ranks)
 * - Attack lookup tables (to be populated)
 *
 * @note
 * - This module is performance-critical and should remain lightweight.
 * - No game-state logic (turn, legality, etc.) should be introduced here.
 */
#pragma once

#include <cstdint>
#include <array>

#include "core/utils.h"
#include "core/metadata.h"
#include "core/types.h"

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
 * - Bit index 0 corresponds to square h1
 * - Bit index 63 corresponds to square a8
 * - Indices increase right-to-left within a rank (H → A),
 *   and bottom-to-top across ranks (1 → 8)
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
     * @brief Returns a bitboard with a single bit set at the given square.
     *
     * @param sq Square enum value (Square::H1 = 0, Square::A8 = 63)
     * @return Bitboard with only the specified square bit set
     *
     * @note
     * - Uses the engine's square indexing convention:
     *   - Squares are indexed right-to-left within a rank (H → A)
     *   - Ranks increase bottom-to-top (1 → 8)
     */
    inline bitmap singleBit(Square sq) { return 1ULL << static_cast<int>(sq);  }

    /**
     * @brief Checks if a square is occupied in the bitboard.
     *
     * @return True if the bit at the given square is set
     */
    inline bool occupiedAt(bitmap b, Square sq) {   return (b >> static_cast<int>(sq)) & 1ULL; }

    /**
     * @brief Sets the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param sq Square enum value
     *
     * @note Performs a bitwise OR with a single-bit mask.
     */
    inline void placeBitAt(bitmap& b, Square sq) {  b |= singleBit(sq);  }

    /**
     * @brief Clears (resets) the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param sq Square enum value
     *
     * @note Performs a bitwise AND with the inverted single-bit mask.
     */
    inline void removeBitAt(bitmap& b, Square sq) { b &= ~singleBit(sq); }

    /**
     * @brief Toggles the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param sq Square enum value
     *
     * @note Flips the bit using XOR. Applying twice restores original state.
     */
    inline void toggleBitAt(bitmap& b, Square sq) { b ^= singleBit(sq);  }

    /**
     * @brief Extracts and removes the least significant set bit (LSB).
     *
     * Finds the square corresponding to the lowest set bit and clears it
     * from the bitboard.
     *
     * @param b Bitboard (modified in-place)
     * @return Square of the extracted bit, or Square::None if empty
     *
     * @note
     * - Uses `__builtin_ctzll` for fast index lookup
     * - Safe for zero input (returns Square::None)
     *
     * @complexity O(1)
     */
    inline Square popLSBSquare(bitmap& b) {
        //Count number of trailing zeros. Hard set b to 64 as method is undefined for parameter 0
        Square sq = b==0? Square::None : utils::intToSquare(__builtin_ctzll(b));
        //Remove LSB bit from bitboard
        b &= (b-1);
        
        return sq;
    }

    /**
     * @brief Prints a visual representation of a bitboard.
     *
     * Displays an 8×8 grid from rank 8 (top) to rank 1 (bottom),
     * using custom symbols for set and unset bits.
     *
     * @note Intended for debugging and visualization only.
     */
    void display(bitmap b, char occupiedSymbol = '1', char emptySymbol='.');

    // File bitboards (columns)
    // Each entry represents one file (column) from FILE_A to FILE_H
    inline constexpr std::array<bitmap, 8> FILE = {
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
    inline constexpr std::array<bitmap, 8> RANK = {
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
    inline constexpr std::array<bitmap, 64> KNIGHT_ATTACKS = {};
    inline constexpr std::array<bitmap, 64> KING_ATTACKS = {};
}