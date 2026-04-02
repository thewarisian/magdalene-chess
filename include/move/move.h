#pragma once

/**
 * @namespace chessmove
 * @brief Defines data structures used to represent and manipulate chess moves.
 *
 * This namespace isolates move-related logic from board representation and
 * bitboard utilities, improving modularity and maintainability.
 *
 * Responsibilities:
 * - Provide compact and efficient representations of moves
 * - Serve as the interface between move generation and higher-level logic
 *
 * Future Extensions:
 * - Move flags (capture, promotion, castling, en passant)
 * - Piece type information
 * - Encoded move formats (e.g., bit-packed integers for performance)
 */
namespace chessmove {
    /**
     * @struct Move
     * @brief Represents a chess move using bitboard square indices and piece metadata.
     *
     * A move consists of:
     * - `fromBitIdx`: Source square index
     * - `toBitIdx`: Destination square index
     * - `attackPieceType`: Piece being moved
     * - `capturedPieceType`: Piece being captured (or 'E' if none)
     *
     * Indexing:
     * - Range: 0–63
     * - Mapping: a1 = 0, h8 = 63
     *
     * Piece Encoding:
     * - White: 'P','N','B','R','Q','K'
     * - Black: 'p','n','b','r','q','k'
     * - Empty: 'E'
     *
     * Design Notes:
     * - Stores enough information to apply moves without querying the board
     * - Simplifies make/unmake move logic
     * - Slightly larger than minimal representations but reduces recomputation
     *
     * @note
     * This structure can be extended to include:
     * - Promotion piece (if any)
     * - Special move flags (castling, en passant, double pawn push)
     * - Move flags (bitmask for fast checks)
     * - Move ordering score (for search optimizations)
     *
     * @warning
     * - Assumes consistency with board state (no validation performed)
     * - Incorrect piece types may corrupt board state during move execution
     */
    struct Move {
        int fromBitIdx;
        int toBitIdx;

        char attackPieceType;
        char capturedPieceType;
    };

    /**
     * @brief Determines whether a move is a double pawn push.
     *
     * A double pawn push occurs when a pawn moves two squares forward
     * from its initial rank:
     * - White pawn: rank 2 → rank 4 (index +16)
     * - Black pawn: rank 7 → rank 5 (index -16)
     *
     * @param m Move to evaluate
     * @return True if the move is a double pawn push, false otherwise
     *
     * @note
     * - Uses bit index arithmetic assuming:
     *     - rank = bitIdx / 8
     * - Does not validate move legality beyond structure
     * - Assumes correct piece type encoding in Move
     *
     * @warning
     * - Incorrect indexing scheme will break this logic
     * - Does not verify that path is unobstructed
     */
    bool isDoublePawnPush(Move m);
}