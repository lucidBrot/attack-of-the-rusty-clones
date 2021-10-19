/*
 *  atc5 became too ugly. No attempt, this time with sorting pointers.
 *  at the time of writing, this one still has not always correct results though...
 *
 *  Runtime Analysis:
 *      reading all jedi: O(n)
 *      sorting them: O(n *log(n) )
 *      update indices: O(n)
 *      find a good starting point: O(n)
 *      looping through the circle: n*<limited_number>
 *          the limited number is bigger when there are more overlaps,
 *          but in turn they are quickly skipped in future iterations because
 *          every considered interval is either marked as taken or as not taken except for the case
 *          where we notice an interval overlap later. but that's still about O(n) on average i think
 *
 *  Faster Ideas:
 *      storing whether an interval collides with a different interval
 *          would be more useful with indices as keys, instead of the interval id. But creating
 *          a hash function is hard. There can be up to 2^36 segments. so the map must be nested
 *          since it only accepts a size_t as hash.
 *          --> TURNS OUT THIS IS SLOWER THAN NOT STORING
 *
 *      finding a good starting point:
 *          they guarantee us one(1) segment with 'few' jedi. We could find that in 
 *          linear time, going through the list of start/end and increase/decrease a counter.
 *          The use of that is that in the following segment there will be 'few' jedi incoming from the left.
 *          Only take the first ending of those starting on the following segment as well.
 *          That should be a constant number of looping through the circle.
 *
 *  Correctness Ideas:
 *      * use a std::reference_wrapper instead of the reference directly
 *      * use ulong instead of uint.
 */
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <utility>
#include <tuple>

typedef unsigned long ulong;
static bool debug = false;
// using the storage is actually slower by around 30-40 seconds.
static const bool use_storage = false;

template <class smthing>
void dbgvec(std::vector<smthing>& v){
    if(debug){
        std::cout << "[ ";
        for (auto entry : v){
            std::cout << entry << " ";
        }
        std::cout << "]\n";
    }
}

class Storage {
    public:
        std::unordered_map<unsigned long, std::unordered_map<unsigned long, 
            std::unordered_map<unsigned long, std::unordered_map<unsigned long, bool>>>> overlaps;
        Storage(){}
        bool get(ulong s1, ulong e1, ulong s2, ulong e2, bool& out_overlaps){
            auto map1 = overlaps.find(s1);
            if (map1 == overlaps.end())
                return false;
            auto map2 = map1->second.find(e1);
            if (map1->second.end() == map2)
                return false;
            auto map3 = map2->second.find(s2);
            if (map2->second.end() == map3)
                return false;
            auto map4 = map3->second.find(e2);
            if (map3->second.end() == map4)
                return false;
            
            out_overlaps = map4->second;
            return true;
        }
        void store(ulong s1, ulong e1, ulong s2, ulong e2, bool val){
            overlaps[s1][e1][s2][e2] = val;
        }
};

bool does_overlap(Storage& s, ulong s1, ulong e1, ulong s2, ulong e2, ulong m){
    // check if stored
    if (use_storage){
        bool out;
        if(s.get(s1, e1, s2, e2, out)){
            return out;
        }
    }
    bool u = use_storage;
    auto ret = [&s, s1, e1, s2, e2, u](bool v){
        if(u){s.store(s1, e1, s2, e2, v);} return v;};

    if (s1 <= e1 && s2 <= e2){
        if (s1 < s2){
            if (s2 <= e1){
                return ret(true);
            } else {
                return ret(false);
            }
        } else {
            if (s1 == s2){
                return ret(true);
            } else { // s1 > s2
                if (s1 <= e2){
                    return ret(true);
                } else {
                    return ret(false);
                }
            }
        }
    } else { // edge cases where the interval wraps around
        if (s1 > e1) {
            // check both subintervals
            bool overlap = 
                does_overlap(s, s1, m-1, s2, e2, m)
                ||
                does_overlap(s, 0, e1, s2, e2, m);
            return ret(overlap);
        } else {
            if (s2 > e2){
                return ret(does_overlap(s, s1, e1, s2, m-1, m) || does_overlap(s, s1, e1, 0, e2, m));
            }
            assert(false && "this should never happen because s2 must be greater than e2 here.");
        }
    }
}

struct JediData {
    ulong start, end;
    ulong last_turn = 0;
    ulong ignored;
    ulong id;
    ulong startindex, endindex;
    bool considered(ulong turn){ return last_turn == turn; }
    bool taken(ulong turn){ return considered(turn) && !ignored; }
    bool is_ignored(ulong turn){return considered(turn) && ignored; }
    void take(ulong turn){ last_turn = turn; ignored = false; }
    void ignore(ulong turn) { last_turn = turn; ignored = true; }
};

struct JediEntry {
    std::reference_wrapper<JediData> data_wrapper;
    bool is_start;
    ulong bucket_start_index;
    ulong index;
    bool init_seen;
    ulong value(){ return (is_start ? data().start : data().end); }
    JediData& data(){ return data_wrapper.get(); }
    JediEntry(JediData& d, bool start):data_wrapper(d), is_start(start){};
    JediEntry& operator= (const JediEntry& other){
        if (this == &other){
            return *this;
        } else {
            data_wrapper = other.data_wrapper;
            is_start = other.is_start;
            bucket_start_index = other.bucket_start_index;
            index = other.index;
            return *this;
        }
    }
};

JediEntry& start(std::vector<JediEntry>& v, JediEntry& e){ return v[e.data().startindex]; }
JediEntry& start(std::vector<JediEntry>& v, ulong i){ return v[v[i].data().startindex]; }
JediEntry& end(std::vector<JediEntry>& v, JediEntry& e){ return v[e.data().endindex]; }
JediEntry& end(std::vector<JediEntry>& v, ulong i){ return v[v[i].data().endindex]; }

int main(){
    Storage s;
    // tests
    assert(does_overlap(s, 1, 2, 2, 3, 100) == true);
    assert(does_overlap(s, 1, 2, 3, 3, 100) == false);
    assert(does_overlap(s, 3, 3, 3, 3, 100) == true);
    assert(does_overlap(s, 2, 3, 1, 1, 100) == false);
    assert(does_overlap(s, 3, 1, 1, 1, 100) == true);
    assert(does_overlap(s, 3, 2, 1, 1, 100) == true);
    assert(does_overlap(s, 3, 2, 1, 2, 100) == true);
    assert(does_overlap(s, 3, 2, 2, 1, 100) == true);
    assert(does_overlap(s, 1, 10, 2, 4, 100) == true);
    assert(does_overlap(s, 3, 5, 6, 2, 100) == false);
    assert(does_overlap(s, 6, 2, 3, 5, 100) == false);
    assert(does_overlap(s, 1, 5, 6, 2, 100) == true);
    assert(does_overlap(s, 6, 2, 1, 5, 100) == true);

    //std::ios_base::sync_with_stdio(false);
    ulong tt; std::cin >> tt;
    while (tt--){
        ulong n, m; std::cin>> n >> m; // n: jedi, m: segments
        std::vector<JediEntry> jedi_entries; jedi_entries.reserve(2*n);
        std::vector<JediData> jedi_data; jedi_data.reserve(n);
        for (ulong i=0; i<n; i++){
            ulong a, b; std::cin >> a >> b;
            a = a % m; b = b % m;
            jedi_data.push_back(JediData{a, b, 0, false, i, 0, 0});
            jedi_entries.push_back(JediEntry(jedi_data[i], true));
            jedi_entries.push_back(JediEntry(jedi_data[i], false));
        }

        if (debug){
            std::cout << "     The complete jedi list before sorting: [ ";
            for (ulong i=0; i<jedi_entries.size(); i++){
                std::cout << (jedi_entries[i].is_start ? "s(" : "e(") << jedi_entries[i].data().id << ": " << jedi_entries[i].data().start << "," << jedi_entries[i].data().end << ") ";
            }
            std::cout << "] " << std::endl;
        }

        // sort jedi, because we will need them sorted anyway.
        std::sort(jedi_entries.begin(), jedi_entries.end(), [](JediEntry& j, JediEntry& other){
                    return j.value() < other.value();
                });

        if (debug){
            std::cout << "     The complete jedi list after sorting: [ ";
            for (ulong i=0; i<jedi_entries.size(); i++){
                std::cout << (jedi_entries[i].is_start ? "s(" : "e(") << jedi_entries[i].data().id << ": " << jedi_entries[i].data().start << "," << jedi_entries[i].data().end << ") ";
            }
            std::cout << "] " << std::endl;
        }

        // update indices. So that we can jump from one end of the interval to the other.
        jedi_entries[0].bucket_start_index = 0;
        for (ulong i=0; i<jedi_entries.size(); i++){
            // store corresponding index
            jedi_entries[i].index = i;
            if (jedi_entries[i].is_start)
                jedi_entries[i].data().startindex = i;
            if (!jedi_entries[i].is_start)
                jedi_entries[i].data().endindex = i;

            // store first of this value as bucketindex
            if (i!=0 && jedi_entries[i].value() != jedi_entries[i-1].value()){
                jedi_entries[i].bucket_start_index = i;
            }
            else if (i!=0 && jedi_entries[i].value() == jedi_entries[i-1].value()){
                jedi_entries[i].bucket_start_index = jedi_entries[i-1].bucket_start_index;
            }
        }

        // it is guaranteed that there is a segment S which belongs to at most 10 jedi.
        // so in the segment that follows it, there is one(1) best choice (the edf) and at most 10 
        // contenders from the left. I have to make sure that this sum is not zero.
        // Going to find it in linear time.
        //
        // I want to find the last index of the bucket that has fewest jedi and then search to the next end and use that as a start. Then search backwards from there to find all starts without corresponding end - these were active and need to be checked too.
        // That is still linear time (and linear memory).
        // To find the bucket with the fewest jedi, I can sum them up and store a pointer to the
        // minimum.
        std::vector<ulong> the_ten_overlaps;
        long long num_jedis_active=0; // could be negative
        long long min_num_jedis_active = n + 1; ulong starter_index = 0;
        for (ulong i=0; i<jedi_entries.size(); i++){
            JediEntry& entry = jedi_entries[i];
            JediEntry& next_entry = jedi_entries[(i + 1) % jedi_entries.size()];
            if (entry.is_start){
                num_jedis_active++;
            } 
            
            if (entry.value() != next_entry.value()){
                // this is the last entry of the current segment
                if (num_jedis_active < min_num_jedis_active){
                    min_num_jedis_active = num_jedis_active;
                    starter_index = i; 
                }
            }
            if (!entry.is_start){
                num_jedis_active--;
            }
        } 
        ulong min_bucket_index = starter_index;
        // search forward until EDF
        do {
            starter_index = (starter_index + 1) % jedi_entries.size();
        } while (jedi_entries[starter_index].is_start);
        the_ten_overlaps.push_back(starter_index);
        // search backwards
        for (ulong i=0; i<jedi_entries.size(); i++){
            ulong index = (min_bucket_index + jedi_entries.size() - i) % jedi_entries.size();
            // if is end, mark start as seen
            // if is start of which end was not seen, mark start as seen and add as active jedi
            JediEntry& entry = jedi_entries[index];
            if (entry.is_start){
                if (!entry.init_seen){
                    the_ten_overlaps.push_back(index);
                }
            } else {
                start(jedi_entries, entry).init_seen = true;
            }
        }
        assert(the_ten_overlaps.size() < 11);

        if (debug){
            std::cout << " --- " << n << " jedi, " << m<< " segments ---\n";
            std::cout << "     Found " << the_ten_overlaps.size() << " starter jedi indices to consider:\n";
            std::cout << "     "; dbgvec(the_ten_overlaps);
            std::cout << "     The complete jedi list: [ ";
            for (ulong i=0; i<jedi_entries.size(); i++){
                std::cout << (jedi_entries[i].is_start ? "s(" : "e(") << jedi_entries[i].data().id << ": " << jedi_entries[i].data().start << "," << jedi_entries[i].data().end << ") ";
            }
            std::cout << "] " << std::endl;

            std::cout << "     The complete value list: [ ";
            for (ulong i=0; i<jedi_entries.size(); i++){
                std::cout << (jedi_entries[i].value()) << ", ";
            }
            std::cout << "] " << std::endl;
        }

        // now circle with each of the overlaps as starting values.
        // jedi vector is sorted by EDF.
        ulong turn = 0; ulong max_taken=0;
        for (ulong start_index : the_ten_overlaps){
            turn++;
            ulong taken = 0;
            if(debug) { std::cout << "New Turn: " << turn << std::endl; }
            // look through list from jedi_start to jedi_end.
            // everything in between, no matter if start or end, is overlapping.
            // for the start, equality with another start also means collison
            //  and equality with another end not.
            // for the end, equality with another start is not collision,
            //  and equality with another end is.
            //
            // Then pick the next end that is not taken or ignored and try again.

            JediEntry& starting_end = end(jedi_entries, start_index);
            ulong starting_end_index = starting_end.index;
            ulong N = jedi_entries.size();
            ulong next_end_index = starting_end_index;
            do {
                JediEntry& je = jedi_entries[next_end_index];
                // for finding the earliest end, consider the ends only.
                if (je.is_start){
                    if (debug) {std::cout << "  " << next_end_index << "(id=" << je.data().id << ") is a start." << std::endl; }
                    next_end_index = (next_end_index + 1) % N;
                    continue;
                }

                if (debug) { std::cout << "  Loop Start with next_end_index = " << next_end_index 
                    << "(id=" << jedi_entries[next_end_index].data().id <<") and num entries is " << N << std::endl; }
                // endofloop: find next end that is neither ignored nor taken.
                // But the check for the ending condition should always be performed as well
                // so we actually check at the start of the loop and skip the whole loop
                // if it's taken or ignored.
                if (je.data().taken(turn) || je.data().is_ignored(turn)){
                    next_end_index = (next_end_index + 1) % N;
                    if (debug) { std::cout << "    skipping." << std::endl; }
                    continue;
                }
                
                // check all that might overlap, generously, by using the bucket index
                // If there is an overlap that is taken, we can't take the current one.
                // So we first check, and then commit.
                std::vector<ulong> checks;
                bool can_be_taken = true;
                JediEntry& je_start = start(jedi_entries, je);
                JediEntry& je_end   =   end(jedi_entries, je);
                // allow the same start, but stop at the same end
                for (ulong index=je_start.bucket_start_index;
                        (jedi_entries[index].data().id != je_end.data().id || jedi_entries[index].is_start) && can_be_taken == true;
                        index = (index + 1) % N){
                    // CHECK
                    bool overlap = does_overlap(s, je.data().start, je.data().end,
                            start(jedi_entries, index).value(), end(jedi_entries, index).value(), m);
                    if (overlap){
                        checks.push_back(index);
                        if ( jedi_entries[index].data().taken(turn) ){
                            can_be_taken = false;
                            if (debug) { std::cout << "    already-taken overlap with " << index << "(id=" << jedi_entries[index].data().id << ")" << std::endl;}
                        }
                    }
                }
                uint total = 0;
                for (ulong index = je_end.index;
                        jedi_entries[index].value() == je_end.value() && can_be_taken == true && total < N+2;
                        index = (index + 1) % N){
                    // CHECK
                    total++; // safeguard against endless loops
                    bool overlap = does_overlap(s, je.data().start, je.data().end,
                            start(jedi_entries, index).value(), end(jedi_entries, index).value(), m);
                    if (overlap){
                        checks.push_back(index);
                        if ( jedi_entries[index].data().taken(turn) ){
                            can_be_taken = false;
                            if (debug) { std::cout << "    already-taken overlap with " << index << "(id=" << jedi_entries[index].data().id << ")" << std::endl;}
                        }
                    }
                }

                // commit: mark as ignored / taken based on overlaps
                if ( can_be_taken ){
                    for (ulong index : checks){
                        JediEntry& entry = jedi_entries[index];
                        if (entry.data().id != je.data().id){
                            entry.data().ignore(turn);
                            if(debug) { std::cout << "      ignore " << index << "(id=" << entry.data().id << ")"<< std::endl; }
                        }
                    }
                    je.data().take(turn);
                    taken++;
                    if (debug) { std::cout << "    Taking current interval " << je.data().id << std::endl; }

                } else {
                    je.data().ignore(turn);
                    if(debug){ std::cout << "    Ignoring current interval " << je.data().id << " because of a lingering overlap" << std::endl;}
                }


                // set next index to modulo num_jedi
                next_end_index = (next_end_index + 1) % N;

            } while ( next_end_index != starting_end_index );

            if (debug) { std::cout << "current maximum: " << taken << ", stored at the time is: " << max_taken << std::endl;}
            if ( taken > max_taken ){
                max_taken = taken;
            }

            // can't take more jedi than there are jedi
            if (max_taken == n){
                break;        
            }

            // can't take more jedi than there are segments
            if (max_taken == m){
                break;
            }
        }
        std::cout << max_taken << '\n';

    }
    return 0;
}
