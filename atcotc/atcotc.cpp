/*
 *  !! This Approach Misunderstood The Exercise !!
 *
 *
 * Every Jedi protects one segment of the circle, chosen from multiple possible segments per jedi.
 * The choices are contiguous intervals.
 * There are m-1 segments.
 * Some jedi must stay out of the battle to avoid clashes with others defending the same segment.
 *
 * Goal: maximize number of jedi without causing explosions.
 *
 * Idea Flow:
 *  Source -1-> Jedi -1-> Segments -1-> Sink
 *  Max Flow is number of Jedi that can fight. 
 *  In O(V^3) = O((m+n)^3)
 *
 * Idea Greedy:
 *  1. For each segment store a set of jedi pointers. For each jedi store a set of segment pointers.
 *  2. Sort jedi by number of segments.
 *  3. Sort segments by number of jedi.
 *  So far O((m+n) * log(m+n))
 *  4. while there is a smallest segment with zero jedi, remove it.
 *  5. while there is a smallest segment with one jedi, take it:
 *      chosen++;
 *      jedis.remove(jedi) // for each segment in the jedi, remove the jedi from the list
 *          O(n * m)
 *      remove jedi from list of jedis in general.
 *  6. while there is a smallest jedi with only one segment option, choose that.
 *      chosen++;
 *      remove segment from all other jedi
 *          O(n* m)
 *  So far O((n*m) + ((m+n)*log(m+n)))
 *  7. while the smallest jedi has multiple options
 *      either a) choose the one with largest other min jedi
 *      or     b) choose the one with fewest number of jedi
 *      ( I haven't decided yet which one makes more sense )
 *     then go to 4. in case something has now become obvious.
 *
 *  Stop at any time if there are no more jedi or no more segments left.
 *  ==> Total of O(V^2)
 *
 *  This idea is quite complex and I'm unsure it is completely correct. So let me do the other one first. If flow is fast enough, no need for greedy.
 *  But the numbers are quite big. O((10^5 * 10^9)^3) = O(10^42)
 *  OTOH since the inputs are limitet, this is constant :P
 *
 */
#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <iostream>
#include <boost/graph/push_relabel_max_flow.hpp>
#define debug true

// Graph Type with nested interior edge properties for flow algorithms
typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS> traits;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property,
    boost::property<boost::edge_capacity_t, long,
        boost::property<boost::edge_residual_capacity_t, long,
            boost::property<boost::edge_reverse_t, traits::edge_descriptor>>>> graph;

typedef traits::vertex_descriptor vertex_desc;
typedef traits::edge_descriptor edge_desc;

// Custom edge adder class, highly recommended
class edge_adder {
  graph &G;

 public:
  explicit edge_adder(graph &G) : G(G) {}

  void add_edge(int from, int to, long capacity) {
#if debug
    std::cout << "Adding Edge " << from << "-(" << capacity << ")->" << to << std::endl;
#endif
    auto c_map = boost::get(boost::edge_capacity, G);
    auto r_map = boost::get(boost::edge_reverse, G);
    const auto e = boost::add_edge(from, to, G).first;
    const auto rev_e = boost::add_edge(to, from, G).first;
    c_map[e] = capacity;
    c_map[rev_e] = 0; // reverse edge has no capacity!
    r_map[e] = rev_e;
    r_map[rev_e] = e;
  }
};

void flow_approach(uint n, uint m){
    graph G(2+n+m);
    vertex_desc source = boost::vertex(0, G);
    vertex_desc sink = boost::vertex(1, G);
    edge_adder adder(G);
    for (uint i=0; i<n; i++){
        // connect the source to the jedi
        adder.add_edge(source, boost::vertex(2+i, G), 1);

        uint a, b; std::cin >> a >> b;
        if (b >= a){
            for (uint j=a; j<=b; j++){
                // segments are enumerated from 1 in the input, but from zero in the graph
                uint jg = j-1;
                // connect the jedi to their segments
                adder.add_edge(boost::vertex(2+i, G), boost::vertex(2+n+jg, G), 1);
            }
        } else {
            // cyclic segments! b<a
            for (uint j=a; j<=m; j++){
                uint jg = j-1;
                adder.add_edge(boost::vertex(2+i, G), boost::vertex(2+n+jg, G), 1);
            }
            for (uint j=1; j<=b; j++){
                uint jg = j-1;
                adder.add_edge(boost::vertex(2+i, G), boost::vertex(2+n+jg, G), 1);
            }
        }
    }

    // connect each segment to the sink
    for (uint i=2+n; i<2+n+m; i++){
        adder.add_edge(boost::vertex(i, G), sink, 1);
    }

    // compute flow
    long flow = boost::push_relabel_max_flow(G, source, sink);
    std::cout << flow << '\n';
}

int main(){
    std::ios_base::sync_with_stdio(false);
    uint tt; std::cin >> tt;
    for (uint t=0; t<tt; t++){
#if debug
        std::cout << "\n--- " << t << " ---" << std::endl;
#endif
        uint num_jedi; uint num_segments;
        std::cin >> num_jedi >> num_segments;
        flow_approach(num_jedi, num_segments);
    }
    return 0;
}
