/**
 * @file types.h
 * @brief Core type definitions and enums used throughout the chess engine.
 *
 * This file defines fundamental data types such as board squares, piece types,
 * colors, and move classifications. These enums form the backbone of the engine's
 * internal representation and are designed to be compact and efficient.
 *
 * All enums use an underlying type of uint8_t (byte) to minimize memory usage.
 */
#pragma once

#include <cstdint>
/**
 * @typedef byte
 * @brief Alias for an 8-bit unsigned integer.
 *
 * Used as the underlying type for enums to ensure compact storage.
 */
using byte = uint8_t;

/**
 * @enum Square
 * @brief Represents all 64 squares on the chessboard.
 *
 * Squares are enumerated in rank-major order from H1 to A8.
 * This layout is typically chosen to align with bitboard representations,
 * where each square corresponds to a bit index.
 *
 * Example:
 * - Square::H1 → least significant bit (bit 0)
 * - Square::A8 → most significant bit (bit 63)
 *
 * @note The ordering is reversed file-wise (H → A) for each rank.
 * @note Square::None represents an invalid or non-existent square.
 */
enum class Square : byte {
    H1, G1, F1, E1, D1, C1, B1, A1,
    H2, G2, F2, E2, D2, C2, B2, A2,
    H3, G3, F3, E3, D3, C3, B3, A3,
    H4, G4, F4, E4, D4, C4, B4, A4,
    H5, G5, F5, E5, D5, C5, B5, A5,
    H6, G6, F6, E6, D6, C6, B6, A6,
    H7, G7, F7, E7, D7, C7, B7, A7,
    H8, G8, F8, E8, D8, C8, B8, A8,
    None
};

// ================== Enum for chess pieces ===================

/**
 * @enum PieceType
 * @brief Represents the type of a chess piece.
 *
 * Includes all standard chess pieces:
 * - PAWN
 * - KNIGHT
 * - BISHOP
 * - ROOK
 * - QUEEN
 * - KING
 *
 * @note PieceType::None represents the absence of a piece.
 */
enum class PieceType : byte {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, None
};

/**
 * @enum Color
 * @brief Represents the color of a chess piece or player.
 *
 * Possible values:
 * - WHITE
 * - BLACK
 *
 * @note Color::None is used for invalid or uninitialized states.
 */
enum class Color : byte {
    WHITE, BLACK, None
};

/**
 * @enum MoveType
 * @brief Classifies different types of chess moves.
 *
 * Categories include:
 * - Quiet            : Non-capturing, non-special move
 * - Capture          : Standard capture move
 * - DoublePawnPush   : Pawn advances two squares from starting rank
 * - EnPassant        : Special pawn capture
 * - Castling         : King-side or queen-side castling
 * - Promotion        : Pawn promotion
 *
 * @note This enum helps encode move semantics efficiently for move generation
 *       and game state updates.
 */
enum class MoveType : byte {
    Quiet, Capture, DoublePawnPush, EnPassant, Castling, Promotion
};