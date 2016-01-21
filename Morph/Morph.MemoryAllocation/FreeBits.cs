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
     * Naive implementation of memory allocation using a bit-mask.
     */
    public unsafe struct FreeBits : IMemoryAllocation
    {
        struct BitMask
        {
            UInt32 bits;

            public void Clear() { bits = 0; }

            public bool this[int index]
            {
                get { return (bits & (1 << index)) != 0; }
                set { if (value) bits |= (1u << index); else bits &= ~(1u << index); }
            }
        }

        RAM ram;
        Size blockSize;
        Size nblocks;
        BitMask used;
        BitMask startPoint;

        #region Bootstrap Code

        static unsafe FreeBits ramEnd(int* ramstart, int ramsz)
        {
            int *objstart = ramstart + (ramsz - sizeof(FreeBits)) / sizeof(int);
            return *(FreeBits*)objstart;
        }

        public void Initialize(RAM ram, Size blockSize)
        {
            this.ram = ram;
            used.Clear();
            startPoint.Clear();
            this.blockSize = blockSize;
            nblocks = ram.sz / blockSize;
        }

        #endregion

        private Size NumBlocks(Size sz)
        {
            return (sz + blockSize - 1) / blockSize;
        }

        private Size RoundUpToBlockSize(Size sz)
        {
            return NumBlocks(sz) * blockSize;
        }

        #region Memory Allocator API

        /**
         * Locates a contiguous unused chunk of size numBytes in the RAM and
         * marks it as used.
         *
         * @param numBytes - size of memory chunk to allocate
         * @return pointer to the newly allocated chunk.
         */
        public void *Alloc(Size numBytes)
        {
            Index firstBlock = AllocBlocks(NumBlocks(numBytes));
            return ram.At((Address)(firstBlock * blockSize));
        }

        public void Free(void * chunk)
        {
            FreeBlocks((Index)(ram.OffsetOf(chunk) / blockSize));
        }

        /**
         * Locates a contiguous sequence of unused blocks and marks them as used.
         *
         * @param blocksRequested - number of blocks to allocate
         * @return the index of the first block in the sequence
         */
        public Index AllocBlocks(Size blocksRequested)
        {
            Size runLength = 0;
            for (Index block = 0; block < nblocks; ++block) {
                if (used[block]) runLength = 0;
                else runLength++;
                if (runLength == blocksRequested) {
                    Index firstBlock = block - (Index)(blocksRequested - 1);
                    Mark(firstBlock, blocksRequested);
                    return firstBlock;
                }
            }

            // Not enough contiguous memory
            throw new InsufficientMemoryException();
        }

        public void FreeBlocks(Index firstBlock)
        {
            if (!startPoint[firstBlock])
                throw new AccessViolationException();

            startPoint[firstBlock] = false;
            for (Index block = firstBlock;
                    block < nblocks && used[block] & !startPoint[block];
                    block++) {
                used[block] = false;
            }
        }

        #endregion


        #region Bitmask Service routines

        internal void Mark(Index firstBlock, Size numBlocks)
        {
            startPoint[firstBlock] = true;
            for (Index block = 0; block < numBlocks; block++) {
                used[firstBlock + block] = true;
            }
        }

        internal void Unmark(Index firstBlock, Size numBlocks)
        {
            for (Index block = 0; block < numBlocks; block++) {
                used[firstBlock + block] = false;
                startPoint[firstBlock + block] = false;
            }
        }

        #endregion


        #region Memory Statistics

        public Size AvailBlocks
        {
            get
            {
                Size nunused = 0;
                for (Index block = 0; block < nblocks; block++) {
                    if (!used[block]) nunused++;
                }
                return nunused;
            }
        }

        public Size AvailBytes
        {
            get
            {
                return AvailBlocks * blockSize;
            }
        }

        #endregion
    }
}
