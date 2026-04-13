/**
 * @file move.h
 * @brief Defines data structures used to represent chess moves.
 *
 * This file provides compact representations of moves used throughout
 * the engine. It acts as the interface between move generation, board
 * state updates, and higher-level logic.
 *
 * @note
 * - Move representations are designed to be lightweight and efficient.
 * - No validation is performed at this level.
 */
#pragma once

#include "core/types.h"

/**
 * @namespace chessmove
 * @brief Contains move-related data structures and utilities.
 *
 * This namespace isolates move representation from board logic,
 * improving modularity and maintainability.
 */
namespace chessmove {
    /**
     * @struct Move
     * @brief Represents a chess move using square indices and move classification.
     *
     * A move consists of:
     * - `fromSquare`: Source square
     * - `toSquare`  : Destination square
     * - `type`      : Type of move (quiet, capture, promotion, etc.)
     *
     * Square Indexing:
     * - Based on the Square enum (0–63)
     * - Must remain consistent with board representation
     *
     * Design Notes:
     * - Minimal representation focused on clarity and extensibility
     * - Additional information (e.g., captured piece, promotion piece)
     *   is inferred from board state when applying the move
     *
     * @note
     * This structure can be extended in the future to include:
     * - Promotion piece type
     * - Explicit captured piece type
     * - Bit-packed representation for performance
     * - Move ordering scores (for search algorithms)
     *
     * @warning
     * - Assumes the move is valid in the current board state
     * - No legality checks are performed at this level
     */
    struct Move {
        Square fromSquare;
        Square toSquare;
        MoveType moveType;
    };
}

// namespace chessmove {
//     bool isDoublePawnPush(const Move& m) {
//         //Restrict to checking pawns
//         if(m.attackPieceType == PieceType::PAWN) { return false; }

//         //casting squares for easy calculation
//         int from = squareToInteger(m.fromSquare);
//         int to = squareToInteger(m.toSquare);

//         //Calculate conditions based on move for white
//         bool pawnOnRank2 = from / 8 == 1;
//         bool movedToRank4 = (to - from) == 16;
//         //Calculate conditions based on move for black
//         bool pawnOnRank7 = from / 8 == 6;
//         bool movedToRank5 = (from - to) == 16;

//         //Calculate and return whether double pawn push
//         return (m.at && pawnOnRank2 && movedToRank4) || 
//         (type == 'p' && pawnOnRank7 && movedToRank5);
//     }
// }