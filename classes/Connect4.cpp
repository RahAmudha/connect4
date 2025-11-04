#include "Connect4.h"

// For players
int lastMove;

// Cache
Connect4::TTEntry Connect4::TT[TT_SIZE];

// Negamax Scores
static const int SCORE_WIN  = 1000000;
static const int SCORE_LOSS = -SCORE_WIN;
static const int SCORE_DRAW = 0;

Connect4::Connect4(){
    _grid = new Grid(7, 6);
    for (int i = 0; i < TT_SIZE; ++i) TT[i].depth = -1;
}

Connect4::~Connect4(){
    delete _grid;
}

Bit* Connect4::PieceForPlayer(const int playerNumber)
{
    Bit *bit = new Bit();
    bit->LoadTextureFromFile(playerNumber == getAIPlayer() ? "yellow.png" : "red.png");
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

void Connect4::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "boardsquare.png");
    startGame();
}

bool Connect4::actionForEmptyHolder(BitHolder &holder)
{
    if (holder.bit()) {
        return false;
    }
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());

    if (bit) {
        int x = holder.getPosition().x / 80;
        int y = holder.getPosition().y / 80;

        if (y != 5 && !(getGrid()->getSquare(x, y+1)->bit())){
            return false;
        }

        bit->setPosition(holder.getPosition());
        holder.setBit(bit);

        // Stores the last move for checking winner
        lastMove = x+y*7;

        endTurn();
        
        return true;
    }   
    return false;
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    return false;
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return false;
}

void Connect4::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Connect4::ownerAt(int index ) const
{
    auto square = _grid->getSquare(index % 7, index / 7);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Connect4::checkForWinner()
{
    constexpr int WIDTH = 7;
    constexpr int HEIGHT = 6;

    Player* board[WIDTH * HEIGHT] = {nullptr};
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (sq && sq->bit())
                board[y * WIDTH + x] = sq->bit()->getOwner();
        }
    }

    int x, y;
    _grid->getCoordinates(lastMove, x, y);
    Player* player = board[y * WIDTH + x];
    if (!player) return nullptr;

    auto get = [&](int xx, int yy) -> Player* {
        if (xx < 0 || xx >= WIDTH || yy < 0 || yy >= HEIGHT) return nullptr;
        return board[yy * WIDTH + xx];
    };

    const int dirs[4][2] = {
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1}
    };

    for (auto& d : dirs) {
        int count = 1;

        for (int step = 1; step < 4; ++step) {
            Player* p = get(x + step*d[0], y + step*d[1]);
            if (p == player) count++;
            else break;
        }

        for (int step = 1; step < 4; ++step) {
            Player* p = get(x - step*d[0], y - step*d[1]);
            if (p == player) count++;
            else break;
        }
        if (count >= 4)
            return player;
    }

    return nullptr;
}

bool Connect4::checkForDraw()
{
    bool isDraw = true;
    _grid->forEachSquare([&isDraw](ChessSquare* square, int x, int y) {
        if (!square->bit()) {
            isDraw = false;
        }
    });
    return isDraw;
}

std::string Connect4::initialStateString()
{
    return "000000000000000000000000000000000000000000";
}

std::string Connect4::stateString()
{
    const int width = 7;
    const int height = 6;
    std::string s(width * height, '0');

    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit* bit = square->bit();
        if (bit && bit->getOwner()) {
            int index = y * width + x;
            s[index] = '0' + (bit->getOwner()->playerNumber() + 1);
        }
    });

    return s;
}

void Connect4::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*7 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit( PieceForPlayer(playerNumber-1) );
        } else {
            square->setBit( nullptr );
        }
    });
}

inline bool hasWon(uint64_t bb)
{
    uint64_t m;

    m = bb & (bb >> 1);
    if (m & (m >> 2)) return true;

    m = bb & (bb >> 7);
    if (m & (m >> 14)) return true;
    if (m & (m >> 16)) return true;

    m = bb & (bb >> 6);
    if (m & (m >> 12)) return true;

    return false;
}

int makeMove(uint64_t &board, uint64_t &allPieces, int col) {
    for (int row = 5; row >= 0; --row) {
        int idx = row*7 + col;
        if ((allPieces & (1ULL << idx)) == 0) {
            board |= (1ULL << idx);
            allPieces |= (1ULL << idx);
            return idx;
        }
    }
    return -1;
}

int findImmediateWinningMove(uint64_t board, uint64_t allPieces) {
    for (int col = 0; col < 7; ++col) {
        if (allPieces & (1ULL << col)) continue;

        uint64_t tmpBoard = board;
        uint64_t tmpAll = allPieces;
        int idx = makeMove(tmpBoard, tmpAll, col);
        if (idx == -1) continue;
        if (hasWon(tmpBoard)) return col;
    }
    return -1;
}

int Connect4::negamax(uint64_t current, uint64_t opponent, uint64_t allPieces,
                      int depth, int alpha, int beta)
{
    uint64_t key = zobrist(current, opponent);
    TTEntry &e = TT[key & (TT_SIZE - 1)];
    if (e.depth >= depth && e.key == key) return e.value;

    if (hasWon(opponent)) return SCORE_LOSS;
    int pieces = __builtin_popcountll(allPieces);
    if (pieces >= 42) return SCORE_DRAW;
    if (depth == 0) return 0;

    // Basic checks but only do on shallow brnaches
    if (depth >= 6){

        // Prune if there is immeadiate win
        int w = findImmediateWinningMove(current, allPieces);
        if (w != -1) return SCORE_WIN;

        // Search for opponent's winning moves
        int dangers = 0;
        for (int col=0; col<7; col++) {
            if (allPieces & (1ULL << col)) continue;
            uint64_t tOpp = opponent, tAll = allPieces;
            if (makeMove(tOpp, tAll, col) != -1 && hasWon(tOpp)) dangers++;
        }
        if (dangers >= 2) return SCORE_LOSS;

        // For moves that create two threats
        for (int col=0; col<7; ++col) {
            if (allPieces & (1ULL << col)) continue;
            uint64_t tCur = current, tAll = allPieces;
            if (makeMove(tCur, tAll, col) == -1) continue;

            int w2 = 0;
            for (int c2=0; c2<7; ++c2) {
                uint64_t ttCur = tCur, ttAll = tAll;
                if (makeMove(ttCur, ttAll, c2) != -1 && hasWon(ttCur)) w2++;
            }
            if (w2 >= 2) return SCORE_WIN;
        }
    }

    // Main Loop
    for (int col : moveOrder) {
        if ((allPieces >> col) & 1) continue;

        uint64_t nextCur = current;
        uint64_t nextAll = allPieces;
        int idx = makeMove(nextCur, nextAll, col);
        if (idx == -1) continue;

        // Immeadiate Win
        if (hasWon(nextCur)) return SCORE_WIN;

        // Immeadiate Loss -> Block
        bool losing = false;
        for (int c = 0; c < 7; ++c) {
            if ((nextAll >> c) & 1) continue;
            uint64_t tmpOp = opponent;
            uint64_t tmpA  = nextAll;
            if (makeMove(tmpOp, tmpA, c) != -1 && hasWon(tmpOp)) {
                losing = true;
                break;
            }
        }
        
        // Recurse
        int val = losing ? (SCORE_LOSS + 1)
                         : -negamax(opponent, nextCur, nextAll, depth-1, -beta, -alpha);

        if (val > alpha) alpha = val;
        if (alpha >= beta) break;
    }

    e.key = key;
    e.depth = depth;
    e.value = alpha;

    return alpha;
}

void Connect4::placePiece(int bestCol) {
    int row = -1;
    for (int y = 5; y >= 0; --y) {
        if (!_grid->getSquare(bestCol, y)->bit()) {
            row = y;
            break;
        }
    }

    if (row != -1) {
        Bit* bit = PieceForPlayer(AI_PLAYER);
        ImVec2 pos = ImVec2(bestCol * 80 + 40, row * 80 + 40);
        bit->setPosition(pos);
        _grid->getSquare(bestCol, row)->setBit(bit);

        lastMove = row * 7 + bestCol;
        endTurn();
    }
}


void Connect4::updateAI() {
    uint64_t aiBoard = 0, humanBoard = 0, allPieces = 0;

    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 7; ++x) {
            Player* owner = ownerAt(y * 7 + x);
            if (!owner) continue;
            int idx = y * 7 + x;
            allPieces |= (1ULL << idx);
            if (owner->playerNumber() == AI_PLAYER) aiBoard |= (1ULL << idx);
            else humanBoard |= (1ULL << idx);
        }
    }

    int bestScore = std::numeric_limits<int>::min();
    int bestCol = -1;

    // Do once at beginning if obvious move
    int winCol = findImmediateWinningMove(aiBoard, allPieces);
    if (winCol != -1) {
        bestCol = winCol;
        placePiece(bestCol);
        return;
    }

    // Obvious block
    std::vector<int> oppWins;
    for (int col = 0; col < 7; ++col) {
        if (allPieces & (1ULL << col)) continue;
        uint64_t tmpHuman = humanBoard;
        uint64_t tmpAll = allPieces;
        int idx = makeMove(tmpHuman, tmpAll, col);
        if (idx == -1) continue;
        if (hasWon(tmpHuman)) oppWins.push_back(col);
    }

    if (oppWins.size() == 1) {
        int blockCol = oppWins[0];
        if (!(allPieces & (1ULL << blockCol))) {
            bestCol = blockCol;
            placePiece(bestCol);
            return;
        }
    } // I don't handle double threats because that is an immeadiate loss

    for (int col : moveOrder) {
        if (allPieces & (1ULL << col)) continue;

        uint64_t tempAI = aiBoard;
        uint64_t tempAll = allPieces;

        int idx = makeMove(tempAI, tempAll, col);
        if (idx == -1) continue;

        int score = -negamax(tempAI, humanBoard, tempAll, 10,
                     std::numeric_limits<int>::min()/2,
                     std::numeric_limits<int>::max()/2);

        if (score > bestScore) {
            bestScore = score;
            bestCol = col;
        }
    }

    if (bestCol != -1) { placePiece(bestCol); }
}


