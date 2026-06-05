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
        //Remove LSB set bit from bitboard (Brian Kernighan's Algorithm)
        b &= (b-1);
        
        return sq;
    }

    /**
     * @brief Reverses all 64 bits of a bitmap.
     *
     * Performs a full bit-reversal such that bit 0 becomes bit 63,
     * bit 1 becomes bit 62, and so on. This is used to mirror a
     * bitboard along both axes simultaneously.
     *
     * Algorithm (4-stage parallel reversal):
     * 1. Byte swap      : Reverses byte order using a single CPU instruction
     * 2. Nibble swap    : Swaps adjacent 4-bit groups within each byte
     * 3. Bit-pair swap  : Swaps adjacent 2-bit groups within each nibble
     * 4. Bit swap       : Swaps adjacent individual bits
     *
     * Each stage doubles the granularity of reversal, analogous to a
     * bitwise merge sort.
     *
     * @param b Bitmap to reverse
     * @return Bitmap with all bits in reversed order
     *
     * @note Branchless and O(1) — compiles to ~7 instructions on modern hardware.
     * @see hypbQuint — primary consumer of this function
     */
    inline bitmap reverseBitmap(bitmap b) {
        //Reverse order of bytes
        b = __builtin_bswap64(b);
        //Swap each nibble with adjacent one
        b = ((b&0xF0F0F0F0F0F0F0F0ULL) >> 4) | ((b&0x0F0F0F0F0F0F0F0FULL) << 4);
        //Swap each bit pair with adjacent one
        b = ((b&0xCCCCCCCCCCCCCCCCULL) >> 2) | ((b&0x3333333333333333ULL) << 2);
        //Swap each bit with adjacent one
        b = ((b&0xAAAAAAAAAAAAAAAAULL) >> 1) | ((b&0x5555555555555555ULL) << 1);
        return b;
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

    // All diagonals with MSB to the right of LSB
    // Each entry represents one diagonal starting from bottom left and going to top right
    inline constexpr std::array<bitmap, 15> DIAGONAL = {
        0x0000000000000001ULL, // H1 only
        0x0000000000000102ULL, // G1-H2
        0x0000000000010204ULL, // F1-H4
        0x0000000001020408ULL, // E1-H5
        0x0000000102040810ULL, // D1-H6
        0x0000010204081020ULL, // C1-H7
        0x0001020408102040ULL, // B1-H8
        0x0102040810204080ULL, // A1-H8 (main diagonal)
        0x0204081020408000ULL, // A2-G8
        0x0408102040800000ULL, // A3-F8
        0x0810204080000000ULL, // A4-E8
        0x1020408000000000ULL, // A5-D8
        0x2040800000000000ULL, // A6-C8
        0x4080000000000000ULL, // A7-B8
        0x8000000000000000ULL  // A8 only
    };

    // All diagonals with MSB to the left of LSB
    // Each entry represents one diagonal starting from bottom right and going to top left
    inline constexpr std::array<bitmap, 15> ANTI_DIAGONAL = {
        0x0000000000000080ULL, // H8 only
        0x0000000000008040ULL, // H7-A2
        0x0000000000804020ULL, // H6-A3
        0x0000000080402010ULL, // H5-A4
        0x0000008040201008ULL, // H4-A5
        0x0000804020100804ULL, // H3-A6
        0x0080402010080402ULL, // H2-A7
        0x8040201008040201ULL, // H1-A8 (main anti-diagonal)
        0x4020100804020100ULL, // A2-G8
        0x2010080402010000ULL, // A3-F8
        0x1008040201000000ULL, // A4-E8
        0x0804020100000000ULL, // A5-D8
        0x0402010000000000ULL, // A6-B7-C8
        0x0201000000000000ULL, // A7-B8
        0x0100000000000000ULL  // A8 only
    };


    /**
     * @brief Returns the file mask for the file containing the given square.
     *
     * Extracts the column (file) of the square from its bit index and returns
     * the corresponding precomputed file mask from the FILE table.
     *
     * @param sq Square whose file mask is requested
     * @return Bitmap with all 8 squares on the same file as sq set
     *
     * @note
     * - File index is derived as: NUM_COLS - 1 - (sq % NUM_COLS)
     * - This accounts for the H→A file ordering in the Square enum
     *   (H1=0, A1=7), mapping correctly to FILE[0]=A, FILE[7]=H
     */
    inline bitmap getFileMask(Square sq) {
        return FILE[chessmeta::NUM_COLS-1-(static_cast<int>(sq)%chessmeta::NUM_COLS)];
    }

    /**
     * @brief Returns the rank mask for the rank containing the given square.
     *
     * Extracts the row (rank) of the square from its bit index and returns
     * the corresponding precomputed rank mask from the RANK table.
     *
     * @param sq Square whose rank mask is requested
     * @return Bitmap with all 8 squares on the same rank as sq set
     *
     * @note Rank index is derived as: sq / NUM_COLS (integer division)
     */
    inline bitmap getRankMask(Square sq) {
        return RANK[static_cast<int>(sq)/chessmeta::NUM_COLS];
    }

    /**
     * @brief Returns the diagonal mask for the diagonal containing the given square.
     *
     * Diagonals run from bottom-left to top-right (H1 → A8 direction).
     * Every square on the same diagonal shares the same value of:
     *   rank + file_offset = (sq / 8) + (sq % 8)
     * which produces a unique index in [0, 14].
     *
     * @param sq Square whose diagonal mask is requested
     * @return Bitmap with all squares on the same diagonal as sq set
     *
     * @note
     * - Index 0 corresponds to the H1 corner (single square)
     * - Index 7 corresponds to the main diagonal (A1–H8)
     * - Index 14 corresponds to the A8 corner (single square)
     */
    inline bitmap getDiagonalMask(Square sq) {
        int n = static_cast<int>(sq);
        return DIAGONAL[n/chessmeta::NUM_COLS + n%chessmeta::NUM_COLS];
    }

    /**
     * @brief Returns the anti-diagonal mask for the anti-diagonal containing the given square.
     *
     * Anti-diagonals run from bottom-right to top-left (A1 → H8 direction).
     * Every square on the same anti-diagonal shares the same value of:
     *   rank + (7 - file_offset) = (sq / 8) + (NUM_COLS - 1 - sq % 8)
     * which produces a unique index in [0, 14].
     *
     * @param sq Square whose anti-diagonal mask is requested
     * @return Bitmap with all squares on the same anti-diagonal as sq set
     *
     * @note
     * - Index 0 corresponds to the H8 corner (single square)
     * - Index 7 corresponds to the main anti-diagonal (H1–A8)
     * - Index 14 corresponds to the A8 corner (single square)
     */
    inline bitmap getAntiDiagonalMask(Square sq) {
        int n = static_cast<int>(sq);
        return ANTI_DIAGONAL[n/chessmeta::NUM_COLS + (chessmeta::NUM_COLS-1-n%chessmeta::NUM_COLS)];
    }

    // Used later for fast move generation
    inline constexpr std::array<bitmap, 64> KNIGHT_ATTACKS = {};
    inline constexpr std::array<bitmap, 64> KING_ATTACKS = {};
}