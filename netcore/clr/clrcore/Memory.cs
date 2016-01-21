namespace clrcore
{
    public class Memory
    {
        public unsafe static void memset(void* p, byte value, uint length)
        {
            byte* bp = (byte*)p;
            for (uint i = 0; i < length; i++)
            {
                *bp = value;
                bp++;
            }
        }

        public unsafe static void memcpy(void* dv, void* sv, uint length)
        {
            byte* d = (byte*)dv;
            byte* s = (byte*)sv;
            for (uint i = 0; i < length; i++)
            {
                *d = *s;
                d++;
                s++;
            }
        }

        public unsafe static void memmove(void *dv, void *sv, uint length )
        {
            byte *d = (byte*) dv;
            byte *s = (byte*) sv;

            if (dv == sv)
                return;

            /* Check for destructive overlap.  */
            if (s < d && d < s + length)
            {
                /* Destructive overlap ... have to copy backwards.  */
                s += length;
                d += length;
                while (length-- > 0)
                    *--d = *--s;
            }
            else
            {
                /* Do an ascending copy.  */
                while (length-- > 0)
                    *d++ = *s++;
            }
        }
    }
}
