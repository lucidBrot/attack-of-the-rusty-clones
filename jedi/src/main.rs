//! Reimplementation of attack of the clones 
//! from algolab. But in rust, this time.
//!
//! we have `m` segments, with 1 <= m <= 10^9

use std::io;
use std::convert::TryFrom;
use thiserror::Error;
use anyhow;

/// a JediPos can either be a start or end position of a Jedi interval.
trait JediPos {
    fn is_start(&self) -> bool;
    fn is_end(&self) -> bool {
        !self.is_start()
    }
}

struct JediStart {
    pos: u32
}
impl JediPos for JediStart {
    fn is_start(&self) -> bool { true }
}

struct JediEnd {
    pos: u32
}
impl JediPos for JediEnd {
    fn is_start(&self) -> bool { true }
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
fn testcase(m: u32, n: u16, workvec: Vec<Box<JediStart>> ){

    // just for now
    let js = JediStart { pos: 1 };
    let je = JediEnd { pos: 2 };
}

/// Expects as input from stdin:
///
/// one single integer followed by a newline, denoting the number of testcases (t <= 35)
/// one line with two integers n and m, separated by a space, denoting the number of jedi. (m <=
/// 10^9, n <= 5*10^4)
/// followed by n lines of two integers a and b
// TODO: how to explicitly allow different kinds of errors? I want to say "this will either be a
// ParseIntError or whatever might happen in the try_from line... 
// Also, would a Box<dyn Error> actually be the way to go? Box is on the heap, right?
// One Alternative seems to be using the thiserror crate: https://stackoverflow.com/a/58337971/2550406
fn main() -> Result<(), MainErrors> {
    println!("Hello, world!"); 

    // read from stdin
    let reader = io::stdin();
    let mut input_text = String::new();
    reader.read_line(&mut input_text).expect("failed to read line");
    let num_testcases = input_text.trim().parse::<u8>()?;
    println!("Parsed num_testcases {}", num_testcases);

    // for each testcase

    for t in 0..num_testcases {
        let m: u32; let n: u16;
        let mut m_n_str = String::new();
        reader.read_line(&mut m_n_str).expect("failed to read line");
        // split on space into two numbers
        // TODO: annotate this in the documentation as magic to look at later
        let [m, n] = match <[&str; 2]>::try_from(m_n_str.split_whitespace().take(2).collect::<Vec<&str>>()) {
            Ok(vals) => vals,
            Err(e) => { return Err(MainErrors::StringSplitError(format!("{:?}", e))); }
        };
        println!("Parsed m,n: {}, {}", m, n);
    }

    Ok(())
}
