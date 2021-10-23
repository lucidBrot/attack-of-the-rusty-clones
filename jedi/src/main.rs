//! Reimplementation of attack of the clones
//! from algolab. But in rust, this time.
//!
//! we have `m` segments, with 1 <= m <= 10^9

use anyhow;
use std::convert::TryFrom;
use std::io;
use thiserror::Error;
#[macro_use] extern crate text_io;

/// a JediPos can either be a start or end position of a Jedi interval.
trait JediPos {
    fn is_start(&self) -> bool;
    fn is_end(&self) -> bool {
        !self.is_start()
    }
}

struct JediStart {
    pos: u32,
}
impl JediPos for JediStart {
    fn is_start(&self) -> bool {
        true
    }
}

struct JediEnd {
    pos: u32,
}
impl JediPos for JediEnd {
    fn is_start(&self) -> bool {
        true
    }
}

#[derive(Debug, Error)]
enum MainErrors {
    #[error(transparent)]
    ParseIntError(#[from] std::num::ParseIntError),
    #[error("Failed to split string into exactly two words: `{0}`")]
    StringSplitError(String),
}

/// Runs one testcase
/// m: number of segments, counting from 1
/// n: number of jedi, counting from 1
/// and a vec containing all jedi's start and end point
fn testcase(m: u32, n: u16, workvec: Vec<Box<JediStart>>) {
    // just for now
    let js = JediStart { pos: 1 };
    let je = JediEnd { pos: 2 };
}

/// Expects as input from stdin:
///
/// one single integer followed by a newline, denoting the number of testcases (t <= 35)
///
/// one line with two integers n and m, separated by a space, denoting the number of jedi. (m <=
/// 10^9, n <= 5*10^4)
///
/// followed by n lines of two integers a and b
// TODO: how to explicitly allow different kinds of errors? I want to say "this will either be a
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
    // 

    println!("Hello, world!");

    // read from stdin
    let reader = io::stdin();
    /*
    let mut input_text = String::new();
    reader
        .read_line(&mut input_text)
        .expect("failed to read line");
    let num_testcases = input_text.trim().parse::<u8>()?;
    */
    let num_testcases: u8 = read!();
    println!("Parsed num_testcases {}", num_testcases);

    // for each testcase

    for t in 0..num_testcases {
        let m: u32;
        let n: u16;
        let mut m_n_str = String::new();
        reader.read_line(&mut m_n_str).expect("failed to read line");
        /*
        // split on space into two numbers
        let [m, n] = match <[&str; 2]>::try_from(
            m_n_str.split_whitespace().take(2).collect::<Vec<&str>>(),
        ) {
            Ok(vals) => vals,
            Err(e) => {
                return Err(MainErrors::StringSplitError(format!("{:?}", e)));
            }
        };
        let m = m.trim().parse::<u16>()?;
        let n = n.trim().parse::<u32>()?;
        */
        scan!("{} {}", m, n);
        println!("Parsed m,n: {}, {}", m, n);

        // read all the jedi into a vec
        // That vec shall contain all starts and seperately all ends
        for i in 0..n {
            // read line containing a and b
            let mut a_b_str = String::new();
            reader.read_line(&mut a_b_str).expect(&format!("failed to read line for {} jedi", i));
            // apparently, using [..] after a vec is a trick to get the stuff out
            if let [a, b] = a_b_str.split_whitespace().map(|x| x.parse::<u32>())
                .collect::<Result<Vec<u32>,_>>()?[..] {
                println!("Jedi {i}: ({a}, {b})", i=i, a=a, b=b);
            } else { return Err(MainErrors::StringSplitError(String::from("failed to parse")))}

        }
    }

    Ok(())
}
