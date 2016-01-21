using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;



namespace Morph.MemoryAllocation
{
    using Address = UInt32;
    using Size = UInt32;
    using Index = Int32;

    public unsafe struct RAM
    {
        //CR: shouldn't this be Address instead of int*?
        public int* start;
        public Size sz;

        //CR: this is an error. using the .NET pointer instead of ours
        public void *At(Address offset)
        {
            //CR: shouldn't this be sizeof Size?
            return (void *)(start + (offset / sizeof(int)));
        }

        public Address OffsetOf(void *pointer)
        {
            //CR: shouldn't this be sizeof Size?
            return (Address)((int*)pointer - start) * sizeof(int);
        }
    }


}
