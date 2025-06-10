#include<iostream>
#include<vector>
#include<fstream>
#include<cmath>
#include<random>
using namespace std;

class metadata{
    public:
    int tag = -1;
    bool dirty = false;
    bool valid = false;
}; 

class cache{
    public:
    char** lines; 
    int line_size;
    int associativity;
    int cache_size;
    int no_of_lines;
    int rep_policy;
    int miss_penalty; 
    int write_allocate; 
    metadata* metadata_array= nullptr;

    //cache stats
    double no_of_hits= 0;
    double no_of_misses= 0;
    double load_hits= 0;
    double total_no_of_loads= 0;
    double store_hits= 0;
    double no_of_stores= 0;
    double no_of_write_backs=0;
    long long total_runtime= 0; //in cycles

    static cache* take_from_file(const string& config_file);
    void setup_cache(){
        no_of_lines = cache_size/line_size;
        lines= new char*[no_of_lines];
        for(int i=0; i< no_of_lines; i++){
            lines[i] = new char[line_size];
        }
        //cout<<"The no of lines in cache are: "<< no_of_lines<<endl;
        metadata_array= new metadata[no_of_lines];
        for(int i=0; i<no_of_lines; i++){
            metadata_array[i].tag= -1;
            metadata_array[i].dirty= false;
            metadata_array[i].valid= false;
        }
        post_setup_cache(); 
    }

    virtual void post_setup_cache()=0;

    void start_cache(const string& trace_file){
        string trace_line;
        string ls, address;
        int no_of_mem_ins;
        long tag, index, word;
        ifstream f2;
        int i=0;
        try{
            f2.open(trace_file);
            if (!f2.is_open()) {
                cout << "Error: Could not open file " << trace_file << endl;
                return;
            }
            while(getline(f2, trace_line)){
                ls= trace_line.substr(0, 1);
                address= trace_line.substr(2, 10);
                no_of_mem_ins= stoi(trace_line.substr(13, 1));
                try {
                    long int_addr = stol(address, nullptr, 16);
                    extract_tag(int_addr, tag, index, word);
                    //cout << "The address in dec is: " << int_addr << endl;
                    cout<<"the val of i is "<<i<<" ";
                }
                catch (const out_of_range& e) {
                    cerr << "Error: Integer out of range." << endl;
                }
                i++;                
                if(ls == "l"){
                    load(tag, index, no_of_mem_ins);
                }
                else{
                    store(tag, index, no_of_mem_ins);
                }
            }
        }
        catch(const exception& e){
            cerr<<"Exception "<<e.what()<<endl;
        }
        //cout<< "The no. of hits are "<< no_of_hits<< " and no. of load hits are "<< load_hits<<endl;
        //cout<< "The no. of misses are "<< no_of_misses<< " and no. of write backs are "<< no_of_write_backs <<endl;
        cout<<endl;
        cout<<"Total Hit rate= "<<(no_of_hits/(no_of_hits+no_of_misses))<<endl;
        //<< " and no of misses = "<< (no_of_misses/(no_of_hits+no_of_misses)) *100<<endl;
        cout<<"Load hit rate: "<< load_hits/total_no_of_loads<<endl;
        cout<<"Total store hit rate: "<< (store_hits/no_of_stores)<<endl;
        cout<<"Total runtime = "<< total_runtime<<endl;
        cout<<"AMAT= "<< 1 + (miss_penalty * (no_of_misses/(no_of_hits+no_of_misses)))<<endl;
        //cout<<total_no_of_loads<<endl;
        
    } 

    virtual void extract_tag(const long int_addr, long& tag, long& index, long& word)=0;
    virtual void load(long tag_got, long index, int no_of_mem_ins)=0;
    virtual void store(long tag_got, long index, int no_of_mem_ins)=0;

    virtual ~cache() {
        for (int i = 0; i < no_of_lines; i++) {
            delete[] lines[i];
            lines[i]= nullptr;
        }
        delete[] lines;
        lines= nullptr;
        delete[] metadata_array;
        metadata_array= nullptr; 
    }
};

class direct_mapped: public cache{
    public:
    string tag, index, offset;

    void extract_tag(const long int_addr, long& tag, long& index, long& word) override{
        int tag_bits, index_bits, word_bits;
        index_bits = log2(no_of_lines);
        word_bits = log2(line_size);
        tag_bits = 32 - (index_bits + word_bits);
        cout<<"No of tag bits: "<< tag_bits<< ", no. of index bits: "<< index_bits<< ", no of word bits: "<< word_bits<< endl;

        long tag_mask = ((1L << tag_bits) - 1) << (index_bits + word_bits);
        long index_mask = ((1L << index_bits) - 1) << word_bits;
        long offset_mask = (1L << word_bits) - 1;

        tag = (int_addr & tag_mask) >> (index_bits + word_bits);
        index = (int_addr & index_mask) >> word_bits;
        word = int_addr & offset_mask;

        cout << "Tag: " << tag << ", Index: " << index << ", Offset: " << word << endl;
    }

    void post_setup_cache() override{
        //yada yada
    }

    void load(long tag_got, long index, int no_of_mem_ins) override{
        total_no_of_loads++;
        total_runtime+= no_of_mem_ins;
        bool hit = (metadata_array[index].tag == tag_got);
        if(hit && metadata_array[index].valid == true){
            no_of_hits++;
            load_hits++;
            total_runtime++;
        }
        else{
            no_of_misses++;
            if (metadata_array[index].valid && metadata_array[index].dirty) {
                no_of_write_backs++;
                //assume that the penalty for write back is the same as the miss penalty as each requires a memory fetch.
                total_runtime+= (miss_penalty*2);
            }    
            metadata_array[index].tag= tag_got;
            metadata_array[index].valid= true;
            metadata_array[index].dirty= false;
        }
    }

    void store(long tag_got, long index, int no_of_mem_ins) override{
        no_of_stores++;
        total_runtime+= no_of_mem_ins;
        bool hit = (metadata_array[index].tag == tag_got);
        if(hit){
            total_runtime++;
            no_of_hits++;
            store_hits++;
            metadata_array[index].dirty=true;
        }
        else{
            no_of_misses++;
            if(write_allocate == 1){
                {
                    if(metadata_array[index].dirty){
                        no_of_write_backs++;
                        total_runtime+= miss_penalty;
                    }
                    metadata_array[index] = {tag_got, true, false};
                }
            }
            total_runtime+= miss_penalty;
        }
    }
};

class associative : public cache{
    public:
    vector<unsigned int> last_access_time;
    unsigned int global_time = 0;
    void extract_tag(const long int_addr, long& tag, long& index, long& word) override {
        int word_bits= log2(line_size);
        long offset_mask= (1L<<word_bits)-1;

        tag= int_addr >> word_bits;
        word= int_addr & offset_mask;
        cout<<"No of tag bits: "<< tag<< ", no. of index bits: "<< word<<endl;
    }

    void post_setup_cache() override{
        last_access_time.resize(no_of_lines, 0);
    }

    int find_replacement_index(){
        if(rep_policy == 0){
            //some random code for random replacement
            static std::random_device rd; //Seed generator
            static std::mt19937 gen(rd()); //Mersenne Twister engine
            std::uniform_int_distribution<> distrib(0, no_of_lines - 1); //Distribution

    return distrib(gen);

        }
        else if(rep_policy ==1){
            //code for LRU
            unsigned int min_time = last_access_time[0];
            int lru_index = 0;
            for (int i = 1; i < no_of_lines; ++i) {
                if (last_access_time[i] < min_time) {
                    min_time = last_access_time[i];
                    lru_index = i;
                }
            }
            return lru_index;
        }
    }

    void update_lru(int index){
        last_access_time[index]= global_time++;
    }

    void load(long tag_got, long index, int no_of_mem_ins) override {
        total_no_of_loads++;
        total_runtime += no_of_mem_ins;
        bool hit = false;
        int hit_index = -1;
        for (int i = 0; i < no_of_lines; i++) {
            if (metadata_array[i].tag == tag_got && metadata_array[i].valid) {
                hit = true;
                hit_index = i;
                break;
            }
        }
        if (hit) {
            no_of_hits++;
            load_hits++;
            total_runtime++;
            update_lru(hit_index);
        } 
        else {
            no_of_misses++;
            int replace_index = find_replacement_index();
            if(metadata_array[replace_index].valid && metadata_array[replace_index].dirty){
                no_of_write_backs++;
                total_runtime+= miss_penalty*2;
            }
    
            metadata_array[replace_index].tag = tag_got;
            metadata_array[replace_index].valid = true;
            metadata_array[replace_index].dirty = false;
        }
    }

    void store(long tag_got, long index, int no_of_mem_ins) override {
        no_of_stores++;
        total_runtime += no_of_mem_ins;
        bool hit = false;
        int hit_index = -1;

        for (int i = 0; i < no_of_lines; ++i) {
            if (metadata_array[i].tag == tag_got && metadata_array[i].valid) {
                hit = true;
                hit_index = i;
                break;
            }
        }
        if(hit){
            total_runtime++;
            no_of_hits++;
            store_hits++;
            metadata_array[hit_index].dirty = true;
            update_lru(hit_index);
        } 
        else{
            no_of_misses++;
            int replace_index = find_replacement_index(); 
            if(write_allocate == 1){
                if(metadata_array[replace_index].valid && metadata_array[replace_index].dirty){
                    no_of_write_backs++;
                    total_runtime+= miss_penalty;
                }
                metadata_array[replace_index].tag = tag_got;
                metadata_array[replace_index].valid = true;
                metadata_array[replace_index].dirty = true;
                update_lru(replace_index);
            }
            total_runtime += miss_penalty;
        }
    }
};

class set_associative: public cache{
    public:
    vector<vector<unsigned int>> last_access_time;
    unsigned int global_time = 0;

    void extract_tag(const long int_addr, long& tag, long& index, long& word) override {
        int set_bits= log2(no_of_lines / associativity);
        int word_bits= log2(line_size);

        long set_mask= ((1L << set_bits) - 1) << word_bits;
        long offset_mask= (1L << word_bits) - 1;

        index= (int_addr&set_mask)>>word_bits; 
        tag= int_addr>>(set_bits + word_bits);
        word= int_addr&offset_mask;

        cout<< "Tag: "<<tag<< ", Index: "<< index<<", Offset: "<< word<< endl;
    }
    void post_setup_cache() override {
        last_access_time.resize(no_of_lines/associativity);
        for (auto& set : last_access_time) {
            set.resize(associativity, 0);
        }
    }
    int find_replacement_index(int set_index) {
        //random replacement
        if (rep_policy == 0) { 
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, associativity - 1);
            return distrib(gen);
        } 
        //LRU replacement
        else if (rep_policy == 1) { 
            unsigned int min_time = last_access_time[set_index][0];
            int lru_way = 0;
            for (int i = 1; i < associativity; ++i) {
                if (last_access_time[set_index][i] < min_time) {
                    min_time = last_access_time[set_index][i];
                    lru_way = i;
                }
            }
            return lru_way;
        }
        return 0;
    }

    void update_lru(int set_index, int way) {
        last_access_time[set_index][way] = global_time++;
    }
    void load(long tag_got, long index, int no_of_mem_ins) override {
        total_no_of_loads++;
        total_runtime += no_of_mem_ins;
        bool hit = false;
        int hit_way = -1;

        for (int way = 0; way < associativity; ++way) {
            if (metadata_array[index * associativity + way].tag == tag_got && metadata_array[index * associativity + way].valid) {
                hit = true;
                hit_way = way;
                break;
            }
        }
        if (hit) {
            no_of_hits++;
            load_hits++;
            total_runtime++;
            update_lru(index, hit_way);
        } 
        else {
            no_of_misses++;
            int replace_way = find_replacement_index(index);
            if(metadata_array[index * associativity + replace_way].valid && metadata_array[index * associativity + replace_way].dirty){
                no_of_write_backs++;
                total_runtime += miss_penalty*2;
            }

            metadata_array[index * associativity + replace_way].tag = tag_got;
            metadata_array[index * associativity + replace_way].valid = true;
            metadata_array[index * associativity + replace_way].dirty = false;
            update_lru(index, replace_way);
        }
    }
    void store(long tag_got, long index, int no_of_mem_ins) override {
        no_of_stores++;
        total_runtime += no_of_mem_ins;
        bool hit = false;
        int hit_way = -1;

        for (int way = 0; way < associativity; ++way) {
            if (metadata_array[index * associativity + way].tag == tag_got && metadata_array[index * associativity + way].valid) {
                hit = true;
                hit_way = way;
                break;
            }
        }

        if (hit) {
            total_runtime++;
            no_of_hits++;
            store_hits++;
            metadata_array[index * associativity + hit_way].dirty = true;
            update_lru(index, hit_way);
        } else {
            no_of_misses++;
            int replace_way = find_replacement_index(index);
            if (write_allocate == 1) {
                if(metadata_array[index * associativity + replace_way].valid && metadata_array[index * associativity + replace_way].dirty){
                    no_of_write_backs++;
                    total_runtime += miss_penalty;
                }
                metadata_array[index * associativity + replace_way].tag = tag_got;
                metadata_array[index * associativity + replace_way].valid = true;
                metadata_array[index * associativity + replace_way].dirty = true;
                update_lru(index, replace_way);
            }
            total_runtime += miss_penalty;
        }
    }
};

cache* cache::take_from_file(const string& config_file){
    int params[6] = {0};
    ifstream f1;
    string line;
    int i=0;
    try{
        f1.open(config_file);
        if (!f1.is_open()) {
            cout << "Error: Could not open file " << config_file << endl;
            return nullptr;
        }
        while(getline(f1, line) && i<6){
        params[i]= stoi(line);
        i++;
    }
    }
    catch(const exception& e){
        cerr<<"Exception "<<e.what()<<endl;
    }
    f1.close();
    cache* mycache= nullptr;
    if(params[1] == 1){
        mycache= new direct_mapped();
    }
    else if(params[1] == 0){
        mycache= new associative();
    }
    else{
        mycache= new set_associative();
    }

    if (mycache) {
        mycache->line_size = params[0];
        mycache->associativity = params[1];
        mycache->cache_size = (params[2]) * 1024;
        mycache->rep_policy = params[3];
        mycache->miss_penalty = params[4];
        mycache->write_allocate = params[5];
        mycache->no_of_lines = mycache->cache_size/mycache->line_size;
        mycache->lines= new char*[mycache->no_of_lines];
        for(int i=0; i< mycache->no_of_lines; i++){
            mycache->lines[i] = new char[params[0]];
        }
    }
    if (mycache!= nullptr){
        return mycache;
    }
    cout<<"Error";
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file> <trace_file>" << std::endl;
        return 1; //error
    }

    std::string configFilePath = argv[1];
    std::string traceFilePath = argv[2];

    cache* mycache = cache::take_from_file(configFilePath);

    if (mycache) {
        mycache->setup_cache();
        mycache->start_cache(traceFilePath);
        delete mycache;
        return 0; //all okay
    } else {
        std::cerr << "Error: Could not initialize cache from config file: " << configFilePath << std::endl;
        return 1; //error
    }
}
