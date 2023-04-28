//Chinaza Ojukwu                        30131145
/// -------------------------------------------------------------------------------------
/// this is the only file you need to edit
/// -------------------------------------------------------------------------------------
///
/// (c) 2023, Pavol Federl, pfederl@ucalgary.ca
/// Do not distribute this file.

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>
#include <cstring>
#include <algorithm>





struct Partition 
{
  int tag;
  int64_t size, addr;

  Partition(int tag2, int64_t size2, int64_t addr2)
  {
    this->tag = tag2;
    this->addr = addr2;
    this->size = size2;
  }


};

typedef std::list<Partition>::iterator PartitionRef;
typedef std::set<PartitionRef>::iterator TreeRef;

struct scmp {
 bool operator()(const PartitionRef & c1, const PartitionRef & c2) const {
 if (c1->size == c2->size)
 return c1->addr < c2->addr;
 else
 return c1->size > c2->size;
 }
};



// I suggest you implement the simulator as a class, like the one below.
// If you decide not to use this class, feel free to remove it.
//Skeleton code from Anshdeep's D2L hints
struct Simulator 
{
    std::list<Partition>  all_blocks;
    std::set<PartitionRef, scmp> free_blocks;
    std::unordered_map<long, std::vector<PartitionRef>> tagged_blocks;
                           
    int64_t page_size2;
    int64_t rq;

  
  Simulator(int64_t page_size) 
  {
    this->all_blocks = std::list<Partition>();
    this->page_size2 = page_size;
    this->rq = 0;
    this->tagged_blocks = std::unordered_map<long, std::vector<PartitionRef>>();

  }

   int64_t load(int64_t z)
  {
    if( z % this->page_size2 != 0)
    {
      return (z/this->page_size2) +1;
    }

    else
    {
      return (z / this->page_size2);
    }
  }

   void remove(PartitionRef pr)
  {
    if(this->free_blocks.find(pr) != this->free_blocks.end())
    {
      TreeRef parRef = this->free_blocks.find(pr);
      this->free_blocks.erase(parRef);
    }

  }


 
  void allocate(int tag, int size)
  {
    // Pseudocode for allocation request:
    // - search through the list of partitions from start to end, and
    //   find the largest partition that fits requested size
    //     - in case of ties, pick the first partition found
    // - if no suitable partition found:
    //     - get minimum number of pages from OS, but consider the
    //       case when last partition is free
    //     - add the new memory at the end of partition list
    //     - the last partition will be the best partition
    // - split the best partition in two if necessary
    //     - mark the first partition occupied, and store the tag in it
    //     - mark the second partition free



    bool ready = false;
    
   //inital run
    if(all_blocks.size() == 0)
     {  
      Partition front = Partition(0, this->page_size2*this->load(size) , 0);
      this->rq = this->rq + this->load(size);      
      this->all_blocks.push_back(front);
      this->free_blocks.insert(this->all_blocks.begin());
    }


    PartitionRef cptr;    

      if(this->free_blocks.size() != 0)
     {
      TreeRef i = this->free_blocks.begin();
      PartitionRef i2 = *i;

      if( size <= i2->size)
      {
        this->free_blocks.erase(i);
        cptr = i2;
        ready = true;
      }
    }

    

   
   // open spot
    if(ready)
    {
      if (0 < cptr->size - size) 
      {
        this->all_blocks.insert(std::next(cptr), Partition(0, cptr->size - size, cptr->addr + size));
        this->free_blocks.insert(std::next(cptr));
      }      
      cptr->tag = tag;
      cptr->size = size;
      this->tagged_blocks[tag].push_back(cptr);
    }

  //new space
   else
  {

      PartitionRef end = std::prev(this->all_blocks.end());

      if (!(end->tag == 0))
      {
 this->rq = this->rq + this->load(size);
        int64_t y =  this->load(size)*this->page_size2;
        this->all_blocks.push_back(Partition(tag, size, end->addr + end->size));
        this->tagged_blocks[tag].push_back(std::prev(this->all_blocks.end()));
        
        if (y-size > 0) 
        {
          this->all_blocks.push_back(Partition(0, y-size, end->addr + end->size + size));
          this->free_blocks.insert(std::prev(this->all_blocks.end()));
        }

      } 
      else 
      {
        
       int64_t y =  end->size +  this->load(size - end->size) * this->page_size2;
       this->rq = this->rq + this->load(size - end->size);

        this->remove(end);
         end->size = size;
        end->tag = tag;
        this->tagged_blocks[tag].push_back(end);
        y = y - size;

        if (0 < y) 
        {
          this->all_blocks.push_back(Partition(0, y,end->addr + end->size));
          this->free_blocks.insert(std::prev(this->all_blocks.end()));
        }
       
      }

  }
  }




void deallocate(int tag) 
{

    // Pseudocode for deallocation request:
    // - for every partition
    //     - if partition is occupied and has a matching tag:
    //         - mark the partition free
    //         - merge any adjacent free partitions
  
    for (auto i = this->tagged_blocks[tag].begin(); i != this->tagged_blocks[tag].end(); ++i) 
    {
        PartitionRef pr = *i;
        pr->tag = 0;
    //try switch
        switch (pr != this->all_blocks.begin()) 
        {
            case true:
                PartitionRef previous = std::prev(pr);
               
                switch (previous->tag == 0) 
                {
                    case true:
                        pr->addr = previous->addr; pr->size = pr->size + previous->size;
                        this->remove(previous);
                        this->all_blocks.erase(previous);
                        break;
                }
                break;
           
        }

        switch (pr != std::prev(this->all_blocks.end())) 
        {
            case true:
                PartitionRef next = std::next(pr);
                switch (next->tag == 0) 
                {
                    case true:
                        pr->size = pr->size + next->size;
                        this->all_blocks.erase(next);
                        this->remove(next);
                        break;
                }
                break;
        }
        this->free_blocks.insert(pr);
    }

    this->tagged_blocks[tag].clear();
}


  MemSimResult getStats()
  {
    // hard-coded results
       MemSimResult result;

    if (this->free_blocks.size() != 0) 
    {
            PartitionRef free_block_head = *this->free_blocks.begin();

      result.max_free_partition_address = free_block_head->addr;
      result.max_free_partition_size = free_block_head->size;
      result.n_pages_requested = this->rq;
      return result;
    } 

    else 
    {
      result.max_free_partition_address = 0;
      result.max_free_partition_size = 0;
      result.n_pages_requested = this->rq;
      return result;
    }

  }

   void check_consistency()
  {
    // mem_sim() calls this after every request to make sure all data structures
    // are consistent. Since this will probablly slow down your code, I suggest
    // you comment out the call below before submitting your code for grading.
    check_consistency_internal();
  }
  void check_consistency_internal()
  {
    // you do not need to implement this method at all - this is just my suggestion
    // to help you with debugging

    // here are some suggestions for consistency checks (see appendix also):

    // make sure the sum of all partition sizes in your linked list is
    // the same as number of page requests * page_size

    // make sure your addresses are correct

    // make sure the number of all partitions in your tag data structure +
    // number of partitions in your free blocks is the same as the size
    // of the linked list

    // make sure that every free partition is in free blocks

    // make sure that every partition in free_blocks is actually free

    // make sure that none of the partition sizes or addresses are < 1
  }
  };

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // if you decide to use the simulator class above, you likely do not need
  // to modify the code below at all
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    //sim.check_consistency();
  }
  return sim.getStats();
}



