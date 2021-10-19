#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>

typedef unsigned long ulong;
static bool debug = true;

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

bool does_overlap(uint s1, uint e1, uint s2, uint e2, uint m){
    if (s1 <= e1 && s2 <= e2){
        if (s1 < s2){
            if (s2 < e1){
                return true;
            } else {
                return false;
            }
        } else {
            if (s1 == s2){
                return true;
            } else { // s1 > s2
                if (s1 <= e2){
                    return true;
                } else {
                    return false;
                }
            }
        }
    } else { // edge cases where the interval wraps around
        if (s1 > e1) {
            // check both subintervals
            bool overlap = 
                does_overlap(s1, m-1, s2, e2, m)
                ||
                does_overlap(0, e1, s2, e2, m);
            return overlap;
        } else {
            if (s2 > e2){
                return does_overlap(s1, e1, s2, m-1, m) || does_overlap(s1, e1, 0, e2, m);
            }
            assert(false && "this should never happen because s2 must be greater than e2 here.");
        }
    }
}

struct Overlap{
    bool does_overlap = false;
    bool is_computed = false;
};

struct JediData {
    uint start, end;
    uint last_turn = 0;
    uint ignored;
    uint id;
    bool considered(uint turn){ return last_turn == turn; }
    bool taken(uint turn){ return considered(turn) && !ignored; }
    bool is_ignored(uint turn){return considered(turn) && ignored; }
    void take(uint turn){ last_turn = turn; ignored = false; }
    void ignore(uint turn) { last_turn = turn; ignored = true; }
};

struct JediEntry {
    JediData& data;
    bool is_start;
    uint value(){ return (is_start ? data.start : data.end); }
    JediEntry(JediData& d, bool start):data(d),is_start(start){};
    JediEntry& operator= (const JediEntry& other){
        if (this == &other){
            return *this;
        } else {
            data = other.data;
            return *this;
        }
    }
};

int main(){
    // tests
    assert(does_overlap(1, 2, 2, 3, 100) == false);
    assert(does_overlap(1, 2, 3, 3, 100) == false);
    assert(does_overlap(3, 3, 3, 3, 100) == true);
    assert(does_overlap(2, 3, 1, 1, 100) == false);
    assert(does_overlap(3, 1, 1, 1, 100) == false);
    assert(does_overlap(3, 2, 1, 1, 100) == true);
    assert(does_overlap(3, 2, 1, 2, 100) == true);
    assert(does_overlap(3, 2, 2, 1, 100) == true);
    assert(does_overlap(1, 10, 2, 4, 100) == true);

    //std::ios_base::sync_with_stdio(false);
    uint tt; std::cin >> tt;
    while (tt--){
        uint n, m; std::cin>> n >> m; // n: jedi, m: segments
        std::vector<JediEntry> jedi_entries; jedi_entries.reserve(2*n);
        std::vector<JediData> jedi_data; jedi_data.reserve(n);
        for (uint i=0; i<n; i++){
            uint a, b; std::cin >> a >> b;
            a = a % m; b = b % m;
            jedi_data.push_back(JediData{a, b, 0, false, i});
            jedi_entries.push_back(JediEntry(jedi_data[i], true));
            jedi_entries.push_back(JediEntry(jedi_data[i], false));
        }

        // sort jedi, because we will need them sorted anyway.
        std::sort(jedi_entries.begin(), jedi_entries.end(), [](JediEntry& j, JediEntry& other){
                    return j.value() < other.value();
                });

        // it is guaranteed that there is a segment which belongs to at most 10 jedi.
        // let's find it. 10 should be good enough.
        std::vector<uint> the_ten_overlaps;
        for (uint i=0; i<jedi_entries.size()-1; i++){
            if (!jedi_entries[i].is_start)
                continue;

            uint cnt_overlaps = 0;
            the_ten_overlaps.clear();
            the_ten_overlaps.push_back(i);
            // TODO: optimize this checking by only considering the ones close-by
            for (uint j=0; j<jedi_entries.size(); j++){
                if (j == i || !jedi_entries[j].is_start)
                    continue;

                bool ov = does_overlap(jedi_entries[i].data.start, jedi_entries[i].data.end,
                                        jedi_entries[j].data.start, jedi_entries[j].data.end, m);
                if (ov) { 
                    cnt_overlaps++;
                    the_ten_overlaps.push_back(j);
                }
                if (cnt_overlaps >= 11){
                    break;
                }
            }
            if (cnt_overlaps < 11){
                break;
            }
        }
        assert(the_ten_overlaps.size() <= 11);
        if (debug){
            std::cout << " --- " << n << " jedi, " << m<< " segments ---\n";
            std::cout << "     Found " << the_ten_overlaps.size() << " starter jedi to consider:\n";
            std::cout << "     "; dbgvec(the_ten_overlaps);
            std::cout << "     The complete jedi list: [ ";
            for (uint i=0; i<jedi_entries.size(); i++){
                std::cout << (jedi_entries[i].is_start ? "s(" : "e(") << jedi_entries[i].data.id << ": " << jedi_entries[i].data.start << "," << jedi_entries[i].data.end << ") ";
            }
            std::cout << "] " << std::endl;

            std::cout << "     The complete value list: [ ";
            for (uint i=0; i<jedi_entries.size(); i++){
                std::cout << (jedi_entries[i].value()) << ", ";
            }
            std::cout << "] " << std::endl;
        }

        // now circle with each of the overlaps as starting values.
        // jedi vector is sorted by EDF.
        uint turn = 0; uint max_taken=0;
        for (uint start_index : the_ten_overlaps){
            turn++;
            if(debug){std::cout << "New turn!" << std::endl; }
            // look through list from jedi_start to jedi_end.
            // everything in between, no matter if start or end, is overlapping.
            // for the start, equality with another start also means collison
            //  and equality with another end not.
            // for the end, equality with another start is not collision,
            //  and equality with another end is.

            JediEntry& start = jedi_entries[start_index];
            JediData& jd = start.data;
            uint jd_id = jd.id;
            uint next_index = start_index;
            JediData& next_jedi = jd;
            uint taken = 0;

            // first, pick the starting jedi and mark all conflicting as conflicting
            // then choose the next ending jedi that is not conflicting
            while (taken==0 || next_jedi.id != jd_id){
                next_index = (next_index + jedi_entries.size()) % jedi_entries.size();
                // check for overlaps
                std::vector<uint> overlaps;
                // roll backwards
                long ni = (next_index-1+jedi_entries.size()) % jedi_entries.size(); 
                while (jedi_entries[ni].value() == jedi_entries[next_index].value() &&
                        next_index!=ni){
                    if(debug){std::cout << "ni=" << ni << ",\tnext_index=" << next_index << ",\tjedi_entries.size()=" << jedi_entries.size() << ",\tni_value="<<jedi_entries[ni].value()<<",\tnext_index_value="<<jedi_entries[next_index].value()<< std::endl;}
                    ni = (ni -1 + jedi_entries.size()) % jedi_entries.size(); }
                if(debug){std::cout << "rolled ni back to " << ni << std::endl;}
                // test for collisions
                bool take_this = true;
                auto stop_cond = [&next_jedi, n](uint val, uint id){return (next_jedi.end < jedi_entries[jedi_entries.size()-1]) ? (val <= next_jedi.end) : (val != 0 || id == next_);}; // TODO what if val already starts at 0?
                for (uint index = (ni + 1) % (long)jedi_entries.size();
                        stop_cond(jedi_entries[index].value());
                        index = (index + 1)%jedi_entries.size()){
                    if (debug){
                        std::cout << "index=" << index << ",\tjedi_entries[index].value()="<<jedi_entries[index].value()<<",\tni="<<ni<<",\tnext_jedi.end="<<next_jedi.end<<",\tjedi_entries.size()="<<jedi_entries.size()<<std::endl;
                    }
                    if (jedi_entries[index].data.id == next_jedi.id)
                        continue;
                    if (debug) {std::cout << "checking for overlaps at " << next_index << std::endl;}
                    if (does_overlap(jedi_entries[index].data.start, jedi_entries[index].data.end,
                                next_jedi.start, next_jedi.end, m)){
                        // if it is overlapping and the other one is already taken, don't take this one
                        overlaps.push_back(index);
                        if (debug) {std::cout << "curr_ind=" << next_index << ": overlap found at " << index << " with id " << jedi_entries[index].data.id << std::endl;}
                        if (jedi_entries[index].data.taken(turn)){
                            if (debug) {std::cout << "The other one is already taken. don't take this." << std::endl; }
                            take_this = false;
                            next_jedi.ignore(turn);
                            break;
                        }

                    }
                }
                if (take_this){
                    // pick this one
                    if (debug) {std::cout << next_jedi.id << " taken.\n" << std::endl;}
                    next_jedi.take(turn); taken++;

                    for (uint index : overlaps){
                        jedi_entries[index].data.ignore(turn);
                        if(debug) {std::cout << jedi_entries[index].data.id << " ignored." << std::endl; }
                    }
                } else {
                    if (debug) { std::cout << next_jedi.id << " not taken.\n" << std::endl;}
                }

                // next iteration: take the next end that is not yet ignored
                uint just_covered_id = next_jedi.id;
                uint infinity_guard = 0;
                do {
                    next_index = (next_index + 1) % jedi_entries.size();
                    next_jedi = jedi_entries[next_index].data;
                    if(debug) { std::cout << "Increased next_index to " << next_index << " with id " << next_jedi.id << std::endl; }
                    infinity_guard++;
                } while ( infinity_guard <= jedi_entries.size() &&
                        (
                         jedi_entries[next_index].is_start 
                        || jedi_entries[next_index].data.is_ignored(turn)
                        || jedi_entries[next_index].data.id==just_covered_id
                        )
                        );
                if (debug){
                    std::cout << "Starting(?) next loop with next_index=" << next_index << " and jedi id " << next_jedi.id << " (jd.id = " << jd_id << ")" << std::endl;
                }
            }

            max_taken = (max_taken > taken) ? max_taken : taken;
            if (max_taken == n){
                break; // best reached.
            }

        }

        std::cout << max_taken << std::endl;

    }
    return 0;
}
