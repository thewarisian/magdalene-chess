/**
 * @file metadata.h
 * @brief Defines global chess metadata and configuration constants.
 *
 * This file contains immutable, high-level constants that describe the
 * structure of the chessboard and default game state. These values are
 * independent of internal representations (e.g., bitboards) and are shared
 * across the entire engine.
 *
 * Contents include:
 * - Standard initial position in Forsyth–Edwards Notation (FEN)
 * - Board dimensions (rows, columns, total squares)
 * - Piece symbol mappings for both colors
 *
 * @note
 * - All values are constexpr and intended for compile-time use where possible.
 * - This file serves as a central source of truth for board configuration.
 */
#pragma once

#include <array>

/**
 * @namespace chessmeta
 * @brief Centralized metadata describing board structure and default game state.
 *
 * This namespace contains high-level, immutable constants that define the
 * logical properties of a chess game. These values are independent of any
 * internal representation (e.g., bitboards, arrays) and are shared across
 * the engine.
 *
 * It provides:
 * - The standard initial position in Forsyth–Edwards Notation (FEN)
 * - Board dimensions and derived constants
 * - Canonical piece symbol mappings
 *
 * Responsibilities:
 * - Acts as a single source of truth for board configuration
 * - Supports initialization of game state via FEN strings
 * - Provides consistent piece encoding for display and parsing
 *
 * @note
 * - FEN strings encode the board rank-by-rank from rank 8 (top)
 *   to rank 1 (bottom), left to right (file a → h).
 * - All values are constexpr and intended to be globally accessible.
 */
namespace chessmeta {
    /**
     * @brief Standard chess starting position in Forsyth–Edwards Notation (FEN).
     *
     * Format:
     * - Piece placement
     * - Active color
     * - Castling availability
     * - En passant target square
     * - Halfmove clock
     * - Fullmove number
     *
     * Example:
     * "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
     */
    inline constexpr const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    /**
     * @brief Number of rows (ranks) on the chessboard.
     */
    inline constexpr int NUM_ROWS = 8;

    /**
     * @brief Number of columns (files) on the chessboard.
     */
    inline constexpr int NUM_COLS = 8;

    /**
     * @brief Total number of squares on the chessboard.
     */
    inline constexpr int NUM_TILES = NUM_ROWS * NUM_COLS;

    /**
     * @brief Mapping from (Color, PieceType) → character representation.
     *
     * Layout:
     * - First index: Color (0 = WHITE, 1 = BLACK)
     * - Second index: PieceType (PAWN → KING)
     *
     * Example:
     * - PIECE_SYMBOL[WHITE][PAWN] → 'P'
     * - PIECE_SYMBOL[BLACK][KNIGHT] → 'n'
     *
     * @note
     * - Assumes PieceType values are ordered as:
     *   PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
     * - Does not include PieceType::None
     */
    inline constexpr std::array<std::array<char, 6>, 2> PIECE_SYMBOL{
        std::array<char, 6>{'P','N','B','R','Q','K'},
        std::array<char, 6>{'p','n','b','r','q','k'}
    };
}