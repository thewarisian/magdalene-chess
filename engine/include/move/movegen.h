/**
 * @file movegen.h
 * @brief Chess Move Generation Engine Module.
 * @details This header defines the namespaces and functions responsible for generating 
 * pseudo-legal and legal chess moves using bitboards (`bb`). It encompasses pawn mechanics, 
 * slider piece vision via Hyperbola Quintessence (`hypbQuint`), attack ray definitions, 
 * castling logic validation, and comprehensive move state aggregation.
 * * Architecture Blueprint:
 * - Bitboards (`bb`) are represented as unsigned 64-bit integers (`unsigned long long`).
 * - Move structures (`move`) are tightly packed representations handled via the `chessmove` interface.
 * - Bit manipulation relies on built-in GCC intrinsics (`__builtin_ctzll`) for high-performance processing.
 * * @version 1.0
 * @date 2026
 * @copyright All rights reserved.
 */

#pragma once

#include <vector>

#include "core/utils.h"
#include "core/types.h"
#include "move/move.h"
#include "bitboard/bitboard.h"

using bb = bitboard::bitmap;

namespace movegen {

    /**
     * @brief Iterates over a bitboard of target tiles and serializes them into valid single moves.
     * @details Extracts destination coordinates bit by bit using low-level bit scans, maps them 
     * back to origin coordinates via a known positional translation offset (`shiftAmt`), and 
     * appends packed moves to the destination collection vector.
     * * @param moves A bitboard containing set bits representing valid destination squares.
     * @param shiftAmt The positional index offset needed to calculate the original 'from' square.
     * @param mt The semantic class description of the move (e.g., Quiet, Capture, EnPassant).
     * @param possibleMoves Output vector where the generated packed move configurations are appended.
     */
    void addSingleMoves(bb moves, int shiftAmt, MoveType mt, std::vector<move>& possibleMoves);

    /**
     * @brief Expands a target destination bitboard into the four standard pawn promotion variations.
     * @details Converts pawn steps reaching the home-row limits into separate specific evaluations 
     * for Knight, Bishop, Rook, and Queen options.
     * * @param moves A bitboard containing set bits on promotion rows.
     * @param shiftAmt The directional offset to compute the origin squares.
     * @param possibleMoves Output vector to append the distinct packed promotion variations.
     */
    void addPromotionMoves(bb moves, int shiftAmt, std::vector<move>& possibleMoves);

    /**
     * @brief High-efficiency vectorization routine for generating complete pawn behavior.
     * @details Independently parses left-hand attacks, right-hand attacks, single-tile steps, 
     * and double-tile pushes. Explicitly maps logic handling for variable color orientation, 
     * active En Passant status markers, and board boundaries.
     * * @param col The current color context moving pieces (WHITE/BLACK).
     * @param pawns Bitboard representing active friendly pawns.
     * @param empty Bitboard mask denoting unoccupied squares.
     * @param enemyPieces Bitboard mask denoting active hostile pieces.
     * @param enPassantSquare Mask highlighting an active valid En Passant square target.
     * @param possibleMoves Output array containing accumulating move outputs.
     */
    void addPossiblePawnMoves(Color col, bb pawns, bb empty, bb enemyPieces, bb enPassantSquare, std::vector<move>& possibleMoves);

    /**
     * @brief Hyperbola Quintessence slider attack calculation algorithm.
     * @details Employs a mathematically optimized subtraction trick along specialized ray scopes 
     * to resolve sliding blockades efficiently without looping. Calculates positive and negative 
     * ray segments instantly using bitboard bit reversals.
     * * @param slider A isolated bitboard representing the single origin square of the slider piece.
     * @param occupied Global bitboard mask containing all active board blockages.
     * @param visionMask Precalculated ray mask highlighting a specific direction/diagonal intersecting the slider.
     * @return bb A bitboard mask detailing every legal coordinate scope visible to the slider.
     */
    bb hypbQuint(bb slider, bb occupied, bb visionMask);

    /**
     * @brief Generates the structural lookup coverage mask for a knight.
     * @param knight Single bit bitboard of the knight location.
     * @return bb Resulting absolute movement range map.
     */
    bb calculateKnightAttacks(bb knight);

    /**
     * @brief Combines diagonal and anti-diagonal attack masks for a bishop.
     * @param bishop Isolated bishop position bitboard.
     * @param occupied Global structural blocker bitboard map.
     * @return bb Combined intersection attack mask map.
     */
    bb calculateBishopAttacks(bb bishop, bb occupied);

    /**
     * @brief Combines file and rank orthogonal attack lines for a rook.
     * @param rook Isolated rook position bitboard.
     * @param occupied Global structural blocker bitboard map.
     * @return bb Combined intersection attack mask map.
     */
    bb calculateRookAttacks(bb rook, bb occupied);

    /**
     * @brief Superpositions bishop and rook logic layouts to resolve Queen visibility.
     * @param queen Isolated queen position bitboard.
     * @param occupied Global structural blocker bitboard map.
     * @return bb Combined omnidirectional movement scope.
     */
    bb calculateQueenAttacks(bb queen, bb occupied);

    /**
     * @brief Generates the localized radial boundary map around a king.
     * @param king Single bit bitboard of the king location.
     * @return bb Resulting immediate movement range.
     */
    bb calculateKingAttacks(bb king);

    /**
     * @brief Dynamic switch dispatcher to route custom piece calculations based on type.
     * @param piece Isolated bitboard tracking targeted item coordinates.
     * @param occupied Global occupancy state profile.
     * @param pt Type classification structure identifier.
     * @return bb Dynamic raw result mask mapping.
     */
    bb calculateAttacksOfType(bb piece, bb occupied, PieceType pt);

    /**
     * @brief Aggregates the combined global threat/influence landscape for an entire piece subset.
     * @details Loops through multiple pieces of the same type via least-significant bit serialization 
     * to form a comprehensive lookup map. Used widely for checking king safety and pins.
     * * @param pieces Bitboard containing all pieces of a specific type class to parse.
     * @param occupied Global layout array of solid blockers.
     * @param sameColPieces Friendly alignment reference mask.
     * @param enemy Hostile alignment reference mask.
     * @param pt PieceType classifier to configure processing rules.
     * @return bb Unified complete capture threat layer map.
     */
    bb calculateMajorMinorAttackScope(bb pieces, bb occupied, bb sameColPieces, bb enemy, PieceType pt);

    /**
     * @brief Populates moves for non-pawn variants by evaluating attack scopes against alliance rules.
     * @details Loops individual items, checks generated visibility patterns against friendly blockades, 
     * handles semantic tagging differentiating 'Quiet' profiles vs 'Capture' profiles, and appends to data vector.
     * * @param pieces Base collection group layout to process.
     * @param occupied Global layout tracking solid obstacles.
     * @param sameColPieces Friendly target alignment exclusions map.
     * @param enemy Opposing side validation configuration.
     * @param pt Dynamic parsing method mapping tracker.
     * @param possibleMoves Accumulating global output container vector.
     */
    void addPossibleNonPawnMovesOfType(bb pieces, bb occupied, bb sameColPieces, bb enemy, PieceType pt, std::vector<move>& possibleMoves);

    /**
     * @brief Consolidates the absolute threat coverage area map that a color choice actively commands.
     * @details Essential function used by the engine to accurately determine if specific squares are 
     * contested, if a check state exists, or if castling movements are obstructed.
     * * @return bb A complete coverage matrix tracking all tiles under active threat by the selected color context.
     */
    bb captureScope(Color col, bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, bb occupied, bb sameColPieces, bb enemyPieces);

    /**
     * @brief Evaluates eligibility flags and calculates dynamic pathways to safely handle structural Castling.
     * @details Validates safety dependencies: prevents execution out of check, checks paths for spatial blockades, 
     * and verifies that the king does not step across squares controlled by active enemy attack rays.
     * * @param col Current player side variant context indicator.
     * @param king King structural position context tracking map.
     * @param occupied Global blocker configuration layout tracking map.
     * @param enemyAttackScope Current comprehensive threat field generated by opposing pieces.
     * @param castleKing Flag permitting King-Side castle execution tracking.
     * @param castleQueen Flag permitting Queen-Side castle execution tracking.
     * @param possibleMoves Target structural vector container tracking accumulated legal moves.
     */
    void addPossibleCastling(Color col, bb king, bb occupied, bb enemyAttackScope, bool castleKing, bool castleQueen, std::vector<move>& possibleMoves);

    /**
     * @brief Unified central entry interface layer used to extract complete move portfolios.
     * @details Orchestrates calls across all internal specific subroutines to combine pawns, sliders, 
     * leaping assets, and unique positional states like castling into a single collection vector.
     * * @return std::vector<move> A comprehensive collection array containing all validated pseudo-legal candidate moves.
     */
    std::vector<move> getAllMoves(Color col, 
        bb pawns, bb knights, bb bishops, bb rooks, bb queens, bb king, 
        bb empty, bb occupied, bb sameColPieces, bb enemyPieces, bb enemyAttackScope, bb enPassantSquare,
        bool castleKing, bool castleQueen);
}