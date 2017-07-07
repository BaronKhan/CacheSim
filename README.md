Cache Simulator
=======
Architecture II coursework 2016. Cache and memory simulator.

### Implementation Approach

The parsing code was written first, making sure the I/O was correct.  
When performing a read request the data is read using the ‘setw()’ and  
setfill('0') functions to set the default width of the data, and ‘hex’  
to read it as a hex value. One slight change near the end of development  
was to read the word data as a string. This allowed the use of word sizes  
greater than 64-bit which was the integer size limit. Now any word size can  
be used. Using the cin.getline() function made it easier to determine  
comment lines and easily remove them.

The cache was then developed using data structures. The cache was  
created as a hierarchy as follows:  

- A ‘Cache’ object contains a vector of sets  
- A ‘Set’ object contains a vector of blocks  
- A ‘Block’ object contains a vector of words  
- A ‘Word’ object contains a vector of bytes (8 bit integer) set to 0  

Each vector size is determined by the command line parameters (which  
are made global for easier access in other functions). The vector  
container was chosen for ease of use (despite the larger overhead).  
For each struct, there is a constructor so that instantiating each  
object is made easier. The ‘Cache’ object has a pointer to a ‘Mem’  
object, which is the memory the cache is associated with. The ‘Mem’  
object also contains a vector of blocks.

Several functions were written for each request. Even though the objects’  
member fields are public (struct was used) it was easier to write  
functions which accessed the member fields, like ‘cache\_read_byte()’ and  
‘mem\_write\_block()’ etc, and use the functions instead.

For implementing the LRU write-back policy, each set has a vector,  
‘block\_order’, containing the index of each block in the set. When a  
block is accessed, its index is moved to the end of the ‘block\_order’  
list so the first index is always the LRU.

### Testing

Several caches were created such as a 4-way cache, a fully-associative  
cache, etc. A test script was written (one for Windows and Linux due to  
different line feed formats) which reads one of these caches, including  
the input, the expected output and the parameters. ‘stdout’ is redirected  
to the ‘.got’ file, and a comparison with the expected output is made.

### Debugging

Throughout the source file, there are debugging lines which print various  
debugging info. When debugging is on, there is a std::ostream object,  
‘debug’ which is set equal to ‘cerr’ to print out debugging comments as  
well as the output of the cache. When debugging is off, ‘debug’ is a  
‘std::stringstream’ object instead so that the comments are now stored  
in the stringstream object instead, removing the debugging. Sometimes,  
it was not possible to use ‘debug’ (e.g. in a function) so ‘std::cerr’  
was temporarily used, and these have been commented out from the code.

For the ‘debug-req’ request the state of the cache is printed. This  
includes all of the sets (and the blocks within the sets) as well as  
whether the blocks are dirty of not (“/d”).
