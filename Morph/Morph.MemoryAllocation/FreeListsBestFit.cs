using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Morph.MemoryAllocation
{
    using Address = UInt32;
    using Size = UInt32;
    using Index = Int32;

    /**
     * Keeps a team of FreeList allocators for different chunk sizes. Each
     * allocator is based on a different block size. When allocating, the best
     * fit among the team is asked to allocate memory.
     * The free lists must be assigned to disjunct memory regions.
     */
    public class FreeListsBestFit
    {
        private FreeList[] pools;

        public unsafe void Initialize(FreeList[] pools_for_regions)
        {
            pools = new FreeList[pools_for_regions.Length];
            Array.Copy(pools, pools_for_regions, pools.Length);

            // Sort regions by start address (using bubble-sort)
            bool swapped = true;
            Index n = pools.Length;
            while (swapped) {
                swapped = false;
                for (Index j = 0; j < n; j++) {
                    if (pools[j].region.ram.start > pools[j + 1].region.ram.start) {
                        // swap j <-> j+1
                        var tmp = pools[j];
                        pools[j] = pools[j + 1];
                        pools[j + 1] = tmp;
                        swapped = true;
                    }
                }
                n--;
            }
        }

        //private FreeList
    }
}
