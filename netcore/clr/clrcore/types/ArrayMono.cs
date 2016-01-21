////
//// System.Array.cs
////
//// Authors:
////   Joe Shaw (joe@ximian.com)
////   Martin Baulig (martin@gnome.org)
////   Dietmar Maurer (dietmar@ximian.com)
////   Gonzalo Paniagua Javier (gonzalo@ximian.com)
////   Jeffrey Stedfast (fejj@novell.com)
////   Marek Safar (marek.safar@gmail.com)
////
//// (C) 2001-2003 Ximian, Inc.  http://www.ximian.com
//// Copyright (C) 2004-2011 Novell, Inc (http://www.novell.com)
//// Copyright (C) 2011 Xamarin Inc (http://www.xamarin.com)
////
//// Permission is hereby granted, free of charge, to any person obtaining
//// a copy of this software and associated documentation files (the
//// "Software"), to deal in the Software without restriction, including
//// without limitation the rights to use, copy, modify, merge, publish,
//// distribute, sublicense, and/or sell copies of the Software, and to
//// permit persons to whom the Software is furnished to do so, subject to
//// the following conditions:
////
//// The above copyright notice and this permission notice shall be
//// included in all copies or substantial portions of the Software.
////
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
////

//using Morph.Collections;

//namespace Morph
//{
//        // FIXME: We are doing way to many double/triple exception checks for the overloaded functions"
//    public abstract class Array : ICollection, IEnumerable, IList //, ICloneable,
//    {
//        // Constructor
//        private Array()
//        {
//        }

//        /*
//         * These methods are used to implement the implicit generic interfaces
//         * implemented by arrays in NET 2.0.
//         * Only make those methods generic which really need it, to avoid
//         * creating useless instantiations.
//         */
//        internal int InternalArray__ICollection_get_Count()
//        {
//            return Length;
//        }

//        internal bool InternalArray__ICollection_get_IsReadOnly()
//        {
//            return true;
//        }

//        internal void InternalArray__ICollection_Clear()
//        {
//            throw new NotSupportedException("Collection is read-only");
//        }

//        internal void InternalArray__RemoveAt(int index)
//        {
//            throw new NotSupportedException("Collection is of a fixed size");
//        }



//        // Properties
//        public int Length
//        {
//            get
//            {
//                int length = this.GetLength(0);

//                for (int i = 1; i < this.Rank; i++)
//                {
//                    length *= this.GetLength(i);
//                }
//                return length;
//            }
//        }

//        public long LongLength
//        {
//            get { return Length; }
//        }

//        public int Rank
//        {
//            get
//            {
//                return this.GetRank();
//            }
//        }

//        // IList interface
//        object IList.this[int index]
//        {
//            get
//            {
//                if (unchecked((uint)index) >= unchecked((uint)Length))
//                    //throw new IndexOutOfRangeException("index");
//                    return null;
//                if (this.Rank > 1)
//                    //throw new ArgumentException(Locale.GetText("Only single dimension arrays are supported."));
//                    return null;
//                return GetValueImpl(index);
//            }
//            set
//            {
//                if (unchecked((uint)index) >= unchecked((uint)Length))
//                    //throw new IndexOutOfRangeException("index");
//                    return;
//                if (this.Rank > 1)
//                    //throw new ArgumentException(Locale.GetText("Only single dimension arrays are supported."));
//                    return;
//                SetValueImpl(value, index);
//            }
//        }

//        int IList.Add(object value)
//        {
//            throw new NotSupportedException();
//        }

//        void IList.Clear()
//        {
//            Array.Clear(this, this.GetLowerBound(0), this.Length);
//        }

//        bool IList.Contains(object value)
//        {
//            if (this.Rank > 1)
//                //throw new RankException(Locale.GetText("Only single dimension arrays are supported."));
//                //TODO: this can be easily implemented, i believe
//                return false;

//            int length = this.Length;
//            for (int i = 0; i < length; i++)
//            {
//                if (Object.Equals(this.GetValueImpl(i), value))
//                    return true;
//            }
//            return false;
//        }

//        int IList.IndexOf(object value)
//        {
//            if (this.Rank > 1)
//                //throw new RankException(Locale.GetText("Only single dimension arrays are supported."));
//                //TODO:
//                return -1;

//            int length = this.Length;
//            for (int i = 0; i < length; i++)
//            {
//                if (Object.Equals(this.GetValueImpl(i), value))
//                    // array index may not be zero-based.
//                    // use lower bound
//                    return i + this.GetLowerBound(0);
//            }

//            unchecked
//            {
//                // lower bound may be MinValue
//                return this.GetLowerBound(0) - 1;
//            }
//        }

//        void IList.Insert(int index, object value)
//        {
//            throw new NotSupportedException();
//        }

//        void IList.Remove(object value)
//        {
//            throw new NotSupportedException();
//        }

//        void IList.RemoveAt(int index)
//        {
//            throw new NotSupportedException();
//        }

//        // InternalCall Methods
//        extern int GetRank();

//        public extern int GetLength(int dimension);

//        public long GetLongLength(int dimension)
//        {
//            return GetLength(dimension);
//        }

//        public extern int GetLowerBound(int dimension);

//        public extern object GetValue(params int[] indices);

//        public extern void SetValue(object value, params int[] indices);

//        // CAUTION! No bounds checking!
//        internal extern object GetValueImpl(int pos);

//        // CAUTION! No bounds checking!
//        internal extern void SetValueImpl(object value, int pos);

//        internal extern static bool FastCopy(Array source, int source_idx, Array dest, int dest_idx, int length);

//        //internal extern static Array CreateInstanceImpl(Type elementType, int[] lengths, int[] bounds);

//        // Properties
//        int ICollection.Count
//        {
//            get
//            {
//                return Length;
//            }
//        }

//        public bool IsSynchronized
//        {
//            get
//            {
//                return false;
//            }
//        }

//        public object SyncRoot
//        {
//            get
//            {
//                return this;
//            }
//        }

//        public bool IsFixedSize
//        {
//            get
//            {
//                return true;
//            }
//        }

//        public bool IsReadOnly
//        {
//            get
//            {
//                return false;
//            }
//        }

//        public IEnumerator GetEnumerator()
//        {
//            return new SimpleEnumerator(this);
//        }


//        public int GetUpperBound(int dimension)
//        {
//            return GetLowerBound(dimension) + GetLength(dimension) - 1;
//        }

//        public object GetValue(int index)
//        {
//            if (Rank != 1)
//                throw new ArgumentException(Locale.GetText("Array was not a one-dimensional array."));
//            if (index < GetLowerBound(0) || index > GetUpperBound(0))
//                throw new IndexOutOfRangeException(Locale.GetText(
//                    "Index has to be between upper and lower bound of the array."));

//            return GetValueImpl(index - GetLowerBound(0));
//        }

//        public object GetValue(int index1, int index2)
//        {
//            int[] ind = { index1, index2 };
//            return GetValue(ind);
//        }

//        public object GetValue(int index1, int index2, int index3)
//        {
//            int[] ind = { index1, index2, index3 };
//            return GetValue(ind);
//        }

//        public object GetValue(long index)
//        {
//            if (index < 0 || index > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            return GetValue((int)index);
//        }

//        public object GetValue(long index1, long index2)
//        {
//            if (index1 < 0 || index1 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index1", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            if (index2 < 0 || index2 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index2", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            return GetValue((int)index1, (int)index2);
//        }

//        public object GetValue(long index1, long index2, long index3)
//        {
//            if (index1 < 0 || index1 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index1", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            if (index2 < 0 || index2 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index2", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            if (index3 < 0 || index3 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index3", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            return GetValue((int)index1, (int)index2, (int)index3);
//        }

//        public void SetValue(object value, long index)
//        {
//            if (index < 0 || index > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            SetValue(value, (int)index);
//        }

//        public void SetValue(object value, long index1, long index2)
//        {
//            if (index1 < 0 || index1 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index1", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            if (index2 < 0 || index2 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index2", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            int[] ind = { (int)index1, (int)index2 };
//            SetValue(value, ind);
//        }

//        public void SetValue(object value, long index1, long index2, long index3)
//        {
//            if (index1 < 0 || index1 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index1", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            if (index2 < 0 || index2 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index2", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            if (index3 < 0 || index3 > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index3", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            int[] ind = { (int)index1, (int)index2, (int)index3 };
//            SetValue(value, ind);
//        }

//        public void SetValue(object value, int index)
//        {
//            if (Rank != 1)
//                throw new ArgumentException(Locale.GetText("Array was not a one-dimensional array."));
//            if (index < GetLowerBound(0) || index > GetUpperBound(0))
//                throw new IndexOutOfRangeException(Locale.GetText(
//                    "Index has to be >= lower bound and <= upper bound of the array."));

//            SetValueImpl(value, index - GetLowerBound(0));
//        }

//        public void SetValue(object value, int index1, int index2)
//        {
//            int[] ind = { index1, index2 };
//            SetValue(value, ind);
//        }

//        public void SetValue(object value, int index1, int index2, int index3)
//        {
//            int[] ind = { index1, index2, index3 };
//            SetValue(value, ind);
//        }

//        //public static Array CreateInstance(Type elementType, int length)
//        //{
//        //    int[] lengths = { length };

//        //    return CreateInstance(elementType, lengths);
//        //}

//        //public static Array CreateInstance(Type elementType, int length1, int length2)
//        //{
//        //    int[] lengths = { length1, length2 };

//        //    return CreateInstance(elementType, lengths);
//        //}

//        //public static Array CreateInstance(Type elementType, int length1, int length2, int length3)
//        //{
//        //    int[] lengths = { length1, length2, length3 };

//        //    return CreateInstance(elementType, lengths);
//        //}

//        //public static Array CreateInstance(Type elementType, params int[] lengths)
//        //{
//        //    if (elementType == null)
//        //        throw new ArgumentNullException("elementType");
//        //    if (lengths == null)
//        //        throw new ArgumentNullException("lengths");

//        //    if (lengths.Length > 255)
//        //        throw new TypeLoadException();

//        //    int[] bounds = null;

//        //    elementType = elementType.UnderlyingSystemType;
//        //    if (!elementType.IsSystemType)
//        //        throw new ArgumentException("Type must be a type provided by the runtime.", "elementType");
//        //    if (elementType.Equals(typeof(void)))
//        //        throw new NotSupportedException("Array type can not be void");
//        //    if (elementType.ContainsGenericParameters)
//        //        throw new NotSupportedException("Array type can not be an open generic type");
//        //    if ((elementType is TypeBuilder) && !(elementType as TypeBuilder).IsCreated())
//        //        throw new NotSupportedException("Can't create an array of the unfinished type '" + elementType + "'.");

//        //    return CreateInstanceImpl(elementType, lengths, bounds);
//        //}

//        //public static Array CreateInstance(Type elementType, int[] lengths, int[] lowerBounds)
//        //{
//        //    if (elementType == null)
//        //        throw new ArgumentNullException("elementType");
//        //    if (lengths == null)
//        //        throw new ArgumentNullException("lengths");
//        //    if (lowerBounds == null)
//        //        throw new ArgumentNullException("lowerBounds");

//        //    elementType = elementType.UnderlyingSystemType;
//        //    if (!elementType.IsSystemType)
//        //        throw new ArgumentException("Type must be a type provided by the runtime.", "elementType");
//        //    if (elementType.Equals(typeof(void)))
//        //        throw new NotSupportedException("Array type can not be void");
//        //    if (elementType.ContainsGenericParameters)
//        //        throw new NotSupportedException("Array type can not be an open generic type");

//        //    if (lengths.Length < 1)
//        //        throw new ArgumentException(Locale.GetText("Arrays must contain >= 1 elements."));

//        //    if (lengths.Length != lowerBounds.Length)
//        //        throw new ArgumentException(Locale.GetText("Arrays must be of same size."));

//        //    for (int j = 0; j < lowerBounds.Length; j++)
//        //    {
//        //        if (lengths[j] < 0)
//        //            throw new ArgumentOutOfRangeException("lengths", Locale.GetText(
//        //                "Each value has to be >= 0."));
//        //        if ((long)lowerBounds[j] + (long)lengths[j] > (long)Int32.MaxValue)
//        //            throw new ArgumentOutOfRangeException("lengths", Locale.GetText(
//        //                "Length + bound must not exceed Int32.MaxValue."));
//        //    }

//        //    if (lengths.Length > 255)
//        //        throw new TypeLoadException();

//        //    return CreateInstanceImpl(elementType, lengths, lowerBounds);
//        //}

//        static int[] GetIntArray(long[] values)
//        {
//            int len = values.Length;
//            int[] ints = new int[len];
//            for (int i = 0; i < len; i++)
//            {
//                long current = values[i];
//                if (current < 0 || current > (long)Int32.MaxValue)
//                    throw new ArgumentOutOfRangeException("values", Locale.GetText(
//                        "Each value has to be >= 0 and <= Int32.MaxValue."));

//                ints[i] = (int)current;
//            }
//            return ints;
//        }

//        //public static Array CreateInstance(Type elementType, params long[] lengths)
//        //{
//        //    if (lengths == null)
//        //        throw new ArgumentNullException("lengths");
//        //    return CreateInstance(elementType, GetIntArray(lengths));
//        //}

//        public object GetValue(params long[] indices)
//        {
//            if (indices == null)
//                throw new ArgumentNullException("indices");
//            return GetValue(GetIntArray(indices));
//        }

//        public void SetValue(object value, params long[] indices)
//        {
//            if (indices == null)
//                throw new ArgumentNullException("indices");
//            SetValue(value, GetIntArray(indices));
//        }

//        public static int BinarySearch(Array array, object value)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (value == null)
//                return -1;

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            if (array.Length == 0)
//                return -1;

//            if (!(value is IComparable))
//                throw new ArgumentException(Locale.GetText("value does not support IComparable."));

//            return DoBinarySearch(array, array.GetLowerBound(0), array.GetLength(0), value, null);
//        }

//        public static int BinarySearch(Array array, object value, IComparer comparer)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            if (array.Length == 0)
//                return -1;

//            if ((comparer == null) && (value != null) && !(value is IComparable))
//                throw new ArgumentException(Locale.GetText(
//                    "comparer is null and value does not support IComparable."));

//            return DoBinarySearch(array, array.GetLowerBound(0), array.GetLength(0), value, comparer);
//        }

//        public static int BinarySearch(Array array, int index, int length, object value)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            if (index < array.GetLowerBound(0))
//                throw new ArgumentOutOfRangeException("index", Locale.GetText(
//                    "index is less than the lower bound of array."));
//            if (length < 0)
//                throw new ArgumentOutOfRangeException("length", Locale.GetText(
//                    "Value has to be >= 0."));
//            // re-ordered to avoid possible integer overflow
//            if (index > array.GetLowerBound(0) + array.GetLength(0) - length)
//                throw new ArgumentException(Locale.GetText(
//                    "index and length do not specify a valid range in array."));

//            if (array.Length == 0)
//                return -1;

//            if ((value != null) && (!(value is IComparable)))
//                throw new ArgumentException(Locale.GetText(
//                    "value does not support IComparable"));

//            return DoBinarySearch(array, index, length, value, null);
//        }

//        public static int BinarySearch(Array array, int index, int length, object value, IComparer comparer)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            if (index < array.GetLowerBound(0))
//                throw new ArgumentOutOfRangeException("index", Locale.GetText(
//                    "index is less than the lower bound of array."));
//            if (length < 0)
//                throw new ArgumentOutOfRangeException("length", Locale.GetText(
//                    "Value has to be >= 0."));
//            // re-ordered to avoid possible integer overflow
//            if (index > array.GetLowerBound(0) + array.GetLength(0) - length)
//                throw new ArgumentException(Locale.GetText(
//                    "index and length do not specify a valid range in array."));

//            if (array.Length == 0)
//                return -1;

//            if ((comparer == null) && (value != null) && !(value is IComparable))
//                throw new ArgumentException(Locale.GetText(
//                    "comparer is null and value does not support IComparable."));

//            return DoBinarySearch(array, index, length, value, comparer);
//        }

//        static int DoBinarySearch(Array array, int index, int length, object value, IComparer comparer)
//        {
//            // cache this in case we need it
//            if (comparer == null)
//                comparer = Comparer.Default;

//            int iMin = index;
//            // Comment from Tum (tum@veridicus.com):
//            // *Must* start at index + length - 1 to pass rotor test co2460binarysearch_iioi
//            int iMax = index + length - 1;
//            int iCmp = 0;
//            try
//            {
//                while (iMin <= iMax)
//                {
//                    // Be careful with overflow
//                    // http://googleresearch.blogspot.com/2006/06/extra-extra-read-all-about-it-nearly.html
//                    int iMid = iMin + ((iMax - iMin) / 2);
//                    object elt = array.GetValueImpl(iMid);

//                    iCmp = comparer.Compare(elt, value);

//                    if (iCmp == 0)
//                        return iMid;
//                    else if (iCmp > 0)
//                        iMax = iMid - 1;
//                    else
//                        iMin = iMid + 1; // compensate for the rounding down
//                }
//            }
//            catch (Exception e)
//            {
//                throw new InvalidOperationException(Locale.GetText("Comparer threw an exception."), e);
//            }

//            return ~iMin;
//        }

//        public static void Clear(Array array, int index, int length)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");
//            if (length < 0)
//                throw new IndexOutOfRangeException("length < 0");

//            int low = array.GetLowerBound(0);
//            if (index < low)
//                throw new IndexOutOfRangeException("index < lower bound");
//            index = index - low;

//            // re-ordered to avoid possible integer overflow
//            if (index > array.Length - length)
//                throw new IndexOutOfRangeException("index + length > size");

//            ClearInternal(array, index, length);
//        }

//        static extern void ClearInternal(Array a, int index, int count);

//        public extern object Clone();

//        public static void Copy(Array sourceArray, Array destinationArray, int length)
//        {
//            // need these checks here because we are going to use
//            // GetLowerBound() on source and dest.
//            if (sourceArray == null)
//                return; //throw new ArgumentNullException("sourceArray");

//            if (destinationArray == null)
//                return; //throw new ArgumentNullException("destinationArray");

//            Copy(sourceArray, sourceArray.GetLowerBound(0), destinationArray,
//                destinationArray.GetLowerBound(0), length);
//        }

//        public static void Copy(Array sourceArray, int sourceIndex, Array destinationArray, int destinationIndex, int length)
//        {
//            if (sourceArray == null)
//                return; //throw new ArgumentNullException("sourceArray");

//            if (destinationArray == null)
//                return; //throw new ArgumentNullException("destinationArray");

//            if (length < 0)
//                return; //throw new ArgumentOutOfRangeException("length", Locale.GetText("Value has to be >= 0.")); ;

//            if (sourceIndex < 0)
//                return; //throw new ArgumentOutOfRangeException("sourceIndex", Locale.GetText("Value has to be >= 0.")); ;

//            if (destinationIndex < 0)
//                return; //throw new ArgumentOutOfRangeException("destinationIndex", Locale.GetText("Value has to be >= 0.")); ;

//            if (FastCopy(sourceArray, sourceIndex, destinationArray, destinationIndex, length))
//                return;

//            int source_pos = sourceIndex - sourceArray.GetLowerBound(0);
//            int dest_pos = destinationIndex - destinationArray.GetLowerBound(0);

//            // re-ordered to avoid possible integer overflow
//            if (source_pos > sourceArray.Length - length)
//                return; //throw new ArgumentException("length");

//            if (dest_pos > destinationArray.Length - length)
//            {
//                string msg = "Destination array was not long enough. Check " +
//                    "destIndex and length, and the array's lower bounds";
//                return; //throw new ArgumentException(msg, string.Empty);
//            }

//            if (sourceArray.Rank != destinationArray.Rank)
//                return; //throw new RankException(Locale.GetText("Arrays must be of same size."));


//            if (!Object.ReferenceEquals(sourceArray, destinationArray) || source_pos > dest_pos)
//            {
//                for (int i = 0; i < length; i++)
//                {
//                    Object srcval = sourceArray.GetValueImpl(source_pos + i);

//                    try
//                    {
//                        destinationArray.SetValueImpl(srcval, dest_pos + i);
//                    }
//                    catch
//                    {
//                        if (src_type.Equals(typeof(Object)))
//                            throw new InvalidCastException();
//                        else
//                            throw new ArrayTypeMismatchException(String.Format(Locale.GetText(
//                                "(Types: source={0};  target={1})"), src_type.FullName, dst_type.FullName));
//                    }
//                }
//            }
//            else
//            {
//                for (int i = length - 1; i >= 0; i--)
//                {
//                    Object srcval = sourceArray.GetValueImpl(source_pos + i);

//                    try
//                    {
//                        destinationArray.SetValueImpl(srcval, dest_pos + i);
//                    }
//                    catch
//                    {
//                        if (src_type.Equals(typeof(Object)))
//                            throw new InvalidCastException();
//                        else
//                            throw new ArrayTypeMismatchException(String.Format(Locale.GetText(
//                                "(Types: source={0};  target={1})"), src_type.FullName, dst_type.FullName));
//                    }
//                }
//            }
//        }

//        public static void Copy(Array sourceArray, long sourceIndex, Array destinationArray,
//                                 long destinationIndex, long length)
//        {
//            if (sourceArray == null)
//                throw new ArgumentNullException("sourceArray");

//            if (destinationArray == null)
//                throw new ArgumentNullException("destinationArray");

//            if (sourceIndex < Int32.MinValue || sourceIndex > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("sourceIndex",
//                    Locale.GetText("Must be in the Int32 range."));

//            if (destinationIndex < Int32.MinValue || destinationIndex > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("destinationIndex",
//                    Locale.GetText("Must be in the Int32 range."));

//            if (length < 0 || length > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("length", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            Copy(sourceArray, (int)sourceIndex, destinationArray, (int)destinationIndex, (int)length);
//        }

//        public static void Copy(Array sourceArray, Array destinationArray, long length)
//        {
//            if (length < 0 || length > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("length", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            Copy(sourceArray, destinationArray, (int)length);
//        }

//        public static int IndexOf(Array array, object value)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            return IndexOf(array, value, 0, array.Length);
//        }

//        public static int IndexOf(Array array, object value, int startIndex)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            return IndexOf(array, value, startIndex, array.Length - startIndex);
//        }

//        public static int IndexOf(Array array, object value, int startIndex, int count)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            // re-ordered to avoid possible integer overflow
//            if (count < 0 || startIndex < array.GetLowerBound(0) || startIndex - 1 > array.GetUpperBound(0) - count)
//                throw new ArgumentOutOfRangeException();

//            int max = startIndex + count;
//            for (int i = startIndex; i < max; i++)
//            {
//                if (Object.Equals(array.GetValueImpl(i), value))
//                    return i;
//            }

//            return array.GetLowerBound(0) - 1;
//        }

//        public void Initialize()
//        {
//            //FIXME: We would like to find a compiler that uses
//            // this method. It looks like this method do nothing
//            // in C# so no exception is trown by the moment.
//        }

//        public static int LastIndexOf(Array array, object value)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Length == 0)
//                return array.GetLowerBound(0) - 1;
//            return LastIndexOf(array, value, array.Length - 1);
//        }

//        public static int LastIndexOf(Array array, object value, int startIndex)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            return LastIndexOf(array, value, startIndex, startIndex - array.GetLowerBound(0) + 1);
//        }

//        public static int LastIndexOf(Array array, object value, int startIndex, int count)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            int lb = array.GetLowerBound(0);
//            // Empty arrays do not throw ArgumentOutOfRangeException
//            if (array.Length == 0)
//                return lb - 1;

//            if (count < 0 || startIndex < lb ||
//                startIndex > array.GetUpperBound(0) || startIndex - count + 1 < lb)
//                throw new ArgumentOutOfRangeException();

//            for (int i = startIndex; i >= startIndex - count + 1; i--)
//            {
//                if (Object.Equals(array.GetValueImpl(i), value))
//                    return i;
//            }

//            return lb - 1;
//        }

//        /* delegate used to swap array elements */
//        delegate void Swapper(int i, int j);

//        static Swapper get_swapper(Array array)
//        {
//            if (array is int[])
//                return new Swapper(array.int_swapper);
//            if (array is double[])
//                return new Swapper(array.double_swapper);
//            if (array is object[])
//            {
//                return new Swapper(array.obj_swapper);
//            }
//            return new Swapper(array.slow_swapper);
//        }

//        public static void Reverse(Array array)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            Reverse(array, array.GetLowerBound(0), array.GetLength(0));
//        }

//        public static void Reverse(Array array, int index, int length)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            if (index < array.GetLowerBound(0) || length < 0)
//                throw new ArgumentOutOfRangeException();

//            // re-ordered to avoid possible integer overflow
//            if (index > array.GetUpperBound(0) + 1 - length)
//                throw new ArgumentException();

//            int end = index + length - 1;
//            object[] oarray = array as object[];
//            if (oarray != null)
//            {
//                while (index < end)
//                {
//                    object tmp = oarray[index];
//                    oarray[index] = oarray[end];
//                    oarray[end] = tmp;
//                    ++index;
//                    --end;
//                }
//                return;
//            }
//            int[] iarray = array as int[];
//            if (iarray != null)
//            {
//                while (index < end)
//                {
//                    int tmp = iarray[index];
//                    iarray[index] = iarray[end];
//                    iarray[end] = tmp;
//                    ++index;
//                    --end;
//                }
//                return;
//            }
//            double[] darray = array as double[];
//            if (darray != null)
//            {
//                while (index < end)
//                {
//                    double tmp = darray[index];
//                    darray[index] = darray[end];
//                    darray[end] = tmp;
//                    ++index;
//                    --end;
//                }
//                return;
//            }
//            // fallback
//            Swapper swapper = get_swapper(array);
//            while (index < end)
//            {
//                swapper(index, end);
//                ++index;
//                --end;
//            }
//        }

//        public static void Sort(Array array)
//        {
//            Sort(array, (IComparer)null);
//        }

//        public static void Sort(Array keys, Array items)
//        {
//            Sort(keys, items, (IComparer)null);
//        }

//        public static void Sort(Array array, IComparer comparer)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            SortImpl(array, null, array.GetLowerBound(0), array.GetLength(0), comparer);
//        }

//        public static void Sort(Array array, int index, int length)
//        {
//            Sort(array, index, length, (IComparer)null);
//        }

//        public static void Sort(Array keys, Array items, IComparer comparer)
//        {
//            if (items == null)
//            {
//                Sort(keys, comparer);
//                return;
//            }

//            if (keys == null)
//                throw new ArgumentNullException("keys");

//            if (keys.Rank > 1 || items.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            SortImpl(keys, items, keys.GetLowerBound(0), keys.GetLength(0), comparer);
//        }

//        public static void Sort(Array keys, Array items, int index, int length)
//        {
//            Sort(keys, items, index, length, (IComparer)null);
//        }

//        public static void Sort(Array array, int index, int length, IComparer comparer)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));

//            if (index < array.GetLowerBound(0))
//                throw new ArgumentOutOfRangeException("index");

//            if (length < 0)
//                throw new ArgumentOutOfRangeException("length", Locale.GetText(
//                    "Value has to be >= 0."));

//            if (array.Length - (array.GetLowerBound(0) + index) < length)
//                throw new ArgumentException();

//            SortImpl(array, null, index, length, comparer);
//        }

//        public static void Sort(Array keys, Array items, int index, int length, IComparer comparer)
//        {
//            if (items == null)
//            {
//                Sort(keys, index, length, comparer);
//                return;
//            }

//            if (keys == null)
//                throw new ArgumentNullException("keys");

//            if (keys.Rank > 1 || items.Rank > 1)
//                throw new RankException();

//            if (keys.GetLowerBound(0) != items.GetLowerBound(0))
//                throw new ArgumentException();

//            if (index < keys.GetLowerBound(0))
//                throw new ArgumentOutOfRangeException("index");

//            if (length < 0)
//                throw new ArgumentOutOfRangeException("length", Locale.GetText(
//                    "Value has to be >= 0."));

//            if (items.Length - (index + items.GetLowerBound(0)) < length || keys.Length - (index + keys.GetLowerBound(0)) < length)
//                throw new ArgumentException();

//            SortImpl(keys, items, index, length, comparer);
//        }

//        private static void SortImpl(Array keys, Array items, int index, int length, IComparer comparer)
//        {
//            if (length <= 1)
//                return;

//            int low = index;
//            int high = index + length - 1;

//#if !BOOTSTRAP_BASIC
//            if (comparer == null && items is object[])
//            {
//                /* Its better to compare typecodes as casts treat long/ulong/long based enums the same */
//                switch (Type.GetTypeCode(keys.GetType().GetElementType()))
//                {
//                    case TypeCode.Int32:
//                        qsort(keys as Int32[], items as object[], low, high);
//                        return;
//                    case TypeCode.Int64:
//                        qsort(keys as Int64[], items as object[], low, high);
//                        return;
//                    case TypeCode.Byte:
//                        qsort(keys as byte[], items as object[], low, high);
//                        return;
//                    case TypeCode.Char:
//                        qsort(keys as char[], items as object[], low, high);
//                        return;
//                    case TypeCode.DateTime:
//                        qsort(keys as DateTime[], items as object[], low, high);
//                        return;
//                    case TypeCode.Decimal:
//                        qsort(keys as decimal[], items as object[], low, high);
//                        return;
//                    case TypeCode.Double:
//                        qsort(keys as double[], items as object[], low, high);
//                        return;
//                    case TypeCode.Int16:
//                        qsort(keys as Int16[], items as object[], low, high);
//                        return;
//                    case TypeCode.SByte:
//                        qsort(keys as SByte[], items as object[], low, high);
//                        return;
//                    case TypeCode.Single:
//                        qsort(keys as Single[], items as object[], low, high);
//                        return;
//                    case TypeCode.UInt16:
//                        qsort(keys as UInt16[], items as object[], low, high);
//                        return;
//                    case TypeCode.UInt32:
//                        qsort(keys as UInt32[], items as object[], low, high);
//                        return;
//                    case TypeCode.UInt64:
//                        qsort(keys as UInt64[], items as object[], low, high);
//                        return;
//                    default:
//                        break;
//                }
//            }
//#endif

//            if (comparer == null)
//                CheckComparerAvailable(keys, low, high);

//            try
//            {
//                qsort(keys, items, low, high, comparer);
//            }
//            catch (Exception e)
//            {
//                throw new InvalidOperationException(Locale.GetText("The comparer threw an exception."), e);
//            }
//        }

//        /* note, these are instance methods */
//        void int_swapper(int i, int j)
//        {
//            int[] array = this as int[];
//            int val = array[i];
//            array[i] = array[j];
//            array[j] = val;
//        }

//        void obj_swapper(int i, int j)
//        {
//            object[] array = this as object[];
//            object val = array[i];
//            array[i] = array[j];
//            array[j] = val;
//        }

//        void slow_swapper(int i, int j)
//        {
//            object val = GetValueImpl(i);
//            SetValueImpl(GetValue(j), i);
//            SetValueImpl(val, j);
//        }

//        void double_swapper(int i, int j)
//        {
//            double[] array = this as double[];
//            double val = array[i];
//            array[i] = array[j];
//            array[j] = val;
//        }

//        static bool QSortArrange(Array keys, Array items, int lo, ref object v0, int hi, ref object v1, IComparer comparer)
//        {
//            IComparable cmp;
//            object tmp;

//            if (comparer != null)
//            {
//                if (comparer.Compare(v1, v0) < 0)
//                {
//                    swap(keys, items, lo, hi);
//                    tmp = v0;
//                    v0 = v1;
//                    v1 = tmp;

//                    return true;
//                }
//            }
//            else if (v0 != null)
//            {
//                cmp = v1 as IComparable;

//                if (v1 == null || cmp.CompareTo(v0) < 0)
//                {
//                    swap(keys, items, lo, hi);
//                    tmp = v0;
//                    v0 = v1;
//                    v1 = tmp;

//                    return true;
//                }
//            }

//            return false;
//        }

//        private static void qsort(Array keys, Array items, int low, int high, IComparer comparer)
//        {
//            //const int QSORT_THRESHOLD = 7;
//            object key, hi, lo;
//            IComparable cmp;
//            int mid, i, k;

//            // TODO: implement InsertionSort when QSORT_THRESHOLD reached

//            // calculate the middle element
//            mid = low + ((high - low) / 2);

//            // get the 3 keys
//            key = keys.GetValueImpl(mid);
//            hi = keys.GetValueImpl(high);
//            lo = keys.GetValueImpl(low);

//            // once we re-order the low, mid, and high elements to be in
//            // ascending order, we'll use mid as our pivot.
//            QSortArrange(keys, items, low, ref lo, mid, ref key, comparer);
//            if (QSortArrange(keys, items, mid, ref key, high, ref hi, comparer))
//                QSortArrange(keys, items, low, ref lo, mid, ref key, comparer);

//            cmp = key as IComparable;

//            // since we've already guaranteed that lo <= mid and mid <= hi,
//            // we can skip comparing them again.
//            k = high - 1;
//            i = low + 1;

//            do
//            {
//                // Move the walls in
//                if (comparer != null)
//                {
//                    while (i < k && comparer.Compare(key, keys.GetValueImpl(i)) >= 0)
//                        i++;

//                    while (k >= i && comparer.Compare(key, keys.GetValueImpl(k)) < 0)
//                        k--;
//                }
//                else if (cmp != null)
//                {
//                    while (i < k && cmp.CompareTo(keys.GetValueImpl(i)) >= 0)
//                        i++;

//                    while (k >= i && cmp.CompareTo(keys.GetValueImpl(k)) < 0)
//                        k--;
//                }
//                else
//                {
//                    // This has the effect of moving the null values to the front if comparer is null
//                    while (i < k && keys.GetValueImpl(i) == null)
//                        i++;

//                    while (k >= i && keys.GetValueImpl(k) != null)
//                        k--;
//                }

//                if (k <= i)
//                    break;

//                swap(keys, items, i, k);

//                // make sure we keep track of our pivot element
//                if (mid == i)
//                    mid = k;
//                else if (mid == k)
//                    mid = i;

//                i++;
//                k--;
//            } while (true);

//            if (k != mid)
//            {
//                // swap the pivot with the last element in the first partition
//                swap(keys, items, mid, k);
//            }

//            // recursively sort each partition
//            if ((k + 1) < high)
//                qsort(keys, items, k + 1, high, comparer);
//            if ((k - 1) > low)
//                qsort(keys, items, low, k - 1, comparer);
//        }

//        private static void CheckComparerAvailable(Array keys, int low, int high)
//        {
//            // move null keys to beginning of array,
//            // ensure that non-null keys implement IComparable
//            for (int i = 0; i < high; i++)
//            {
//                object obj = keys.GetValueImpl(i);
//                if (obj == null)
//                    continue;
//                if (!(obj is IComparable))
//                {
//                    //string msg = Locale.GetText("No IComparable interface found for type '{0}'.");
//                    //throw new InvalidOperationException(String.Format(msg, obj.GetType()));
//                }
//            }
//        }

//        private static void swap(Array keys, Array items, int i, int j)
//        {
//            object tmp = keys.GetValueImpl(i);
//            keys.SetValueImpl(keys.GetValueImpl(j), i);
//            keys.SetValueImpl(tmp, j);

//            if (items != null)
//            {
//                tmp = items.GetValueImpl(i);
//                items.SetValueImpl(items.GetValueImpl(j), i);
//                items.SetValueImpl(tmp, j);
//            }
//        }


//        public void CopyTo(Array array, int index)
//        {
//            if (array == null)
//                throw new ArgumentNullException("array");

//            // The order of these exception checks may look strange,
//            // but that's how the microsoft runtime does it.
//            if (this.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));
//            if (index + this.GetLength(0) > array.GetLowerBound(0) + array.GetLength(0))
//                throw new ArgumentException("Destination array was not long " +
//                    "enough. Check destIndex and length, and the array's " +
//                    "lower bounds.");
//            if (array.Rank > 1)
//                throw new RankException(Locale.GetText("Only single dimension arrays are supported."));
//            if (index < 0)
//                throw new ArgumentOutOfRangeException("index", Locale.GetText(
//                    "Value has to be >= 0."));

//            Copy(this, this.GetLowerBound(0), array, index, this.GetLength(0));
//        }

//        public void CopyTo(Array array, long index)
//        {
//            if (index < 0 || index > Int32.MaxValue)
//                throw new ArgumentOutOfRangeException("index", Locale.GetText(
//                    "Value must be >= 0 and <= Int32.MaxValue."));

//            CopyTo(array, (int)index);
//        }

//        internal class SimpleEnumerator : IEnumerator       //, ICloneable
//        {
//            Array enumeratee;
//            int currentpos;
//            int length;

//            public SimpleEnumerator(Array arrayToEnumerate)
//            {
//                this.enumeratee = arrayToEnumerate;
//                this.currentpos = -1;
//                this.length = arrayToEnumerate.Length;
//            }

//            public object Current
//            {
//                get
//                {
//                    // Exception messages based on MS implementation
//                    if (currentpos < 0)
//                        throw new InvalidOperationException(Locale.GetText(
//                            "Enumeration has not started."));
//                    if (currentpos >= length)
//                        throw new InvalidOperationException(Locale.GetText(
//                            "Enumeration has already ended"));
//                    // Current should not increase the position. So no ++ over here.
//                    return enumeratee.GetValueImpl(currentpos);
//                }
//            }

//            public bool MoveNext()
//            {
//                //The docs say Current should throw an exception if last
//                //call to MoveNext returned false. This means currentpos
//                //should be set to length when returning false.
//                if (currentpos < length)
//                    currentpos++;
//                if (currentpos < length)
//                    return true;
//                else
//                    return false;
//            }

//            public void Reset()
//            {
//                currentpos = -1;
//            }

//            public object Clone()
//            {
//                return MemberwiseClone();
//            }
//        }



//        //
//        // The constrained copy should guarantee that if there is an exception thrown
//        // during the copy, the destination array remains unchanged.
//        // This is related to System.Runtime.Reliability.CER
//        public static void ConstrainedCopy(Array sourceArray, int sourceIndex, Array destinationArray, int destinationIndex, int length)
//        {
//            Copy(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
//        }
//    }
//}
