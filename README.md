# OwnChess 
## About:
This was an AI course project in 2015. It is a "chess engine" able to defeat Windows 7 chess game in difficult mode ... sometimes :-)
Its implementation was done on the C ++ language, compiling itself as a dll. This dll exports a set of functions that allowed interaction with the engine. The presented here is one of the recovered versions.
Example of exported functions:
- Set limits on the number of intermediate nodes to analyze (Ex: SET_MAX_NODES )
- Computer make move. (Ex: computer_make_move )
- Player move (Ex: player_make_move)
- Get current board status (Ex: Get_Board )
- Rotate board, It allows to play with the other side or play against itself (Ex: transform_and_change)

## Implemented in this version:
1. Negamax
2. Iterative Deepening
3. Alpha-Beta Pruning (With sorting of nodes using the iterative deepening and the previous calculate values)  
4. Search until stability is reached. When abrupt changes in the evaluation happens from the previous level (Ex: a piece is captured, another version of this code include when the king was under attack)
5. Positions Tables

## I experimented with in other versions with:
 - Null-Movement
 - Killer-Move (a variant)
 - Principal Variation Search.
 
 ## GUI
 I implemented a simple GUI in Windows Forms. You can donwnload it from [ChessGUI ](bin/)
 
 ## My recommendations (4 years latter :-) )
- The structure of the project should be improved (Although the project has ~ 450 lines of code) 
- I should have followed the same naming convention.
- Some global variable practices could be improved
- The data structure for the board is far from optimal in both memory and speed(constant increments).
- I should have implemented universal chess interface in order to evaluate against other chess engines (and use in common GUI)
- Of course many other recomendations are possible too.
