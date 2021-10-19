/*
 *  redo after first exam.
 *
 *  Circles of m segments.
 *  Jedi are fixed-size intervals with fixed location. They are not allowed to overlap.
 *  Calculate the largest number of Jedi that can be used without overlaps.
 *
 *  Input:
 *      Each jedi has a start and end interval. These inputs start at 1.
 *      Each jedi has a number. That starts at 1 as well.
 *      So I want to subtract 1 whenever I can.
 *      No jedi can wrap-around multiple times.
 *
 *  Test Cases:
 *      The first two test cases guarantee that there is a segment covered by no Jedi.
 *      The third test case does not guarantee that - we will have to find the least-covered segment.
 *
 *  Approach:
 *      # Find a starting point
 *        When there is a segment with no Jedi, that means there will not be any
 *        Jedi sneaking in from the left and ending there or starting there. Hence
 *        we can list the Jedi that overlap with the following segment and start EDF there.
 *
 *      # Shift Coordinates
 *        The start of the first taken jedi becomes coordinate 0. So every future jedi ending
 *        is larger in the new coords. In the first two test cases, there won't be an
 *        overlapping jedi. In the third, we must ignore all Jedi ending later than m-1.
 *
 *      # Earliest Deadline First
 *        Keep taking the earliest deadline Jedi that is available. Ignore all Jedi that
 *        have a start point earlier than the previously taken end.
 *
 *      # Handling Overlaps in the third Test Case
 *        There might be an overlap coming in from the left. We will have to first find the
 *        segment with the least overlapping jedi and then try taking all jedi in that list once.
 *        Possible Optimization: If there are many starting in that segment, we still only have to
 *        try the last one of those - and any from the left.
 *
 *        But: Any from the left that end later than the last of the normal ones don't make sense to
 *        take. So that means we need to check all the jedi in the segment, sorted by how soon they
 *        end, only up to when some jedi started at zero and ended there. As soon as a jedi starts
 *        at 1, we can stop that.
 *
 *  Implementation:
 *      We read the jedi and intervals and edit their coordinates to start at 0.
 *
 *      Then we need to find the segment with the least overlapping jedi (and which they are).
 *      So I guess store a vector of jedi per segment (and later sort only the one we need sorted).
 *      
 *      There could be a huge number of segments of which we only need a few of. So preprocess the
 *      segments by going once through the list of jedi and marking every segment that has a start
 *      or end as important. Then go through the list of segments once and map them to smaller
 *      segment numbers. By storing the new coord in the segment and the segment ptr in a vec.
 *      However, I will also keep the segments before and after the chosen one, to make sure
 *      empty space is accounted for.
 */
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <queue>

class Jedi {
    public:
        uint id;
        uint orig_a, orig_b;
        uint shifted_a, shifted_b;
        Jedi (uint id, uint orig_a, uint orig_b): id(id), orig_a(orig_a), orig_b(orig_b) {

        }
        Jedi(){}
};

class Segment {
    public:
    bool relevant = false;
    uint original_index;
    uint new_index;
    uint shifted_index;
    std::vector<Jedi*> starting_here;
    std::vector<Jedi*> ending_here;
};

void testcase(){
    uint n, m; std::cin >> n >> m;
    // store all jedi and segments for future reference
    std::vector<Segment> original_segments (m);
    std::vector<Jedi> jedis (n);

    // read in jedi
    for (uint i=0; i<n; i++){
        uint a, b; std::cin >> a >> b;
        jedis.push_back(Jedi(i, a-1, b-1));
        // mark segment as relevant
        original_segments[a].relevant = true;
        original_segments[a].original_index = a;
        original_segments[a].starting_here.push_back(&jedis[i]);
        original_segments[b].relevant = true;
        original_segments[b].original_index = b;
        original_segments[a].ending_here.push_back(&jedis[i]);
    }

    // reduce segments
    std::vector<Segment*> relevant_segments;
    for (uint i=0; i<m; i++){
        if (original_segments[i].relevant){
            original_segments[i].new_index = i;
            relevant_segments.push_back(&original_segments[i]);
        } else {
            // also store neighbours of relevant segments
            uint left = (i==0) ? m-1 : i+1;
            uint right = (i==m-1) ? 0 : i+1;
            if (original_segments[left].relevant || original_segments[right].relevant){
                original_segments[i].new_index = i;
                relevant_segments.push_back(&original_segments[i]);
            }

        }
    }

    // for each relevant segment, count jedi.
    long num_jedi = 0; // this might become negative
    Segment* emptiest_segment = NULL;
    long emptiest_num_jedi = 0;
    for (uint i=0; i<relevant_segments.size(); i++){
        Segment* seg = relevant_segments[i];
        num_jedi += seg->starting_here.size();
        // those that end here are still part of this segment
        if (emptiest_segment == NULL || num_jedi < emptiest_num_jedi ){
            emptiest_segment = seg;
            emptiest_num_jedi = num_jedi;
        }
        num_jedi -= seg->ending_here.size();
    }

    // now we know which segment is the emptiest one.
    // For the first two test cases, we can assume this is zero.
    // TODO: third test case.
    // The best place to start EDF is hence after that empty segment.

    // to make thinking easier, we shift everything so that this segment has the smallest
    // index, at zero.
    uint shiftby = emptiest_segment->new_index;
    for (Segment* seg : relevant_segments){
        long tmp = ((long)(seg->new_index)) - shiftby;
        if (tmp < 0){
            tmp += m;
        }
        seg->shifted_index = tmp;

        // same shifting applied to jedi
        for (Jedi* jedi : seg->starting_here){
            jedi->shifted_a = tmp;
        }
        for (Jedi* jedi : seg->ending_here){
            jedi->shifted_b = tmp;
        }
    }

    // emptiest element is now at shifted_index 0.

    // Test cases 1 and 2: do EDF scheduling
    auto jediptrsort = [](Jedi* a, Jedi* b){return a->shifted_b > b->shifted_b;};
    std::priority_queue<Jedi*, std::vector<Jedi*>, decltype(jediptrsort)> jediq(jediptrsort);

    // Take whichever jedi ends first. 
    // 1. check if can be taken
    // 2. take this one
    long largest_end_taken = -1;
    long num_jedi_taken = 0;
    while (!jediq.empty()){
        Jedi* jp = jediq.top(); jediq.pop();        
        // assuming none wrap around zero, we only need to check the start.
        // because the end of each will be above the start.
        // TODO: for test 3, consider how an end below start impacts this
        if ( jp->shifted_a <= largest_end_taken ){
            continue;
        } else if ( jp->shifted_b <= largest_end_taken ){
            continue;
        } else {
            largest_end_taken = jp->shifted_b;
            num_jedi_taken ++;
        }
    }

    std::cout << num_jedi_taken << '\n';

}

int main(){
    std::ios_base::sync_with_stdio(false);
    uint tt; std::cin >> tt;
    for (uint t=0; t<tt; t++){
        testcase();
    }
    return 0;
}
