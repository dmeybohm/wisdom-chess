// A call like foo(x) in a comment is not code.
void example (int x)
{
    good (x); // but bad(x) in a trailing comment is not code either
    good (x); /* nor bad(x) here */
    /*
       nor bad(x) on a line inside a block comment,
     */
    good (x); /* and a block that starts here
       and ends here */ good (x);
    const char* text = "a string with bad(x) and // a fake comment";
    bad(x); // this one is real
}
