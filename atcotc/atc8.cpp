/*
 *  Redo AGAIN.
 *  This time no explicit storing of segments, not even temporarily to find the interesting ones.
 *
 *  Using a map instead to keep track of the interesting segments only.
 *
 */
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <queue>
#include <map>
#include <climits>
#include <deque>

class Jedi {
    public:
    uint id;
    int start;
    int end;
    Jedi(uint id, int start, int end):id(id), start(start), end(end){}
    Jedi(){}
};

void testcase(){
    uint n, m; std::cin >> n >> m;
    std::vector<Jedi> jedis; // abusing language to make a plural of "jedi" 
                                 // that is distinguishable from singular "jedi"

    // fill a map of all relevant segments and their total increase/decrease in overlapping jedi
    std::map<int, int> derivative;
    for (uint i=0; i<n; i++){
        int start, end; std::cin >> start >> end;
        jedis.push_back(Jedi(i, start, end));
        derivative[start]++;
        //derivative[(end+1<m)? end+1 : 1]--; // end still is on the current one
        derivative[end]--; // keep track of the state right after the segment

        // Problem: Jedi that already have started are not counted if we were to iterate over the
        //          derivatives now
        // Solution: Count all Jedi that are wrapping around.
        // (reminder that segments start with index 1, not 0)
        derivative[1] += (end < start);
    }

    // find the segment with the least overlaps. All relevant segments are indices in our map.
    // and the map is sorted.
    // The emptiest segment will not have zero intervals, because we only look 
    // at jedi-occupied intervals.
    int counter = 0; int smallest_counter = INT_MAX; int emptiest_segment=0;
    for ( auto p : derivative ){
        int seg = p.first;
        int val = p.second;

        counter += val;
        if ( counter < smallest_counter && counter > 0 ){
            smallest_counter = counter;
            emptiest_segment = seg;
        }
    }
    emptiest_segment++;

    // shift the segment numbers so that the smallest number is where we start.
    // This makes it easier to know when you're done looping around the end.
    for ( Jedi& jedi : jedis ){
        jedi.start -= emptiest_segment;
        jedi.end -= emptiest_segment;
        if (jedi.start < 0)
            jedi.start += m;
        if (jedi.end < 0)
            jedi.end += m;
    }

    // Make a list of the overlaps at the emptiest segment
    // (reminder: it is not zero. There is at least one interval)
    std::vector<Jedi> initial_jedi;
    for ( Jedi& jedi : jedis ){
        if ( jedi.start <= jedi.end  &&
                jedi.start <= emptiest_segment &&
                jedi.end >= emptiest_segment ){
            initial_jedi.push_back(jedi);
        }
        if ( jedi.start > jedi.end &&
                ( jedi.start <= emptiest_segment || jedi.end >= emptiest_segment ) 
            ){
            initial_jedi.push_back(jedi);
        }
    }

    // sort jedi by end
    std::sort(jedis.begin(), jedis.end(), [](Jedi& a, Jedi& b){return a.end < b.end;} );

    // every of those initial jedi is potentially taken.
    // Potential Speedup: only keep the soonest end of the ones with start >= 0
    // So we now start with each once and keep the best.
    int best_num_taken = 0;
    for (Jedi& initial : initial_jedi){
        // create a copy of the sorted jedi
        std::deque<Jedi> myjedi (jedis.begin(), jedis.end());
        int largest_end = initial.end;
        int num_taken = 1;
        // pop until the interval can be taken without overlapping.
        // Since we already have at least one interval taken, nothing else can sneak in from the
        // left.
        while (!myjedi.empty()){ 
            if ( myjedi.front().end <= largest_end || myjedi.front().end < myjedi.front().start 
                    || myjedi.front().start >= initial.start) {
                myjedi.pop_front();
            } else {
                // take this jedi
                largest_end = myjedi.front().end;
                num_taken++;
                myjedi.pop_front();
            }
        }
        // now figure out if that is better than the other initial choices
        best_num_taken = std::max(best_num_taken, num_taken);
    }

    std::cout << best_num_taken << '\n';
    
}   

int main(){
    std::ios_base::sync_with_stdio(false);
    uint tt; std::cin >> tt;
    for (uint t=0; t<tt; t++){
        testcase();
    }
    return 0;
}
