//! Reimplementation of attack of the clones 
//! from algolab. But in rust, this time.
//!
//! we have `m` segments, with 1 <= m <= 10^9

/// a Jedi is an interval including both start and end segments and anything in between.
struct Jedi {
    /// start and end should both be $\leq m$, which is the number of test cases
    start: u32,
    /// start and end should both be $\leq m$, which is the number of test cases
    end: u32,
}

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
}
