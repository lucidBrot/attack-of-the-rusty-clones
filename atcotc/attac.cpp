/*
 * !! This Approach Is Too Slow !!
 *
 *  Greedy Approach.
 *  This time, I'm assuming the jedi are stupid enough to not be able to choose which segment they defend. Instead, they always protect all of their segments.
 *
 *  1. Choose jedi with least conflicts
 *  2. repeat.
 *
 *  To do so, sort the jedi by number of conflicts and update those numbers when a jedi is chosen.
 *  (Remove all conflicting jedi. On removal of a jedi, reduce the conflict number of conflicting jedi. Then remove the chosen jedi.)
 */
#include <vector>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#define debug false

class Jedi {
public:
    uint a, b; // zero-indexed
    bool removed = false;
    std::unordered_set<Jedi*> conflicts;
    Jedi(uint a, uint b): a(a), b(b){ 
#   if debug
        std::cout << "[DEBUG]: Create jedi [" << a << ", " << b << "]" << std::endl;
#   endif 
    }
    void add_conflict(Jedi* other){
        if (other != this){
            this->conflicts.insert(other);
        }
    }
    void choose(std::vector<std::unordered_set<Jedi*>>& segments_vector,
            std::vector<bool>& segment_available, uint m){
#       if debug
        std::cout << "[DEBUG]: choose jedi [" << a << ", " << b << "]" << std::endl;
#       endif
        // remove other jedi from their segments
        // that in turn removes them from my conflict set
        while (!conflicts.empty()){
            (*conflicts.begin())->remove(segments_vector, m);
        }
        // mark segment as used
        for (uint i=a; i%m != (b+1)%m; i = (i+1)%m){ 
#       if debug
            std::cout << "[DEBUG]: mark segment " << i << " as used" << std::endl;
#       endif
            segment_available[i] = false;
        }
        // remove myself from segments. 
        this->remove(segments_vector, m);
    }
    void remove(std::vector<std::unordered_set<Jedi*>>& segments_vector, uint m){
#       if debug
        std::cout << "[DEBUG]: remove jedi [" << a << ", " << b << "]" << std::endl;
#       endif
        // remove myself from segments
        for (uint i=a; i%m != (b+1)%m; i = (i+1)%m){ 
#if debug   
            std::cout << "          removing myself from segment " << i << std::endl;
#endif
            segments_vector[i].erase(this);
        }
        // remove myself from conflicts of other jedi
        for (Jedi* jp : conflicts){
#if debug   
            std::cout << "          removing myself from jedi conflict [" << (jp->a) << ", " << (jp->b) << "]" << std::endl;
#endif
            jp->conflicts.erase(this);
        }
        this -> removed = true;
#       if debug
        std::cout << ":" << std::endl;
#       endif
    }
};

void greedy_approach(uint n, uint m){
    std::vector<Jedi> jedi; jedi.reserve(n);
    std::vector<Jedi*> jediptrs(n);
    std::vector<std::unordered_set<Jedi*>> segments(m);
    std::vector<bool> segments_available(m, true);
#if debug
    std::cout << "[DEBUG]: --- " << n << ", " << m << " ---" << std::endl;
#endif
    for (uint i=0; i<n; i++){   // Read in Jedi
        uint a, b; std::cin >> a >> b; // one-based count of segments
        uint az = a-1; uint bz=b-1;    // zero-based count of segments
        jedi.push_back(Jedi(az, bz));
        jediptrs[i] = &jedi[i];
        // add jedi to segment
        // since the segment specifiers can be cyclic, we take them modulo
        for ( uint j=az; j%m != (bz+1)%m; j = (j+1)%m){
            segments[j].insert(&jedi[i]);
        }
    }
    // find conflicts
    for (uint i=0; i<m; i++){
        std::unordered_set<Jedi*>& segment = segments[i];
        for (Jedi* jedi_a : segment){
            for (Jedi* other : segment){
                jedi_a->add_conflict(other);
            }
        }
    }

    uint chosen = 0; bool segments_are_remaining = true;
    do {
#if debug
        std::cout << "[DEBUG]: sorting..." << std::endl;
#endif
        // sort jedi by num conflicts. But only sort pointers, to keep the addresses intact.
        std::sort(jediptrs.begin(), jediptrs.end(), [](Jedi* jeda, Jedi* jedb){
                return jeda->conflicts.size() < jedb->conflicts.size();
                });

        // take first jedi
#if debug
        std::cout << "[DEBUG]: garbage cleaning..." << std::endl;
#endif
        while (jediptrs.size() > 0) { // garbage cleaning
            if (jediptrs[0]->removed){
                jediptrs.erase(jediptrs.begin()+0);
            } else { break; }
        }
        if (jediptrs.size() == 0){
            break;
        }
#       if debug
        std::cout << "[DEBUG]: garbage cleaned." << std::endl;
#       endif

        jediptrs[0]->choose(segments, segments_available, m); chosen++;
        // this should have updated all segments by removing jedi from them,
        // and all jedi conflicts by removing the removed jedi from their conflicts.
        // So now every jedi knows the correct number of conflicts. 
        // TODO: is the segments_available vector useless?
#       if debug
        std::cout << "." << std::endl;
#       endif

        // update jediptr vector
        jediptrs.erase(jediptrs.begin()+0);
#       if debug
        std::cout << "." << std::endl;
#       endif

        // sort again, and take the first.
        // stop when all segments chosen or all jedi chosen
        segments_are_remaining = false;
        for (bool b : segments_available){
            segments_are_remaining |= b;
        }
#       if debug
        std::cout << "[DEBUG]: segments_are_remaining = " << segments_are_remaining << std::endl;
#       endif
    } while ( (jediptrs.size() > 0) && (segments_are_remaining) );

    std::cout << chosen << '\n';
}

int main(){
    std::ios_base::sync_with_stdio(false);
    uint tt; std::cin >> tt;
    for (uint t=0; t<tt; t++){
        uint num_jedi; uint num_segments;
        std::cin >> num_jedi >> num_segments;
        greedy_approach(num_jedi, num_segments);
    }
    return 0;

}
