#include <iostream>
#include <cstring>
using namespace std;
struct Node{
    string puzzle;
    struct Node *next;
    struct Node *parent;
};

struct Queue{
    Node *front, *rear;
    int len = 0;
    Queue(){
        front = rear = NULL;
    }
    int size(){
        return len;
    }
 
    bool empty(){
        return len == 0;
    }
 
    void push(string puz, Node *par){
        Node *node = new Node;
        node->puzzle = puz;
        node->parent = par;
        node->next = NULL;
        if (empty()){
            front = rear = node;
        }
        else{
            rear->next = node;
            rear = rear->next;
        }
        len++;
    }
 
    void pop(){
        Node *delNode = front;
        front = delNode->next;
        len--;
    }
    Node* q_front(){
        return front;
    }
    bool find_puzzle(string temp){
        Node *iter = front;
        for(int i = 0; i < size(); i++){
            if(iter->puzzle == temp){
                return false;
            }
            iter = iter->next;
        }
        return true;

    }
};

struct set{
    string puzzle;
    set *left;
    set *right;
};
struct Set{
    set *_head = NULL;
    Set(){
        _head = NULL;
    }
    void insert(string puz){
        set *temp = new set;
        set *iter = _head;
        temp->puzzle = puz;
        temp->left = NULL;
        temp->right = NULL;
        if(_head == NULL){
            _head = temp;
            return;
        }
        else{
            set *parent = NULL;
            while(iter != NULL){
                if(puz > iter->puzzle){
                    parent = iter;
                    iter = iter->right;
                }
                else if(puz < iter->puzzle){
                    parent = iter;
                    iter = iter->left;
                }
                else
                    break;
            }
            if(puz > parent->puzzle)
                parent->right = temp;
            else
                parent->left = temp;
        }
    }
    bool find(string puz){
        set *iter = _head;
        while(iter != NULL){
            if(puz == iter->puzzle)
                return true;
            if(puz > iter->puzzle)
                iter = iter->right;
            else if(puz < iter->puzzle)
                iter = iter->left;
        }
        return false;
    }
    set *head(){
        return _head;
    }
};


string state[3] = {"132078564", "370162548", "756342108"};
string final = "123804765";

int find_position(string state)
{
    for(int i = 0; i < state.size(); i++){
        if(state[i] == '0')
            return i;
    }
    return -1;
}
void print(string state){
    for(int i = 0; i < state.size(); i++){
        cout << state[i] << " ";
        if(i % 3 == 2)
            cout << "\n";
    }
    return;
}
string swap_puzzle(string check, int ind, int change_ind){
    char temp = check[ind];
    check[ind] = check[change_ind];
    check[change_ind] = temp;
    return check;
}

int main(){
    int result = 0;
    int isfind = 0;

    for(int i = 0; i < 3; i++){
        string puzzle = state[i];
        cout << "-------------\n";
        cout << i + 1 << " puzzle\n";
        Queue q;
        Set done;
        result = 0;
        isfind = 0;
        q.push(puzzle, NULL);
        done.insert(puzzle);
        // 같은 level node 검사하기
        while(!q.empty()){
            int size = q.size();
            for(int j = 0; j < size; j++){
                Node *check = q.q_front();
                q.pop();
                // 만약 완료 됐으면 끝
                if(check->puzzle == final){
                    cout << "how many move the puzzle? " << result << "\n";
                    for(int level = result + 1; level > 0; level--){
                        Node *temp_Node = check;
                        for(int temp_level = 0; temp_level < level - 1; temp_level++)
                            temp_Node = temp_Node->parent;
                        print(temp_Node->puzzle);
                        cout << "\n";
                    }

                    isfind = 1;
                    break;
                }
                // 0 찾기
                int ind = find_position(check->puzzle);
                int row, col;
                // row값 찾기
                if(ind == 0)
                    row = 0;
                else
                    row = ind / 3;
                // col값 찾기
                if(ind == 0)
                    col = 0;
                else   
                    col = ind % 3;
                // 위치 바꾸고 큐에 넣기
                string temp;
                string temp1 = check->puzzle;
                Node *parent = check;
                if(ind - 3 >= 0){
                    temp = swap_puzzle(temp1, ind, ind - 3);
                    // 없었던 값이면 set에 넣기
                    if(!done.find(temp)){
                        q.push(temp, parent);
                        done.insert(temp);
                    }
                }
                if(ind - 1 >= 0 && (ind - 1) / 3 == row){
                    temp = swap_puzzle(temp1, ind, ind - 1);
                    // 없었던 값이면 set에 넣기
                    if(!done.find(temp)){
                        q.push(temp, parent);
                        done.insert(temp);
                    }
                }
                if(ind + 1 <= 8 && (ind + 1) / 3 == row){
                    temp = swap_puzzle(temp1, ind, ind + 1);
                    // 없었던 값이면 set에 넣기
                    if(!done.find(temp)){
                        q.push(temp, parent);
                        done.insert(temp);//여기서 문제
                    }
                }
                if(ind + 3 <= 8){
                    temp = swap_puzzle(temp1, ind, ind + 3);
                    // 없었던 값이면 set에 넣기
                    if(!done.find(temp)){
                        q.push(temp, parent);
                        done.insert(temp);
                    }
                }
            }
            //찾은게 있으면 다음 puzzle풀기
            if(isfind == 1)
                break;
            //깊이
            result++;
        }
        if(isfind == 0)
            cout << "there is no path back to the correct state\n";
    }
    cout << "finished\n";
    
    return 0;
}