using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Morph.MemoryAllocation
{
    using Address = UInt32;
    using Size = UInt32;
    using Index = Int32;

    using Link = FreeList.Link;


    unsafe class FreeListCoalesce : FreeList
    {

        /**
         * Releases a previously allocated block.
         * Tries to coalesce with existing free chunks before/after the released block.
         */
        public unsafe override void FreeBlock(void *block)
        {
            Console.WriteLine("FreeList[{0}] free block #{1} @ 0x{2:x}", region.blockSize, region.BlockIndex(block), (int)block);

            Link *record = (Link*)block;
            Link* prev = null;
            for (Link* link = head; link != null && link < record; link = link->next)
                prev = link;

            record->freeBlocks = 1;

            if (prev != null) {
                record->next = prev->next;
                prev->next = record;
            }
            else {
                record->next = head;
                head = record;
            }

            Coalesce(record);

            if (prev != null) {
                Coalesce(prev);
            }
        }

        /**
         * Merges two adjacent free-list items if they are also adjacent in the region
         * (i.e. there's no used memory between them).
         */
        internal unsafe bool Coalesce(Link *link)
        {
            if (link->next != null) {
                Address endOfChunk = (Address)link + link->freeBlocks * region.blockSize;
                if (endOfChunk == (Address)link->next) {
                    link->freeBlocks += link->next->freeBlocks;
                    link->next = link->next->next;
                    return true;
                }
            }
            return false;
        }



        #region Extension for Buddy Technique

        /**
         * @note Assumes that 'block' is currently NOT free in this pool.
         */
        public override bool ExtendRight(void *block)
        {
            Link *record = (Link*)block;
            Link* link;
            Link* prev = null;
            for (link = head; link != null && link < record; link = link->next)
                prev = link;

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

        public override bool ExtendLeft(void *block)
        {
            Link* record = (Link*)block;
            Link* link;
            Link* prev = null;
            Link* prevprev = null;
            for (link = head; link != null && link < record; link = link->next) {
                prevprev = prev;
                prev = link;
            }

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

    }
}
