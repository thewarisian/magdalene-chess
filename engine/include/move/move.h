/**
 * @file move.h
 * @brief Chess move encoding, extraction, and state mutation interface.
 * @details This header defines a highly optimized 16-bit packed representation of a chess move,
 * alongside utilities to serialize and deserialize its attributes. Encoding bits efficiently
 * allows millions of nodes to be processed rapidly during minimax alpha-beta search.
 * It also encapsulates the state transition methods required to navigate forward and backward
 * across the minimax evaluation tree.
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
 * @brief Type alias for a packed 16-bit chess move structure.
 * @details Represented as a primitive scalar integer to ensure it passes entirely within a single
 * CPU register during fast evaluation tree sorting routines.
 */
using move = uint16_t;

/**
 * @brief Type alias for an 64-bit unsigned bitboard configuration mask.
 */
using bb = bitboard::bitmap;

/**
 * @brief Type alias for a packed 4-bit castling state configuration byte.
 * @details Layout maps sequentially: Bit 0 = WK, Bit 1 = WQ, Bit 2 = BK, Bit 3 = BQ.
 */
using castling_flags = uint8_t;

/**
 * @brief Statically cached precalculated move maps and toggles for all four castling permutations.
 * @details These constants reflect precalculated layouts mapping out King origins, target squares, 
 * rook masks, and specialized bitwise toggles. Bypassing dynamic generation during deep runtime 
 * alpha-beta lookups maximizes node throughput.
 */
namespace castling {
    constexpr move WHITE_KING_SIDE = 0x5043;    ///< Precalculated packed bitmask for White O-O
    constexpr move WHITE_QUEEN_SIDE = 0x6143;   ///< Precalculated packed bitmask for White O-O-O
    constexpr move BLACK_KING_SIDE = 0x5E7B;    ///< Precalculated packed bitmask for Black O-O
    constexpr move BLACK_QUEEN_SIDE = 0x6F7B;   ///< Precalculated packed bitmask for Black O-O-O

    // --- White Rooks ---
    constexpr bb WHITE_KS_ROOK = 0x1ULL;                ///< White Kingside Rook home square (H1, Bit 0)
    constexpr bb WHITE_QS_ROOK = 0x80ULL;               ///< White Queenside Rook home square (A1, Bit 7)

    // --- Black Rooks ---
    constexpr bb BLACK_KS_ROOK = 0x0100000000000000ULL; ///< Black Kingside Rook home square (H8, Bit 56)
    constexpr bb BLACK_QS_ROOK = 0x8000000000000000ULL; ///< Black Queenside Rook home square (A8, Bit 63)

    // --- White Rook Toggles ---
    constexpr bb WHITE_KS_ROOK_TOGGLE = 0x5ULL;         ///< Combined XOR toggle mask for White O-O (H1 <-> F1)
    constexpr bb WHITE_QS_ROOK_TOGGLE = 0x90ULL;        ///< Combined XOR toggle mask for White O-O-O (A1 <-> D1)

    // --- Black Rook Toggles ---
    constexpr bb BLACK_KS_ROOK_TOGGLE = 0x50000000000000ULL;   ///< Combined XOR toggle mask for Black O-O (H8 <-> F8)
    constexpr bb BLACK_QS_ROOK_TOGGLE = 0x9000000000000000ULL; ///< Combined XOR toggle mask for Black O-O-O (A8 <-> D8)

    /**
     * @brief Verifies whether specific castling conditions remain valid for a given player profile.
     * @param col The active player color.
     * @param cr The current runtime packed castling rights register.
     * @param queenSide Selects Queenside evaluation if true, defaults to false (Kingside).
     * @return true if the requested target option remains intact, false if permanently lost.
     */
    inline bool canCastle(Color col, castling_flags cr, bool queenSide = false) { 
        int shiftVal = (static_cast<int>(col) << 1) + (queenSide ? 1 : 0);
        return (cr >> shiftVal) & 1; 
    }

    /**
     * @brief Modifies a castling rights byte in place to permanently revoke an option.
     * @param col The targeted player profile color.
     * @param[out] cr Reference to the active live castling byte being stripped.
     * @param queenSide Selects Queenside invalidation if true, defaults to false (Kingside).
     */
    inline void disableCastling(Color col, castling_flags& cr, bool queenSide = false) { 
        int shiftVal = (static_cast<int>(col) << 1) + (queenSide ? 1 : 0);
        cr &= static_cast<castling_flags>(~(1 << shiftVal));
    }
}

/**
 * @brief Lightweight snapshot struct designed to store destructive state parameters.
 * @details Captures vital board properties that are structurally overwritten or cleared out
 * during forward move executions. Retaining this history permits single-turn rollbacks inside 
 * `undoMove` with zero data loss. Arranged linearly to fit within minimal cache line spacing.
 */
struct moveHistory {
    bb enPassantSquare;             ///< Captured active global En Passant target bitmask before turn changes.
    castling_flags castlingRights;  ///< Packed castling rights byte configuration prior to move execution.
    PieceType capturedType;         ///< Explicit category identifying the victim piece type if a capture occurred.
};

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
    inline Square getToSquare(move m) { return static_cast<Square>((m >> 6) & 0x3F); }

    /**
     * @brief Extracts the structural move type metadata from a packed move.
     * @param m The 16-bit packed move.
     * @return MoveType The isolated 4-bit action flag identifier (uses mask 0xF / 0b1111).
     */
    inline MoveType getMoveType(move m) { return static_cast<MoveType>((m >> 12) & 0xF); }

    /**
     * @brief Executes a pseudo-legal move on the board state, modifying bitboards in place.
     *
     * This function handles the forward mutation of all core bitboard layouts for both players 
     * based on the provided move data. It applies an implicit piece-identification strategy to 
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
     * search tree, ensure that historic state properties are cached in the `moveHistory` tracking 
     * capsule *before* invocation to facilitate safe rollback execution via `undoMove`.
     *
     * @param[in]     col               The Color of the player making the active move.
     * @param[in]     m                 The packed 16-bit move integer containing the from-square, 
     * to-square, and move type flags.
     * @param[in,out] whitePawns        Reference to the White pawns bitboard layout.
     * @param[in,out] whiteKnights      Reference to the White knights bitboard layout.
     * @param[in,out] whiteBishops      Reference to the White bishops bitboard layout.
     * @param[in,out] whiteRooks        Reference to the White rooks bitboard layout.
     * @param[in,out] whiteQueens       Reference to the White queens bitboard layout.
     * @param[in,out] whiteKing         Reference to the White king bitboard layout.
     * @param[in,out] blackPawns        Reference to the Black pawns bitboard layout.
     * @param[in,out] blackKnights      Reference to the Black knights bitboard layout.
     * @param[in,out] blackBishops      Reference to the Black bishops bitboard layout.
     * @param[in,out] blackRooks        Reference to the Black rooks bitboard layout.
     * @param[in,out] blackQueens       Reference to the Black queens bitboard layout.
     * @param[in,out] blackKing         Reference to the Black king bitboard layout.
     * @param[in,out] enPassantSquare   Reference to the single active global En Passant target mask. 
     * Will be updated if a double pawn push occurs, or cleared to 
     * 0ULL for all other non-double-push actions.
     * @param[in,out] cr                Reference to the packed castling flags bitmask byte. Permanently 
     * revokes associated rights if a King moves, a Rook moves, or an 
     * undisturbed starting Rook is captured.
     */
    void makeMove(Color col, move m, 
        bb& whitePawns, bb& whiteKnights, bb& whiteBishops, bb& whiteRooks, bb& whiteQueens, bb& whiteKing,
        bb& blackPawns, bb& blackKnights, bb& blackBishops, bb& blackRooks, bb& blackQueens, bb& blackKing,
        bb& enPassantSquare, castling_flags& cr);

    /**
     * @brief Rolls back a previously executed move, perfectly restoring the preceding board state.
     *
     * This function acts as the exact mathematical inverse to `makeMove`. It steps backward in time 
     * across the search tree, utilizing the stored history capsule to reconstruct structural changes 
     * that cannot be natively calculated via simple bitwise XOR logic.
     * * It handles state restoration operations in a strict order of operations:
     * - Downgrades promoted pieces back into pawns before sliding them backward.
     * - Teleports the active player's moving piece backward from the target square to its origin.
     * - Resurrects standard captured pieces back onto their exact death squares using historical type data.
     * - Re-asserts en passant captured pawns to their relative ranks behind the landing target.
     * - Rewinds companion Rook shifts for both Kingside and Queenside castling types.
     * - Instantly restores historical En Passant masks and Castling Rights configurations.
     *
     * @param[in]     col               The Color of the player who originally made the move.
     * @param[in]     m                 The packed 16-bit move integer that needs to be reversed.
     * @param[in]     hist              Read-only reference to the snapshot data structure containing 
     * the fragile board attributes overwritten during `makeMove`.
     * @param[in,out] whitePawns        Reference to the White pawns bitboard layout.
     * @param[in,out] whiteKnights      Reference to the White knights bitboard layout.
     * @param[in,out] whiteBishops      Reference to the White bishops bitboard layout.
     * @param[in,out] whiteRooks        Reference to the White rooks bitboard layout.
     * @param[in,out] whiteQueens       Reference to the White queens bitboard layout.
     * @param[in,out] whiteKing         Reference to the White king bitboard layout.
     * @param[in,out] blackPawns        Reference to the Black pawns bitboard layout.
     * @param[in,out] blackKnights      Reference to the Black knights bitboard layout.
     * @param[in,out] blackBishops      Reference to the Black bishops bitboard layout.
     * @param[in,out] blackRooks        Reference to the Black rooks bitboard layout.
     * @param[in,out] blackQueens       Reference to the Black queens bitboard layout.
     * @param[in,out] blackKing         Reference to the Black king bitboard layout.
     * @param[in,out] enPassantSquare   Reference to the active global En Passant target mask, reverted 
     * to its preceding historical value.
     * @param[in,out] cr                Reference to the packed castling flags bitmask byte, reverted 
     * to its preceding historical value.
     */
    void undoMove(Color col, move m, moveHistory& hist,
        bb& whitePawns, bb& whiteKnights, bb& whiteBishops, bb& whiteRooks, bb& whiteQueens, bb& whiteKing,
        bb& blackPawns, bb& blackKnights, bb& blackBishops, bb& blackRooks, bb& blackQueens, bb& blackKing,
        bb& enPassantSquare, castling_flags& cr);
}