/*
 * Fourth Attempt
 * Same idea as in the previous attempt, but with some speed optimizations:
 * * keep track of covered interval instead of each segment on its own
 * * use a set to skip segments in shorter time than with vector.erase()
 *
 *
 */
#include <vector>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <cassert>
#include <climits>
#define debug false

class Jedi {
    public:
    uint a, b;
    uint id;
    Jedi (uint a, uint b, uint id) : a(a), b(b), id(id) {}

};

bool overlaps(uint a, uint b, uint c, uint d, uint s){
    // right side exclusive, left side inclusive
    // TODO: handle cycle
    if (b < a)
        return overlaps(a, s-1, c, d, s) || overlaps(0, b, c, d, s);
    if (d < c)
        return overlaps(a, b, c, s-1, s) || overlaps(a, b, 0, d, s);

    if (a < b && c < d){
        if (a <= c && c < b)
            return true;
        if (a <= c && c >= b)
            return false;
        if ( c <= a && b < c)
            return true;
        if (c <= a && b >= c)
            return false;
    }

    
    if (a == b && c <=a && d >= b)
        return true;
    if (c == d && a <= c && b >= d)
        return true;
    if ((a == b || c == d))
        return false;

    if (a < b && b < c && b < d)
        return false;
    if ( a < b && b == c && c < d)
        return false;
    if (a < b && b < c)
        return true;
    if (a < c && c < b)
        return true;
    if (a < d && d < b)
        return true;
    if (a > c && a < d)
        return true;
    if (b > c && b < d)
        return true;
    if (d > a && d < b)
        return true;
    if (a == c && b > a && d > c)
        return true;
    if (b == d && a < b && c < d)
        return true;

    return false;
    
}

void testcase_one(uint n, uint m){
    std::vector<Jedi> jedi; jedi.reserve(n);
    // read jedi
    for (uint i=0; i<n; i++){
        uint a, b; std::cin >> a >> b;
        jedi.push_back(Jedi(a-1,b-1, i));
    }
    // sort jedi
    std::sort(jedi.begin(), jedi.end(), [](Jedi& ja, Jedi& jb){
        if (ja.b == jb.b)
            { return ja.a > jb.a; } // it should come earlier if it is shorter
            else { return ja.b < jb.b; }
            });

    // count which segment has the least jedi
    uint min_ctr = UINT_MAX;
    std::unordered_set<Jedi*> min_segment_jedi;
    uint min_segment_id;
    for (uint i=0; i<m; i++){
        uint ctr = 0;
        std::unordered_set<Jedi*> segment_jedi;
        for (uint j=0; j<n; j++){
            // and while we're at it, fix the jedi id because we sorted
            if (i==0){
                jedi[j].id = j;
            }

            if ((jedi[j].a <= jedi[j].b && jedi[j].a <= i && jedi[j].b >= i) ||
                (jedi[j].a > jedi[j].b && (jedi[j].a <= i || jedi[j].b >= i ) )
               ){ 
                ctr++;
                segment_jedi.insert(&jedi[j]);
            }
        }
        if (ctr < min_ctr && ctr > 0){
            min_ctr = ctr;
            min_segment_jedi = segment_jedi;
            min_segment_id = i;
        }
    }
    assert(min_ctr > 0);

    uint goal=0;
    // start at each of the segment_jedi in the segment with the least overlaps
    // (but at least one jedi)
    // and do earliest-deadline-first scheduling
    // until either all jedi are in use or all segments are gone
    for (Jedi* jp : min_segment_jedi){
        uint used_segment_start = jp->a; // inclusive
        uint used_segment_end = (jp->b + 1) % m; // exclusive
        uint used_segment_size = (used_segment_end - used_segment_start) %m;

        uint jedi_scheduled = 1;
        uint jedi_considered = 0;
        Jedi& next_jedi = *jp;
        while (jedi_scheduled < n && used_segment_size < m && jedi_considered < n){
            jedi_considered++;
            // if overlaps, don't take it
            if (!overlaps(used_segment_start, used_segment_end, next_jedi.a, (next_jedi.b+1)%m, m)){
                // if it does not overlap, take it
                used_segment_end = (next_jedi.b + 1) %m;
                jedi_scheduled++;
                if (used_segment_end == used_segment_start){
                    break;
                }
            }
            next_jedi = jedi[(next_jedi.id + 1) % n];
        }

        if (jedi_scheduled > goal){
            goal = jedi_scheduled;
        }
    }

    std::cout << goal << '\n';
}

int main(){
    std::ios_base::sync_with_stdio(false);

    // test my function
    assert(!overlaps(1, 2, 3, 4, 10));
    assert(!overlaps(1, 2, 2, 3, 10));
    assert(overlaps(1, 3, 2, 4, 10));
    assert(overlaps(2, 1, 4, 5, 10));
    assert(!overlaps(3, 4, 5, 1, 10));
    assert(overlaps(8, 3, 9, 2, 10));
    assert(overlaps(8, 3, 9, 4, 10));
    assert(overlaps(8, 3, 7, 2, 10));
    assert(overlaps(3, 1, 2, 0, 4));

    uint tt; std::cin >> tt;
    for (uint t=0; t<tt; t++){
        uint num_jedi; uint num_segments;
        std::cin >> num_jedi >> num_segments;
        testcase_one(num_jedi, num_segments);
    }
    return 0;

}
