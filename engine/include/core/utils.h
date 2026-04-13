#pragma once

/**
 * @file utils.h
 * @brief Utility helper functions for converting between internal chess representations.
 *
 * This header provides small inline helper functions used across the engine
 * to convert between integers, characters, and strongly-typed enums such as
 * Square, Color, and PieceType.
 *
 * These functions are designed to be lightweight and inlined for performance.
 */
#include <cctype>

#include "types.h"
#include "metadata.h"

namespace utils {
    /**
     * @brief Converts an integer index to a Square enum.
     *
     * This function performs a direct cast from an integer to the Square type.
     * It assumes the integer is within valid bounds of the Square enumeration.
     *
     * @param i Integer index representing a square (typically 0–63).
     * @return Square Corresponding square enum value.
     *
     * @note No bounds checking is performed.
     */
    inline Square intToSquare(int i) {
        return static_cast<Square>(i);
    }

    /**
     * @brief Retrieves the character symbol for a given piece and color.
     *
     * Uses a lookup table (chessmeta::PIECE_SYMBOL) to return the appropriate
     * character representation (e.g., 'P', 'n', etc.).
     *
     * @param col Color of the piece (WHITE or BLACK).
     * @param piece Type of the piece (PAWN, KNIGHT, etc.).
     * @return char Character symbol representing the piece.
     *
     * @note Assumes valid Color and PieceType values.
     */
    inline char getPieceSymbol(Color col, PieceType piece) {
        //Chose white or black symbol depending on color
        return chessmeta::PIECE_SYMBOL[static_cast<int>(col)][static_cast<int>(piece)];
    }

    /**
     * @brief Determines the color of a piece from its character representation.
     *
     * Uppercase characters are interpreted as WHITE pieces,
     * lowercase characters as BLACK pieces.
     *
     * @param ch Character representing a piece.
     * @return Color Corresponding color (WHITE, BLACK, or None if invalid).
     */
    inline Color charToColor(char ch) {
        if (ch >= 'A' && ch <= 'Z') return Color::WHITE;
        if (ch >= 'a' && ch <= 'z') return Color::BLACK;
        return Color::None;
    }

    /**
     * @brief Converts a character to its corresponding PieceType.
     *
     * The function is case-insensitive and maps standard chess piece
     * characters to their respective PieceType enum values.
     *
     * Supported mappings:
     * - 'p' → PAWN
     * - 'n' → KNIGHT
     * - 'b' → BISHOP
     * - 'r' → ROOK
     * - 'q' → QUEEN
     * - 'k' → KING
     *
     * @param ch Character representing a piece.
     * @return PieceType Corresponding piece type, or PieceType::None if invalid.
     */
    inline PieceType charToPieceType(char ch) {
        switch (std::tolower(ch)) {
            case 'p': return PieceType::PAWN;
            case 'n': return PieceType::KNIGHT;
            case 'b': return PieceType::BISHOP;
            case 'r': return PieceType::ROOK;
            case 'q': return PieceType::QUEEN;
            case 'k': return PieceType::KING;
            default:  return PieceType::None;
        }
    }
}