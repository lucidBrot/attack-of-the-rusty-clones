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
use std::cmp::Ordering;
use whiteread::parse_line;
use std::io::prelude::*;
use std::io::BufReader;
use std::path::Path;
use std::fs::File;
use std::collections::HashSet;

/// a JediPos can either be a start or end position of a Jedi interval.
/// It contains the segment id where it is located, and the jedi id.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Ord)]
enum JediPos {
    // order matters here!
    Start(u32, u16),
    End(u32, u16),
}

/// sort first by segment number,
/// then by Start < End
/// then by jedi number
impl PartialOrd for JediPos {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some((self.segment(), !self.is_start(), self.jedi_id()).cmp(&(
            other.segment(),
            !other.is_start(),
            other.jedi_id(),
        )))
    }
}

impl JediPos {
    fn segment(self) -> u32 {
        match self {
            JediPos::Start(v, _) => v,
            JediPos::End(v, _) => v,
        }
    }
    fn jedi_id(self) -> u16 {
        match self {
            JediPos::Start(_, j) => j,
            JediPos::End(_, j) => j,
        }
    }
    fn is_start(self) -> bool {
        match self {
            JediPos::Start(_, _) => true,
            JediPos::End(_, _) => false,
        }
    }
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
fn testcase(m: u32, n: u16, positions: &Vec<JediPos>) -> u16 {
    // find best starting point by enumerating number of active jedi at each relevant segment.
    // A "relevant segment" is one that a jedi starts or ends at, so the segments in `positions`.
    println!("Running testcase with m = {} and n = {}", m, n);
    if positions.len() < 15 { dbg!(positions); }

    // at the first segment, there will be two cases:
    //   * it's a start: There will be 1 active jedi count
    //   * it's an end : There will be -1 active jedi count
    // There are at most ( 5*10^4 * 2 ) entries in the vec => i32
    let mut current_num_active: i32 = 0;
    let mut smallest_num_active: i32 = 0;
    let mut sna_index: usize = 0;
    let mut sna_is_set: bool = false; // the existence of this bool allows us to ignore the
                                      // initial value of the smallest_num_active variable,
                                      // same for sna_index. That's just useful because rust
                                      // insists on being able to prove that they are initialized,
                                      // and it can't do that on an iterator. Because maybe, the
                                      // loop is never run (the iterator is empty)
    assert!(
        positions.len() > 0,
        "Empty position vectors are not supported"
    );
    assert!(
        (2 * n as usize) == positions.len(),
        "n is inconsistent with positions"
    );
    for (i, p) in positions.iter().enumerate() {
        current_num_active += match p {
            JediPos::Start(_, _) => 1,
            JediPos::End(_, _) => -1,
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
    println!(
        "Smallest number of Active Jedi is at index {} where the value is {}",
        sna_index, smallest_num_active
    );

    // Figure out the correct of the starting jedi by trying all options to start with.
    // That should be feasible, since the absolute value of active jedi here is <= 10
    // The start jedi is either one of the jedi overlapping the sna_index segment,
    // or the first jedi to end afterwards.
    // The simplest way to do this is to just try the next 11 jedi that end. This way,
    // there is no need for keeping track of which jedi start positions have been encountered.
    let starting_jedi: Vec<&JediPos> = positions
        .iter()
        .cycle()
        .skip(sna_index)
        .filter(|el| match el {
            JediPos::Start(_, _) => false,
            JediPos::End(_, _) => true,
        })
        .take(11)
        .collect();
    // This could be optimized a little, because not always will there be ten active jedi.
    // But whatever, that should not be too relevant.

    // For each starting jedi, we do the same task
    let max_jedi_in_fight: u16 = starting_jedi
        .iter()
        .map(|el| count_edf(&positions, **el, sna_index))
        .max()
        .expect("there must be at least one jedi in total.");
    println!("{}", max_jedi_in_fight);
    return max_jedi_in_fight;
}

/// given a starting_jedi_nr, we will stop looping when that Start(nr) is encountered
/// ~~~ make sure it comes before any End() of other jedi. We wouldn't want them to be picked
/// there. So any end containing the value of the start segment shall be ignored.
/// To loop correctly, we need to know the index of the startin_jedi's End as well.
/// But this is annoying to code, so instead I simply take the sna_index and search from there.
fn count_edf(positions: &Vec<JediPos>, starting_jedi: JediPos, sna_index: usize) -> u16 {
    assert!(matches!(starting_jedi, JediPos::End(_, _)));
    let starting_jedi_id = starting_jedi.jedi_id();
    // skip ahead to the start
    let myiter = positions
        .iter()
        .cycle()
        .skip(sna_index)
        .skip_while(|el| match el {
            JediPos::Start(_, _) => true,
            JediPos::End(_v, jedi_id) => *jedi_id != starting_jedi_id,
        });
    // my test shows that the failing entry is not skipped, as expected.
    println!("Skipped ahead {} steps, and then to the end of the starting jedi", sna_index);
    dbg!(starting_jedi);

    // the first element in myiter is End(starting_jedi_nr)
    // from then on, it will simply loop around. It is my own job to stop at
    // Start(starting_jedi_nr). As well as ignoring any End(starting_jedi_start_segment)
    // Ignoring those is relevant because we can't end one jedi on the same space as our initial
    // jedi starts. Luckily, there are no additional checks required due to the vec being ordered
    // such that Start is less than End with the same values.
    //
    // TODO: check that the jedi I want to take has not started before the previous end.
    // i.e. the set of jedi we are allowed to take is limited to the ones I have seen starting
    // during the iteration. Whenever I take any End, this set is emptied again. 
    // Looping around is no issue for this, because if I took the starting jedi (which I did)
    // then I could not take any potential jedi that had their start before the starting_jedi and
    // their end after the starting_jedi's start.
    // This set will be at worst (n-1) jedi big - that's when all non-starting jedi
    // start at the same time. Since n fits into a u16, we would need at most 2**16 * 8 bytes.
    // That's 512 KiB. I should have those lying around.
    // The runtime is linear in n, so that's great as well.
    let mut allowed_jedi_id_set = HashSet::<u16>::new();
    // the starting_jedi is always allowed.
    allowed_jedi_id_set.insert(starting_jedi.jedi_id());
    assert!(false, "todo: write a loop that uses this allowed_jedi_id_set\
        and skips jedi ends that are not allowed.");
    let mut result: u16 = 0;
    for jedipos in myiter {
        match jedipos {
            JediPos::Start(_v, jedi_id) => {
                // stop the iteration when we find the start of the starting jedi
                if *jedi_id == starting_jedi_id { break; };
                // add the jedi that just started to the set of allowed jedi
                allowed_jedi_id_set.insert(*jedi_id);
            },
            JediPos::End(_v, jedi_id) => {
                // if the jedi is allowed, take it  
                if allowed_jedi_id_set.contains(*jedi_id) {
                    result += 1;
                    // if the jedi was taken, all valid jedi so far are now
                    // no longer allowed to be taken because they overlap
                    allowed_jedi_id_set.remove(*jedi_id);
                }
            }
        };
    }
    println!(" result was {} jedi.\n", result);

    return result as u16;
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

    let num_testcases: u8 = read!();
    println!("Parsed num_testcases {}", num_testcases);

    // for each testcase

    for _t in 0..num_testcases {
        let (n, m): (u16, u32) = parse_line()?;
        println!("Parsed m,n: {}, {}", m, n);

        // read all the jedi into a vec
        // That vec shall contain all starts and seperately all ends
        let mut positions: Vec<JediPos> = Vec::with_capacity((2 * n).into());
        for jedi_id in 0..n {
            // read line containing a and b
            let (a, b): (u32, u32) = parse_line()?;
            positions.push(JediPos::Start(a, jedi_id));
            positions.push(JediPos::End(b, jedi_id));
        }

        // sort that vec ascending
        positions.sort();

        // run testcase
        testcase(m, n, &positions);
    }

    Ok(())
}

#[cfg(test)]
mod tests{
    // import names from outer scope
    use super::*;

    fn vericfy (n: u16, m: u32, testjedivec: Vec<u32>, expected_result: u16){
        let mut testjedidata = testjedivec.iter().peekable();
        let mut positions: Vec<JediPos> = Vec::with_capacity((2 * n).into());
        let mut jedi_id = 0;
        loop {
            let a = *(testjedidata.next().unwrap());
            let b = *testjedidata.next().expect("should exist");
            jedi_id += 1;
            positions.push(JediPos::Start(a, jedi_id));
            positions.push(JediPos::End(b, jedi_id));
            if let None = testjedidata.peek() { break; }
            else { println!("[vericfy]: Another round!"); }
        }

        let result = testcase(m, n, &positions);
        assert_eq!(result, expected_result, "Result was {} instead of {}", result, expected_result);
    }

    /// reproduces what the eric_hole.in file tests
    #[test]
    fn eric_hole(){
        vericfy(2, 6, vec![1,1, 4,5], 2);
    }

    /// This function takes a filename and reads `filename.in` and `filename.out`
    /// then, runs a test for it.
    fn test_from_files (in_file: &str, out_file: &str){
        let in_path = Path::new(in_file);
        let out_path = Path::new(out_file);
        let file_in =  File::open(&in_path).expect("Can't open input file");
        let file_out =  File::open(&out_path).expect("Can't open target output file");

        let mut reader = BufReader::new(&file_in);
        let mut num_testcases_str = String::new();
        reader.read_line(&mut num_testcases_str).expect("error while reading");
        let num_testcases: usize = num_testcases_str.trim().parse::<usize>().unwrap();

        let mut out_reader = BufReader::new(&file_out);

        for _t in 0..num_testcases {
            let mut headline = String::new();
            reader.read_line(&mut headline).expect("error while reading");
            let mut nm = headline.trim().split_whitespace();
            let (n, m): (u16, u32) = (nm.next().unwrap().parse::<u16>().unwrap(), nm.next().unwrap().parse::<u32>().unwrap());
            let mut positions: Vec<u32> = Vec::with_capacity((2 * n).into());
            for _jedi_id in 0..n {
                let mut line = String::new();
                reader.read_line(&mut line).expect("error while reading");
                let mut ab = line.trim().split_whitespace();
                let (a, b): (u32, u32) = (ab.next().unwrap().parse::<u32>().unwrap(), 
                    ab.next().unwrap().parse::<u32>().unwrap());
                // push the start and the end as ints
                //positions.push(JediPos::Start(a, jedi_id));
                //positions.push(JediPos::End(b, jedi_id));
                positions.push(a);
                positions.push(b);
            }

            // sort that vec ascending
            positions.sort();
            let mut result = String::new();
            out_reader.read_line(&mut result).expect("error while reading output file");
            let res: u16 = result.trim().parse::<u16>().unwrap();
            
            vericfy(n, m, positions, res);
        }

    }

    /// load test file from ../atcotc/ as .in and .out file
    fn test_my_file(name : &str){
        let in_name = format!("../atcotc/{}.in", name);
        let out_name = format!("../atcotc/{}.out", name);
        test_from_files(&*in_name, &*out_name);
    }

    #[test]
    fn eric_contains(){
        test_my_file("eric_contains");
    }

    #[test]
    fn sample(){
        test_my_file("sample");
    }

    // That's getting annoying. Test the rest using a macro.
    // This macro generates same code as above
    macro_rules! tmf {
        ($name:ident) => {
            #[test]
            fn $name() {
                test_my_file(stringify!($name));
            }
        };
    }

    tmf!(eric_manyjedi);
    tmf!(eric_just_touching);
    tmf!(eric_maxjedi);
    tmf!(eric_maxjedi_moresegments);
    tmf!(eric_minboth);
    tmf!(eric_maxseg);
    tmf!(eric_minjedi);
    tmf!(eric_minseg);
    tmf!(eric_no_overlaps);
    tmf!(eric_overlaps);
    tmf!(eric_overlaps_zero);
    tmf!(eric_sameends);
    tmf!(eric_stacking);
    tmf!(eric_tot_seg2);
    tmf!(s1);
    tmf!(s2);
    tmf!(s3);
    tmf!(test1);
    tmf!(test2);
    tmf!(test3);
    tmf!(test1_pt3);
    tmf!(test1_pt4);

    /// tests behaviour of take_while
    /// view the output with `cargo test test_take_while -- --nocapture`
    #[test]
    fn test_take_while(){
        let v: Vec<u32> = vec![1,2,3,4,5,6,7,8];
        // this could (should) be a for loop, but I wanted to see if this works too
        let r = v.iter().take_while(|&&el| el<7).map(|&el| {print!("{}, ", el); el}).collect::<Vec<u32>>();
        assert_eq!(r, vec![1,2,3,4,5,6]);
    }

    #[test]
    fn test_skip_while(){
        let v: Vec<u32> = vec![1,2,3,4,5,6,7,8];
        // this could (should) be a for loop, but I wanted to see if this works too
        let r = v.iter().skip_while(|&&el| el<7).map(|&el| {print!("{}, ", el); el}).collect::<Vec<u32>>();
        assert_eq!(r, vec![7,8]);
    }

    /*
    /// tests behaviour of deconstructing with `if let`  in a filter
    /// I disabled it though, because this is currently experimental.
    /// Alternative is to use match! (expr, pattern)
    /// #[test]
    fn test_filtering(){
        let v: Vec<JediPos> = vec![JediPos::Start(1,2), JediPos::End(3,4)];
        let r: Vec<JediPos> = v.filter(|el| let JediPos::Start(_,_) = el).collect();
        assert_eq!(r, vec![JediPos::Start(1,2)]);
    }
    */

    #[test]
    fn test_enum_order (){

        // testing enum order:
        let mut dbg_vec2 = vec![
            JediPos::End(1, 1),
            JediPos::Start(1, 1),
            JediPos::Start(2, 1),
            JediPos::Start(1, 2),
        ];
        dbg_vec2.sort();
        assert_eq!(
            dbg_vec2,
            vec![
            JediPos::Start(1, 1),
            JediPos::Start(1, 2),
            JediPos::End(1, 1),
            JediPos::Start(2, 1)
            ]
        );
    }

}
