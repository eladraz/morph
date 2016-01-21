namespace clrcore
{
    /*
     * Called by the compiler for every object allocation, reference increase and dereference.
     * The class responsible for allocating memory and storing object metadata.
     *
     * Every change in the function prototypes (implementation is free), should be also changed in
     * "runnable/FrameworkMethods.cpp" file in the compiler
     *
     * Author: Elad Raz
     */
    class GarbageCollector
    {
#if MEMORY_HELPER_STATISTICS
        public static uint MemoryCounter = 0;  //added by David
#endif // MEMORY_HELPER_STATISTICS

        /*
         *  Memory layout
         *  -------------
         *  +0       -4        +00  Size (Not including this header)
         *  +1       -3        +04  Reference Count
         *  +2       -2        +08  Finalize pointer
         *  +3       -1        +0C  VTbl pointer
         */
        public unsafe struct BlockHeader
        {
            public uint refCount;
            public void* vTbl;
        }

        /*
         * From an Object instance, return the RTTI information.
         * This is the basic for System.Type implementation
         */
        public unsafe static ushort getRTTI(void *buffer)
        {
            BlockHeader* header = garbageCollectorGetBlockHeader(buffer);
            return VirtualTable.getObjectRTTI(header->vTbl);
        }

        public unsafe static ushort getRTTI(System.Object obj)
        {
            return getRTTI(Morph.Imports.convert(obj));
        }

        /*
         * From an object instance, return the block header
         */
        public unsafe static BlockHeader* garbageCollectorGetBlockHeader(void* buffer)
        {
            return (BlockHeader*)((uint)buffer - (uint)sizeof(BlockHeader));
        }

        // Only the compiler should call this function
        private unsafe static void* garbageCollectorNewObject(void* vtbl)
        {
            VirtualTable.VirtualTableHeader* vtblHeader = VirtualTable.getVirtualTableHeader(vtbl);
            uint objectSize = vtblHeader->m_objectSize;

            uint metadataSize   = (uint) sizeof(BlockHeader);
            uint requiredSize = metadataSize + objectSize;
            void* allocated     = Morph.Imports.allocate(requiredSize);
            BlockHeader* header = (BlockHeader*) allocated;
            void* data          = (void *) ((uint) allocated + metadataSize);

            header->vTbl = vtbl;
            header->refCount = 0;

#if MEMORY_HELPER_STATISTICS
            MemoryCounter += requiredSize; //added by David
#endif // MEMORY_HELPER_STATISTICS

            clrcore.Memory.memset(data, 0, objectSize);
            return data;
        }

        public unsafe static uint garbageCollectorGetObjectSize(void* buffer)
        {
            BlockHeader* header = garbageCollectorGetBlockHeader(buffer);
            return (uint)(VirtualTable.getVirtualTableHeader(header->vTbl)->m_objectSize);
        }

        public unsafe static void* garbageCollectorGetVTbl(void* buffer)
        {
            if (buffer == null)
                return null;
            return garbageCollectorGetBlockHeader(buffer)->vTbl;
        }

        // Only the compiler should call this function  [and Array.Copy]
        internal unsafe static void garbageCollectorIncreaseReference(void* buffer)
        {
            if (buffer != null)
                ++(garbageCollectorGetBlockHeader(buffer)->refCount);
        }

        internal unsafe static void garbageCollectorDecreaseReferenceNoDestroy(void* buffer)
        {
            if (buffer == null)
                return;
            garbageCollectorGetBlockHeader(buffer)->refCount--;
        }

        internal unsafe static void garbageCollectorDestroyIfNoReference(void* buffer)
        {
            if (buffer == null)
                return;

            BlockHeader* block = garbageCollectorGetBlockHeader(buffer);
            if ((block->refCount) > 0)
                return;

            // Call finalizer and then deallocate
            void* vtbl = block->vTbl;
            void* dtor = *(void**)(vtbl);
            if (dtor != null)
            {
                // Simple object
                Morph.Imports.callFunction(dtor, buffer);
            }

#if MEMORY_HELPER_STATISTICS
            MemoryCounter -= block->numberOfElements; //Added by David
#endif // MEMORY_HELPER_STATISTICS
            Morph.Imports.free((void*)block);
        }

        internal unsafe static void garbageCollectorDecreaseReference(void* buffer)
        {
            if (buffer == null)
                return;

            BlockHeader* block = garbageCollectorGetBlockHeader(buffer);
            // TODO! Interlocked increase
            if (--(block->refCount) > 0)
                return;

            // Call finalizer and then deallocate
            void* vtbl = block->vTbl;
            void* dtor = *(void**)(vtbl);
            if (dtor != null)
            {
                // Simple object
                Morph.Imports.callFunction(dtor, buffer);
            }

#if MEMORY_HELPER_STATISTICS
            MemoryCounter -= block->numberOfElements; //Added by David
#endif // MEMORY_HELPER_STATISTICS
            Morph.Imports.free((void*)block);
        }
    }
}
