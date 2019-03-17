#include "main.h"
#include <cstdio>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <unordered_map>


#define global_evaluate_pos() ( evaluate_player(1) - evaluate_player(0) )
#define is_white_piece(a) ( 'A' <= a && a <= 'Z')
#define is_black_piece(a) ( 'a' <= a && a <= 'z')

typedef unsigned long long ull;

enum {PAWN,HORSE,BISHOP,ROOT,QUEEN,KING};


inline int get_piece_by_char(char a){  //P , H , B , R ,Q , K;
 if(a == 'P' || a == 'p') return PAWN;
 if(a == 'H' || a == 'h') return HORSE;
 if(a == 'B' || a == 'b') return BISHOP;
 if(a == 'R' || a == 'r') return ROOT;
 if(a == 'Q' || a == 'q') return QUEEN;
 if(a == 'K' || a == 'k') return KING;
 assert(false);
 return -1;
}


// Max value for avoiding search of stability minimax (less than one pawn in last rows)
const int max_positional_difference = 169 ;
// Static weights or prices of (PAWN,HORSE,BISHOP,ROOT...KING in different positions of the board)
static constexpr int prices[6][8][8] = {
    {
        {100,100,100,100,100,100,100,100},
        {100,100,100,100,100,100,100,100},
        {100,115,100,100,100,100,115,100},
        {100,100,100,125,125,100,100,100},
        {130,130,130,130,130,130,130,130},
        {140,140,140,140,140,140,140,140},
        {170,170,170,170,170,170,170,170},
        {100,100,100,100,100,100,100,100}
    },
    {
        {350,345,350,350,350,350,345,350},
        {350,360,360,360,360,360,360,350},
        {350,360,370,370,370,370,360,350},
        {350,360,370,380,380,370,360,350},
        {350,360,370,380,380,370,360,350},
        {350,360,370,370,370,370,360,350},
        {350,360,360,360,360,360,360,350},
        {350,350,350,350,350,350,350,350}
    },
    {
        {350,350,350,350,350,350,350,350},
        {350,370,350,350,350,350,370,350},
        {350,350,350,370,370,350,350,350},
        {350,350,370,370,370,370,350,350},
        {350,350,370,370,370,370,350,350},
        {350,350,350,370,370,350,350,350},
        {350,370,350,350,350,350,370,350},
        {350,350,350,350,350,350,350,350}
    },
    {
        {600,600,630,630,630,630,600,600},
        {600,600,630,630,630,630,600,600},
        {600,600,600,600,600,600,600,600},
        {600,600,600,600,600,600,600,600},
        {600,600,600,600,600,600,600,600},
        {600,600,600,600,600,600,600,600},
        {600,600,600,600,600,600,600,600},
        {600,600,600,600,600,600,600,600}
    },
    {
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300},
        {1300,1300,1300,1300,1300,1300,1300,1300}
    },
    {
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
        {100000,100000,100000,100000,100000,100000,100000,100000},
    }


};

struct game{ //can be improved in order to avoid board copy
    char board[8][8];
    bool RC,RL , rc, rl; //right castling enabled, left castling for both players
    inline double evaluate_position(){ // player 1 = whites, player 2 = blacks; //for the moment not positional evaluation (except simple position by tables above)
        double points = 0;
        for(int i = 0 ; i < 8 ;i++)
            for(int j = 0 ; j < 8 ;j++){
                char ch = board[i][j];
                if(ch == '.') continue;
                points += ( (is_white_piece(ch))? 1 : -1) * prices[ get_piece_by_char(ch) ][i][j];
            }
        return points;
    }

    ull hash_board_position(){ //naive hash
        ull has = 0 , bas = 'z' - '.' + 1;
        for(int i = 0 ; i < 8 ;i++)
            for(int j = 0 ; j < 8 ;j++)
                has  = has * bas + board[i][j] - '.';
        has  = has * bas + RC * 8 + RL * 4 + rc * 2 + rl ;
        return has;
    }

    inline void apply_valid_move(const int &x,const int &y, const int &nx ,const int &ny){

        stat += prices[get_piece_by_char(board[x][y])][nx][ny] - prices[get_piece_by_char(board[x][y])][x][y];
        if(board[nx][ny] != '.') stat += prices[get_piece_by_char(board[nx][ny])][7-nx][ny]; //captura de la ficha contraria

        board[nx][ny] = board[x][y];
        board[x][y] = '.';
        if(nx == 7 && board[nx][ny] == 'P') {board[nx][ny] = 'Q'; stat += prices[QUEEN][nx][ny] - prices[PAWN][nx][ny]; }
        if(board[nx][ny] == 'K'){
            if(ny - y == 2){ //short castling
                board[0][7] = '.' ; board[0][5] = 'R';
                stat += prices[ROOT][0][5] - prices[ROOT][0][7];
            }
            if(y - ny == 2){ //large castling
                board[0][0] = '.' ; board[0][3] = 'R';
                stat += prices[ROOT][0][3] - prices[ROOT][0][0];
            }
            RC = false; RL = false; //can't do any other castling
        }
        if(x == 0 && y == 0) RL = false; //can't do large castling
        if(x == 0 && y == 7) RC = false; //can't do short castling
    }
    int stat;
} MAIN;

inline void transform_and_change(){ //Rotate the board for change (white-black) interpretation
    int diff = 'a' - 'A';
    for(int i = 0;  i < 8 ; i++)
        for(int j = 0 ; j < 8 ;j++){
            if( is_white_piece(MAIN.board[i][j]) )
                MAIN.board[i][j] += diff;
            else if( is_black_piece(MAIN.board[i][j]) )
                MAIN.board[i][j] -= diff;
        }
    for(int i = 0 ; i < 4 ;i++)
        for(int j = 0 ; j < 8 ;j++)
            std::swap(MAIN.board[i][j] , MAIN.board[7-i][j]); //improve
    std::swap(MAIN.rc,MAIN.RC); std::swap(MAIN.rl,MAIN.RL);
    MAIN.stat = -MAIN.stat;
}

const char Board[8][9] = { //This is not the visualization!!. The interpretation is: 0,0 => 1A cell
            "RHBQKBHR",
            "PPPPPPPP",
            "........",
            "........",
            "........",
            "........",
            "pppppppp",
            "rhbqkbhr"
};
 DLL_EXPORT void init(){
    srand(time(0));
    for(int i = 0 ; i < 8 ;i++)
        for(int j= 0 ; j < 9 ; j++)
            MAIN.board[i][j] = Board[i][j];
    MAIN.RC = MAIN.rc = MAIN.rl = MAIN.RL = 1;
    MAIN.stat = 0;
}


//Print Board for debug.
void print_board(){
    for(int i = 0 ; i < 8 ;i++){
        for(int j= 0 ; j < 8 ;j++)
            printf("%c",MAIN.board[7-i][j]);
        printf("\n");
    }

}

///////////Pieces basic delta moves////////////////
int num_basic_mov[6] = {4,8,4,4,8,8}; //Number of basic moves per piece //P , H , B , R ,Q , K;
int basic_mov[6][8][2] = { ///////////!!Do not change order of moves it DOES matter!!/////////////
    {
        {1,0},{2,0},{1,1},{1,-1} 
    },
    {
        {1,2},{1,-2},{2,1},{2,-1},{-1,2},{-1,-2},{-2,1},{-2,-1} //horse
    },
    {
       {1,1} , {1,-1}, {-1,1} , {-1,-1}
    },
    {
       {0,1} , {0,-1}, {1,0} , {-1,0}
    },
    {
       {1,1} , {1,-1}, {0,1} , {0,-1}, {1,0} ,{-1,1} , {-1,-1} , {-1,0}
    },
    {
        {1,1} , {1,-1}, {-1,1} , {-1,-1} ,{0,1} , {0,-1}, {1,0} , {-1,0}
    }

}; //P , H , B , R ,Q , K;*/
inline bool under_attack(const int &y){ //Check if 0,y is under attack (only called few times to check if castling is available)
    if((y>0 && MAIN.board[1][y-1] == 'p') || (y<7 && MAIN.board[1][y+1] == 'p')) return 1; //pawn
    for(int i = 0 ; i < 4 ;i++){ //horse
        int nx  = basic_mov[1][i][0], ny = y + basic_mov[1][i][1];
        if(nx < 0 || nx > 7 || ny < 0 || ny > 7) continue;
        if(MAIN.board[nx][ny] == 'h') return 1;
    }
    for(int p = 2 ; p <= 4 ; p++){ //bishop, castle, queen
        for(int i = 0 ; i < num_basic_mov[p] ;i++)
            for(int ds = 1 ; ;ds++){
                int nx = 0 + ds*basic_mov[p][i][0] , ny = y + ds*basic_mov[p][i][1];
                if(nx < 0 || nx >= 8 || ny < 0 || ny >= 8) break;
                char cas = MAIN.board[nx][ny];
                if( is_white_piece(cas) ) break;
                if(is_black_piece(cas) && get_piece_by_char(cas) == p) return 1;
                if( is_black_piece(cas) ) break;
            }
    }
    return false;
}
//////////////This function generate all moves for a single piece///////////////
void generate_basic_mov(int x, int y , int &num , int NX[] , int NY []){ //It assumes a white piece exists in x,y
    num = 0;
    char piece = MAIN.board[x][y];
    int pie = get_piece_by_char(piece); //Expects white piece 
    for(int i = 0 ; i < num_basic_mov[pie] ;i++) //All posible basic moves for the piece
        for(int ds = 1 ; ;ds++){
            int nx = x + ds*basic_mov[pie][i][0] , ny = y + ds*basic_mov[pie][i][1];
            if(nx < 0 || nx >= 8 || ny < 0 || ny >= 8) break; //piece is out of board, skip this move.
            if( is_white_piece(MAIN.board[nx][ny]) ) break; //Can't land over his own pieces.
            if(pie == PAWN){
                if(ny != y && !is_black_piece(MAIN.board[nx][ny]) ) break; //If it's a pawn and is going to capture, the captured piece must exists ;-) ...
                if(nx - x == 2 && ( x != 1 || MAIN.board[2][y] != '.' ) ) break; //Two steps for pawn only at start of his journey
                if(y == ny && is_black_piece(MAIN.board[nx][ny])) break; //The pawn can't capture when moving forward.
            }
            NX[num] = nx;
            NY[num++] = ny;
            if( is_black_piece(MAIN.board[nx][ny]) || pie == HORSE || pie == KING || pie == PAWN) break;
        }
    if(pie == KING){
        if(under_attack(4) == true) return; //The king is under attack
        if(MAIN.RC && MAIN.board[0][5]=='.' && MAIN.board[0][6]=='.' && MAIN.board[0][7] == 'R' &&
           under_attack(6) == false && under_attack(5) == false){ // Short Castling is possible?
            NX[num] = 0; NY[num++] = 6;
        }
        if(MAIN.RL && MAIN.board[0][1]=='.' && MAIN.board[0][2]=='.' &&  MAIN.board[0][3]=='.' && MAIN.board[0][0] == 'R' && // Large Castling is possible?
          under_attack(2) == false && under_attack(3) == false) {
            NX[num] = 0; NY[num++] = 2;
        }
    }
}

//This function is mainly in order to check input from user and testing purposes
bool is_valid_move(const int &x,const int &y, const int &nx ,const int &ny){
    int X[31] ,Y[31];
    int num = 0;
    if(!is_white_piece(MAIN.board[x][y])) return false;
    generate_basic_mov(x,y,num,X,Y);
    for(int i = 0 ; i < num ;i++)
        if(X[i] == nx && Y[i] == ny)
            return true;
    return false;
}

//Make valid move from user
inline void apply_valid_move(const int &x,const int &y, const int &nx ,const int &ny){
    MAIN.board[nx][ny] = MAIN.board[x][y];
    MAIN.board[x][y] = '.';
    if(nx == 7 && MAIN.board[nx][ny] == 'P') MAIN.board[nx][ny] = 'Q';
    if(MAIN.board[nx][ny] == 'K'){
        if(ny - y == 2){ //enroque corto
            MAIN.board[0][7] = '.' ; MAIN.board[0][5] = 'R';
        }
        if(y - ny == 2){ //enroque largo
            MAIN.board[0][0] = '.' ; MAIN.board[0][3] = 'R';
        }
        MAIN.RC = false; MAIN.RL = false; //No more Castlings
    }
    if(x == 0 && y == 0) MAIN.RL = false; //No more short Castling
    if(x == 0 && y == 7) MAIN.RC = false; //No more large Castling
}

//High level make move
// -> return false if move is not valid
// -> return true if move is valid and play it.
DLL_EXPORT int player_make_move(int x,int y, int nx ,int ny){
    if(is_valid_move(x,y,nx,ny) == false) return 0;
    apply_valid_move(x,y,nx,ny);
    return 1;
}
// Return current board position
 DLL_EXPORT int* Get_Board(){
    int * __board = new int[65]; //65
    for(int i = 0 ; i < 8 ;i++)
        for(int j = 0 ; j < 8 ;j++)
            __board[j + i*8] =MAIN.board[i][j];
     __board[64] = 0;
     return __board;
}

//Struct for moves 
//Is required for sorting moves from previous depth search
// and process first previous best moves (in order to improve alpha-beta prunning)
// and other heuristics
struct mov{
    int cx,cy; //currentx, currenty
    int nx,ny; //newx , newy
    double cost;
    int idx;
};

//Operator load
bool operator < (const mov &A,const mov &B){
    return A.cost > B.cost;
}
// Generate all valid moves for white (can be improved in many ways)
void Generate_all_valid_movement(std::vector<mov> &MV){
    int NX[31] , NY[31] , num;
    for(int i = 0 ; i < 8 ; i++)
        for(int j = 0 ; j < 8 ; j++)
            if(is_white_piece(MAIN.board[i][j])){
                generate_basic_mov(i,j,num,NX,NY);
                for(int k = 0 ; k < num ;k++)
                    MV.push_back( (mov){i,j,NX[k], NY[k]});
            }
}
// Initial MAX number of internal nodes. (Can be modified from outside DLL EXPORT FUNCTION)
int MAX_NODES = 200000;
// Infinite score... :-) 
const double best_val = 2e6;

//Best mov
mov best_mv;

// Save hashed boards for order moves f
struct hashed_board{
    double val;
    int depth;
};
//double _init_points;
std::unordered_map<ull,hashed_board > mp; //hash and depth
std::unordered_map<ull,hashed_board >::iterator it_mp;
int nod = 0;
// Hard stuff of searching the best with certain depth (stability has no limit) is done here
double search_best_mov(const int depth , double alpha = -best_val, double beta = best_val){ //para hacer siempre el min_max, con comodidad utilizar
    nod++;
    if(std::abs(MAIN.stat) > prices[5][0][0]/2) return MAIN.stat; //End of the game. One king is missing
    game cur; //Can be improved for not doing this on program stack
    int idx = 0;
    std::vector<mov> MV;
    Generate_all_valid_movement(MV);
    std::vector<ull> _hash(MV.size());
    std::vector<game> B(MV.size());
    for(int i = 0 ; i < (int)MV.size() ;i++){ //Generate future moves
        MV[i].idx = i;
        cur = MAIN;
        MAIN.apply_valid_move(MV[i].cx, MV[i].cy , MV[i].nx, MV[i].ny);
        transform_and_change();
        B[i] = MAIN;
        _hash[i] = MAIN.hash_board_position();
        if((it_mp = mp.find(_hash[i])) == mp.end() )
            MV[i].cost = -best_val;//evaluate_position(); //unknown cost of the position (I don't trust)
        else MV[i].cost = (*it_mp).second.val;
        MAIN = cur;
    }
    std::sort(MV.begin(),MV.end()); //In order to improve alpha-beta
    cur = MAIN;
    int limit = ((depth <= 4)?MV.size():MV.size());
    double best_curr = -best_val; //Estimation of current position ...
    for(int i = 0 ; i < limit ;i++){
        MAIN = B[MV[i].idx];
        ull has = _hash[MV[i].idx];
        double ret; //For holding the evaluation of ith-move
        if(depth != 1){ //Limit not reached
            if( (it_mp = mp.find(has)) == mp.end() || (*it_mp).second.depth < depth){ //Look if this position has been seen from another path (merge some branches)
				ret = -search_best_mov(depth - 1,-beta,-alpha);
				mp[has] =   hashed_board{ret,depth}; //I memorize this board for this depth
            }else ret = (*it_mp).second.val;
        }else { //Depth limit reached
            if(-MAIN.stat - cur.stat >=  max_positional_difference){ //Check if last move was not an stable one (ex: capture something)
                ret = -search_best_mov(1,-beta,-alpha);
            }else   ret = -MAIN.stat; //The last move was stable
        }

        if(best_curr < ret){best_curr = ret;}
        if(alpha < ret){ idx = i; alpha = ret;}
        if(beta <= alpha){ //Alpha beta pruning
            break;
        }
    }
    MAIN = cur;
    if(idx != -1 && limit > 0) best_mv = MV[idx];
    return best_curr;
}

DLL_EXPORT int GET_NODES(){ return nod;}
int depth;
DLL_EXPORT int GET_LAST_DEPTH() { return depth;}
DLL_EXPORT void SET_MAX_NODES(int max_val) { MAX_NODES = max_val;}

mov played;

DLL_EXPORT int *GET_LAST_MOVE(){
    int *_ret = new int[4];
    _ret[0] = played.cx ; _ret[1] = played.cy; _ret[2] = played.nx ; _ret[3] = played.ny;
    return _ret;

}

// This function indicate the computer to make a move
// Increment depth analysis until limit of internal nodes is reached after previous
// search this can cause some delays in many situations
DLL_EXPORT void computer_make_move(){
    transform_and_change();
    for(depth = 1 ;  ; depth++){
        nod = 0;
        search_best_mov(depth,-best_val,best_val);
        played = best_mv;
        if(nod > MAX_NODES) break;
    }
    if( 0 == player_make_move(played.cx,played.cy,played.nx,played.ny)){
        printf("Something is wrong because virtual played move fake move\n");
    }
    transform_and_change();
    mp.clear();
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            init();
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // successful
}
