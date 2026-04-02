#pragma once

/**
 * @namespace chessmeta
 * @brief Contains high-level metadata describing the chessboard and game configuration.
 *
 * This namespace defines logical properties of the chessboard that are independent
 * of internal representations such as bitboards. It provides:
 * - The standard initial position in Forsyth–Edwards Notation (FEN)
 * - Board dimensions and derived constants
 *
 * Responsibilities:
 * - Acts as a central source of truth for board size and layout
 * - Provides default game state initialization via FEN
 *
 * @note
 * - FEN encodes the board rank-by-rank from rank 8 (top) to rank 1 (bottom).
 * - These values are immutable and intended to be shared across the engine.
 */
namespace chessmeta {
    // Initial position in FEN notation (full board + basic state)
    // FEN encodes the board row by row from rank 8 (top) to rank 1 (bottom)
    inline constexpr const char* INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Board dimensions
    inline constexpr int NUM_ROWS = 8;
    inline constexpr int NUM_COLS = 8;
    inline constexpr int NUM_TILES = NUM_ROWS * NUM_COLS;
}