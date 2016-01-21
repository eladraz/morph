using clrcore;
using Morph.Collections;

namespace Morph
{
    public class Array //: ICloneable, ICollection, IList, IEnumerable
    {
        // Order is very important here since this function is called by the compiler
        private static unsafe Array compilerInstanceNewArray1(uint numberOfElements, void* vtbl)
        {
            return new Array(vtbl, numberOfElements);
        }

        // TODO! Will be translated into "inline" when the feature will be possible
        // "sizeOfElements" is duplicated due to the fact that the function will be inline, so the optimizer
        // might use it as "const" and replace MUL with bitwise shifts
        private static unsafe byte* compilerGetArrayOffset(Array arr, uint index, uint sizeofElements)
        {
            return arr.m_buffer + (index * sizeofElements);
        }

        private unsafe Array(void* vtbl, uint numberOfElements)
        {
            m_internalTypeVtbl = vtbl;
            m_numberOfElements = numberOfElements;
            VirtualTable.VirtualTableHeader* vtblHeader = VirtualTable.getVirtualTableHeader(vtbl);
            m_objectSize = vtblHeader->m_objectSize;

            if (VirtualTable.virtualTableGetInterfaceLocation(vtbl, Morph.ValueType.getRTTI()) == 0xFFFFFFFF)
            {
                // Allocating an array[] the memory per instance is sizeof(object).
                // If the object is structure (not this case), the size of instance is vtblHeader->m_objectSize
                // Otherwise, the size per instance is void* length
                m_objectSize = (ushort)(sizeof(void*));
            }

            // Alignment note: The VTBL is already aligned by the compiler according to layout policy
            m_dataSize = m_objectSize * numberOfElements;

            m_buffer = (byte*)Morph.Imports.allocate(m_dataSize);
            clrcore.Memory.memset(m_buffer, 0, m_dataSize);
        }

        ~Array()
        {
            unsafe
            {
                if ((VirtualTable.virtualTableGetInterfaceLocation(m_internalTypeVtbl, Morph.ValueType.getRTTI()) == 0xFFFFFFFF))
                {
                    // Call destructor only for managed objects and not valuetype (structs)
                    void** arr = (void**)m_buffer;
                    for (uint i = 0; i < m_numberOfElements; i++, ++arr)
                    {
                        clrcore.GarbageCollector.garbageCollectorDecreaseReference(*arr);
                    }
                }

                Morph.Imports.free((void*)m_buffer);
            }
        }

        internal unsafe byte* internalGetBuffer()
        {
            return m_buffer;
        }

        private unsafe void* m_internalTypeVtbl;
        private unsafe byte* m_buffer;
        private uint m_numberOfElements;
        private uint m_dataSize;
        private ushort m_objectSize;
        private int m_rank = 1;

        public int Rank
        {
            get
            {
                return m_rank;
            }
        }

        //see: http://msdn.microsoft.com/en-us/library/z50k9bft.aspx

        public unsafe static void Copy(Array sourceArray, Array destinationArray, int length)
        {
            Copy(sourceArray, 0, destinationArray, 0, length);
        }


        public unsafe static void Copy(Array sourceArray, int srcIndex, Array destinationArray, int dstIndex, int length)
        {
            //Check boundries:
            if (sourceArray == null)
                throw new System.ArgumentNullException("sourceArray is null");

            if (destinationArray == null)
                throw new System.ArgumentNullException("destinationArray is null");

            if (sourceArray.Rank != destinationArray.Rank)
                throw new System.RankException();

            if (length < 0)
                throw new System.ArgumentOutOfRangeException("length", "Value has to be >= 0.");

            if (srcIndex < 0)
                throw new System.ArgumentOutOfRangeException("sourceIndex", "Value has to be >= 0.");

            if (dstIndex < 0)
                throw new System.ArgumentOutOfRangeException("destinationIndex", "Value has to be >= 0.");

            if (length > sourceArray.Length - srcIndex)
                throw new System.ArgumentException("length is greater than the number of elements from sourceIndex to the end of sourceArray.");

            if (length > destinationArray.Length - dstIndex)
                throw new System.ArgumentException("length is greater than the number of elements from destinationIndex to the end of destinationArray.");

            unsafe
            {
                bool dstIsValueType = VirtualTable.virtualTableGetInterfaceLocation(destinationArray.m_internalTypeVtbl, Morph.ValueType.getRTTI()) != 0xFFFFFFFF;
                bool srcIsValueType = VirtualTable.virtualTableGetInterfaceLocation(sourceArray.m_internalTypeVtbl, Morph.ValueType.getRTTI()) != 0xFFFFFFFF;

                if (dstIsValueType != srcIsValueType)
                {
                    throw new System.InvalidCastException("Morph cannot copy value-type array to refernce-type array, or referenc-typ array to value-type array");
                }
                if (sourceArray.m_objectSize != destinationArray.m_objectSize)
                {
                    throw new System.InvalidCastException("Cannot copy arrays with different object-sizes");
                }


                //if the destination is a value type and initialized - decrease ref counts.
                if (!(dstIsValueType))
                {
                    //use array.clear to remove the ref-counts
                    Array.Clear(destinationArray, dstIndex, length);
                }

                //no copy
                clrcore.Memory.memmove(destinationArray.m_buffer + dstIndex * destinationArray.m_objectSize, sourceArray.m_buffer + srcIndex * sourceArray.m_objectSize, (uint) (length * sourceArray.m_objectSize));

                //increase refcount if this is a reference-type array
                if (!dstIsValueType)
                {
                    for (int i = 0; i < length; i++)
                    {
                        void** arr = (void**)destinationArray.m_buffer;
                        if (arr[i] != null)
                        {
                            GarbageCollector.garbageCollectorIncreaseReference(arr[i]);
                        }
                    }
                }
            }
        }


        public unsafe static void Clear(Array array, int index, int length)
        {
            if (array == null)
            {
                 throw new System.NullReferenceException();
            }

            if (index < 0 || length < 0 || length + index > array.m_numberOfElements)
            {
                throw new System.IndexOutOfRangeException();
            }

            unsafe{
                if (VirtualTable.virtualTableGetInterfaceLocation(array.m_internalTypeVtbl, Morph.ValueType.getRTTI()) == 0xFFFFFFFF)
                {
                    for (int i = 0; i < length; i++)
                    {
                        void** arr = (void**)array.m_buffer;
                        if (arr[i] != null)
                        {
                            GarbageCollector.garbageCollectorDecreaseReference(arr[i]);
                        }
                    }
                }

                clrcore.Memory.memset(array.m_buffer + index * array.m_objectSize, 0, (uint)(length * array.m_objectSize));
            }
        }

        public unsafe System.Object Clone()
        {
            Array result;
            result = new Array(m_internalTypeVtbl, m_numberOfElements);
            return result;
        }


        #region Properties
        public int Length
        {
            get
            {
                int length = this.GetLength(0);

                for (int i = 1; i < this.Rank; i++)
                {
                    length *= this.GetLength(i);
                }
                return length;
            }
        }

        #endregion

        public static int LastIndexOf( Array array, Object value)
        {
            throw new System.NotImplementedException();
        }
        public static int LastIndexOf(Array array, Object value, int startIndex)
        {
            throw new System.NotImplementedException();
        }
        public static int LastIndexOf(Array array, Object value, int startIndex, int count)
        {
            throw new System.NotImplementedException();
        }

        private unsafe int GetRank()
        {
            return m_rank;
        }

        public unsafe int GetLength(int dimension)
        {
            //TODO: add support for multiple dimenstions
            return (int) m_numberOfElements;
        }

        public int GetLowerBound(int dimension)
        {
            throw new System.NotImplementedException();
        }

        //public void CopyTo(System.Array array, int index)
        //{
        //    if (array == null)
        //        throw new System.ArgumentNullException("array");

        //    // The order of these exception checks may look strange,
        //    // but that's how the microsoft runtime does it.
        //    if (this.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");
        //    if (index + this.GetLength(0) > array.GetLowerBound(0) + array.GetLength(0))
        //        throw new System.ArgumentException("Destination array was not long " +
        //            "enough. Check destIndex and length, and the array's " +
        //            "lower bounds.");
        //    if (array.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");
        //    if (index < 0)
        //        throw new System.ArgumentOutOfRangeException("index", "Value has to be >= 0.");

        //    Copy(Imports.convertToArray(this), this.GetLowerBound(0), array, index, this.GetLength(0));
        //}

        //public static System.Array CreateInstance(System.Type elementType, int length)
        //{
        //    int[] lengths = { length };

        //    return CreateInstance(elementType, lengths);
        //}

        //public static System.Array CreateInstance(System.Type elementType, int length1, int length2)
        //{
        //    int[] lengths = { length1, length2 };

        //    return CreateInstance(elementType, lengths);
        //}

        //public static System.Array CreateInstance(System.Type elementType, int length1, int length2, int length3)
        //{
        //    int[] lengths = { length1, length2, length3 };

        //    return CreateInstance(elementType, lengths);
        //}

        //public static System.Array CreateInstance(System.Type elementType, params int[] lengths)
        //{
        //    if (elementType == null)
        //        throw new System.ArgumentNullException("elementType");
        //    if (lengths == null)
        //        throw new System.ArgumentNullException("lengths");

        //    if (lengths.Length > 255)
        //        throw new System.TypeLoadException();

        //    int[] bounds = null;

        //    elementType = elementType.UnderlyingSystemType;
        //    //if (!elementType.IsSystemType)
        //    //    throw new System.ArgumentException("Type must be a type provided by the runtime.", "elementType");
        //    if (elementType.Equals(typeof(void)))
        //        throw new System.NotSupportedException("Array type can not be void");
        //    if (elementType.ContainsGenericParameters)
        //        throw new System.NotSupportedException("Array type can not be an open generic type");
        //    //if ((elementType is TypeBuilder) && !(elementType as TypeBuilder).IsCreated())
        //    //    throw new System.NotSupportedException("Can't create an array of the unfinished type '" + elementType + "'.");

        //    return CreateInstanceImpl(elementType, lengths, bounds);
        //}

        //public static System.Array CreateInstance(System.Type elementType, int[] lengths, int[] lowerBounds)
        //{
        //    if (elementType == null)
        //        throw new System.ArgumentNullException("elementType");
        //    if (lengths == null)
        //        throw new System.ArgumentNullException("lengths");
        //    if (lowerBounds == null)
        //        throw new System.ArgumentNullException("lowerBounds");

        //    elementType = elementType.UnderlyingSystemType;
        //    //if (!elementType.IsSystemType)
        //    //    throw new System.ArgumentException("Type must be a type provided by the runtime.", "elementType");
        //    if (elementType.Equals(typeof(void)))
        //        throw new System.NotSupportedException("Array type can not be void");
        //    if (elementType.ContainsGenericParameters)
        //        throw new System.NotSupportedException("Array type can not be an open generic type");

        //    if (lengths.Length < 1)
        //        throw new System.ArgumentException("Arrays must contain >= 1 elements.");

        //    if (lengths.Length != lowerBounds.Length)
        //        throw new System.ArgumentException("Arrays must be of same size.");

        //    for (int j = 0; j < lowerBounds.Length; j++)
        //    {
        //        if (lengths[j] < 0)
        //            throw new System.ArgumentOutOfRangeException("lengths", "Each value has to be >= 0.");
        //        if ((long)lowerBounds[j] + (long)lengths[j] > (long)Int32.MaxValue)
        //            throw new System.ArgumentOutOfRangeException("lengths", "Length + bound must not exceed Int32.MaxValue.");
        //    }

        //    if (lengths.Length > 255)
        //        throw new System.TypeLoadException();

        //    return CreateInstanceImpl(elementType, lengths, lowerBounds);
        //}

        //public int GetUpperBound(int dimension)
        //{
        //    return GetLowerBound(dimension) + GetLength(dimension) - 1;
        //}

        //public object GetValue(int index)
        //{
        //    if (Rank != 1)
        //        throw new System.ArgumentException("Array was not a one-dimensional array.");
        //    if (index < GetLowerBound(0) || index > GetUpperBound(0))
        //        throw new System.IndexOutOfRangeException("Index has to be between upper and lower bound of the array.");

        //    return GetValueImpl(index - GetLowerBound(0));
        //}

        //public object GetValue(int index1, int index2)
        //{
        //    int[] ind = { index1, index2 };
        //    return GetValue(ind);
        //}

        //public object GetValue(int index1, int index2, int index3)
        //{
        //    int[] ind = { index1, index2, index3 };
        //    return GetValue(ind);
        //}

        //object IList.this[int index]
        //{
        //    get
        //    {
        //        if (unchecked((uint)index) >= unchecked((uint)Length))
        //            throw new System.IndexOutOfRangeException("index");
        //        if (this.Rank > 1)
        //            throw new System.ArgumentException("Only single dimension arrays are supported.");
        //        return GetValueImpl(index);
        //    }
        //    set
        //    {
        //        if (unchecked((uint)index) >= unchecked((uint)Length))
        //            throw new System.IndexOutOfRangeException("index");
        //        if (this.Rank > 1)
        //            throw new System.ArgumentException("Only single dimension arrays are supported.");
        //        SetValueImpl(value, index);
        //    }
        //}

        //int IList.Add(object value)
        //{
        //    throw new System.NotSupportedException();
        //}

        //void IList.Clear()
        //{
        //    Array.Clear(Imports.convertToArray(this), this.GetLowerBound(0), this.Length);
        //}

        //bool IList.Contains(object value)
        //{
        //    if (this.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");

        //    int length = this.Length;
        //    for (int i = 0; i < length; i++)
        //    {
        //        if (Object.Equals(this.GetValueImpl(i), value))
        //            return true;
        //    }
        //    return false;
        //}

        //int IList.IndexOf(object value)
        //{
        //    if (this.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");

        //    int length = this.Length;
        //    for (int i = 0; i < length; i++)
        //    {
        //        if (Object.Equals(this.GetValueImpl(i), value))
        //            // array index may not be zero-based.
        //            // use lower bound
        //            return i + this.GetLowerBound(0);
        //    }

        //    unchecked
        //    {
        //        // lower bound may be MinValue
        //        return this.GetLowerBound(0) - 1;
        //    }
        //}

        //void IList.Insert(int index, object value)
        //{
        //    throw new System.NotSupportedException();
        //}

        //void IList.Remove(object value)
        //{
        //    throw new System.NotSupportedException();
        //}

        //void IList.RemoveAt(int index)
        //{
        //    throw new System.NotSupportedException();
        //}

        //public static int IndexOf(Array array, object value)
        //{
        //    if (array == null)
        //        throw new System.ArgumentNullException("array");

        //    return IndexOf(array, value, 0, array.Length);
        //}

        //public static int IndexOf(Array array, object value, int startIndex)
        //{
        //    if (array == null)
        //        throw new System.ArgumentNullException("array");

        //    return IndexOf(array, value, startIndex, array.Length - startIndex);
        //}

        //public static int IndexOf(Array array, object value, int startIndex, int count)
        //{
        //    if (array == null)
        //        throw new System.ArgumentNullException("array");

        //    if (array.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");

        //    // re-ordered to avoid possible integer overflow
        //    if (count < 0 || startIndex < array.GetLowerBound(0) || startIndex - 1 > array.GetUpperBound(0) - count)
        //        throw new System.ArgumentOutOfRangeException();

        //    int max = startIndex + count;
        //    for (int i = startIndex; i < max; i++)
        //    {
        //        if (Object.Equals(array.GetValueImpl(i), value))
        //            return i;
        //    }

        //    return array.GetLowerBound(0) - 1;
        //}

        //int ICollection.Count
        //{
        //    get
        //    {
        //        return Length;
        //    }
        //}

        //public bool IsSynchronized
        //{
        //    get
        //    {
        //        return false;
        //    }
        //}

        //public object SyncRoot
        //{
        //    get
        //    {
        //        return this;
        //    }
        //}

        //public bool IsFixedSize
        //{
        //    get
        //    {
        //        return true;
        //    }
        //}

        //public bool IsReadOnly
        //{
        //    get
        //    {
        //        return false;
        //    }
        //}

        //public IEnumerator GetEnumerator()
        //{
        //    return new SimpleEnumerator(this);
        //}

        //internal class SimpleEnumerator : IEnumerator, ICloneable
        //{
        //    Array enumeratee;
        //    int currentpos;
        //    int length;

        //    public SimpleEnumerator(Array arrayToEnumerate)
        //    {
        //        this.enumeratee = arrayToEnumerate;
        //        this.currentpos = -1;
        //        this.length = arrayToEnumerate.Length;
        //    }

        //    public object Current
        //    {
        //        get
        //        {
        //            // Exception messages based on MS implementation
        //            if (currentpos < 0)
        //                throw new System.InvalidOperationException("Enumeration has not started.");
        //            if (currentpos >= length)
        //                throw new System.InvalidOperationException("Enumeration has already ended");
        //            // Current should not increase the position. So no ++ over here.
        //            return enumeratee.GetValueImpl(currentpos);
        //        }
        //    }

        //    public bool MoveNext()
        //    {
        //        //The docs say Current should throw an exception if last
        //        //call to MoveNext returned false. This means currentpos
        //        //should be set to length when returning false.
        //        if (currentpos < length)
        //            currentpos++;
        //        if (currentpos < length)
        //            return true;
        //        else
        //            return false;
        //    }

        //    public void Reset()
        //    {
        //        currentpos = -1;
        //    }

        //    public object Clone()
        //    {
        //        return MemberwiseClone();
        //    }
        //}

        //public static void Sort(Array array)
        //{
        //    Sort(array, (IComparer)null);
        //}

        //public static void Sort(Array keys, Array items)
        //{
        //    Sort(keys, items, (IComparer)null);
        //}

        //public static void Sort(Array array, IComparer comparer)
        //{
        //    if (array == null)
        //        throw new System.ArgumentNullException("array");

        //    if (array.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");

        //    SortImpl(array, null, array.GetLowerBound(0), array.GetLength(0), comparer);
        //}

        //public static void Sort(Array array, int index, int length)
        //{
        //    Sort(array, index, length, (IComparer)null);
        //}

        //public static void Sort(Array keys, Array items, IComparer comparer)
        //{
        //    if (items == null)
        //    {
        //        Sort(keys, comparer);
        //        return;
        //    }

        //    if (keys == null)
        //        throw new System.ArgumentNullException("keys");

        //    if (keys.Rank > 1 || items.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");

        //    SortImpl(keys, items, keys.GetLowerBound(0), keys.GetLength(0), comparer);
        //}

        //public static void Sort(Array keys, Array items, int index, int length)
        //{
        //    Sort(keys, items, index, length, (IComparer)null);
        //}

        //public static void Sort(Array array, int index, int length, IComparer comparer)
        //{
        //    if (array == null)
        //        throw new System.ArgumentNullException("array");

        //    if (array.Rank > 1)
        //        throw new System.RankException("Only single dimension arrays are supported.");

        //    if (index < array.GetLowerBound(0))
        //        throw new System.ArgumentOutOfRangeException("index");

        //    if (length < 0)
        //        throw new System.ArgumentOutOfRangeException("length", "Value has to be >= 0.");

        //    if (array.Length - (array.GetLowerBound(0) + index) < length)
        //        throw new System.ArgumentException();

        //    SortImpl(array, null, index, length, comparer);
        //}

        //public static void Sort(Array keys, Array items, int index, int length, IComparer comparer)
        //{
        //    if (items == null)
        //    {
        //        Sort(keys, index, length, comparer);
        //        return;
        //    }

        //    if (keys == null)
        //        throw new System.ArgumentNullException("keys");

        //    if (keys.Rank > 1 || items.Rank > 1)
        //        throw new System.RankException();

        //    if (keys.GetLowerBound(0) != items.GetLowerBound(0))
        //        throw new System.ArgumentException();

        //    if (index < keys.GetLowerBound(0))
        //        throw new System.ArgumentOutOfRangeException("index");

        //    if (length < 0)
        //        throw new System.ArgumentOutOfRangeException("length", "Value has to be >= 0.");

        //    if (items.Length - (index + items.GetLowerBound(0)) < length || keys.Length - (index + keys.GetLowerBound(0)) < length)
        //        throw new System.ArgumentException();

        //    SortImpl(keys, items, index, length, comparer);
        //}

        //private static void SortImpl(Array keys, Array items, int index, int length, IComparer comparer)
        //{
        //    if (length <= 1)
        //        return;

        //    int low = index;
        //    int high = index + length - 1;

        //    //if (comparer == null && items is object[])
        //    //{
        //    //    if (keys is int[])
        //    //    {
        //    //        qsort(keys as int[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is long[])
        //    //    {
        //    //        qsort(keys as long[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is char[])
        //    //    {
        //    //        qsort(keys as char[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is double[])
        //    //    {
        //    //        qsort(keys as double[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is uint[])
        //    //    {
        //    //        qsort(keys as uint[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is ulong[])
        //    //    {
        //    //        qsort(keys as ulong[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is byte[])
        //    //    {
        //    //        qsort(keys as byte[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //    if (keys is ushort[])
        //    //    {
        //    //        qsort(keys as ushort[], items as object[], low, high);
        //    //        return;
        //    //    }
        //    //}

        //    if (comparer == null)
        //        CheckComparerAvailable(keys, low, high);

        //    try
        //    {
        //        qsort(keys, items, low, high, comparer);
        //    }
        //    catch (System.Exception e)
        //    {
        //        throw new System.InvalidOperationException("The comparer threw an exception.", e);
        //    }
        //}

        //private static void qsort(Array keys, Array items, int low0, int high0, IComparer comparer)
        //{
        //    int low = low0;
        //    int high = high0;

        //    // Be careful with overflows
        //    int mid = low + ((high - low) / 2);
        //    object keyPivot = keys.GetValueImpl(mid);
        //    IComparable cmpPivot = keyPivot as IComparable;

        //    while (true)
        //    {
        //        // Move the walls in
        //        if (comparer != null)
        //        {
        //            while (low < high0 && comparer.Compare(keyPivot, keys.GetValueImpl(low)) > 0)
        //                ++low;
        //            while (high > low0 && comparer.Compare(keyPivot, keys.GetValueImpl(high)) < 0)
        //                --high;
        //        }
        //        else
        //        {
        //            if (keyPivot == null)
        //            {
        //                // This has the effect of moving the null values to the front if comparer is null
        //                while (high > low0 && keys.GetValueImpl(high) != null)
        //                    --high;
        //                while (low < high0 && keys.GetValueImpl(low) == null)
        //                    ++low;
        //            }
        //            else
        //            {
        //                while (low < high0 && cmpPivot.CompareTo(keys.GetValueImpl(low)) > 0)
        //                    ++low;
        //                while (high > low0 && cmpPivot.CompareTo(keys.GetValueImpl(high)) < 0)
        //                    --high;
        //            }
        //        }

        //        if (low <= high)
        //        {
        //            swap(keys, items, low, high);
        //            ++low;
        //            --high;
        //        }
        //        else
        //            break;
        //    }

        //    if (low0 < high)
        //        qsort(keys, items, low0, high, comparer);
        //    if (low < high0)
        //        qsort(keys, items, low, high0, comparer);
        //}

        //private static void CheckComparerAvailable(Array keys, int low, int high)
        //{
        //    // move null keys to beginning of array,
        //    // ensure that non-null keys implement IComparable
        //    for (int i = 0; i < high; i++)
        //    {
        //        object obj = keys.GetValueImpl(i);
        //        if (obj == null)
        //            continue;
        //        if (!(obj is IComparable))
        //        {
        //            string msg = "No IComparable interface found for type '" + obj.GetType().ToString() + "'.";
        //            throw new System.InvalidOperationException(msg);
        //        }
        //    }
        //}

        //private static void swap(Array keys, Array items, int i, int j)
        //{
        //    object tmp = keys.GetValueImpl(i);
        //    keys.SetValueImpl(keys.GetValueImpl(j), i);
        //    keys.SetValueImpl(tmp, j);

        //    if (items != null)
        //    {
        //        tmp = items.GetValueImpl(i);
        //        items.SetValueImpl(items.GetValueImpl(j), i);
        //        items.SetValueImpl(tmp, j);
        //    }
        //}

        public static void Reverse(System.Array array)
        {

            //if (array == null)
            //    throw new System.ArgumentNullException("array");

            //Reverse(array, array.GetLowerBound(0), array.GetLength(0));
        }

        public static void Reverse(System.Array array, int index, int length)
        {
            //if (array == null)
            //    throw new System.ArgumentNullException("array");

            //if (array.Rank > 1)
            //    throw new System.RankException("Only single dimension arrays are supported.");

            //if (index < array.GetLowerBound(0) || length < 0)
            //    throw new System.ArgumentOutOfRangeException();

            //// re-ordered to avoid possible integer overflow
            //if (index > array.GetUpperBound(0) + 1 - length)
            //    throw new System.ArgumentException();

            //int end = index + length - 1;
            //object[] oarray = Imports.convertToArray(array) as object[];
            //if (oarray != null)
            //{
            //    while (index < end)
            //    {
            //        object tmp = oarray[index];
            //        oarray[index] = oarray[end];
            //        oarray[end] = tmp;
            //        ++index;
            //        --end;
            //    }
            //    return;
            //}
            //int[] iarray = Imports.convertToArray(array) as int[];
            //if (iarray != null)
            //{
            //    while (index < end)
            //    {
            //        int tmp = iarray[index];
            //        iarray[index] = iarray[end];
            //        iarray[end] = tmp;
            //        ++index;
            //        --end;
            //    }
            //    return;
            //}

            //// fallback
            //Swapper swapper = get_swapper(array);
            //while (index < end)
            //{
            //    swapper(index, end);
            //    ++index;
            //    --end;
            //}
        }

        //delegate void Swapper(int i, int j);

        //static Swapper get_swapper(Array array)
        //{
        //    if (Imports.convertToArray(array) is int[])
        //        return new Swapper(array.int_swapper);

        //    if (Imports.convertToArray(array) is object[])
        //    {
        //        return new Swapper(array.obj_swapper);
        //    }

        //    return new Swapper(array.slow_swapper);
        //}

        //void int_swapper(int i, int j)
        //{
        //    int[] array = Imports.convertToArray(this) as int[];
        //    int val = array[i];
        //    array[i] = array[j];
        //    array[j] = val;
        //}

        //void obj_swapper(int i, int j)
        //{
        //    object[] array = Imports.convertToArray(this) as object[];
        //    object val = array[i];
        //    array[i] = array[j];
        //    array[j] = val;
        //}

        //void slow_swapper(int i, int j)
        //{
        //    object val = GetValueImpl(i);
        //    SetValueImpl(GetValue(j), i);
        //    SetValueImpl(val, j);
        //}

    }
}