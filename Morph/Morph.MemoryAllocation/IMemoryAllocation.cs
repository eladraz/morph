using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Morph.MemoryAllocation
{
    using Address = UInt32;
    using Size = UInt32;
    using Index = Int32;

    public unsafe interface IMemoryAllocation
    {
        void *Alloc(Size numBytes);
        void Free(void *chunk);
    }
}
