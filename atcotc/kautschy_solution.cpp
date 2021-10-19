#include<iostream>
#include<vector>
#include<queue>
#include<set>
#include<tuple>
#include<algorithm>
#include<queue>

using namespace std;

typedef priority_queue<long , vector<long>, greater_equal<long>> Min_Heap;



void print(vector<int> &i){
    for(int j: i){
        cout << j << " ";
    }
    cerr << "\n\n";
}



int one_itteration(vector<pair<long, long>> &jedi, long from, long to, int index, int start_index){ // between from and to it is forbidden to be, index is the jedi to chose next
    if(index == start_index) return 0;
    //take this jedi
    long a = jedi[index].first, b = jedi[index].second;
    bool taking_him = false;

    // figure out if it is allowed to take him
    // from being smaller than to
    if(from <= to && (
            (a < from && b < from && a <= b) ||
            (a > to && b < from) || 
            (a > to && b > to && a <= b))){
                from = a;
                taking_him = true;
    }
    // from being larger than to
    else if((from >= to) && 
            (a > to && a < from && b < from && b > to && b >= a)){
                from  = a;
                taking_him = true;
    }

    int new_index = (index > 0) ? index -1 : jedi.size()-1;
    if(taking_him){
        return 1 + one_itteration(jedi, from, to, new_index, start_index);
    }
    else{
        return one_itteration(jedi, from, to, new_index, start_index);
    }
}


/*
void translate_coordinate(pair<long, long>& j, long offset, long circle_size){
    if(j.first < offset){
        j.first = j.first + circle_size;
        j.second = j.second + circle_size;
    }
    else if(j.second < offset){
        j.second = j.second + circle_size;
    }
}


void sort_jedi(vector<pair<int, int>> &jedi, vector<pair< int, int>> &temp_jedi, int index, long circle_size){
    long offset = jedi[index].first;
    temp_jedi.clear();
    for(int i = index ; i < jedi.size(); i ++){
        temp_jedi.push_back(jedi[i]);
    }
    for(int i = 0; i< index; i++){
        // if a < offset and b > offset not take it
        pair<long, long> j = jedi[i];
        long a = j.first, b = j.second;
        if (( a < offset && b > offset) // |----a---offset---b---| 
            || ( a < offset && b < offset && b < a) //|----b---a---offset----|
            || ( a > offset && b > offset && a > b )){ //|----offset---b----a---|
            continue;
        }
        translate_coordinate(j, offset, circle_size);
        temp_jedi.push_back(j); 
    }

}

*/
long get_the_segments(vector<pair<long, long>>& jedi, vector<int>& interesting_indices, long m){
    Min_Heap heap;
    long segment = m-1;
    // prepare heap
    for(int i= 0; i < jedi.size(); i ++){
        if(jedi[i].second < jedi[i].first) heap.push(jedi[i].second);
    }
    for(int i = 0; i < jedi.size(); i++){

        long a = jedi[i].first, b =  jedi[i].second;
        if(a > b) b+= m; // shift b by m if it would go over the boundary
        // without this new element
        while(heap.top() < a - 1 && !heap.empty()){
            heap.pop();
        }
        if(heap.size() <= 10){
            segment = (a>0) ? a - 1 : m-1;
            //cout << "COUNTER " << heap.size() << "\n";
            interesting_indices.push_back(i);
            break;
        }
        // with this new element
        while(heap.top() < a && !heap.empty()){
            heap.pop();
        }
        heap.push(b);
    }


    for(int i = 0; i< jedi.size(); i++){
        long a = jedi[i].first, b = jedi[i].second;
        if( a <= b && a <=segment && b >= segment) interesting_indices.push_back(i); // |---a--i--b--|
        else if ( a > b){ // |-----b---a----|
            if (!(b < segment && a > segment)) { // !|-----b---i---a---|
                interesting_indices.push_back(i);
            }
        }
    }
    return segment;
}



int main(){
    ios_base::sync_with_stdio(false);
    int testcases; cin >> testcases;
    for(int t = 0; t< testcases; t++){

        int n, m; cin >> n >> m;
        vector<pair<long, long>> jedi(n);
        for (int i = 0; i < n; i ++){
            long a; cin >> a;
            long b; cin >> b;
 
            jedi[i] = make_pair(a, b);
        }

        sort(jedi.begin(), jedi.end());
        vector<pair<long, long>> tmep_jedi;

        vector<int> interesting;
        long segment = get_the_segments(jedi, interesting, m);
        //print(interesting);

        int max = 0;
        for( /*int i = 0; i < jedi.size(); i++){*/int i : interesting){
            long from = jedi[i].first, to = jedi[i].second;
            int new_index = (i > 0) ? i -1 : jedi.size()-1;
            int temp = one_itteration(jedi, from, to, new_index, i);
            if(temp >= max){
                max = temp + 1;
            }
        }
        int new_index = (interesting[0] > 0) ? interesting[0] -1 : jedi.size()-1;
        int temp = one_itteration(jedi, segment, segment, new_index, interesting[0]); // when not choosing any of the interesint ones
        max = (temp > max) ? temp : max;
        cout << max<< "\n";

    }
}

