#pragma once
#include "Game.h"

class Connect4 : public Game
{
public:

    Connect4();
    ~Connect4();

    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void        stopGame() override;

	void        updateAI() override;
    bool        gameHasAI() override { return true; }
    Grid*       getGrid() override { return _grid; }
    
    static const int TT_BITS = 18;
    static const int TT_SIZE = 1 << TT_BITS;

    struct TTEntry {
        uint64_t key;
        int8_t   depth;
        int32_t  value;
    };

    static TTEntry TT[TT_SIZE];
    
private:
    Bit *       PieceForPlayer(const int playerNumber);
    Player*     ownerAt(int index ) const;
    int         negamax(uint64_t current, uint64_t opponent, uint64_t allPieces,
                      int depth, int alpha, int beta);
    void        placePiece(int bestCol);

    Grid*       _grid;

    static constexpr int moveOrder[7] = {3,2,4,1,5,0,6};

    // This thing is so cool
    // Basically I am hashing the boards so that if there is a repeated board I can prune the branch quickly
    inline uint64_t zobrist(uint64_t current, uint64_t opponent) {
        return (current * 0x9e3779b97f4a7c15ULL) ^ (opponent * 0xc3a5c85c97cb3127ULL);
    }

};
