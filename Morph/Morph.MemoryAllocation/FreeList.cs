using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;



namespace Morph.MemoryAllocation
{
    using Address = UInt32;
    using Size = UInt32;
    using Index = Int32;

    /**
     * Allocates blocks of constant size from a pool of blocks.
     * When a block is allocated, the free region is shrinked by 1 block.
     * When a block is freed, it is added to the pool, possibly merged with
     * an existing region if it is adjacent.
     */
    public unsafe class FreeList : IMemoryAllocation
    {
        public struct Region
        {
            public RAM ram;
            public Size blockSize;
            public Size numBlocks;

            public Index BlockIndex(void *pointer)
            {
                return (Index)(ram.OffsetOf(pointer) / blockSize);
            }
        }

        public unsafe struct Link
        {
            public Size freeBlocks;
            //CR: this is an error. using the reguler .NET pointer instead of our pointer.
            public Link* next;
        }

        internal Region region;
        protected Link* head;

        public unsafe void Initialize(RAM ram, Size blockSize, bool startFree=true)
        {
            Debug.Assert(blockSize >= sizeof(Link), "need at least enough bytes in free block for link");

            region.ram = ram;
            region.blockSize = blockSize;
            region.numBlocks = region.ram.sz / blockSize;

            if (startFree) {
                head = (Link*)region.ram.At(0);
                head->freeBlocks = region.numBlocks;
                head->next = null;
            }
            else {
                head = null;
            }
        }

        #region Memory Allocation API

        public void *Alloc(Size numBytes)
        {
            if (numBytes > region.blockSize)
                throw new OutOfMemoryException();
            return AllocBlock();
        }

        public void Free(void *chunk)
        {
            FreeBlock(chunk);
        }

        public bool CanAlloc()
        {
            return head != null;
        }

        public unsafe void *AllocBlock()
        {
            if (!CanAlloc())
                throw new OutOfMemoryException();

            var block = (void *)(head);
            head = Forward1(head);

            return block;
        }

        /**
         * Releases a previously allocated block.
         * Tries to coalesce with existing free chunks before/after the released block.
         */
        public unsafe virtual void FreeBlock(void *block)
        {
            Link *record = (Link*)block;
            record->freeBlocks = 1;
            record->next = head;

            head = record;
        }

        #endregion

        #region Extension for Buddy Technique

        /**
         * @note Assumes that 'block' is currently NOT free in this pool.
         */
        public virtual bool ExtendRight(void *block)
        {
            Link *record = (Link*)block;
            Link* link;
            Link* prev = null;
            for (link = head; link != null && link < record; link = link->next)
                prev = link; //find the record previous to ours from the head of the list

            if (link != null && (Address)record + region.blockSize == (Address)link) {
                if (prev != null)
                    prev->next = Forward1(link);
                else
                    head = Forward1(link);
                return true;
            }
            else {
                return false;
            }
        }

        public virtual bool ExtendLeft(void *block)
        {
            Link* record = (Link*)block;
            Link* link;
            Link* prev = null;
            Link* prevprev = null;
            for (link = head; link != null && link < record; link = link->next) {
                prevprev = prev;
                prev = link;
            }

            //CR: why multiply by the mount of free blocks? wont i always extend with my neighbor?
            if (prev != null &&
                (Address)prev + region.blockSize * prev->freeBlocks == (Address)record) {
                if (--prev->freeBlocks == 0) {
                    if (prevprev != null) {
                        prevprev->next = prev->next;
                    }
                    else {
                        head = prev->next;
                    }
                }
                return true;
            }
            else {
                return false;
            }
        }

        #endregion

        /**
         * Makes the next block into our Link (points us to the next block and the
         * number of freeBlocks). After that we can return the current block as the memory.
         */
        protected unsafe Link* Forward1(Link* link)
        {
            if (link->freeBlocks == 1) {
                return link->next;
            }
            else {
                var newLink = (Link*)((Address)link + region.blockSize);
                newLink->next = link->next;
                newLink->freeBlocks = link->freeBlocks - 1;
                return newLink;
            }
        }

        /**
         * Illustrates the state of the memory in a text string, marking
         * allocated blocks with '#' and free blocks with '-', where the start
         * of each free region is depicted by '+'.
         */
        public string MemoryMap
        {
            get
            {
                var b = new StringBuilder();
                b.Append('#', (int)region.numBlocks);
                for (var link = head; link != null; link = link->next) {
                    var dashes = new StringBuilder();
                    dashes.Append('-', (int)link->freeBlocks);
                    dashes[0] = '+';
                    int free_index = region.BlockIndex((void*)link);
                    b.Remove(free_index, dashes.Length);
                    b.Insert(free_index, dashes);
                }
                return b.ToString();
            }
        }

    }
}
