//! Reimplementation of attack of the clones
//! from algolab. But in rust, this time.
//!
//! we have `m` segments, with 1 <= m <= 10^9

//use anyhow;
//use std::convert::TryFrom;
//use std::io;
use thiserror::Error;
#[macro_use]
extern crate text_io;
use whiteread::parse_line;

/// a JediPos can either be a start or end position of a Jedi interval.
#[derive(Clone, Copy, Debug, PartialOrd, Ord, PartialEq, Eq)]
enum JediPos {
    Start(u32),
    End(u32),
}

#[derive(Debug, Error)]
enum MainErrors {
    #[error(transparent)]
    ParseIntError(#[from] std::num::ParseIntError),
    /*    #[error("Failed to split string into exactly two words: `{0}`")]
        StringSplitError(String),
    */
    #[error(transparent)]
    WhitereadParseError(#[from] whiteread::reader::Error),
}

/// Runs one testcase
/// m: number of segments, counting from 1
/// n: number of jedi, counting from 1
/// and a vec containing all jedi's start and end point
fn testcase(m: u32, n: u16, positions: &Vec<JediPos>) {
    // find best starting point by enumerating number of active jedi at each relevant segment.
    // A "relevant segment" is one that a jedi starts or ends at, so the segments in `positions`.

    // at the first segment, there will be two cases:
    //   * it's a start: There will be 1 active jedi count
    //   * it's an end : There will be -1 active jedi count
    // There are at most ( 5*10^4 * 2 ) entries in the vec => i32
    let mut current_num_active: i32 = 0;
    let mut smallest_num_active: i32 = 0;
    let mut sna_index : usize = 0;
    let mut sna_is_set : bool = false; // the existence of this bool allows us to ignore the 
                                       // initial value of the smallest_num_active variable,
                                       // same for sna_index. That's just useful because rust
                                       // insists on being able to prove that they are initialized,
                                       // and it can't do that on an iterator. Because maybe, the
                                       // loop is never run (the iterator is empty)
    for (i, p) in positions.iter().enumerate() {
        current_num_active += match p {
            JediPos::Start(v) => 1,
            JediPos::End(v) => -1,
        };

        if !sna_is_set || (current_num_active < smallest_num_active) {
            sna_is_set = true;
            smallest_num_active = current_num_active;
            sna_index = i;
        }

        // if there are less than ten jedi, we can technically stop early.
        // But we don't actually know. Because it could be that the starting point of the iteration
        // was at a place where 100 jedi overlap, so the best place would be at a negative number
        // and the value zero does not mean anything in absolute terms.
    }
    println!("Smallest number of Active Jedi is at index {} where the value is {}", sna_index, smallest_num_active);

}

/// Expects as input from stdin:
///
/// one single integer followed by a newline, denoting the number of testcases (t <= 35)
///
/// one line with two integers n and m, separated by a space, denoting the number of jedi. (m <=
/// 10^9, n <= 5*10^4)
///
/// followed by n lines of two integers a and b
// how to explicitly allow different kinds of errors? I want to say "this will either be a
// ParseIntError or whatever might happen in the try_from line...
// Also, would a Box<dyn Error> actually be the way to go? Box is on the heap, right?
// One Alternative seems to be using the thiserror crate: https://stackoverflow.com/a/58337971/2550406
fn main() -> Result<(), MainErrors> {
    // This function uses multiple ways to parse, as an exercise for myself.
    // see answers here: https://stackoverflow.com/a/33631589/2550406
    // Ways I've found:
    // Way1: read line, trim, parse, ?
    // Way2: let [m, n] = match stuff
    //          this has verbose but explicit error handling
    // Way3: A one-liner that uses map to parse after splitting, then collects the contents and
    //          uses a questionmark to get the result out.
    //          https://stackoverflow.com/a/33630950/2550406
    // Way4: using the macros read! and scan! from text_io crate
    //          https://stackoverflow.com/a/33631589/2550406
    // Way5: if let [Ok(m), Ok(n)] = & split map collect
    //          https://stackoverflow.com/a/33631317/2550406
    // Way6: Using whiteread, a crate with a function-based interface specifically for
    //          white-space separated text only.
    //          https://crates.io/crates/whiteread
    //          It features parse_line()? and parse_string("1 3 4")?
    //

    println!("Hello, world!");
    let num_testcases: u8 = read!();
    println!("Parsed num_testcases {}", num_testcases);

    // for each testcase

    for _t in 0..num_testcases {
        let (n, m): (u16, u32) = parse_line()?;
        println!("Parsed m,n: {}, {}", m, n);

        // read all the jedi into a vec
        // That vec shall contain all starts and seperately all ends
        let mut positions: Vec<JediPos> = Vec::with_capacity((2 * n).into());
        for _i in 0..n {
            // read line containing a and b
            let (a, b): (u32, u32) = parse_line()?;
            positions.push(JediPos::Start(a));
            positions.push(JediPos::End(b));
        }

        // sort that vec ascending
        positions.sort();

        // run testcase
        testcase(m, n, &positions);
    }

    Ok(())
}
