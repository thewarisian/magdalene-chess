#include "board/chessboard.h"
#include "core/utils.h"

namespace chessboard {
    void GameBoard::updateBitboards() {
        allWhitePieces = allBlackPieces = 0ULL;
        //All white pieces on board
        for(auto& bb : pieceBitboard[static_cast<int>(Color::WHITE)]) {    allWhitePieces |= bb;   }
        for(auto& bb : pieceBitboard[static_cast<int>(Color::BLACK)]) {    allBlackPieces |= bb;   }

        //Squares with some piece
        occupiedSquares = allWhitePieces | allBlackPieces;
        //Squares with no piece
        emptySquares = ~occupiedSquares;
    }

    void GameBoard::updateBoardAfterMove(const chessmove::Move& m,
        Color attackPieceColor, PieceType attackPieceType, 
        Color capturedPieceColor, PieceType capturedPieceType) {
        //Update en passant
        if(attackPieceType == PieceType::PAWN && m.moveType == MoveType::DoublePawnPush) { 
            int idx = static_cast<int>(m.toSquare);
            int shiftDirection = (attackPieceColor == Color::WHITE)? -1 : 1;
            enPassantSq = utils::intToSquare(idx + shiftDirection * 8);
        }
        //Since en passant can only be made immediately after double push
        else { enPassantSq = Square::None; }

        //Reverse player;
        whiteToMove = !whiteToMove;
        //Change all bitboards
        updateBitboards();
        //TODO add other updates
    }

    // ===================== INTERNAL METHODS (board initialisation / manipulation) =====================

    bitboard::bitmap& GameBoard::getBitboardOf(Color col, PieceType piece) {
        return pieceBitboard[static_cast<int>(col)][static_cast<int>(piece)];
    }

    void GameBoard::placePiece(Color col, PieceType piece, Square sq) {
        bitboard::bitmap& b = pieceBitboard[static_cast<int>(col)][static_cast<int>(piece)];
        bitboard::placeBitAt(b, sq);
    }

    void GameBoard::removePiece(Color col, PieceType piece, Square sq) {
        bitboard::bitmap& b = pieceBitboard[static_cast<int>(col)][static_cast<int>(piece)];
        bitboard::removeBitAt(b, sq);
    }

    void GameBoard::setBoard(const std::string& reducedFen) {
        int pos = 0;
        for(int j = reducedFen.size() - 1; j >= 0; j--) {
            const char& ch = reducedFen[j];
            Square sq = utils::intToSquare(pos);
            Color pieceColor = utils::charToColor(ch);
            PieceType pieceType = utils::charToPieceType(ch);
            
            placePiece(pieceColor, pieceType, sq);

            // Digit represents number of consecutive empty squares
            if(ch >= '1' && ch <= '8') {    pos += (ch-'0');   }
            // Ignored due to linear bottom-up indexing
            else if(ch == '/') {    continue;   }
            else {  pos++;  }
        }
    }

    void GameBoard::applyFenString(const std::string& fen) {
        std::istringstream iss(fen);
        std::string reducedFen, toMove, castling, enpass;

        if(!(iss >> reducedFen >> toMove >> castling >> enpass >> halfMove >> fullMove)) {
            throw std::invalid_argument("Malformed FEN");
        }

        // Set piece positions
        setBoard(reducedFen);

        // Player to move
        whiteToMove = toMove == "w";

        // Castling rights
        whiteCanCastleKingSide = whiteCanCastleQueenSide = false;
        blackCanCastleKingSide = blackCanCastleQueenSide = false;
        for(char ch : castling) {
            switch(ch) {
                case 'K': whiteCanCastleKingSide = true; break;
                case 'Q': whiteCanCastleQueenSide = true; break;
                case 'k': blackCanCastleKingSide = true; break;
                case 'q': blackCanCastleQueenSide = true; break;
            }
        }

        // En passant target
        enPassantSq = enpass == "-" ? Square::None : tileStringToSquare(enpass);
    }

    void GameBoard::applyBoardMatrix(const std::array<std::array<char, 8>, 8>& boardMatrix) {
        //Places pieces from matrix onto board
        for(int i = chessmeta::NUM_ROWS-1; i >= 0; i--) {
            for(int j = chessmeta::NUM_COLS-1; j >= 0; j--) {
                int bitIdx = (chessmeta::NUM_TILES-1) - (chessmeta::NUM_COLS*i + j);
                Square sq = utils::intToSquare(bitIdx);

                Color pieceColor = utils::charToColor(boardMatrix[i][j]);
                PieceType pieceType = utils::charToPieceType(boardMatrix[i][j]);

                placePiece(pieceColor, pieceType, sq);
            }
        }
    }

    // ===================== CONSTRUCTORS =====================

    GameBoard::GameBoard(const std::string& fen) {
        // Reset all bitboards
        pieceBitboard = {};

        applyFenString(fen);
        updateBitboards();
    }

    GameBoard::GameBoard(const std::array<std::array<char, 8>, 8> boardMatrix, bool wToMove, 
    bool wCanCastKing, bool wCanCastQueen, bool bCanCastKing, bool bCanCastQueen,
    Square enPassSq, int numHalf, int numFull) {
        //Setting other parameters
        whiteToMove = wToMove;
        whiteCanCastleKingSide = wCanCastKing;
        whiteCanCastleQueenSide = wCanCastQueen;
        blackCanCastleKingSide = bCanCastKing;
        blackCanCastleQueenSide = bCanCastQueen;
        enPassantSq = enPassSq;
        halfMove = numHalf;
        fullMove = numFull;

        // Reset all bitboards
        pieceBitboard = {};

        applyBoardMatrix(boardMatrix);

        updateBitboards();
    }

    // ===================== PUBLIC GETTER METHODS =====================

    std::string GameBoard::toString() const {
        std::string rep = "";

        for(int i = 0; i < chessmeta::NUM_TILES; i++) {
            if(i != 0 && i % 8 == 0) rep = "\n\n" + rep;

            Square sq = utils::intToSquare(i);
            char symbol = '_';
            
            for(int col = 0; col < 2; col++) {
                for(int type = 0; type < 6; type++) {
                    if(bitboard::occupiedAt(pieceBitboard[col][type], sq)) {
                        symbol = chessmeta::PIECE_SYMBOL[col][type];
                        goto found;
                    }
                }
            }

            found: rep = std::string(1, symbol) + "  " + rep;
        }
        return "\n" + rep + "\n";
    }

    bitboard::bitmap GameBoard::copyPieceBitboard(Color col, PieceType piece) const {
        return pieceBitboard[static_cast<int>(col)][static_cast<int>(piece)];
    }

    bitboard::bitmap GameBoard::copyAllPiecesBitboard(Color colourFilter) const {
        switch(colourFilter) {
            case Color::WHITE: return allWhitePieces;
            case Color::BLACK: return allBlackPieces;
            default: return occupiedSquares;
        }
    }

    // ========================= MUTATOR METHODS =======================

    void GameBoard::makeMove(const chessmove::Move& move,
        Color attackPieceColor, PieceType attackPieceType, 
        Color capturedPieceColor, PieceType capturedPieceType) {
        //TODO: Handle Promotions, En Passant, Castling
        removePiece(capturedPieceColor, capturedPieceType, move.toSquare);
        removePiece(attackPieceColor, attackPieceType, move.fromSquare);
        placePiece(attackPieceColor, attackPieceType, move.toSquare);

        updateBoardAfterMove(move, attackPieceColor, attackPieceType, capturedPieceColor, capturedPieceType);
    }
}
