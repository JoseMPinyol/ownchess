#ifndef __MAIN_H__
#define __MAIN_H__

#include <cstdio>
#include <windows.h>

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

DLL_EXPORT void init();
//DLL_EXPORT int play_alone(int x,int y, int nx ,int ny);
DLL_EXPORT int player_make_move(int x,int y, int nx ,int ny);
DLL_EXPORT void computer_make_move();
//char DLL_EXPORT __board[100];
DLL_EXPORT int* Get_Board();
DLL_EXPORT void transform_and_change();
DLL_EXPORT int GET_LAST_DEPTH();
DLL_EXPORT int GET_NODES();
DLL_EXPORT int *GET_LAST_MOVE();
DLL_EXPORT void SET_MAX_NODES(int max_val);
#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
