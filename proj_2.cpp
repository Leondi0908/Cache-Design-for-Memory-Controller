#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <array>
#include <bitset>



#define CACHE_SETS 16
#define MEM_SIZE 2048
#define CACHE_WAYS 1
#define BLOCK_SIZE 1 // bytes per block
#define DM 0
#define FA 1


using namespace std;

struct cache_set
{
	int tag; // you need to compute offset and index to find the tag.
	int lru_position; // for FA only
	int data; // the actual data stored in the cache/memory

	// For 4-ways-sets-associative
	int tag2[4]; // you need to compute offset and index to find the tag.
	int data2[4]; // the actual data stored in the cache/memory
	int lru_position2[4];
};


struct trace
{
	bool MemR;
	bool MemW;
	int adr;
	int data;
};

/*
Either implement your memory_controller here or use a separate .cpp/.c file for memory_controller and all the other functions inside it (e.g., LW, SW, Search, Evict, etc.)
*/

// Our Memory Controller that includes FSM Design, LW Design, SW Design, and MM Design.
int memory_controller(bool cur_MemR, bool cur_MemW, int &cur_data, int cur_adr, int status, int &miss, int type, int& hit, cache_set myCache[], int myMem[]);

// Helper Function to get the right tag bits
unsigned long right_Shift(bitset<32> instruction, int shift);

// LW design
bool lw_search(int& adr, int& data, int& miss, int& hit, int type, cache_set myCache[], int myMem[]);

// SW design
bool sw_search(int& adr, int& data, int type, cache_set myCache[], int myMem[]);

// The case Cache Miss in LW design, it also includes the Evict function
void CacheMiss(cache_set myCache[], int& data, int adr, int myMem[], int type, int index, int tag);

// Finding the best position to Evict
int find_LRUPosition(cache_set myCache[], int type, int index);

// Updating the LRU position everytime the block is refered
void update_LRU(cache_set myCache[], int type, int pos, int index);

// Updating the data and tag of spefic index in LW design
void update_data_and_tag(int type, int index, int pos, int data, int tag, cache_set myCache[]);

int main(int argc, char* argv[]) // the program runs like this: ./program <filename> <mode>
{
	// input file (i.e., test.txt)
	string filename = argv[1];

	// mode for replacement policy
	int type;

	ifstream fin;

	// opening file
	fin.open(filename.c_str());
	if (!fin) { // making sure the file is correctly opened
		cout << "Error opening " << filename << endl;
		exit(1);
	}

	if (argc > 2)
		type = stoi(argv[2]); // the input could be either 0 or 1 (for DM and FA)
	else type = 0;// the default is DM.


	// reading the text file
	string line;
	vector<trace> myTrace;
	int TraceSize = 0;
	string s1, s2, s3, s4;
	while (getline(fin, line))
	{
		stringstream ss(line);
		getline(ss, s1, ',');
		getline(ss, s2, ',');
		getline(ss, s3, ',');
		getline(ss, s4, ',');
		myTrace.push_back(trace());
		myTrace[TraceSize].MemR = stoi(s1);
		myTrace[TraceSize].MemW = stoi(s2);
		myTrace[TraceSize].adr = stoi(s3);
		myTrace[TraceSize].data = stoi(s4);
		//cout<<myTrace[TraceSize].MemW << endl;
		TraceSize += 1;
	}


	// defining a fully associative or direct-mapped cache
	cache_set myCache[CACHE_SETS]; // 1 set per line. 1B per Block
	// defining a 4-sets-ways-associative

	int myMem[MEM_SIZE];


	// initializing
	for (int i = 0; i < CACHE_SETS; i++)
	{
		myCache[i].tag = -1; // -1 indicates that the tag value is invalid. We don't use a separate VALID bit. 
		myCache[i].lru_position = i;  // 0 means lowest position
		myCache[i].data = 0;
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			myCache[i].tag2[j] = -1; // -1 indicates that the tag value is invalid. We don't use a separate VALID bit. 
			myCache[i].data2[j] = 0;
			myCache[i].lru_position2[j] = j;
		}

	// counters for miss rate
	int accessL = 0; //////
	int accessS = 0;
	int miss = 0; // this has to be updated inside your memory_controller
	int hit = 0;
	int status = 1;
	int clock = 0;
	int traceCounter = 0;
	bool cur_MemR;
	bool cur_MemW;
	int cur_adr;
	int cur_data;
	
	// this is the main loop of the code
	while (traceCounter < TraceSize) {
		if (status == 1)
		{
			cur_MemR = myTrace[traceCounter].MemR;
			cur_MemW = myTrace[traceCounter].MemW;
			cur_data = myTrace[traceCounter].data;
			cur_adr = myTrace[traceCounter].adr;
			traceCounter += 1;
			if (cur_MemR == 1)
				accessL += 1;
			else if (cur_MemW == 1)
				accessS += 1;
		}
		// YOUR CODE

		status = memory_controller(cur_MemR, cur_MemW, cur_data, cur_adr, status, miss, type, hit, myCache, myMem); // in your memory controller you need to implement your FSM, LW, SW, and MM. 
		clock += 1;
	}

	// Counting the case of last status is still in negative state.
	while (status < 1)
	{
		status = memory_controller(cur_MemR, cur_MemW, cur_data, cur_adr, status, miss, type, hit, myCache, myMem); // in your memory controller you need to implement your FSM, LW, SW, and MM. 
		clock += 1;
	}

	
	float miss_rate = miss / (float)accessL;
	
	// printing the final result
	cout << "Load Access: " << accessL << endl;
	cout << "Store Access: " << accessS << endl;
	cout << "Number of Instruction: " << traceCounter << endl;
	cout << "Number of Miss: " << miss << endl;
	cout << "Number of Hits: " << hit << endl;
	cout << "Number of Cycle: " << clock << endl;
	cout << "Miss Rate: " << miss_rate << endl;
	cout << "(" << clock << ", " << miss_rate << ")" << endl;
	// closing the file
	fin.close();

	return 0;
}

int memory_controller(bool cur_MemR, bool cur_MemW, int& cur_data, int cur_adr, int status, int& miss, int type, int& hit, cache_set myCache[], int myMem[])
{

	// FSM_Design 
	// Going to Control Signal
	if (status == 1)
	{
		
		// Going to LW design
		if (cur_MemR == 1)
		{
			if (lw_search(cur_adr, cur_data, miss, hit, type, myCache, myMem))
			{
				return 1;
			}
			else
			{
				return (-3);
			}
		}
		// Going to SW design
		else if (cur_MemW == 1)
		{
			if (sw_search(cur_adr, cur_data, type, myCache, myMem))
			{
				cur_data = 0;
				return 1;
			}
			else
			{
				cur_data = 0;
				return 1;
			}
		}
		// The case of R-Type
		else
		{
			cur_data = myMem[cur_adr];
			return 1;
		}
	}
	else if (status == 0)
	{
		cur_data = myMem[cur_adr];
		return 1;
	}
	// Memory Delay happening here
	else if (status < 0)
	{
		int temp = status;
		temp++;
		return (temp);
	}

		
}

unsigned long right_Shift(bitset<32> instruction, int shift)
{
	return((instruction.to_ulong()) >> shift);
}

bool lw_search(int& adr, int& data, int& miss, int& hit, int type, cache_set myCache[], int myMem[])
{
	int temp = adr;
	bitset<32> address = temp;
	bitset<28> tag = right_Shift(address, 4);
	bitset<4> index = temp;
	bitset<30> tag_2 = right_Shift(address, 2);
	bitset<2> index_2 = temp;

	// Direct-Mapped
	if (type == 0)
	{
		// if the condition True, it is a Hit, Miss otherwise.
		if (myCache[index.to_ulong()].tag == tag.to_ulong())
		{
			hit++;
			data = myCache[index.to_ulong()].data;
			return true;
		}
		else
		{
			miss++;
			CacheMiss(myCache, data, adr, myMem, type, index.to_ulong(), tag.to_ulong());
			return false;
		}
	}
	// Fully-Associative
	else if (type == 1)
	{
		for (int i = 0; i < CACHE_SETS; i++)
		{
			// if the condition True, it is a Hit, Miss otherwise.
			if (myCache[i].tag == address.to_ulong())
			{
				hit++;
				data = myCache[i].data;
				update_LRU(myCache, type, myCache[i].lru_position, i);
				myCache[i].lru_position = 15;
				
				return true;
			}
		}
		miss++;
		CacheMiss(myCache, data, adr, myMem, type, index.to_ulong(), address.to_ulong());
		return false;
	}
	// 4-ways-sets-associaive
	else
	{
		for (int i = 0; i < 4; i++)
		{
			// if the condition True, it is a Hit, Miss otherwise.
			if (myCache[index_2.to_ulong()].tag2[i] == tag_2.to_ulong())
			{
				hit++;
				data = myCache[index_2.to_ulong()].data2[i];
				update_LRU(myCache, type, myCache[index_2.to_ulong()].lru_position2[i], index_2.to_ulong());
				myCache[index_2.to_ulong()].lru_position2[i] = 3;
				return true;
			}
		}
		miss++;
		CacheMiss(myCache, data, adr, myMem, type, index_2.to_ulong(), tag_2.to_ulong());
		return false;
	}
}

bool sw_search(int& adr, int& data, int type, cache_set mycache[], int myMem[])
{
	bitset<32> address = adr;
	bitset<28> tag = right_Shift(address, 4);
	bitset<4> index = adr;
	bitset<30> tag_2 = right_Shift(address, 2);
	bitset<2> index_2 = adr;

	// Direct-Mapped
	if (type == 0)
	{
		// if the condition True, it is a Hit, Miss otherwise.
		if (mycache[index.to_ulong()].tag == tag.to_ulong())
		{
			mycache[index.to_ulong()].data = data;
			myMem[adr] = data;
			return true;
		}
		else
		{
			myMem[adr] = data;
			return false;
		}
	}
	// Fully-Associative
	else if (type == 1)
	{
		for (int i = 0; i < CACHE_SETS; i++)
		{
			// if the condition True, it is a Hit, Miss otherwise.
			if (mycache[i].tag == address.to_ulong())
			{
				mycache[i].data = data;
				myMem[adr] = data;
				return true;
			}
		}
		myMem[adr] = data;
		return false;
	}
	// 4-ways-sets-associaive
	else
	{
		for (int i = 0; i < 4; i++)
		{
			// if the condition True, it is a Hit, Miss otherwise.
			if (mycache[index_2.to_ulong()].tag2[i] == tag_2.to_ulong())
			{
				mycache[index_2.to_ulong()].data2[i] = data;
				myMem[adr] = data;
				return true;
			}
		}
		myMem[adr] = data;
		return false;
	}
}

void CacheMiss(cache_set myCache[], int& data, int adr, int myMem[], int type, int index, int tag)
{
	data = myMem[adr];
	// Evict
	// Finding the LRU Position that is zero, then return the index
	int pos = find_LRUPosition(myCache, type, index);
	// Seperating between DM, FA, and 4-way SA
	if (type == 0)
	{
		update_data_and_tag(type, index, pos, data, tag, myCache);
	}
	else if (type == 1)
	{
		update_data_and_tag(type, pos, index, data, tag, myCache);
	}
	else
		update_data_and_tag(type, index, pos, data, tag, myCache);
	
}

int find_LRUPosition(cache_set myCache[], int type, int index)
{
	if (type == 2)
	{
		for (int j = 0; j < 4; j++)
		{
			// Find the LRU from LRU_position 
			if (myCache[index].lru_position2[j] == 0)
			{
				// Updating the LRU postion
				update_LRU(myCache, type, myCache[index].lru_position2[j], index);
				// Change the LRU positon to the recent refered.
				myCache[index].lru_position2[j] = 3;
				return j;
			}
		}
	}
	else if (type == 1)
	{
		for (int i = 0; i < CACHE_SETS; i++)
		{
			if (myCache[i].lru_position == 0)
			{
				// Updating the LRU postion
				update_LRU(myCache, type, myCache[i].lru_position, index);
				// Change the LRU positon to the recent refered.
				myCache[i].lru_position = 15;
				return i;
			}
		}
	}
}
void update_LRU(cache_set myCache[], int type, int pos, int index)
{
	if (type == 2)
	{
		for (int j = 0; j < 4; j++)
		{
			// Everytime the LRU position of the block is more than the LRU position that is refered
			// Then the LRU position of the block will be minus one
			if (myCache[index].lru_position2[j] > pos)
			{
				myCache[index].lru_position2[j] = myCache[index].lru_position2[j] - 1;
			}
		}
	
			
	}
	else if (type == 1)
	{
		for (int i = 0; i < CACHE_SETS; i++)
		{
			// Everytime the LRU position of the block is more than the LRU position that is refered
			// Then the LRU position of the block will be minus one
			if (myCache[i].lru_position > pos)
			{
				myCache[i].lru_position = myCache[i].lru_position - 1;
			}
		}	
	}

}

void update_data_and_tag(int type, int index, int pos, int data, int tag, cache_set myCache[])
{
	if (type == 2)
	{
		myCache[index].tag2[pos] = tag;
		myCache[index].data2[pos] = data;
	}
	else 
	{
		myCache[index].tag = tag;
		myCache[index].data = data;
	}
	
}