//
// System.Buffer.cs
//
// Authors:
//   Paolo Molaro (lupus@ximian.com)
//   Dan Lewis (dihlewis@yahoo.co.uk)
//
// (C) 2001 Ximian, Inc.  http://www.ximian.com
//

//
// Copyright (C) 2004 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//



namespace Morph {
	public static class Buffer {

		public unsafe static int ByteLength (System.Array array)
		{
			// note: the other methods in this class also use ByteLength to test for
			// null and non-primitive arguments as a side-effect.

            if (array == null)
            {
                //throw new ArgumentNullException ("array");
            }

            int length = (int)clrcore.GarbageCollector.garbageCollectorGetObjectSize(Morph.Imports.convert(array));
            if (length < 0)
            {
                //throw new ArgumentException (Locale.GetText ("Object must be an array of primitives."));
            }

			return length;
		}

		public static byte GetByte (System.Array array, int index)
		{
            if (index < 0 || index >= ByteLength(array))
            {
                //throw new ArgumentOutOfRangeException ("index", Locale.GetText("Value must be non-negative and less than the size of the collection."));
            }

            return ((byte[])array)[index];
		}

		public static void SetByte (System.Array array, int index, byte value)
		{
            if (index < 0 || index >= ByteLength(array))
            {
                //throw new ArgumentOutOfRangeException ("index", Locale.GetText("Value must be non-negative and less than the size of the collection."));
            }

            ((byte[])array)[index] = value;
		}

        public unsafe static void BlockCopy(System.Array src, int srcOffset, System.Array dst, int dstOffset, int count)
		{
            if (src == null)
            {
                //throw new ArgumentNullException ("src");
            }

            if (dst == null)
            {
                //throw new ArgumentNullException ("dst");
            }

            if (srcOffset < 0)
            {
                //throw new ArgumentOutOfRangeException ("srcOffset", Locale.GetText("Non-negative number required."));
            }

            if (dstOffset < 0)
            {
                //throw new ArgumentOutOfRangeException ("dstOffset", Locale.GetText ("Non-negative number required."));
            }

            if (count < 0)
            {
                //throw new ArgumentOutOfRangeException ("count", Locale.GetText ("Non-negative number required."));
            }

			// We do the checks in unmanaged code for performance reasons
            object o = src;
            Morph.Array src_array = (Morph.Array)o;
            o = dst;
            Morph.Array dst_array = (Morph.Array)o;
            clrcore.Memory.memcpy(dst_array.internalGetBuffer() + dstOffset, src_array.internalGetBuffer() + srcOffset, (uint)count);
		}
	}
}
