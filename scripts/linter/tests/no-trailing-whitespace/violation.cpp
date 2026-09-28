namespace wisdom 
{
    // A comment.  
    auto answer() -> int
    {
        
        return 42;  
    }

    /* A block comment 
       over two lines. */
    auto text() -> std::string_view
    {
        return R"(first   
second)"; 
    }
}
