/*
 * mem_sim.cpp
 * Cache and memory simulator
 * Baron Khan
 * bak14

 * References:
 * 1. getline() - http://www.cplusplus.com/reference/istream/istream/getline/
 * 2. Reading hex value from stdin - http://stackoverflow.com/questions/13196589/getting-hex-through-cin
 * 3. Different shell script formats for Linux/Windows - http://askubuntu.com/questions/304999/not-able-to-execute-a-sh-file-bin-bashm-bad-interpreter
 */

#include <iostream>
#include <sstream>
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <string>

#define READ_HIT 1
#define READ_MISS 0

using namespace std;


/* STRUCTURES */

struct Word {
        int word_size;
        vector<uint8_t> data;
        //constructor
        Word(int bytes_per_word): word_size(bytes_per_word) {
                for (int i=0; i<bytes_per_word; i++) {
                        data.push_back(0);
                }
        }
};

struct Block {
        bool valid;
        bool dirty;
        int tag;
        int block_size;
        vector<Word> words;
        //constructor
        Block(int bytes_per_word, int words_per_block): block_size(words_per_block), valid(false), dirty(false) {
                for (int i=0; i < words_per_block; i++) {
                        words.push_back(Word(bytes_per_word));
                }
        }
};


struct Mem {
        uint32_t total_size;    //in blocks
        uint32_t block_size;    //in words
        uint32_t word_size;     //in bytes
        vector<Block> blocks;
        //constructor
        Mem(int total_blocks,int words_per_block,int bytes_per_word): total_size(total_blocks), block_size(words_per_block), word_size(bytes_per_word) {
                for (int i=0; i < total_blocks; i++) {
                        blocks.push_back(Block(bytes_per_word, words_per_block));
                }
        }

};


//Note: a set represents a line of cache
struct CacheSet {
        int set_size;
        vector<Block> blocks;
        vector<int> block_order;     //change order of block indices in list for LRU, move index to back when accessed.
        //constructor
        CacheSet(int bytes_per_word, int words_per_block, int blocks_per_set): set_size(blocks_per_set) {
                for (int i=0; i < blocks_per_set; i++) {
                        blocks.push_back(Block(bytes_per_word, words_per_block));
                        block_order.push_back(i);
                }
        }
};


struct Cache {
        int cache_size;
        Mem* mem;
        vector<CacheSet> sets;
        //constructor
        Cache(int bytes_per_word, int words_per_block, int blocks_per_set, int sets_per_cache, Mem* m): mem(m), cache_size(sets_per_cache){
                for (int i=0; i < sets_per_cache; i++) {
                        sets.push_back(CacheSet(bytes_per_word, words_per_block, blocks_per_set));
                }
        }
};


/* FUNCTIONS */

uint8_t cache_read_byte(const Cache& cache, int set_index, int block_index, int word_index, int byte_index);

void cache_read_word(const Cache& cache, int set_index, int block_index, int word_index, Word& word);

void cache_write_word(Cache& cache, int set_index, int block_index, int word_index, const Word& word);

string word_to_str(const Word& word);

void str_to_word(const string& data, Word& word);

void cache_read_block(const Cache& cache, int set_index, int block_index, Block& block);

void cache_write_block(Cache& cache, int set_index, int block_index, const Block& block);

void debug_request(const Cache& cache);

bool read_request(Cache& cache, unsigned addr, int& set_index, string& data, int& time);

bool write_request(Cache& cache, unsigned addr, int& set_index, const string& data, int& time);

void blocks_MRU_place_last(vector<int>& block_order, int element);

uint8_t mem_read_byte(Mem* mem, int block_address, int word_index, int byte_index);

void mem_read_word(Mem* mem, int block_address, int word_index, Word& word);

void mem_read_block(Mem* mem, int block_address, Block& block);

void mem_write_block(Mem* mem, int block_address, const Block& block);

int flush_request(Cache& cache);

//No longer used
//unsigned word_to_val(const Word& word);
//void val_to_word(uint64_t data, Word& word);


/* GLOBAL VARIABLES */

//initialise with default values
unsigned address_bits = 8;
unsigned bytes_per_word = 2;
unsigned words_per_block = 2;
unsigned blocks_per_set = 1;
unsigned sets_per_cache = 2;
unsigned hit_time = 1;
unsigned read_time = 2;
unsigned write_time = 2;


/* MAIN */

int main(int argc, char *argv[]) {

        //Debugging Mode

        //ostream& debug = cerr;		//Debugging ON
        stringstream debug;		        //Debugging OFF


        //Input parameters
        if (argc >= 9) {
                debug << "Command line parameters: " << endl;
                for (int i=1; i < argc; i++) {
                        debug << argv[i] << " ";
                }
                debug << endl;

                address_bits = atoi(argv[1]);
                bytes_per_word = atoi(argv[2]);
                words_per_block = atoi(argv[3]);
                blocks_per_set = atoi(argv[4]);
                sets_per_cache = atoi(argv[5]);
                hit_time = atoi(argv[6]);
                read_time = atoi(argv[7]);
                write_time = atoi(argv[8]);
        }



        //Determine size of mem (in blocks) from number of address bits
        int mem_size = (pow(2,address_bits))/(bytes_per_word*words_per_block);
        debug << "memory size is " << mem_size << " blocks" << endl;

        Mem* mem = new Mem(mem_size, words_per_block, bytes_per_word);
        Cache cache(bytes_per_word, words_per_block, blocks_per_set, sets_per_cache, mem);


        //Debugging leftovers
        /*
        debug << "Cache initialised to all " << hex << unsigned(cache.sets[0].blocks[0].words[0].data[1]) << endl;
        Word word_temp(bytes_per_word);
        cache_read_word(cache, 0, 0, 0, word_temp);
        debug << "word at cache address 0: " << setw(bytes_per_word*2) << setfill('0') << hex << word_to_str(word_temp) << endl;

        mem_read_word(mem, 0, 0, word_temp);
        debug << "word at mem address 0: " << setw(bytes_per_word*2) << setfill('0') << hex << word_to_str(word_temp) << endl;
        string data_s = "89AB";
        str_to_word(data_s, word_temp);
        cache_write_word(cache, 0, 0, 0, word_temp);
        debug << "writing 0x89AB to cache address 0: " << endl;
        cache_read_word(cache, 0, 0, 0, word_temp);
        debug << "new word at cache address 0: " << setw(bytes_per_word*2) << setfill('0') << hex << word_to_str(word_temp) << endl;
        */


        //Read input
        string cmd;
        char line[1024];

        while (cin.getline(line, 1024)) {
                if(line[0] != '#') { // comment line - see Reference 1
                        // parse line
                        stringstream ss(line);
                        ss >> cmd;
                        debug << "current cmd is " << ss.str() << endl;

                        if(cmd == "read-req") {
                                unsigned addr;
                                debug << "# This is a read request. Please type address" << endl;
                                ss >>  dec >> addr;
                                debug << "# Reading from address 0x" << hex << addr << endl;


                                // Now handle a read at address "addr"
                                int set_index = 0;
                                bool hit = true;
                                //uint64_t data = 0;
                                string data;
                                int time = 0;

                                hit = read_request(cache, addr, set_index, data, time);
                                debug << "finished read-req" << endl;

                                cout << "read-ack " << dec << set_index << ((hit) ? " hit " : " miss ") << dec << time << " " <<
                                setw(bytes_per_word*2) << setfill('0') << hex << uppercase << data << endl;

                        }
                        else if(cmd == "write-req") {
                                unsigned addr;
                                string data;
                                debug << "# This is a write request. Please type address" << endl;
                                ss >> dec >> addr;
                                debug << "# What data to write?" << endl;
                                ss >> hex >> data; // Read hex value - see Reference 2 in discussion.md
                                debug << "# Writing to byte address " << dec << addr << " value 0x" << hex << data << endl;

                                //write request
                                int set_index = 0;
                                bool hit = true;
                                int time = 0;

                                hit = write_request(cache, addr, set_index, data, time);
                                debug << "finished read-req" << endl;

                                cout << "write-ack " << dec << set_index << ((hit) ? " hit " : " miss ") << dec << time << endl;


                        }
                        else if(cmd == "flush-req") {
                                debug << "# This is a flush request" << endl;
                                //flush request
                                int time = flush_request(cache);
                                cout << "flush-ack " << dec << time << endl;

                        }
                        else if(cmd == "debug-req") {
                                debug << "# This is a debug request" << endl;
                                //debug request
                                cout << "debug-ack-begin" << endl;
                                //print debugging info
                                debug_request(cache);
                                cout << "debug-ack-end" << endl;

                        }
                }
        }

        return 0;
}


/* FUNCTIONS */

uint8_t cache_read_byte(const Cache& cache, int set_index, int block_index, int word_index, int byte_index) {
        return cache.sets[set_index].blocks[block_index].words[word_index].data[byte_index];
}


void cache_read_word(const Cache& cache, int set_index, int block_index, int word_index, Word& word) {
        word = cache.sets[set_index].blocks[block_index].words[word_index];
}


void cache_write_word(Cache& cache, int set_index, int block_index, int word_index, const Word& word) {
        cache.sets[set_index].blocks[block_index].words[word_index] = word;
}



string word_to_str(const Word& word) {
        stringstream temp;
        for (int i=bytes_per_word-1; i>-1; i--) {
                //MSB has higher index
                temp << setw(2) << setfill('0') << hex << uppercase << unsigned(word.data[i]);
        }
        return temp.str();
}


void str_to_word(const string& data, Word& word) {
        for (int i=0; i < bytes_per_word; i++) {
                //MSB has higher index
                stringstream temp;
                if ((i*2) < data.length()) {
                        temp << hex << data.substr(i*2,2);
                        unsigned value;
                        temp >> hex >> value;
                        word.data[bytes_per_word-1-i] = value;
                }
        }
}


void cache_read_block(const Cache& cache, int set_index, int block_index, Block& block) {
        block = cache.sets[set_index].blocks[block_index];
}


void cache_write_block(Cache& cache, int set_index, int block_index, const Block& block) {
        cache.sets[set_index].blocks[block_index] = block;
}


void debug_request(const Cache& cache) {
        cerr << "# [Printing cache state to stderr on debug request]: " << endl;
        for (int i=0; i < cache.sets.size(); i++) {
                cerr << "# S" << dec << i << "={";
                for (int j=0; j < cache.sets[i].blocks.size(); j++) {
                        if (cache.sets[i].blocks[j].valid)
                                cerr << "B" << dec << cache.sets[i].blocks[j].tag << ((cache.sets[i].blocks[j].dirty) ? "/d" : "");
                        if (j != cache.sets[i].blocks.size()-1)
                                cerr << ",";
                }
                cerr << "}" << endl;
        }
        cerr << "# [Finished printing state]" << endl;
}


bool read_request(Cache& cache, unsigned addr, int& set_index, string& data, int& time) {
        int word_address = addr / bytes_per_word;
        int block_address = word_address / words_per_block;
        //cerr << "block address is " << block_address << endl;
        int block_offset = word_address % words_per_block;      //word offset in block
        set_index = block_address % sets_per_cache;
        //cerr << "set index is " << set_index << endl;

        Mem* mem = cache.mem;
        bool valid;
        unsigned tag;
        //search blocks in set for correct block
        for (int i=0; i < blocks_per_set; i++) {
                valid = cache.sets[set_index].blocks[i].valid;
                tag = cache.sets[set_index].blocks[i].tag;
                if (valid && tag == block_address) {
                        //HIT!
                        time += hit_time;
                        //data = cache_get_word(cache, set_index, i, block_offset);
                        Word word_data(bytes_per_word);
                        cache_read_word(cache, set_index, i, block_offset, word_data);
                        data = word_to_str(word_data);
                        //must change block order and place i index last for LRU policy
                        blocks_MRU_place_last(cache.sets[set_index].block_order, i);
                        return READ_HIT;
                }
        }
        //MISS!
        int LRU_block_index = cache.sets[set_index].block_order[0];

        //check if bit is dirty of LRU block, if so, need to write old block to memory first
        if (cache.sets[set_index].blocks[LRU_block_index].dirty == true) {
                time+=write_time;
                //write old block to memory
                Block old_block(bytes_per_word, words_per_block);
                cache_read_block(cache, set_index, LRU_block_index, old_block);
                //find out where block comes from in memory by looking at tag.
                int mem_block_address = old_block.tag;
                mem_write_block(mem, mem_block_address, old_block);
                //block in cache is no longer dirty
                cache.sets[set_index].blocks[LRU_block_index].dirty = false;


        }

        //Read new block from memory and place in LRU block
        Block new_block(bytes_per_word, words_per_block);
        mem_read_block(mem, block_address, new_block);
        new_block.tag = block_address;
        new_block.dirty = false;
        new_block.valid = true;
        cache_write_block(cache, set_index, LRU_block_index, new_block);
        time+= read_time;


        //read word from new block in cache and store in data
        Word word_data(bytes_per_word);
        cache_read_word(cache, set_index, LRU_block_index, block_offset, word_data);
        data = word_to_str(word_data);
        time += hit_time;

        //must change block order and place LRU index last to become MRU
        //cerr << "LRU Index is " << LRU_block_index << endl;
        blocks_MRU_place_last(cache.sets[set_index].block_order, LRU_block_index);
        //cerr << "finished function" << endl;
        return READ_MISS;

}



bool write_request(Cache& cache, unsigned addr, int& set_index, const string& data, int& time) {
        int word_address = addr / bytes_per_word;
        int block_address = word_address / words_per_block;
        //cerr << "block address is " << block_address << endl;
        int block_offset = word_address % words_per_block;      //word offset in block
        set_index = block_address % sets_per_cache;
        //cerr << "set index is " << set_index << endl;

        Mem* mem = cache.mem;
        bool valid;
        unsigned tag;
        //search blocks in set for correct block
        for (int i=0; i < blocks_per_set; i++) {
                valid = cache.sets[set_index].blocks[i].valid;
                tag = cache.sets[set_index].blocks[i].tag;
                if (valid && tag == block_address) {
                        //HIT!
                        time += hit_time;
                        //write new word to block
                        Word word_data(bytes_per_word);
                        str_to_word(data, word_data);
                        cache_write_word(cache, set_index, i, block_offset, word_data);
                        cache.sets[set_index].blocks[i].dirty = true;
                        //must change block order and place i index last for LRU policy
                        blocks_MRU_place_last(cache.sets[set_index].block_order, i);
                        return READ_HIT;
                }
        }
        //MISS!
        int LRU_block_index = cache.sets[set_index].block_order[0];

        //check if bit is dirty, if so, need to write old block to memory first
        if (cache.sets[set_index].blocks[LRU_block_index].dirty == true) {
                time+=write_time;
                //write old block to memory
                Block old_block(bytes_per_word, words_per_block);
                cache_read_block(cache, set_index, LRU_block_index, old_block);
                //find out where block comes from in memory by looking at tag.
                int mem_block_address = old_block.tag;
                mem_write_block(mem, mem_block_address, old_block);
                //block in cache is no longer dirty
                cache.sets[set_index].blocks[LRU_block_index].dirty = false;


        }



        //Read new block from memory and place in LRU block

        Block new_block(bytes_per_word, words_per_block);
        //Note: If there is only one word per block, no need to bring new block from memory
        //(will be completely overwritten anyway)
        //=> Do NOT include time for memory read!
        if (words_per_block > 1) {
                mem_read_block(mem, block_address, new_block);
                time+= read_time;
        }
        new_block.tag = block_address;
        new_block.dirty = true;
        new_block.valid = true;
        cache_write_block(cache, set_index, LRU_block_index, new_block);



        //write new word onto new block in cache

        Word word_data(bytes_per_word);
        str_to_word(data, word_data);
        cache_write_word(cache, set_index, LRU_block_index, block_offset, word_data);
        cache.sets[set_index].blocks[LRU_block_index].dirty = true;
        time+=hit_time;

        //must change block order and place i index last for LRU policy
        blocks_MRU_place_last(cache.sets[set_index].block_order, LRU_block_index);

        return READ_MISS;



}


//Finds an element in the vector and places it at the end.
void blocks_MRU_place_last(vector<int>& block_order, int element) {
        if (block_order.size() == 1) {
                return;
        }
        bool found = false;
        int index = 0;
        int i = 0;
        while (!(found) && (i < block_order.size())) {
                if (block_order[i] == element) {
                        index = i;
                        found = true;
                }
                i++;
        }
        block_order.erase(block_order.begin()+(i-1));
        block_order.push_back(element);
}


uint8_t mem_read_byte(Mem* mem, int block_address, int word_index, int byte_index) {
        return mem->blocks[block_address].words[word_index].data[byte_index];
}

void mem_read_word(Mem* mem, int block_address, int word_index, Word& word) {
        word = mem->blocks[block_address].words[word_index];
}


void mem_read_block(Mem* mem, int block_address, Block& block) {
        block = mem->blocks[block_address];
}

void mem_write_block(Mem* mem, int block_address, const Block& block) {
        mem->blocks[block_address] = block;
}

int flush_request(Cache& cache) {
        int time = 0;
        for (int i=0; i<cache.sets.size(); i++) {
                for (int j=0; j<cache.sets[i].blocks.size(); j++) {
                        if (cache.sets[i].blocks[j].dirty == true) {
                                Block old_block(bytes_per_word, words_per_block);
                                cache_read_block(cache, i, j, old_block);
                                int mem_block_address = old_block.tag;
                                mem_write_block(cache.mem, mem_block_address, old_block);
                                cache.sets[i].blocks[j].dirty = false;
                                time+=write_time;
                        }
                }
        }
        return time;
}

/*No longer used
unsigned word_to_val(const Word& word) {
        uint64_t data = 0;
        for (int i=0; i<bytes_per_word; i++) {
                //MSB has higher index
                data = data | ((word.data[i]) << (i*8));
        }
        return data;
}

void val_to_word(uint64_t data, Word& word) {
        for (int i=0; i<bytes_per_word; i++) {
                //MSB has higher index
                word.data[i] = data >> ((i*8) & 0xFF);
        }
}
*/
