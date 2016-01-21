using System;

namespace clrcore
{
    /// <summary>
    /// Allocation of small memory blocks (from 1 byte to 32 bytes) can be
    /// effectively manage by small overhead packets. This done by dividing
    /// the block into 4 bytes sub-blocks. Each sub-block can be either free
    /// allocated or allocate-descriptor block.
    ///
    /// Here is a small chart of empty superblock memory:
    ///
    ///   /---------------------------------------------------------------------\
    ///   | Free  | Free  |  Free  |  Free  |  Free  |  Free  |  Free  |  Free  |
    ///   \-N-------N---------N--------N-------N---------N-------N--------N-----/
    ///     \-----/  \-----/  \-----/  \-----/  \-----/  \-----/  \-----/ \-||
    ///
    /// As you can see each free block points in the beginning to the next free
    /// block.
    ///
    /// When a memory is allocated a pool of 1 or more contiguous blocks are united
    /// for the allocated block and the one block previous for the contiguous block
    /// will be used for the allocation-descriptor block. This special block contains
    /// the information about the allocated block: Number of blocks and a magic
    /// number.
    ///
    /// See 'FreeBlock' for the free block descriptor
    /// See 'AllocatedDescriptorBlock' for the allocation-descriptor block
    ///
    /// The block are index by thier position in the chain. Block number 5 start
     /// after 20 bytes from the beginning of the superblock.
     ///
     /// In order to reduce the over-head blocks, the number of blocks are limited
     /// to 2^16, which limit the super-block size to 2^18 - 256kb of data.
     /// Also, the maximum allocating unit is 1kb of data.
     ///
     /// Some more information about this kind of allocation:
     ///    - The overhead for each packet is very small.
     ///    - Allocation is very fast, the same for destruction.
     ///
     /// Fragmentation note:
     ///      It's recommended to allocate similar block sizes to reduce fragmentation
     ///      see how the SuperiorMemoryManager construct couple of small-memory heaps
     ///      and play with them.
    /// </summary>
    unsafe class FreeList
    {
        public const uint MINIMUM_ALLOCATION_UNIT = sizeof(uint);

        /// <summary>
        /// Constructor. Initialize a block of memory and transform it into a free list of smaller fixed size blocks
        /// </summary>
        /// <param name="buffer">Free list memory</param>
        /// <param name="blockSize">The allocation unit. Including 4 bytes of the allocation header. See MINIMUM_ALLOCATION_UNIT</param>
        public FreeList(Buffer buffer, uint blockSize)
        {
            if (blockSize < MINIMUM_ALLOCATION_UNIT)
                blockSize = MINIMUM_ALLOCATION_UNIT;

            m_buffer = buffer;
            m_blockSize = blockSize;

            // The total number of block is the lower-bound of blocks which can be
            // fit into 'superBlockLength'
            m_totalNumberOfBlocks = buffer.getLength() / m_blockSize;

            // Start by reset the memory. Set all memory to be free blocks
            // The first block will point to the last block and vis versa

            m_firstFreeBlock = 0;
        }

        /// <summary>
        /// For blockID (id) return the block ID of the next free block
        /// </summary>
        /// <param name="blockId">blockId</param>
        /// <returns>Returns the block ID of the next free block</returns>
        private uint getNextFreeBlock(uint blockId)
        {
            byte* location = (byte*)m_buffer.getBuffer() + (blockId * m_blockSize);
            return *(uint*)location;
        }

        /// <summary>
        /// Set the next-block-id value of a block-id
        /// </summary>
        /// <param name="blockId"></param>
        /// <param name="nextBlockId"></param>
        private void setNextFreeBlock(uint blockId, uint nextBlockId)
        {
            byte* location = (byte*)m_buffer.getBuffer() + (blockId * m_blockSize);
            *(uint*)location = nextBlockId;
        }

        private Buffer m_buffer;
        private uint m_blockSize;
        // The total number of blocks
        private uint m_totalNumberOfBlocks;
        // The first free block pointer
        private uint m_firstFreeBlock;
    }
}
