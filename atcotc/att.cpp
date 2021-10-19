/* 
 * !! This is Too Slow because I could use Range checks instead of validating each one !!
 * It's also wrong, sometimes...
 *
 * Third Attempt. After attac.cpp.
 * Idea:
 *
 * At every iteration step, consider the jedi interval who ends soonest. if it does not collide, take it. 
 * Retry for every possible starting position - i.e. at every jedi start.
 */

#include <iostream>
#include <algorithm>
#include <vector>
//#define debug true

class Jedi {
public:
    uint a, b; // zero-based
    Jedi(uint a, uint b): a(a), b(b) {}
};

int main(){
    std::ios_base::sync_with_stdio(false);
    uint tt; std::cin >> tt;
    for (uint t=0; t<tt; t++){
        uint n, m;
        std::cin >> n >> m;
        std::vector<Jedi> jedi; jedi.reserve(n);

        for (uint i = 0; i<n; i++){
            uint ao, bo; std::cin >> ao >> bo;
            uint a = ao - 1; uint b = bo - 1;
            jedi.push_back(Jedi(a, b));
        }

        // sort by end of interval
        std::sort(jedi.begin(), jedi.end(), [](Jedi& a, Jedi& b){
                if (a.b != b.b){
                return a.b < b.b; 
                } else {
                // secondarily sort by length. Shorter length first.
                return b.a < a.a;
                }
                });

        // find out which segment has the least jedis
        std::vector<uint> segs(m);
        uint min_seg = 0; bool initialized = false;
        for (uint i=0; i<n; i++){
            Jedi& jed = jedi[i];
            for (uint a = jed.a; a%m < jed.b; a = (a+1)%m){
                segs[a]++;
            }
        }
        for (uint i=0; i<m; i++){
#if debug
            std::cout << "        [" << segs[i] << "]" <<  std::endl;
#endif
            if ((!initialized || segs[i] <= segs[min_seg] ) && segs[i] >= 1){
                min_seg = i;
                initialized = true;
            }
        }

        // find starting jedi
        std::vector<uint> start_indexes;
        for (uint i = 0; i<n; i++){
            if ((jedi[i].a)%m <= min_seg && (jedi[i].b)%m >= min_seg){
                start_indexes.push_back(i);
            }
        }
#if debug
        std::cout << "[DEBUG]: found " << start_indexes.size() << " starting jedi" << std::endl;
#endif

        // running multiple times for different start positions that overlap min_seg
        uint taken_jedi = 0;
        for (uint run : start_indexes){
            std::vector<bool> segment_taken (m);
            uint takens = 0; uint takenj = 0;
            // pick the soonest end jedi until we're around the circle
            for (uint i=0; i<n; i++){
                uint index = (run + i) % n;
                Jedi& jed = jedi[index];
                bool can_be_taken = true;
                for (uint ja = jed.a; ja != (jed.b+1)%m; ja = (ja + 1) % m){
                    if (segment_taken[ja]){
                        can_be_taken = false;
                        break;
                    }
                }
                if (can_be_taken){
                    for (uint ja = jed.a; ja != (jed.b+1)%m; ja = (ja + 1) % m){
                        segment_taken[ja] = true;
                        takens++;
                    }
                    takenj++;
                }
                if (takens == n){
                    break;
                }
            }
            taken_jedi = (takenj > taken_jedi) ? takenj : taken_jedi;
        }
        

        std::cout << taken_jedi << std::endl;

    }
    return 0;
}
