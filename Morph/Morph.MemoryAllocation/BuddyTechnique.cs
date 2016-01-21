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
     * A very fast and fairly efficient memory allocation scheme based on fixed block sizes of
     * increasing powers of two, each of which may be split into half to accommodate a smaller
     * chunk.
     *
     * @see http://en.wikipedia.org/wiki/Buddy_memory_allocation
     */
    public unsafe class BuddyTechnique : IMemoryAllocation
    {
        FreeList[] pools;
        Index[] allocedSizes;

        public void Initialize(RAM ram, Size largestBlockSize, Size smallestBlockSize)
        {
            Size numPools = 1;
            //creating a FreeList for each block size
            for (Size s = largestBlockSize; s > smallestBlockSize; s >>= 1) {
                numPools += 1;
            }

            // @ohno I have to use "new" here... how to avert this???
            pools = new FreeList[numPools];
            Size bs = largestBlockSize;

            for (Index pool_index = 0; pool_index < pools.Length; pool_index++, bs >>= 1) {
                pools[pool_index] = new FreeList();
                pools[pool_index].Initialize(ram, bs, pool_index == 0);
            }

            allocedSizes = new Index[pools[pools.Length - 1].region.numBlocks];
        }

        #region Memory Allocator API

        public void *Alloc(Size numBytes)
        {
            for (int pool_index = pools.Length - 1; pool_index >= 0; --pool_index) {
                var pool = pools[pool_index];
                if (pool.region.blockSize >= numBytes && pool.CanAlloc()) {
                    void *block = pool.Alloc(numBytes);
                    // Split block in half -- if our block size is twice the size and it's not the
                    // smallest block
                    while (pool.region.blockSize >= numBytes * 2 && pool_index < pools.Length - 1) {
                        pool = pools[++pool_index];
                        pool.Free((void*)( (Address)block + pool.region.blockSize ));
                    }
                    //this sets for which blockSize this smallest block is allocated.
                    allocedSizes[LastPool.region.BlockIndex(block)] = pool_index;
                    return block;
                }
            }

            throw new OutOfMemoryException();
        }

        public void Free(void *chunk)
        {
            var pool_index = allocedSizes[LastPool.region.BlockIndex(chunk)];
            FreeList pool = null;
            bool extended = false;
            do {
                pool = pools[pool_index];
                if (pool_index > 0) {
                    if ((pool.region.ram.OffsetOf(chunk) & pool.region.blockSize) == 0) {
                        extended = pool.ExtendRight(chunk);
                    }
                    else {
                        extended = pool.ExtendLeft(chunk);
                        if (extended)
                            chunk = (void*)((Address)chunk - pool.region.blockSize);
                    }
                }
            }
            while (pool_index-- > 0 && extended);
            pool.Free(chunk);
        }

        #endregion

        private FreeList LastPool
        {
            get { return pools[pools.Length - 1]; }
        }

        #region Debugging

        public string MemoryMap
        {
            get
            {
                var b = new StringBuilder(pools[pools.Length-1].MemoryMap);
                foreach (var pool in pools) {
                    var map = pool.MemoryMap;
                    int zoom = b.Length / map.Length;
                    for (int char_idx = 0; char_idx < map.Length; ++char_idx) {
                        if (map[char_idx] != '#') {
                            for (int i = 0; i < zoom; ++i)
                                b[char_idx * zoom + i] = (i==0) ? map[char_idx] : '-';
                        }
                    }
                }
                return b.ToString();
            }
        }

        public string[] SubMemoryMaps
        {
            get
            {
                return (from pool in pools
                        select string.Format("{1} {0}",
                                             pool.region.blockSize,
                                             pool.MemoryMap))
                    .ToArray();
            }
        }

        #endregion

    }
}
