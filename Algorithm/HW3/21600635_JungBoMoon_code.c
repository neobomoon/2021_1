#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void insertion_sort(int *a, int size);
void heap_sort(int *a, int size);
void build_max_heap(int *a, int size);
void max_heapify(int *a, int i, int size);
void quick_sort(int *a, int start, int end);
int partition(int *a, int start, int end);
void init_array(int *a, int i);
void print_array(int **a);
void print_line(int *a, int size);




int main(){
    int **array = malloc(sizeof(int *) * 10);
    clock_t start, finish;
    double time = 0;

    for(int i = 0; i < 10; i++){
        array[i] = malloc(sizeof(int) * (i + 1) * 10);
    }

    int i = 0;
    while(i < 10){
        srand(i);
        for(int j = 0; j < (i + 1) * 10; j++){
            array[i][j] = rand() % ((i + 1) * 20);
        }
        i++;
    }

    print_array(array);

    for(i = 0; i < 10; i++){
        int size = (i + 1) * 10;
        printf("-----------------------\n");
        printf("size : %d\n", size);
        //insertion sort
        start = clock();
        insertion_sort(array[i], size);
        finish = clock();
        time = (double)(finish - start) / 1000;
        printf("(insertion sort : %f sec)\n", time);
        print_line(array[i], size);
        init_array(array[i], i);

        //heap sort
        start = clock();
        heap_sort(array[i], size);
        finish = clock();
        time = (double)(finish - start) / 1000;
        printf("(Heap sort : %f sec)\n", time);
        print_line(array[i], size);
        init_array(array[i], i);

        //quick sort
        start = clock();
        quick_sort(array[i], 0, size - 1);
        finish = clock();
        time = (double)(finish - start) / 1000;
        printf("(Quick sort : %f sec)\n", time);
        print_line(array[i], size);

        printf("-----------------------\n");
    }


    for(i = 0; i < 10; i++){
        free(array[i]);
    }
    free(array);

    return 0;
}

void insertion_sort(int *a, int size){
    for(int i = 1; i < size; i++){
        int key = a[i];
        int j = i - 1;
        while(j >= 0 && a[j] > key){
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
    return;
}

void heap_sort(int *a, int size){
    build_max_heap(a, size);
    for(int i = size - 1; i >= 0; i--){
        int temp = a[0];
        a[0] = a[i];
        a[i] = temp;
        size--;
        max_heapify(a, 0, size);
    }
    return;
}
void build_max_heap(int *a, int size){
    for(int i = size / 2 - 1; i >= 0; i--){
        max_heapify(a, i, size);
    }
    return;
}
void max_heapify(int *a, int i, int size){
    int left = i * 2 + 1;
    int right = i * 2 + 2;
    int max = i;
    if(left < size && a[left] > a[i])
        max = left;
    if(right < size && a[right] > a[max])
        max = right;
    if(max != i){
        int temp = a[i];
        a[i] = a[max];
        a[max] = temp;
        max_heapify(a, max, size);
    }
    return;
}

void quick_sort(int *a, int start, int end){
    if(start < end){
        int q = partition(a, start, end);
        quick_sort(a, start, q - 1);
        quick_sort(a, q + 1, end);
    }
    return;
}

int partition(int *a, int start, int end){
    int x = a[end];
    int i = start - 1;
    for(int j = start; j < end; j++){
        if(a[j] <= x){
            i = i + 1;
            int temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }
    int temp = a[i + 1];
    a[i + 1] = a[end];
    a[end] = temp;
    return i + 1;
}

void init_array(int *a, int i){
    srand(i);
    for(int j = 0; j < (i + 1) * 10; j++){
        a[j] = rand() % ((i + 1) * 20);
    }
    return;
}

void print_array(int **a){
    //array출력
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < (i + 1) * 10; j++){
            printf("%d ", a[i][j]);
        }
        printf("\n");
    }
    return;
}

void print_line(int *a, int size){
    for(int i = 0; i < size; i++){
        printf("%d ", a[i]);
    }
    printf("\n\n");
    return;
}