#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H


#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>

using namespace std;

struct cache_set
{
	int tag; // you need to compute offset and index to find the tag.
	int lru_position; // for FA only
	int data; // the actual data stored in the cache/memory
	// add more things here if needed
};

class Memory_Controller
{
public:
	Memory_Controller(int size)
	{
		sets.resize(size);
		for (int i = 0; i < sets.size(); i++)
		{
			sets[i].tag = 0;
			sets[i].lru_position = i;
			sets[i].data = 0;
		}

	}


	void Fsm_Design()
	{
		if (load == 1)
		{
			
		}
		else if (store == 1)
		{

		}
		else if (load != 1 && store != 1)
		{
			
		}
	}
	
	bool search(int &adr, int &data, int type)
	{
		bitset<32> address = adr;
		bitset<28> tag = right_Shift(address, 4);
		bitset<4> index = adr;
		bitset<30> tag_2 = adr;
		bitset<2> index_2 = right_Shift(address, 2);
		
		// Direct-Mapped
		if (type == 0)
		{
			if (sets[index.to_ulong()].tag == tag.to_ulong())
			{
				data = sets[index.to_ulong()].data;
				return true;
			}
			else
				return false;
				
		}

		// Fully-Associative
		else if (type == 1)
		{
			for (int i = 0; i < sets.size(); i++)
			{
				if (sets[i].tag == tag.to_ulong())
				{
					data = sets[i].data;
					return true;
				}
			}
			return false;
		}
		
		// 4-ways-sets-associaive
		else
		{			

		}

		return false;
	}

private:
	vector<cache_set> sets;
	vector< vector<cache_set> > four_sets;

	bool load;
	bool store;
	bool r_type;
	bool hit;


	unsigned long right_Shift(bitset<32> instruction, int shift)
	{
		return((instruction.to_ulong()) >> shift);
	}

	// Find the least reference block
	int LRU_counter()
	{
		int min = 0;
		int temp = set_counter[0];
		for (int i = 1; i < set_counter.size(); i++)
		{
			if (temp > set_counter[i])
			{
				min = i;
				temp = set_counter[i];
			}
		}
		return min;
	}


};

#endif // FSM_DESIGN_H