#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <windows.h>
using namespace std;
int n = 0;
void mcm(int* mat);
void result(int** s, int i, int j, int* num);



int main(){
    //1번
    int mat1[] = {1,2,3,4,5};
    n = sizeof(mat1) / sizeof(int) - 1;
    mcm(mat1);

    //2번
    int mat2[] = {4,3,2,1};
    n = sizeof(mat2) / sizeof(int) - 1;
    mcm(mat2);

    //3번
    int mat3[] = {2,4,5,1,6};
    n = sizeof(mat3) / sizeof(int) - 1;
    mcm(mat3);

    //4번
    int mat4[] = {5,7,2,4,5};
    n = sizeof(mat4) / sizeof(int) - 1;
    mcm(mat4);

    //5번
    int mat5[] = {9,1,5,6,4};
    n = sizeof(mat5) / sizeof(int) - 1;
    mcm(mat5);

    //6번
    int mat6[] = {3,6,5,2};
    n = sizeof(mat6) / sizeof(int) - 1;
    mcm(mat6);

    //7번
    int mat7[] = {7,1,8,4};
    n = sizeof(mat7) / sizeof(int) - 1;
    mcm(mat7);

    //8번
    int mat8[] = {10,2,4,3};
    n = sizeof(mat8) / sizeof(int) - 1;
    mcm(mat8);

    //9번
    int mat9[] = {6,1,9,3};
    n = sizeof(mat9) / sizeof(int) - 1;
    mcm(mat9);

    //10번
    int mat10[] = {30,35,15,5,10,20,25};
    n = sizeof(mat10) / sizeof(int) - 1;
    mcm(mat10);
}


void mcm(int* mat){
    //동적 메모리할당
    int **cost_mat = new int*[n];
    int **s = new int*[n];
    for (int i = 0; i < n; ++i) {
        cost_mat[i] = new int[n];
        s[i] = new int[n];
    }
    //초기화
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            s[i][j] = 0;
            cost_mat[i][j] = INT_MAX;
        }
    }

    for(int i = 0; i < n; i++){
        cost_mat[i][i] = 0;
    }

    //cost와 k 구하기
    for(int l = 1; l < n; l++){
        for(int i = 0; i < n - l; i++){
            int j = i + l;
            for(int k = i; k < j; k++){
                int cost = cost_mat[i][k] + cost_mat[k + 1][j] + mat[i] * mat[k + 1] * mat[j + 1];
                if(cost < cost_mat[i][j]){
                    cost_mat[i][j] = cost;
                    s[i][j] = k + 1;
                }
            }
        }
    }

    //결과 출력
    cout << "cost matrix\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            cout.width(10);
            cout << cost_mat[i][j];
        }
        cout << "\n";
    }
    cout << "\nk matrix\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            cout.width(10);
            cout << s[i][j];
        }
        cout << "\n";
    }
    //결과출력
    int num = 1;
    result(s, 0, n - 1, &num);
    cout << "\n";
    cout << "value : " << cost_mat[0][n - 1];
    cout << "\n________________________________\n\n";
    
    //메모리 해제
    for (int i = 0; i < n; ++i) {
        delete[] cost_mat[i];
        delete[] s[i];
    }
}

void result(int** s, int i, int j, int* num){
    Sleep(50);
    if(i == j){
        cout << "A" << *num;
        *num = *num + 1;
        return;
    }
    cout << "(";
    result(s, i, s[i][j] - 1, num);
    result(s, s[i][j], j, num);
    cout << ")";
}