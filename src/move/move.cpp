#include "move/move.h"

namespace chessmove {
    bool isDoublePawnPush(Move m) {
        char type = m.attackPieceType;
        //Restrict to checking pawns
        if(type != 'P' && type != 'p') { return false; }

        //Calculate conditions based on move for white
        bool pawnOnRank2 = m.fromBitIdx / 8 == 1;
        bool movedToRank4 = (m.toBitIdx - m.fromBitIdx) == 16;
        //Calculate conditions based on move for black
        bool pawnOnRank7 = m.fromBitIdx / 8 == 6;
        bool movedToRank5 = (m.fromBitIdx - m.toBitIdx) == 16;

        //Calculate and return whether double pawn push
        return (type == 'P' && pawnOnRank2 && movedToRank4) || 
        (type == 'p' && pawnOnRank7 && movedToRank5);
    }
}