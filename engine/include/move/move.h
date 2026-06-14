/**
 * @file move.h
 * @brief Chess move encoding and extraction interface.
 * @details This header defines a highly optimized 16-bit packed representation of a chess move, 
 * alongside utilities to serialize and deserialize its attributes. Encoding bits efficiently 
 * allows millions of nodes to be processed rapidly during minimax alpha-beta search.
 * * --- Move Bit Layout (16-bit unsigned integer) ---
 * Bits 0-5   (6 bits) : From Square (0 to 63)
 * Bits 6-11  (6 bits) : To Square (0 to 63)
 * Bits 12-15 (4 bits) : Move Type Flags (Quiet, Capture, Promotion, Castling, etc.)
 * * Layout visualization:
 * [  MoveType  ][   ToSquare   ][  FromSquare  ]
 * 15 14 13 12   11 10 9 8 7 6   5 4 3 2 1 0
 * * @version 1.0
 * @date 2026
 * @copyright All rights reserved.
 */

#pragma once

#include "core/types.h"
#include "bitboard/bitboard.h"

/**
 * @brief Type alias for a packed chess move.
 * @details Represented as a 16-bit unsigned integer to optimize memory footprint and cache locality.
 */
using move = uint16_t;
using bb = bitboard::bitmap;

namespace chessmove {
    /**
     * @brief Packs structural move attributes into a unified 16-bit integer.
     * @param from The starting coordinates of the moving piece.
     * @param to The target landing coordinates of the moving piece.
     * @param mt Categorical flag describing the semantic type of the move.
     * @return move The consolidated, bit-shifted 16-bit move packet.
     */
    inline move packMoveInfo(Square from, Square to, MoveType mt) {
        return static_cast<move>(from) | static_cast<move>(to) << 6 | static_cast<move>(mt) << 12;
    }

    /**
     * @brief Extracts the origin square from a packed move.
     * @param m The 16-bit packed move.
     * @return Square The isolated starting square (uses mask 0x3F / 0b111111).
     */
    inline Square getFromSquare(move m) { return static_cast<Square>(m & 0x3F); }

    /**
     * @brief Extracts the destination square from a packed move.
     * @param m The 16-bit packed move.
     * @return Square The isolated destination square.
     */
    inline Square getToSquare(move m) { return static_cast<Square>((m>>6) & 0x3F); }

    /**
     * @brief Extracts the structural move type metadata from a packed move.
     * @param m The 16-bit packed move.
     * @return MoveType The isolated 4-bit action flag identifier (uses mask 0xF / 0b1111).
     */
    inline MoveType getMoveType(move m) { return static_cast<MoveType>((m>>12) & 0xF); }

    
    /**
     * @brief Executes a pseudo-legal move on the board state, modifying bitboards in place.
     *
     * This function handles the mutation of all core bitboard layouts for both players based 
     * on the provided move data. It applies an implicit piece-identification strategy to 
     * optimize piece retrieval without bloating the move structure, executing the move 
     * via fast bitwise XOR toggles.
     * * It comprehensively handles all standard and special chess moves, including:
     * - Basic quiet moves and standard captures (implicit enemy eviction).
     * - Double pawn pushes (generating the resulting En Passant target square).
     * - En Passant captures (clearing the relative pawn behind the destination).
     * - Multi-stage Pawn Promotions (stripping the baseline pawn and inserting the target piece).
     * - Castling variations (automatically moving the King and executing companion Rook swaps).
     *
     * @note This function mutates the state in place. To ensure consistency across an Alpha-Beta 
     * search tree, ensure that historic state properties (like previous en passant squares 
     * and castling rights) are cached in an external historical tracker before invocation 
     * to facilitate safe recovery during `undoMove`.
     *
     * @param[in]  col               The Color of the player making the active move.
     * @param[in]  m                 The packed 16-bit move integer containing the from-square, 
     * to-square, and move type flags.
     * @param[out] whitePawns        Reference to the White pawns bitboard layout.
     * @param[out] whiteKnights      Reference to the White knights bitboard layout.
     * @param[out] whiteBishops      Reference to the White bishops bitboard layout.
     * @param[out] whiteRooks        Reference to the White rooks bitboard layout.
     * @param[out] whiteQueens       Reference to the White queens bitboard layout.
     * @param[out] whiteKing         Reference to the White king bitboard layout.
     * @param[out] blackPawns        Reference to the Black pawns bitboard layout.
     * @param[out] blackKnights      Reference to the Black knights bitboard layout.
     * @param[out] blackBishops      Reference to the Black bishops bitboard layout.
     * @param[out] blackRooks        Reference to the Black rooks bitboard layout.
     * @param[out] blackQueens       Reference to the Black queens bitboard layout.
     * @param[out] blackKing         Reference to the Black king bitboard layout.
     * @param[out] enPassantSquare   Reference to the single active global En Passant target mask. 
     * Will be updated if a double pawn push occurs, or cleared to 
     * 0ULL for all other non-double-push actions.
     */
    void makeMove(Color col, move m, 
        bb& whitePawns, bb& whiteKnights, bb& whiteBishops, bb& whiteRooks, bb& whiteQueens, bb& whiteKing,
        bb& blackPawns, bb& blackKnights, bb& blackBishops, bb& blackRooks, bb& blackQueens, bb& blackKing,
        bb& enPassantSquare);
}

/**
 * @brief Statically cached precalculated move maps for all four castling permutations.
 * @details These hexadecimal literals reflect perfectly pre-packed configurations mapping out 
 * exact King origins, target squares, and Castling MoveType flags. Avoiding dynamic generation 
 * during runtime tree evaluations minimizes overhead.
 */
namespace castling_cache {
    constexpr move WHITE_KING_SIDE = 0x5043;   ///< Precalculated packed bitmask for White O-O
    constexpr move WHITE_QUEEN_SIDE = 0x6143;  ///< Precalculated packed bitmask for White O-O-O
    constexpr move BLACK_KING_SIDE = 0x5E7B;   ///< Precalculated packed bitmask for Black O-O
    constexpr move BLACK_QUEEN_SIDE = 0x6F7B;  ///< Precalculated packed bitmask for Black O-O-O

    // --- White Rook Toggles ---
    constexpr bb WHITE_KS_ROOK_TOGGLE = 0x5ULL;               // H1 <-> F1
    constexpr bb WHITE_QS_ROOK_TOGGLE = 0x90ULL;              // A1 <-> D1

    // --- Black Rook Toggles ---
    constexpr bb BLACK_KS_ROOK_TOGGLE = 0x50000000000000ULL;  // H8 <-> F8
    constexpr bb BLACK_QS_ROOK_TOGGLE = 0x9000000000000000ULL; // A8 <-> D8
}