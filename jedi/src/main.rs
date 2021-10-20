//! Reimplementation of attack of the clones 
//! from algolab. But in rust, this time.
//!
//! we have `m` segments, with 1 <= m <= 10^9

use std::io;

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

/// Runs one testcase
/// m: number of segments, counting from 1
/// n: number of jedi, counting from 1
/// un
fn testcase(m: u32, n: u16){

    // just for now
    let js = JediStart { pos: 1 };
    let je = JediEnd { pos: 2 };
}

fn main() {
    println!("Hello, world!"); 

    // run testcases
    // todo
    testcase(1,2);

    // read from stdin
    let reader = io::stdin();
    let mut input_text = String::new();
    reader.read_line(&mut input_text).expect("failed to read line");
    let input_opt = input_text.trim().parse::<u32>();
    let input_int = match input_opt {
        Ok(smth) => smth,
        Err(e) => {
            panic!("Parsing Failed.");
        }
    };
    println!("Parsed Integer {}", input_int);
}
