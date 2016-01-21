//Added SortedList.cs - System.Collections.SortedList implementation.

namespace Morph.Collections
{

    /// <summary>
    ///  Represents a collection of associated keys and values
    ///  that are sorted by the keys and are accessible by key
    ///  and by index.
    /// </summary>
    public class SortedList : System.Collections.IDictionary, System.Collections.ICollection,
                              System.Collections.IEnumerable, System.ICloneable
    {


        internal struct Slot
        {
            internal object key;
            internal object value;
        }

        const int INITIAL_SIZE = 16;

        private enum EnumeratorMode : int { KEY_MODE = 0, VALUE_MODE, ENTRY_MODE }

        private int inUse;
        private int modificationCount;
        private Slot[] table;
        private System.Collections.IComparer comparer;
        private int defaultCapacity;

        //
        // Constructors
        //
        public SortedList()
            : this(null, INITIAL_SIZE)
        {
        }

        public SortedList(int initialCapacity)
            : this(null, initialCapacity)
        {
        }

        public SortedList(System.Collections.IComparer comparer, int capacity)
        {
            if (capacity < 0)
                throw new System.ArgumentOutOfRangeException("capacity");

            if (capacity == 0)
                defaultCapacity = 0;
            else
                defaultCapacity = INITIAL_SIZE;

            this.comparer = comparer;
            InitTable(capacity, true);
        }

        public SortedList(System.Collections.IComparer comparer)
        {
            this.comparer = comparer;
            InitTable(INITIAL_SIZE, true);
        }


        public SortedList(System.Collections.IDictionary d)
            : this(d, null)
        {
        }

        // LAMESPEC: MSDN docs talk about an InvalidCastException but
        // I wasn't able to duplicate such a case in the unit tests.
        public SortedList(System.Collections.IDictionary d, System.Collections.IComparer comparer)
        {
            if (d == null)
                throw new System.ArgumentNullException("dictionary");

            InitTable(d.Count, true);
            this.comparer = comparer;

            System.Collections.IDictionaryEnumerator it = d.GetEnumerator();
            while (it.MoveNext())
            {
                Add(it.Key, it.Value);
            }
        }

        //
        // Properties
        //

        // ICollection

        public virtual int Count
        {
            get
            {
                return inUse;
            }
        }

        public virtual bool IsSynchronized
        {
            get
            {
                return false;
            }
        }

        public virtual object SyncRoot
        {
            get
            {
                return this;
            }
        }


        // IDictionary

        public virtual bool IsFixedSize
        {
            get
            {
                return false;
            }
        }


        public virtual bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        public virtual System.Collections.ICollection Keys
        {
            get
            {
                return new ListKeys(this);
            }
        }

        public virtual System.Collections.ICollection Values
        {
            get
            {
                return new ListValues(this);
            }
        }



        public virtual object this[object key]
        {
            get
            {
                if (key == null)
                    throw new System.ArgumentNullException();
                return GetImpl(key);
            }
            set
            {
                if (key == null)
                    throw new System.ArgumentNullException();
                if (IsReadOnly)
                    throw new System.NotSupportedException("SortedList is Read Only.");
                if (Find(key) < 0 && IsFixedSize)
                    throw new System.NotSupportedException("Key not found and SortedList is fixed size.");

                PutImpl(key, value, true);
            }
        }

        public virtual int Capacity
        {
            get
            {
                return table.Length;
            }

            set
            {
                int current = this.table.Length;

                if (inUse > value)
                {
                    throw new System.ArgumentOutOfRangeException("capacity too small");
                }
                else if (value == 0)
                {
                    // return to default size
                    Slot[] newTable = new Slot[defaultCapacity];
                    System.Array.Copy(table, newTable, inUse);
                    this.table = newTable;
                }
                else if (value > inUse)
                {
                    Slot[] newTable = new Slot[value];
                    System.Array.Copy(table, newTable, inUse);
                    this.table = newTable;
                }
                else if (value > current)
                {
                    Slot[] newTable = new Slot[value];
                    System.Array.Copy(table, newTable, current);
                    this.table = newTable;
                }
            }
        }

        //
        // Public instance methods.
        //

        // IEnumerable

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return new Enumerator(this, EnumeratorMode.ENTRY_MODE);
        }


        // IDictionary

        public virtual void Add(object key, object value)
        {
            PutImpl(key, value, false);
        }


        public virtual void Clear()
        {
            defaultCapacity = INITIAL_SIZE;
            this.table = new Slot[defaultCapacity];
            inUse = 0;
            modificationCount++;
        }

        public virtual bool Contains(object key)
        {
            if (null == key)
                throw new System.ArgumentNullException();

            try
            {
                return (Find(key) >= 0);
            }
            catch (System.Exception)
            {
                throw new System.InvalidOperationException();
            }
        }


        public virtual System.Collections.IDictionaryEnumerator GetEnumerator()
        {
            return new Enumerator(this, EnumeratorMode.ENTRY_MODE);
        }

        public virtual void Remove(object key)
        {
            int i = IndexOfKey(key);
            if (i >= 0) RemoveAt(i);
        }


        // ICollection

        public virtual void CopyTo(System.Array array, int arrayIndex)
        {
            if (null == array)
                throw new System.ArgumentNullException();

            if (arrayIndex < 0)
                throw new System.ArgumentOutOfRangeException();

            if (array.Rank > 1)
                throw new System.ArgumentException("array is multi-dimensional");
            if (arrayIndex >= array.Length)
                throw new System.ArgumentNullException("arrayIndex is greater than or equal to array.Length");
            if (Count > (array.Length - arrayIndex))
                throw new System.ArgumentNullException("Not enough space in array from arrayIndex to end of array");

            System.Collections.IDictionaryEnumerator it = GetEnumerator();
            int i = arrayIndex;

            while (it.MoveNext())
            {
                array.SetValue(it.Entry, i++);
            }
        }



        // ICloneable

        public virtual object Clone()
        {
            SortedList sl = new SortedList(this, comparer);
            sl.modificationCount = this.modificationCount;
            return sl;
        }




        //
        // SortedList
        //

        public virtual System.Collections.IList GetKeyList()
        {
            return new ListKeys(this);
        }


        public virtual System.Collections.IList GetValueList()
        {
            return new ListValues(this);
        }


        public virtual void RemoveAt(int index)
        {
            Slot[] table = this.table;
            int cnt = Count;
            if (index >= 0 && index < cnt)
            {
                if (index != cnt - 1)
                {
                    System.Array.Copy(table, index + 1, table, index, cnt - 1 - index);
                }
                else
                {
                    table[index].key = null;
                    table[index].value = null;
                }
                --inUse;
                ++modificationCount;
            }
            else
            {
                throw new System.ArgumentOutOfRangeException("index out of range");
            }
        }

        public virtual int IndexOfKey(object key)
        {
            if (null == key)
                throw new System.ArgumentNullException();

            int indx = 0;
            try
            {
                indx = Find(key);
            }
            catch (System.Exception)
            {
                throw new System.InvalidOperationException();
            }

            return (indx | (indx >> 31));
        }


        public virtual int IndexOfValue(object value)
        {
            if (inUse == 0)
                return -1;

            for (int i = 0; i < inUse; i++)
            {
                Slot current = this.table[i];

                if (Equals(value, current.value))
                    return i;
            }

            return -1;
        }


        public virtual bool ContainsKey(object key)
        {
            if (null == key)
                throw new System.ArgumentNullException();

            try
            {
                return Contains(key);
            }
            catch (System.Exception)
            {
                throw new System.InvalidOperationException();
            }
        }


        public virtual bool ContainsValue(object value)
        {
            return IndexOfValue(value) >= 0;
        }


        public virtual object GetByIndex(int index)
        {
            if (index >= 0 && index < Count)
                return table[index].value;

            else
                throw new System.ArgumentOutOfRangeException("index out of range");
        }


        public virtual void SetByIndex(int index, object value)
        {
            if (index >= 0 && index < Count)
                table[index].value = value;

            else
                throw new System.ArgumentOutOfRangeException("index out of range");
        }


        public virtual object GetKey(int index)
        {
            if (index >= 0 && index < Count)
                return table[index].key;

            else
                throw new System.ArgumentOutOfRangeException("index out of range");
        }

        public static SortedList Synchronized(SortedList list)
        {
            if (list == null)
                throw new System.ArgumentNullException("Base list is null.");

            return new SynchedSortedList(list);
        }

        public virtual void TrimToSize()
        {
            // From Beta2:
            // Trimming an empty SortedList sets the capacity
            // of the SortedList to the default capacity,
            // not zero.
            if (Count == 0)
                Resize(defaultCapacity, false);
            else
                Resize(Count, true);
        }


        //
        // Private methods
        //


        private void Resize(int n, bool copy)
        {
            Slot[] table = this.table;
            Slot[] newTable = new Slot[n];
            if (copy) System.Array.Copy(table, 0, newTable, 0, n);
            this.table = newTable;
        }


        private void EnsureCapacity(int n, int free)
        {
            Slot[] table = this.table;
            Slot[] newTable = null;
            int cap = Capacity;
            bool gap = (free >= 0 && free < Count);

            if (n > cap)
            {
                newTable = new Slot[n << 1];
            }

            if (newTable != null)
            {
                if (gap)
                {
                    int copyLen = free;
                    if (copyLen > 0)
                    {
                        System.Array.Copy(table, 0, newTable, 0, copyLen);
                    }
                    copyLen = Count - free;
                    if (copyLen > 0)
                    {
                        System.Array.Copy(table, free, newTable, free + 1, copyLen);
                    }
                }
                else
                {
                    // Just a resizing, copy the entire table.
                    System.Array.Copy(table, newTable, Count);
                }
                this.table = newTable;
            }
            else if (gap)
            {
                System.Array.Copy(table, free, table, free + 1, Count - free);
            }
        }


        private void PutImpl(object key, object value, bool overwrite)
        {
            if (key == null)
                throw new System.ArgumentNullException("null key");

            Slot[] table = this.table;

            int freeIndx = -1;

            try
            {
                freeIndx = Find(key);
            }
            catch (System.Exception)
            {
                throw new System.InvalidOperationException();
            }

            if (freeIndx >= 0)
            {
                if (!overwrite)
                {
                    throw new System.ArgumentException("Key '" + key.ToString() + "' already exists in list.");
                }

                table[freeIndx].value = value;
                ++modificationCount;
                return;
            }

            freeIndx = ~freeIndx;

            if (freeIndx > Capacity + 1)
                throw new System.Exception("SortedList::internal error (" + key.ToString() + ", " + value.ToString() + ") at [" + freeIndx.ToString() + "]");


            EnsureCapacity(Count + 1, freeIndx);

            table = this.table;
            table[freeIndx].key = key;
            table[freeIndx].value = value;

            ++inUse;
            ++modificationCount;

        }


        private object GetImpl(object key)
        {
            int i = Find(key);

            if (i >= 0)
                return table[i].value;
            else
                return null;
        }

        private void InitTable(int capacity, bool forceSize)
        {
            if (!forceSize && (capacity < defaultCapacity))
                capacity = defaultCapacity;
            this.table = new Slot[capacity];
            this.inUse = 0;
            this.modificationCount = 0;
        }

        private void CopyToArray(System.Array arr, int i,
                       EnumeratorMode mode)
        {
            if (arr == null)
                throw new System.ArgumentNullException("arr");

            if (i < 0 || i + this.Count > arr.Length)
                throw new System.ArgumentOutOfRangeException("i");

            System.Collections.IEnumerator it = new Enumerator(this, mode);

            while (it.MoveNext())
            {
                arr.SetValue(it.Current, i++);
            }
        }


        private int Find(object key)
        {
            Slot[] table = this.table;
            int len = Count;

            if (len == 0) return ~0;

            System.Collections.IComparer comparer = (this.comparer == null)? System.Collections.Comparer.Default : this.comparer;

            int left = 0;
            int right = len - 1;

            while (left <= right)
            {
                int guess = (left + right) >> 1;

                int cmp = comparer.Compare(table[guess].key, key);
                if (cmp == 0) return guess;

                if (cmp < 0) left = guess + 1;
                else right = guess - 1;
            }

            return ~left;
        }



        //
        // Inner classes
        //


        private sealed class Enumerator : System.ICloneable, System.Collections.IDictionaryEnumerator, System.Collections.IEnumerator
        {

            private SortedList host;
            private int stamp;
            private int pos;
            private int size;
            private EnumeratorMode mode;

            private object currentKey;
            private object currentValue;

            bool invalid = false;

            private readonly static string xstr = "SortedList.Enumerator: snapshot out of sync.";

            public Enumerator(SortedList host, EnumeratorMode mode)
            {
                this.host = host;
                stamp = host.modificationCount;
                size = host.Count;
                this.mode = mode;
                Reset();
            }

            public Enumerator(SortedList host)
                : this(host, EnumeratorMode.ENTRY_MODE)
            {
            }

            public void Reset()
            {
                if (host.modificationCount != stamp || invalid)
                    throw new System.InvalidOperationException(xstr);

                pos = -1;
                currentKey = null;
                currentValue = null;
            }

            public bool MoveNext()
            {
                if (host.modificationCount != stamp || invalid)
                    throw new System.InvalidOperationException(xstr);

                Slot[] table = host.table;

                if (++pos < size)
                {
                    Slot entry = table[pos];

                    currentKey = entry.key;
                    currentValue = entry.value;
                    return true;
                }

                currentKey = null;
                currentValue = null;
                return false;
            }

            public System.Collections.DictionaryEntry Entry
            {
                get
                {
                    if (invalid || pos >= size || pos == -1)
                        throw new System.InvalidOperationException(xstr);

                    return new System.Collections.DictionaryEntry(currentKey,
                                                currentValue);
                }
            }

            public object Key
            {
                get
                {
                    if (invalid || pos >= size || pos == -1)
                        throw new System.InvalidOperationException(xstr);
                    return currentKey;
                }
            }

            public object Value
            {
                get
                {
                    if (invalid || pos >= size || pos == -1)
                        throw new System.InvalidOperationException(xstr);
                    return currentValue;
                }
            }

            public object Current
            {
                get
                {
                    if (invalid || pos >= size || pos == -1)
                        throw new System.InvalidOperationException(xstr);

                    switch (mode)
                    {
                        case EnumeratorMode.KEY_MODE:
                            return currentKey;
                        case EnumeratorMode.VALUE_MODE:
                            return currentValue;
                        case EnumeratorMode.ENTRY_MODE:
                            return this.Entry;

                        default:
                            throw new System.NotSupportedException(mode + " is not a supported mode.");
                    }
                }
            }

            // ICloneable

            public object Clone()
            {
                Enumerator e = new Enumerator(host, mode);
                e.stamp = stamp;
                e.pos = pos;
                e.size = size;
                e.currentKey = currentKey;
                e.currentValue = currentValue;
                e.invalid = invalid;
                return e;
            }
        }

        private class ListKeys : System.Collections.IList, System.Collections.IEnumerable
        {

            private SortedList host;


            public ListKeys(SortedList host)
            {
                if (host == null)
                    throw new System.ArgumentNullException();

                this.host = host;
            }

            //
            // ICollection
            //

            public virtual int Count
            {
                get
                {
                    return host.Count;
                }
            }

            public virtual bool IsSynchronized
            {
                get
                {
                    return host.IsSynchronized;
                }
            }

            public virtual object SyncRoot
            {
                get
                {
                    return host.SyncRoot;
                }
            }

            public virtual void CopyTo(System.Array array, int arrayIndex)
            {
                host.CopyToArray(array, arrayIndex, EnumeratorMode.KEY_MODE);
            }


            //
            // IList
            //

            public virtual bool IsFixedSize
            {
                get
                {
                    return true;
                }
            }

            public virtual bool IsReadOnly
            {
                get
                {
                    return true;
                }
            }


            public virtual object this[int index]
            {
                get
                {
                    return host.GetKey(index);
                }
                set
                {
                    throw new System.NotSupportedException("attempt to modify a key");
                }
            }

            public virtual int Add(object value)
            {
                throw new System.NotSupportedException("IList::Add not supported");
            }

            public virtual void Clear()
            {
                throw new System.NotSupportedException("IList::Clear not supported");
            }

            public virtual bool Contains(object key)
            {
                return host.Contains(key);
            }


            public virtual int IndexOf(object key)
            {
                return host.IndexOfKey(key);
            }


            public virtual void Insert(int index, object value)
            {
                throw new System.NotSupportedException("IList::Insert not supported");
            }


            public virtual void Remove(object value)
            {
                throw new System.NotSupportedException("IList::Remove not supported");
            }


            public virtual void RemoveAt(int index)
            {
                throw new System.NotSupportedException("IList::RemoveAt not supported");
            }


            //
            // IEnumerable
            //

            public virtual System.Collections.IEnumerator GetEnumerator()
            {
                return new SortedList.Enumerator(host, EnumeratorMode.KEY_MODE);
            }


        }

        private class ListValues : System.Collections.IList, System.Collections.IEnumerable
        {

            private SortedList host;


            public ListValues(SortedList host)
            {
                if (host == null)
                    throw new System.ArgumentNullException();

                this.host = host;
            }

            //
            // ICollection
            //

            public virtual int Count
            {
                get
                {
                    return host.Count;
                }
            }

            public virtual bool IsSynchronized
            {
                get
                {
                    return host.IsSynchronized;
                }
            }

            public virtual object SyncRoot
            {
                get
                {
                    return host.SyncRoot;
                }
            }

            public virtual void CopyTo(System.Array array, int arrayIndex)
            {
                host.CopyToArray(array, arrayIndex, EnumeratorMode.VALUE_MODE);
            }


            //
            // IList
            //

            public virtual bool IsFixedSize
            {
                get
                {
                    return true;
                }
            }

            public virtual bool IsReadOnly
            {
                get
                {
                    return true;
                }
            }


            public virtual object this[int index]
            {
                get
                {
                    return host.GetByIndex(index);
                }
                set
                {
                    throw new System.NotSupportedException("This operation is not supported on GetValueList return");
                }
            }

            public virtual int Add(object value)
            {
                throw new System.NotSupportedException("IList::Add not supported");
            }

            public virtual void Clear()
            {
                throw new System.NotSupportedException("IList::Clear not supported");
            }

            public virtual bool Contains(object value)
            {
                return host.ContainsValue(value);
            }


            public virtual int IndexOf(object value)
            {
                return host.IndexOfValue(value);
            }


            public virtual void Insert(int index, object value)
            {
                throw new System.NotSupportedException("IList::Insert not supported");
            }


            public virtual void Remove(object value)
            {
                throw new System.NotSupportedException("IList::Remove not supported");
            }


            public virtual void RemoveAt(int index)
            {
                throw new System.NotSupportedException("IList::RemoveAt not supported");
            }


            //
            // IEnumerable
            //

            public virtual System.Collections.IEnumerator GetEnumerator()
            {
                return new SortedList.Enumerator(host, EnumeratorMode.VALUE_MODE);
            }

        }

        private class SynchedSortedList : SortedList
        {

            private SortedList host;

            public SynchedSortedList(SortedList host)
            {
                if (host == null)
                    throw new System.ArgumentNullException();
                this.host = host;
            }

            public override int Capacity
            {
                get
                {
                    lock (host.SyncRoot)
                    {
                        return host.Capacity;
                    }
                }
                set
                {
                    lock (host.SyncRoot)
                    {
                        host.Capacity = value;
                    }
                }
            }

            // ICollection

            public override int Count
            {
                get
                {
                    return host.Count;
                }
            }

            public override bool IsSynchronized
            {
                get
                {
                    return true;
                }
            }

            public override object SyncRoot
            {
                get
                {
                    return host.SyncRoot;
                }
            }



            // IDictionary

            public override bool IsFixedSize
            {
                get
                {
                    return host.IsFixedSize;
                }
            }


            public override bool IsReadOnly
            {
                get
                {
                    return host.IsReadOnly;
                }
            }

            public override System.Collections.ICollection Keys
            {
                get
                {
                    System.Collections.ICollection keys = null;
                    lock (host.SyncRoot)
                    {
                        keys = host.Keys;
                    }
                    return keys;
                }
            }

            public override System.Collections.ICollection Values
            {
                get
                {
                    System.Collections.ICollection vals = null;
                    lock (host.SyncRoot)
                    {
                        vals = host.Values;
                    }
                    return vals;
                }
            }



            public override object this[object key]
            {
                get
                {
                    lock (host.SyncRoot)
                    {
                        return host.GetImpl(key);
                    }
                }
                set
                {
                    lock (host.SyncRoot)
                    {
                        host.PutImpl(key, value, true);
                    }
                }
            }



            // ICollection

            public override void CopyTo(System.Array array, int arrayIndex)
            {
                lock (host.SyncRoot)
                {
                    host.CopyTo(array, arrayIndex);
                }
            }


            // IDictionary

            public override void Add(object key, object value)
            {
                lock (host.SyncRoot)
                {
                    host.PutImpl(key, value, false);
                }
            }

            public override void Clear()
            {
                lock (host.SyncRoot)
                {
                    host.Clear();
                }
            }

            public override bool Contains(object key)
            {
                lock (host.SyncRoot)
                {
                    return (host.Find(key) >= 0);
                }
            }

            public override System.Collections.IDictionaryEnumerator GetEnumerator()
            {
                lock (host.SyncRoot)
                {
                    return host.GetEnumerator();
                }
            }

            public override void Remove(object key)
            {
                lock (host.SyncRoot)
                {
                    host.Remove(key);
                }
            }



            public override bool ContainsKey(object key)
            {
                lock (host.SyncRoot)
                {
                    return host.Contains(key);
                }
            }

            public override bool ContainsValue(object value)
            {
                lock (host.SyncRoot)
                {
                    return host.ContainsValue(value);
                }
            }


            // ICloneable

            public override object Clone()
            {
                lock (host.SyncRoot)
                {
                    return (host.Clone() as SortedList);
                }
            }



            //
            // SortedList overrides
            //

            public override object GetByIndex(int index)
            {
                lock (host.SyncRoot)
                {
                    return host.GetByIndex(index);
                }
            }

            public override object GetKey(int index)
            {
                lock (host.SyncRoot)
                {
                    return host.GetKey(index);
                }
            }

            public override System.Collections.IList GetKeyList()
            {
                lock (host.SyncRoot)
                {
                    return new ListKeys(host);
                }
            }


            public override System.Collections.IList GetValueList()
            {
                lock (host.SyncRoot)
                {
                    return new ListValues(host);
                }
            }

            public override void RemoveAt(int index)
            {
                lock (host.SyncRoot)
                {
                    host.RemoveAt(index);
                }
            }

            public override int IndexOfKey(object key)
            {
                lock (host.SyncRoot)
                {
                    return host.IndexOfKey(key);
                }
            }

            public override int IndexOfValue(object val)
            {
                lock (host.SyncRoot)
                {
                    return host.IndexOfValue(val);
                }
            }

            public override void SetByIndex(int index, object value)
            {
                lock (host.SyncRoot)
                {
                    host.SetByIndex(index, value);
                }
            }

            public override void TrimToSize()
            {
                lock (host.SyncRoot)
                {
                    host.TrimToSize();
                }
            }


        } // SynchedSortedList

    } // SortedList

} // System.Collections
