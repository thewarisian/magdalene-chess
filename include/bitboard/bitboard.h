#pragma once

#include <cstdint>
#include <array>

#include "meta/metadata.h"

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
    inline bitmap singleBit(int bitIdx) {   return 1ULL << bitIdx;  }

    /**
     * @brief Checks whether a given square is occupied in a bitboard.
     *
     * @param b Bitboard to query
     * @param i Square index (0 = a1, 63 = h8)
     * @return True if the bit at index i is set, false otherwise
     */
    inline bool occupiedAt(bitmap b, int i) {   return (b >> i) & 1ULL; }

    /**
     * @brief Sets the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     *
     * @note Performs a bitwise OR with a single-bit mask.
     */
    inline void placeBitAt(bitmap& b, int i) {  b |= singleBit(i);  }

    /**
     * @brief Clears (resets) the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     *
     * @note Performs a bitwise AND with the inverted single-bit mask.
     */
    inline void removeBitAt(bitmap& b, int i) { b &= ~singleBit(i); }

    /**
     * @brief Toggles the bit corresponding to a square in a bitboard.
     *
     * @param b Bitboard to modify
     * @param i Square index (0 = a1, 63 = h8)
     *
     * @note Flips the bit using XOR. Applying twice restores original state.
     */
    inline void toggleBitAt(bitmap& b, int i) { b ^= singleBit(i);  }

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
    inline int popLSBIndex(bitmap b) {
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