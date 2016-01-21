#define __Morph

using clrcore;
using System.Text;

namespace Morph
{
    using tchar = System.Byte;

    public unsafe class String
    {
        //This waits for TICKET#27
        // TODO! Chars implements only as ASCII (for now)
        // Use #define for all other white-space !
        internal static readonly char[] WhitespaceChars =
            { (char) 0x9, (char) 0xA, (char) 0xB, (char) 0xC, (char) 0xD, (char) 0x20, (char) 0xA0
#if !__Morph
                ,(char) 0x2000, (char) 0x2001, (char) 0x2002, (char) 0x2003, (char) 0x2004, (char) 0x2005,
              (char) 0x2006, (char) 0x2007, (char) 0x2008, (char) 0x2009, (char) 0x200A, (char) 0x200B,
              (char) 0x3000, (char) 0xFEFF
#endif
              };

        internal tchar* m_buffer;
        internal uint m_size;
        internal bool m_isInHeap;


        public static readonly string Empty = ""; //new String('a', 0);

        //Constructors:

        public unsafe String(char c, int length)
        {
            tchar* tmp = (tchar*)Morph.Imports.allocate((uint)length);

            for (int i = 0; i < length; i++)
                tmp[i] = (tchar)c;

            initInternals((uint)length, tmp, true);
            Morph.Imports.free(tmp);
        }

        public unsafe String(char[] c, int startIndex, int length)
        {
            tchar* tmp = (tchar*)Morph.Imports.allocate((uint)length);


            for (int i = 0; i < length; i++)
                tmp[i] = (tchar) c[startIndex + i];

            initInternals((uint)length, tmp, true);
            Morph.Imports.free(tmp);
        }

        //public String(byte[] value)
        //{
        //    fixed (tchar* p = value)
        //    {
        //        initInternals((uint)value.Length, p, false);
        //    }
        //}

        private void Make(char[] value)
        {
            tchar* p = (tchar*)Morph.Imports.convert(value);
            initInternals((uint)value.Length, (tchar*)p, true);
        }

        public String(char[] value)
        {
            Make(value);
        }

        /**
         * Called by the compiler to save space. Instance a new string object from .data section
         * and size
         */
        //////private static Morph.String compilerInstanceNewString(uint size, tchar* source)
        //////{
        //////    return new String(size, source, false);
        //////}

        private static string compilerInstanceNewString(uint size, tchar* source)
        {
            return Morph.Imports.convertToString(new String(size, source, false));
        }

        /**
         * Private string constructor
         *
         * Create a new string, either from .data or copy to heap
         */
        internal String(uint size, tchar* source, bool shouldCopyToHeap)
        {
            initInternals(size, source, shouldCopyToHeap);
        }

        private void initInternals(uint size, tchar* source, bool shouldCopyToHeap)
        {
            m_size = size;
            if (shouldCopyToHeap)
            {
                uint bsize = size * sizeof(tchar);
                m_buffer = (tchar*)Morph.Imports.allocate(bsize);
                Memory.memcpy(m_buffer, source, bsize);
                m_isInHeap = true;
            }
            else
            {
                m_buffer = source;
                m_isInHeap = false;
            }
        }


        /**
         * Private string constructor (Multi-string)
         */
        private String(uint size1, uint size2, tchar* s1, tchar* s2)
        {
            m_size = size1 + size2;
            m_buffer = (tchar*)Morph.Imports.allocate((m_size) * sizeof(tchar));
            uint bsize = size1 * sizeof(tchar);
            Memory.memcpy(m_buffer, s1, bsize);
            Memory.memcpy(m_buffer + bsize, s2, size2 * sizeof(tchar));
            m_isInHeap = true;
        }

        public override string ToString()
        {
            // TODO! Compiler optimization
            return Morph.Imports.convertToString(this);
        }

        /**
         *
         */
        ~String()
        {
            if (m_isInHeap)
            {
                Morph.Imports.free(m_buffer);
            }
        }



        [System.Runtime.CompilerServices.IndexerName("Chars")]
        public char this[int index]
        {
            get
            {
                if (index < m_size)
                {
                    return (char)(m_buffer[index]);
                }
                else
                {
                    throw new System.IndexOutOfRangeException();
                }
            }
        }

        //statics
        public unsafe static int Compare(String strA, String strB)
        {
            return Compare(strA, strB, false);
        }

        public static int Compare(String strA, String strB, bool ignoreCase)
        {
            if (strA == null && strB == null)
            {
                return 0;
            }

            if (strA == null)
            {
                return -1;
            }

            if (strB == null)
            {
                return 1;
            }
            uint len;
            len = (strA.m_size < strB.m_size ? strA.m_size : strB.m_size);

            int diff;

            for (int i = 0; i < len; i++)
            {
                tchar A = strA.m_buffer[i];
                tchar B = strB.m_buffer[i];
                if (ignoreCase)
                {
                    if (B >= 'a' && B <= 'z')
                    {
                        B = (tchar)(B - (tchar)32);
                    }

                    if (A >= 'a' && A <= 'z')
                    {
                        A = (tchar)(A - (tchar)32);
                    }
                }
                diff = A - B;

                if (diff != 0)
                {
                    return diff;
                }
            }


            if (strA.m_size != strB.m_size)
            {
                return (int)strA.m_size - (int)strB.m_size;
            }

            return 0;
        }

        public static bool Equals(String a, String b)
        {
            if ((System.Object)a == (System.Object)b)
            {
                return true;
            }

            if ((System.Object)a == null || (System.Object)b == null)
            {
                return false;
            }

            return a.Equals(b);
        }

        //public static bool Equals(System.Object a, System.Object b)
        //{
        //    if (a == b)
        //        return true;

        //    if (a == null || b == null)
        //    {
        //        return false;
        //    }

        //    return ((string)a).Equals((string)b);
        //}

        public override unsafe int GetHashCode()
        {
            tchar* cc = m_buffer;
            tchar* end = cc + m_size - 1;
            int h = 0;
            for (; cc < end; cc += 2)
            {
                h = (h << 5) - h + *cc;
                h = (h << 5) - h + cc[1];
            }
            ++end;
            if (cc < end)
                h = (h << 5) - h + *cc;
            return h;
        }

        public static bool IsNullOrEmpty(String value)
        {
            if (value == null)
                return true;

            if (value.m_size == 0)
                return true;

            return false;
        }

        //Propereties:
        public int Length
        {
            get { return (int)m_size; }
        }

        public unsafe tchar* getTCharBuffer()
        {
            return m_buffer;
        }


        public static String Concat(String a, String b)
        {
            if (a == null)
                return b;

            if (b == null)
                return a;

            return new String(a.m_size, b.m_size, a.m_buffer, b.m_buffer);
        }

        public static String Concat(String a, String b, String c)
        {
            String ab = Concat(a, b);
            return new String(ab.m_size, c.m_size, ab.m_buffer, c.m_buffer);
        }

        // Be carefull from recursive, use System.String instead of Morph.String
        public static System.String Concat(System.Object a, System.Object b)
        {
            if (a == null)
                return b.ToString();

            if (b == null)
                return a.ToString();

            return System.String.Concat(a.ToString(), b.ToString());
        }

        public static System.String Concat(System.Object a, System.Object b, System.Object c)
        {
            return System.String.Concat(a.ToString(), System.String.Concat(b.ToString(), c.ToString()));
        }

        public static System.String Concat(System.Object a, System.Object b, System.Object c, System.Object d)
        {
            return System.String.Concat(System.String.Concat(a.ToString(), b.ToString()), System.String.Concat(c.ToString(), d.ToString()));
        }

        public static String Concat(String a, String b, String c, String d)
        {
            return Concat(Concat(a, b), Concat(c, d));
        }

        public int CompareTo(System.Object value)
        {
            if (value == null)
            {
                return 1;
            }

            //if (!(value is String))
            //{
            //    throw new ArgumentException(Environment.GetResourceString("Arg_MustBeString"));
            //}

            return String.Compare(this, (String)value);
        }

        public int CompareTo(String strB)
        {
            return Compare(this, strB);
        }

        public override bool Equals(object obj)
        {

            if (!(obj is String))
            {
                return false;
            }

            return this.Equals((String)obj);
        }

        public bool Equals(String str)
        {
            if (str == null)
            {
                return false;
            }

            return (Compare(this, str, false) == 0);
        }

        public string ToLower()
        {
            char[] s = new char[Length];

            for (int i = 0; i < Length; i++)
                if (this[i] >= 'A' && this[i] <= 'Z')//TICKET#53
                    s[i] = (char)(this[i] - 'A' + 'a');
                else
                    s[i] = (char)this[i];

            return new string(s);
        }

        public string ToUpper()
        {
            char[] s = new char[Length];

            for (int i = 0; i < Length; i++)
                if (this[i] >= 'a' && this[i] <= 'z') //TICKET#53
                    s[i] = (char)(this[i] + 'A' - 'a');
                else
                    s[i] = (char)this[i];

            return new string(s);
        }

        public string Substring(int startIndex)
        {
            if (startIndex == 0)
                return Imports.convertToString(this);
            if (startIndex < 0 || startIndex > this.Length)
                throw new System.ArgumentOutOfRangeException("startIndex");

            return compilerInstanceNewString((uint)(Length - startIndex), m_buffer + startIndex);
        }

        public string Substring(int startIndex, int length)
        {
            if (length < 0)
                throw new System.ArgumentOutOfRangeException("length", "Cannot be negative.");
            if (startIndex < 0)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot be negative.");
            if (startIndex > this.Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot exceed length of string.");
            if (startIndex > this.Length - length)
                throw new System.ArgumentOutOfRangeException("length", "startIndex + length > this.length");
            if (startIndex == 0 && length == this.Length)//TICKET#53
                return Imports.convertToString(this);

            return compilerInstanceNewString((uint)(length), m_buffer + startIndex);
        }

        public static bool operator ==(String a, String b)
        {
            return Equals(a, b);
        }

        public static bool operator !=(String a, String b)
        {
            return !Equals(a, b);
        }

        public System.Object Clone()
        {
            return this;
        }

        public static string Copy(String str)
        {
            if (str == null)
                throw new System.ArgumentNullException("str");

            return compilerInstanceNewString((uint)str.Length, str.m_buffer);
        }

        public char[] ToCharArray()
        {
            return ToCharArray(0, Length);
        }

        public char[] ToCharArray(int startIndex, int length)
        {
            if (startIndex < 0)
                throw new System.ArgumentOutOfRangeException("startIndex", "< 0");
            if (length < 0)
                throw new System.ArgumentOutOfRangeException("length", "< 0");
            if (startIndex > this.Length - length)
                throw new System.ArgumentOutOfRangeException("startIndex", "Must be greater than the length of the string.");
            char[] tmp = new char[length];

            unsafe
            {
                fixed (char* dest = tmp)
                    Memory.memcpy(dest, m_buffer + startIndex, (uint)length);
            }
            return tmp;
        }

        public string Trim()
        {
            if (Length == 0)
                return String.Empty;

            int start = FindNotWhiteSpace(0, Length, 1);

            if (start == Length)
                return String.Empty;

            int end = FindNotWhiteSpace(Length - 1, start, -1);

            int newLength = end - start + 1;
            if (newLength == Length)
                return Morph.Imports.convertToString(this);

            return Morph.Imports.convertToString(compilerInstanceNewString((uint)newLength, m_buffer + start));
        }

        private int FindNotWhiteSpace(int pos, int target, int change)
        {
            while (pos != target)
            {
                char c = this[pos];
                if (!char.IsWhiteSpace(c))
                    break;

                pos += change;
            }

            return pos;
        }

        private  int FindNotInTable(int pos, int target, int change, char[] table)
        {
            unsafe
            {
                fixed (char* tablePtr = table)
                {
                    while (pos != target)
                    {
                        tchar c = m_buffer[pos];
                        int x = 0;
                        while (x < table.Length)
                        {
                            if (c == tablePtr[x])
                                break;
                            x++;
                        }
                        if (x == table.Length)
                            return pos;
                        pos += change;
                    }
                }
                return pos;
            }
        }

        public string Trim(params char[] trimChars)
        {
            if (trimChars == null || trimChars.Length == 0)
                return Trim();

            if (Length == 0)
                return String.Empty;
            int start = FindNotInTable(0, Length, 1, trimChars);

            if (start == Length)
                return String.Empty;

            int end = FindNotInTable(Length - 1, start, -1, trimChars);

            int newLength = end - start + 1;
            if (newLength == Length)
                return Morph.Imports.convertToString(this);

            return Morph.Imports.convertToString(compilerInstanceNewString((uint)newLength, m_buffer + start));
        }

        public string TrimStart(params char[] trimChars)
        {
            if (Length == 0)
                return String.Empty;
            int start;
            if (trimChars == null || trimChars.Length == 0)
                start = FindNotWhiteSpace(0, Length, 1);
            else
                start = FindNotInTable(0, Length, 1, trimChars);

            if (start == 0)
                return Morph.Imports.convertToString(this);

            return Morph.Imports.convertToString(compilerInstanceNewString((uint)(Length - start), m_buffer + start));
        }

        public string TrimEnd(params char[] trimChars)
        {
            if (Length == 0)
                return String.Empty;
            int end;
            if (trimChars == null || trimChars.Length == 0)
                end = FindNotWhiteSpace(Length - 1, -1, -1);
            else
                end = FindNotInTable(Length - 1, -1, -1, trimChars);

            end++;
            if (end == Length)
                return Morph.Imports.convertToString(this);

            return Morph.Imports.convertToString(compilerInstanceNewString((uint)(end), m_buffer));
        }

        public bool StartsWith(String value)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");

            if (value.Length > Length)
                return false;

            for (int i = 0; i < value.Length; i++)
                if (value[i] != this[i])
                    return false;

            return true;
        }

        public string Remove(int startIndex)
        {
            if (startIndex < 0)
                throw new System.ArgumentOutOfRangeException("startIndex", "StartIndex can not be less than zero");
            if (startIndex >= this.Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "StartIndex must be less than the length of the string");

            return Remove(startIndex, Length - startIndex);
        }

        public unsafe string Remove(int startIndex, int count)
        {
            if (startIndex < 0)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot be negative.");
            if (count < 0)
                throw new System.ArgumentOutOfRangeException("count", "Cannot be negative.");
            if (startIndex > this.Length - count)
                throw new System.ArgumentOutOfRangeException("count", "startIndex + count > this.length");

            return Morph.Imports.convertToString(Concat(compilerInstanceNewString((uint)startIndex, m_buffer),
                                                        Morph.Imports.convertToString(compilerInstanceNewString((uint)(Length - count - startIndex), m_buffer + startIndex + count))));
        }

        public string Replace(char oldChar, char newChar)
        {
            if (oldChar == newChar)
                return Imports.convertToString(this);

            char[] s = new char[Length];

            for (int i = 0; i < Length; i++)
                if (this[i] == oldChar)
                    s[i] = newChar;
                else
                    s[i] = this[i];

            return new string(s);
        }

        public string PadLeft(int totalWidth)
        {
            return PadLeft(totalWidth, ' ');
        }

        public unsafe string PadLeft(int totalWidth, char paddingChar)
        {
            //LAMESPEC: MSDN Doc says this is reversed for RtL languages, but this seems to be untrue

            if (totalWidth < 0)
                throw new System.ArgumentOutOfRangeException("totalWidth", "< 0");

            if (totalWidth < Length)
                return Morph.Imports.convertToString(this);

            tchar* tmp = (tchar*)Morph.Imports.allocate((uint)totalWidth * sizeof(tchar));

                tchar* padPos = tmp;
                tchar* padTo = tmp + (totalWidth - Length);
                while (padPos != padTo)
                {
                    *padPos = (tchar)paddingChar;
                    padPos++;
                }

                Memory.memcpy(padTo, m_buffer, (uint)Length);

                return Morph.Imports.convertToString(compilerInstanceNewString((uint)totalWidth, tmp));
        }

        public string PadRight(int totalWidth)
        {
            return PadRight(totalWidth, ' ');
        }

        public unsafe string PadRight(int totalWidth, char paddingChar)
        {
            //LAMESPEC: MSDN Doc says this is reversed for RtL languages, but this seems to be untrue

            if (totalWidth < 0)
                throw new System.ArgumentOutOfRangeException("totalWidth", "< 0");

            if (totalWidth < Length)
                return Morph.Imports.convertToString(this);
            if (totalWidth == 0)
                return String.Empty;

            tchar* tmp = (tchar*)Morph.Imports.allocate((uint)totalWidth * sizeof(tchar));


                Memory.memcpy(tmp, m_buffer, (uint)Length);

                tchar* padPos = tmp + Length;
                tchar* padTo = tmp + totalWidth;
                while (padPos != padTo)
                {
                    *padPos = (tchar)paddingChar;
                    padPos++;
                }

                return Morph.Imports.convertToString(compilerInstanceNewString((uint)totalWidth, tmp));
        }

        public unsafe void CopyTo(int sourceIndex, char[] destination, int destinationIndex, int count)
        {
            if (destination == null)
                throw new System.ArgumentNullException("destination");
            if (sourceIndex < 0)
                throw new System.ArgumentOutOfRangeException("sourceIndex", "Cannot be negative");
            if (destinationIndex < 0)
                throw new System.ArgumentOutOfRangeException("destinationIndex", "Cannot be negative.");
            if (count < 0)
                throw new System.ArgumentOutOfRangeException("count", "Cannot be negative.");
            if (sourceIndex > Length - count)
                throw new System.ArgumentOutOfRangeException("sourceIndex", "sourceIndex + count > Length");
            if (destinationIndex > destination.Length - count)
                throw new System.ArgumentOutOfRangeException("destinationIndex", "destinationIndex + count > destination.Length");

            fixed (char* dest = destination)
                Memory.memcpy(dest + destinationIndex, m_buffer + sourceIndex, (uint)count);
        }

        public bool EndsWith(String value)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");

            if (Length < value.Length)
                return false;

            int len = Length - value.Length;

            for (int i = 0; i < value.Length; i++)
                if (value[i] != this[len + i])
                    return false;

            return true;
        }

        public int IndexOf(char value)
        {
            if (Length == 0)
                return -1;

            return IndexOfUnchecked(value, 0, Length);
        }

        public int IndexOf(char value, int startIndex)
        {
            if (startIndex < 0)
                throw new System.ArgumentOutOfRangeException("startIndex", "< 0");
            if (startIndex > Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "startIndex > this.length");

            if ((startIndex == 0 && Length == 0) || (startIndex == Length))//TICKET#53
                return -1;

            return IndexOfUnchecked(value, startIndex, Length - startIndex);
        }

        public int IndexOf(char value, int startIndex, int count)
        {
            if (startIndex < 0 || startIndex > Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot be negative and must be< 0");
            if (count < 0)
                throw new System.ArgumentOutOfRangeException("count", "< 0");
            if (startIndex > Length - count)
                throw new System.ArgumentOutOfRangeException("count", "startIndex + count > this.length");

            if ((startIndex == 0 && Length == 0) || (startIndex == Length) || (count == 0))//TICKET#53
                return -1;

            return IndexOfUnchecked(value, startIndex, count);
        }

        internal unsafe int IndexOfUnchecked(char value, int startIndex, int count)
        {
            // It helps JIT compiler to optimize comparison
            int value_32 = (int)value;

                tchar* ptr = m_buffer + startIndex;
                tchar* end_ptr = ptr + (count >> 3 << 3);

                while (ptr != end_ptr)
                {
                    if (*ptr == value_32)
                        return (int)(ptr - m_buffer);
                    if (ptr[1] == value_32)
                        return (int)(ptr - m_buffer + 1);
                    if (ptr[2] == value_32)
                        return (int)(ptr - m_buffer + 2);
                    if (ptr[3] == value_32)
                        return (int)(ptr - m_buffer + 3);
                    if (ptr[4] == value_32)
                        return (int)(ptr - m_buffer + 4);
                    if (ptr[5] == value_32)
                        return (int)(ptr - m_buffer + 5);
                    if (ptr[6] == value_32)
                        return (int)(ptr - m_buffer + 6);
                    if (ptr[7] == value_32)
                        return (int)(ptr - m_buffer + 7);

                    ptr += 8;
                }

                end_ptr += count & 0x07;
                while (ptr != end_ptr)
                {
                    if (*ptr == value_32)
                        return (int)(ptr - m_buffer);

                    ptr++;
                }

                return -1;
        }

        public int IndexOfAny(char[] anyOf)
        {
            if (anyOf == null)
                throw new System.ArgumentNullException();
            if (Length == 0)
                return -1;

            return IndexOfAnyUnchecked(anyOf, 0, Length);
        }

        public int IndexOfAny(char[] anyOf, int startIndex)
        {
            if (anyOf == null)
                throw new System.ArgumentNullException();
            if (startIndex < 0 || startIndex > Length)
                throw new System.ArgumentOutOfRangeException();

            return IndexOfAnyUnchecked(anyOf, startIndex, Length - startIndex);
        }

        public int IndexOfAny(char[] anyOf, int startIndex, int count)
        {
            if (anyOf == null)
                throw new System.ArgumentNullException();
            if (startIndex < 0 || startIndex > Length)
                throw new System.ArgumentOutOfRangeException();
            if (count < 0 || startIndex > Length - count)
                throw new System.ArgumentOutOfRangeException("count", "Count cannot be negative, and startIndex + count must be less than length of the string.");

            return IndexOfAnyUnchecked(anyOf, startIndex, count);
        }

        private unsafe int IndexOfAnyUnchecked(char[] anyOf, int startIndex, int count)
        {
            if (anyOf.Length == 0)
                return -1;

            if (anyOf.Length == 1)
                return IndexOfUnchecked(anyOf[0], startIndex, count);

            fixed (char* any = anyOf)
            {
                char highest = *any;
                char lowest = *any;

                char* end_any_ptr = any + anyOf.Length;
                char* any_ptr = any;
                while (++any_ptr != end_any_ptr)
                {
                    if (*any_ptr > highest)
                    {
                        highest = *any_ptr;
                        continue;
                    }

                    if (*any_ptr < lowest)
                        lowest = *any_ptr;
                }

                tchar* ptr = m_buffer + startIndex;
                tchar* end_ptr = ptr + count;

                while (ptr != end_ptr)
                {
                    if (*ptr > highest || *ptr < lowest)
                    {
                        ptr++;
                        continue;
                    }

                    if (*ptr == (tchar)(*any))
                        return (int)(ptr - m_buffer);

                    any_ptr = any;
                    while (++any_ptr != end_any_ptr)
                    {
                        if (*ptr == (tchar)(*any_ptr))
                            return (int)(ptr - m_buffer);
                    }

                    ptr++;
                }

            }
            return -1;
        }

        public int IndexOf(String value)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");
            if (value.Length == 0)
                return 0;
            if (this.Length == 0)
                return -1;

            return IndexOf(value, 0, Length);
        }

        public int IndexOf(String value, int startIndex)
        {
            return IndexOf(value, startIndex, this.Length - startIndex);
        }

        public unsafe int IndexOf(String value, int startIndex, int count)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");
            if (startIndex < 0 || startIndex > this.Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot be negative, and should not exceed length of string.");
            if (count < 0 || startIndex > this.Length - count)
                throw new System.ArgumentOutOfRangeException("count", "Cannot be negative, and should point to location in string.");

            if (value.Length == 0)
                return startIndex;

            if (startIndex == 0 && this.Length == 0)//TICKET#53
                return -1;

            if (count == 0)
                return -1;

            tchar* st1 = m_buffer + startIndex, end1 = m_buffer + startIndex + count;
            tchar* st2 = value.m_buffer, end2 = value.m_buffer + value.Length;

            int index = -1;

            while (st1 != end1)
            {
                int tmp2 = *st1;

                if (*st1 == *st2)
                {
                    int tmp = *st2;
                    st2++;
                }
                else
                {
                    st2 = value.m_buffer;
                }

                st1++;

                if (st2 == end2)
                {
                    index = (int)(st1 - m_buffer) - value.Length;
                    break;
                }
            }

            return index;
        }

        public unsafe string Insert(int startIndex, String value)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");

            if (startIndex < 0 || startIndex > Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot be negative and must be less than or equal to length of string.");

            if (value.Length == 0)
                return Imports.convertToString(this);
            if (this.Length == 0)
                return Imports.convertToString(value);
            tchar* tmp = (tchar*)Imports.allocate((uint)(Length + value.Length));

                tchar* dst = tmp;
                Memory.memcpy(dst, m_buffer, (uint)startIndex);
                dst += startIndex;
                Memory.memcpy(dst, value.m_buffer, (uint)value.Length);
                dst += value.Length;
                Memory.memcpy(dst, m_buffer + startIndex, (uint)(Length - startIndex));

            return Morph.Imports.convertToString(compilerInstanceNewString((uint)(Length + value.Length), tmp));
        }

        public static bool IsNullOrWhiteSpace (string value)
		{
			if ((value == null) || (value.Length == 0))
				return true;
			foreach (char c in value)
				if (!Char.IsWhiteSpace (c))
					return false;
			return true;
		}

        public int LastIndexOf(char value)
        {
            if (this.Length == 0)
                return -1;

            return LastIndexOfUnchecked(value, this.Length - 1, this.Length);
        }

        public int LastIndexOf(char value, int startIndex)
        {
            return LastIndexOf(value, startIndex, startIndex + 1);
        }

        public int LastIndexOf(char value, int startIndex, int count)
        {
            if (this.Length == 0)
                return -1;

            // >= for char (> for string)
            if ((startIndex < 0) || (startIndex >= this.Length))
                throw new System.ArgumentOutOfRangeException("startIndex", "< 0 || >= this.Length");
            if ((count < 0) || (count > this.Length))
                throw new System.ArgumentOutOfRangeException("count", "< 0 || > this.Length");
            if (startIndex - count + 1 < 0)
                throw new System.ArgumentOutOfRangeException("startIndex - count + 1 < 0");

            return LastIndexOfUnchecked(value, startIndex, count);
        }

        internal unsafe int LastIndexOfUnchecked(char value, int startIndex, int count)
        {
            // It helps JIT compiler to optimize comparison
            int value_32 = (int)value;

            tchar* ptr = m_buffer + startIndex;
            tchar* end_ptr = ptr - (count >> 3 << 3);

            while (ptr != end_ptr)
            {
                if (*ptr == value_32)
                    return (int)(ptr - m_buffer);
                if (ptr[-1] == value_32)
                    return (int)(ptr - m_buffer) - 1;
                if (ptr[-2] == value_32)
                    return (int)(ptr - m_buffer) - 2;
                if (ptr[-3] == value_32)
                    return (int)(ptr - m_buffer) - 3;
                if (ptr[-4] == value_32)
                    return (int)(ptr - m_buffer) - 4;
                if (ptr[-5] == value_32)
                    return (int)(ptr - m_buffer) - 5;
                if (ptr[-6] == value_32)
                    return (int)(ptr - m_buffer) - 6;
                if (ptr[-7] == value_32)
                    return (int)(ptr - m_buffer) - 7;

                ptr -= 8;
            }

            end_ptr -= count & 0x07;
            while (ptr != end_ptr)
            {
                if (*ptr == value_32)
                    return (int)(ptr - m_buffer);

                ptr--;
            }
            return -1;
        }

        public int LastIndexOf(String value)
        {
            return LastIndexOf(value, this.Length - 1, this.Length);
        }

        public int LastIndexOf(String value, int startIndex)
        {
            int max = startIndex;
            if (max < this.Length)
                max++;
            return LastIndexOf(value, startIndex, max);
        }

        public int LastIndexOf(String value, int startIndex, int count)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");

            if (this.Length == 0)
                return Imports.convertToString(value) == String.Empty ? 0 : -1;
            // -1 > startIndex > for string (0 > startIndex >= for char)
            if ((startIndex < -1) || (startIndex > this.Length))
                throw new System.ArgumentOutOfRangeException("startIndex", "< 0 || > this.Length");
            if ((count < 0) || (count > this.Length))
                throw new System.ArgumentOutOfRangeException("count", "< 0 || > this.Length");
            if (startIndex - count + 1 < 0)
                throw new System.ArgumentOutOfRangeException("startIndex - count + 1 < 0");

            if (value.Length == 0)
                return Math.Min(this.Length - 1, startIndex);

            if (startIndex == 0 && this.Length == 0)//TICKET#53
                return -1;

            // This check is needed to match undocumented MS behaviour
            if (this.Length == 0 && value.Length > 0)//TICKET#53
                return -1;

            if (count == 0)
                return -1;

            if (startIndex == this.Length)
                startIndex--;

            tchar* end1 = m_buffer + startIndex - count - 1, st1 = m_buffer + startIndex;
            tchar* end2 = value.m_buffer - 1, st2 = value.m_buffer + value.Length - 1;

            int i = startIndex, index = -1;

            while (st1 != end1)
            {
                if (st2 == end2)
                {
                    index = i + 1;
                    break;
                }

                if (*st1 == *st2)
                {
                    st2--;
                }
                else
                {
                    st2 = value.m_buffer + value.Length - 1;
                }

                st1--;
                i--;
            }

            return index;
        }

        public int LastIndexOfAny(char[] anyOf)
        {
            if (anyOf == null)
                throw new System.ArgumentNullException();
            if (this.Length == 0)
                return -1;

            return LastIndexOfAnyUnchecked(anyOf, this.Length - 1, this.Length);
        }

        public int LastIndexOfAny(char[] anyOf, int startIndex)
        {
            if (anyOf == null)
                throw new System.ArgumentNullException();
            if (this.Length == 0)
                return -1;

            if (startIndex < 0 || startIndex >= this.Length)
                throw new System.ArgumentOutOfRangeException("startIndex", "Cannot be negative, and should be less than length of string.");

            if (this.Length == 0)
                return -1;

            return LastIndexOfAnyUnchecked(anyOf, startIndex, startIndex + 1);
        }

        public int LastIndexOfAny(char[] anyOf, int startIndex, int count)
        {
            if (anyOf == null)
                throw new System.ArgumentNullException();
            if (this.Length == 0)
                return -1;

            if ((startIndex < 0) || (startIndex >= this.Length))
                throw new System.ArgumentOutOfRangeException("startIndex", "< 0 || > this.Length");
            if ((count < 0) || (count > this.Length))
                throw new System.ArgumentOutOfRangeException("count", "< 0 || > this.Length");
            if (startIndex - count + 1 < 0)
                throw new System.ArgumentOutOfRangeException("startIndex - count + 1 < 0");

            if (this.Length == 0)
                return -1;

            return LastIndexOfAnyUnchecked(anyOf, startIndex, count);
        }

        private unsafe int LastIndexOfAnyUnchecked(char[] anyOf, int startIndex, int count)
        {
            if (anyOf.Length == 1)
                return LastIndexOfUnchecked(anyOf[0], startIndex, count);


            char* start = (char*)m_buffer;

            fixed (char* testStart = anyOf)
            {
                char* ptr = start + startIndex;
                char* ptrEnd = ptr - count;
                char* test;
                char* testEnd = testStart + anyOf.Length;

                while (ptr != ptrEnd)
                {
                    test = testStart;
                    while (test != testEnd)
                    {
                        if (*test == *ptr)
                            return (int)(ptr - start);
                        test++;
                    }
                    ptr--;
                }
                return -1;
            }
        }

        public bool Contains(char c)
        {
            return IndexOf(c) != -1;
        }

        public bool Contains(String value)
        {
            return IndexOf(value) != -1;
        }

        public static string Join(Morph.String separator, Morph.Object[] value)
        {
            Morph.String[] strArr = new Morph.String[value.Length];
            for (int i = 0; i < value.Length; i++)
            {
                strArr[i] = (Morph.String)Imports.convertToObject(value[i].ToString());
            }

            return Join( separator, strArr);
        }

        public static string Join(Morph.String separator, Morph.String[] value)
		{
            if (value == null)
                throw new System.ArgumentNullException("value");
			if (separator == null)
                separator = (Morph.String)Morph.Imports.convertToObject(String.Empty);

			return JoinUnchecked (separator, value, 0, value.Length);
		}

        public static string Join(Morph.String separator, Morph.String[] value, int startIndex, int count)
		{
            if (value == null)
                throw new System.ArgumentNullException("value");
            if (startIndex < 0)
                throw new System.ArgumentOutOfRangeException("startIndex", "< 0");
            if (count < 0)
                throw new System.ArgumentOutOfRangeException("count", "< 0");
            if (startIndex > value.Length - count)
                throw new System.ArgumentOutOfRangeException("startIndex", "startIndex + count > value.length");

			if (startIndex >= value.Length)
				return String.Empty;
            if (separator == null)
                separator = (Morph.String) Morph.Imports.convertToObject(String.Empty);

			return JoinUnchecked (separator, value, startIndex, count);
		}

        private static unsafe string JoinUnchecked(Morph.String separator, Morph.String[] value, int startIndex, int count)
		{
			// Unchecked parameters
			// startIndex, count must be >= 0; startIndex + count must be <= value.length
			// separator and value must not be null

			int length = 0;
			int maxIndex = startIndex + count;
			// Precount the number of characters that the resulting string will have
			for (int i = startIndex; i < maxIndex; i++) {
				if (value[i] != null)
                    length += value[i].Length;
			}
			length += separator.Length * (count - 1);

            if (length <= 0)
                return Morph.String.Empty;

			tchar* tmp = (tchar*)Imports.allocate((uint)length);

			maxIndex--;
            tchar* dest = tmp;

			// Copy each string from value except the last one and add a separator for each
			int pos = 0;
			for (int i = startIndex; i < maxIndex; i++) {
				String source = value[i];
				if (source != null) {
					if (source.Length > 0) {
                        tchar* src = source.m_buffer;
						Memory.memcpy (dest + pos, src, (uint)source.Length);
						pos += source.Length;
					}
				}
				if (separator.Length > 0) {
                    Memory.memcpy(dest + pos, separator.m_buffer, (uint)separator.Length);
					pos += separator.Length;
				}
			}


			// Append last string that does not get an additional separator
			Morph.String sourceLast = value[maxIndex];
			if (sourceLast != null) {
				if (sourceLast.Length > 0) {
                    Memory.memcpy(dest + pos, sourceLast.m_buffer, (uint)sourceLast.Length);
				}
			}

            return compilerInstanceNewString((uint)length, tmp);
		}

        public static string Join(string separator, params object[] values)
        {
            string[] st = new string[values.Length];

            for (int i = 0; i < values.Length; i++)
                st[i] = values[i].ToString();

            return Join(separator, st);
        }

        public string[] Split(params char[] separator)
        {
            return Split(separator, Int32.MaxValue);
        }

        public string[] Split(char[] separator, int count)
        {
            if (separator == null || separator.Length == 0)
                separator = WhitespaceChars;

            if (count < 0)
                throw new System.ArgumentOutOfRangeException("count");

            if (count == 0)
                return new string[0];

            if (count == 1)
                return new string[1] { Imports.convertToString(this) };

            string[] res = new string[count];
            int indx = 0, last = 0;

            for (int i = 0; i < Length; i++)
                if (Belong(this[i], separator))
                {
                    res[indx] = this.Substring(last, i);
                    indx++;
                    last = i + 1;
                }

            return res;
        }

        private bool Belong(char c, char[] a)
        {
            foreach (char cc in a)
                if (c == cc)
                    return true;

            return false;
        }

        public static string Format(string format, object arg0)
        {
            return Format(null, format, new object[] { arg0 });
        }

        public static string Format(string format, object arg0, object arg1)
        {
            return Format(null, format, new object[] { arg0, arg1 });
        }

        public static string Format(string format, object arg0, object arg1, object arg2)
        {
            return Format(null, format, new object[] { arg0, arg1, arg2 });
        }

        public static string Format(string format, params object[] args)
        {
            StringBuilder b = FormatHelper(null, format, args);
            return b.ToString();
        }

        internal static StringBuilder FormatHelper(StringBuilder result, string format, params object[] args)
        {
            if (format == null)
                throw new System.ArgumentNullException("format");
            if (args == null)
                throw new System.ArgumentNullException("args");

            if (result == null)
            {
                /* Try to approximate the size of result to avoid reallocations */
                int i, len;

                len = 0;
                for (i = 0; i < args.Length; ++i)
                {
                    string s = args[i] as string;
                    if (s != null)
                        len += s.Length;
                    else
                        break;
                }
                if (i == args.Length)
                    result = new StringBuilder(len + format.Length);
                else
                    result = new StringBuilder();
            }

            int ptr = 0;
            int start = ptr;
            while (ptr < format.Length)
            {
                char c = format[ptr++];

                if (c == '{')
                {
                    result.Append(format, start, ptr - start - 1);

                    // check for escaped open bracket

                    if (format[ptr] == '{')
                    {
                        start = ptr++;
                        continue;
                    }

                    // parse specifier

                    int n, width;
                    bool left_align;
                    string arg_format;

                    ParseFormatSpecifier(format, ref ptr, out n, out width, out left_align, out arg_format);
                    if (n >= args.Length)
                        throw new System.FormatException("Index (zero based) must be greater than or equal to zero and less than the size of the argument list.");

                    // format argument

                    object arg = args[n];

                    string str;

                    str = arg.ToString();

                    // pad formatted string and append to result

                    if (width > str.Length)
                    {
                        const char padchar = ' ';
                        int padlen = width - str.Length;

                        if (left_align)
                        {
                            result.Append(str);
                            result.Append(padchar, padlen);
                        }
                        else
                        {
                            result.Append(padchar, padlen);
                            result.Append(str);
                        }
                    }
                    else
                        result.Append(str);

                    start = ptr;
                }
                else if (c == '}' && ptr < format.Length && format[ptr] == '}')
                {
                    result.Append(format, start, ptr - start - 1);
                    start = ptr++;
                }
                else if (c == '}')
                {
                    throw new System.FormatException("Input string was not in a correct format.");
                }
            }

            if (start < format.Length)
                result.Append(format, start, format.Length - start);

            return result;
        }

        private static void ParseFormatSpecifier (string str, ref int ptr, out int n, out int width,
		                                          out bool left_align, out string format)
		{
			int max = str.Length;

			// parses format specifier of form:
			//   N,[\ +[-]M][:F]}
			//
			// where:
			// N = argument number (non-negative integer)

			n = ParseDecimal (str, ref ptr);
			if (n < 0)
				throw new System.FormatException ("Input string was not in a correct format.");

			// M = width (non-negative integer)

			if (ptr < max && str[ptr] == ',') {
				// White space between ',' and number or sign.
				++ptr;
				while (ptr < max && Char.IsWhiteSpace (str [ptr]))
					++ptr;
				int start = ptr;

				format = str.Substring (start, ptr - start);

				left_align = (ptr < max && str [ptr] == '-');
				if (left_align)
					++ ptr;

				width = ParseDecimal (str, ref ptr);
				if (width < 0)
					throw new System.FormatException ("Input string was not in a correct format.");
			}
			else {
				width = 0;
				left_align = false;
				format = String.Empty;
			}

			// F = argument format (string)

			if (ptr < max && str[ptr] == ':') {
				int start = ++ ptr;
				while (ptr < max && str[ptr] != '}')
					++ ptr;

				format += str.Substring (start, ptr - start);
			}
			else
				format = null;

			if ((ptr >= max) || str[ptr ++] != '}')
				throw new System.FormatException ("Input string was not in a correct format.");
		}

        private static int ParseDecimal (string str, ref int ptr)
		{
			int p = ptr;
			int n = 0;
			int max = str.Length;

			while (p < max) {
				char c = str[p];
				if (c < '0' || '9' < c)
					break;

				n = n * 10 + c - '0';
				++ p;
			}

			if (p == ptr || p == max)
				return -1;

			ptr = p;
			return n;
		}
    }
}

