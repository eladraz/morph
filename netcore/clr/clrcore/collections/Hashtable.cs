//
// System.Collections.Hashtable.cs
//
// Author:
//   Sergey Chaban (serge@wildwestsoftware.com)
//

//
// Copyright (C) 2004-2005 Novell, Inc (http://www.novell.com)
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


namespace Morph.Collections
{

    public class Hashtable : IDictionary, ICollection, IEnumerable
    {
        internal struct Slot
        {
            internal System.Object key;
            internal System.Object value;
        }

        internal class KeyMarker : System.Object
        {
            public readonly static KeyMarker Removed = new KeyMarker();

            public bool Equals(System.Object obj)
            {
                if (obj is KeyMarker)
                {
                    return (this == ((KeyMarker)obj));
                }
                return false;
            }
        }

        //
        // Private data
        //

        const int CHAIN_MARKER = ~Int32.MaxValue;


        private Slot[] table;
        // Hashcodes of the corresponding entries in the slot table. Kept separate to
        // help the GC
        private int[] hashes;
        private HashKeys hashKeys;
        private HashValues hashValues;
        private IHashCodeProvider hcpRef;
        private IComparer comparerRef;
        private IEqualityComparer equalityComparer;

        private int inUse;
        private int modificationCount;
        private int loadFactor;
        private int threshold;

        private static readonly int[] primeTbl;

        //TICKET#27
        //= {
        //    11,
        //    19,
        //    37,
        //    73,
        //    109,
        //    163,
        //    251,
        //    367,
        //    557,
        //    823,
        //    1237,
        //    1861,
        //    2777,
        //    4177,
        //    6247,
        //    9371,
        //    14057,
        //    21089,
        //    31627,
        //    47431,
        //    71143,
        //    106721,
        //    160073,
        //    240101,
        //    360163,
        //    540217,
        //    810343,
        //    1215497,
        //    1823231,
        //    2734867,
        //    4102283,
        //    6153409,
        //    9230113,
        //    13845163
        //};

        //
        // Constructors
        //

        //Static constructor
        static Hashtable()
        {
            int index = 0;
            primeTbl = new int[34];
            primeTbl[index++] = 11;
            primeTbl[index++] = 19;
            primeTbl[index++] = 37;
            primeTbl[index++] = 73;
            primeTbl[index++] = 109;
            primeTbl[index++] = 163;
            primeTbl[index++] = 251;
            primeTbl[index++] = 367;
            primeTbl[index++] = 557;
            primeTbl[index++] = 823;
            primeTbl[index++] = 1237;
            primeTbl[index++] = 1861;
            primeTbl[index++] = 2777;
            primeTbl[index++] = 4177;
            primeTbl[index++] = 6247;
            primeTbl[index++] = 9371;
            primeTbl[index++] = 14057;
            primeTbl[index++] = 21089;
            primeTbl[index++] = 31627;
            primeTbl[index++] = 47431;
            primeTbl[index++] = 71143;
            primeTbl[index++] = 106721;
            primeTbl[index++] = 160073;
            primeTbl[index++] = 240101;
            primeTbl[index++] = 360163;
            primeTbl[index++] = 540217;
            primeTbl[index++] = 810343;
            primeTbl[index++] = 1215497;
            primeTbl[index++] = 1823231;
            primeTbl[index++] = 2734867;
            primeTbl[index++] = 4102283;
            primeTbl[index++] = 6153409;
            primeTbl[index++] = 9230113;
            primeTbl[index++] = 13845163;
        }

        public Hashtable() : this(0, 1) { }

        //[Obsolete("Please use Hashtable(int, float, IEqualityComparer) instead")]
        public Hashtable(int capacity, int loadFactor, IHashCodeProvider hcp, IComparer comparer)
        {
            if (capacity < 0)
            {
                capacity = 1;
                throw new System.ArgumentOutOfRangeException ("capacity", "negative capacity");
            }
            if (loadFactor < 1 || loadFactor > 10)
            {
                loadFactor = 1;
                throw new System.ArgumentOutOfRangeException("loadFactor", "load factor");
            }

            if (capacity == 0) ++capacity;
            this.loadFactor = ((int)(loadFactor * 3)) / 4;
            int tableSize = capacity / this.loadFactor;
            if (tableSize > Int32.MaxValue)
            {
                throw new System.ArgumentException("Size is too big");
            }

            int size = (int)tableSize;
            size = ToPrime(size);
            this.SetTable(new Slot[size], new int[size]);

            this.hcp = hcp;
            this.comparer = comparer;

            this.inUse = 0;
            this.modificationCount = 0;
        }

        public Hashtable(int capacity, int loadFactor) :
            this(capacity, loadFactor, null, null)
        {
        }

        public Hashtable(int capacity)
            : this(capacity, 1)
        {
        }

        //
        // Used as a faster Clone
        //
        internal Hashtable(Hashtable source)
        {
            inUse = source.inUse;
            loadFactor = source.loadFactor;

            table = (Slot[])source.table.Clone();
            hashes = (int[])source.hashes.Clone();
            threshold = source.threshold;
            hcpRef = source.hcpRef;
            comparerRef = source.comparerRef;
            equalityComparer = source.equalityComparer;
        }

        //[Obsolete ("Please use Hashtable(int, IEqualityComparer) instead")]
        public Hashtable(int capacity,
                          IHashCodeProvider hcp,
                          IComparer comparer)
            : this(capacity, 1, hcp, comparer)
        {
        }

        //[Obsolete ("Please use Hashtable(IDictionary, float, IEqualityComparer) instead")]
        public Hashtable(IDictionary d, int loadFactor,
                          IHashCodeProvider hcp, IComparer comparer)
            : this(d != null ? d.Count : 0,
                        loadFactor, hcp, comparer)
        {

            if (d == null)
            {
                throw new System.ArgumentNullException("dictionary");
            }

            IDictionaryEnumerator it = d.GetEnumerator();
            while (it.MoveNext())
            {
                Add(it.Key, it.Value);
            }

        }

        public Hashtable(IDictionary d, int loadFactor)
            : this(d, loadFactor, null, null)
        {
        }


        public Hashtable(IDictionary d)
            : this(d, 1)
        {
        }

        //[Obsolete ("Please use Hashtable(IDictionary, IEqualityComparer) instead")]
        public Hashtable(IDictionary d, IHashCodeProvider hcp, IComparer comparer)
            : this(d, 1, hcp, comparer)
        {
        }

        //[Obsolete ("Please use Hashtable(IEqualityComparer) instead")]
        public Hashtable(IHashCodeProvider hcp, IComparer comparer)
            : this(1, 1, hcp, comparer)
        {
        }

        //protected Hashtable (SerializationInfo info, StreamingContext context)
        //{
        //    serializationInfo = info;
        //}

        public Hashtable(IDictionary d, IEqualityComparer equalityComparer)
            : this(d, 1, equalityComparer)
        {
        }

        public Hashtable(IDictionary d, int loadFactor, IEqualityComparer equalityComparer)
            : this(d, loadFactor)
        {
            this.equalityComparer = equalityComparer;
        }

        public Hashtable(IEqualityComparer equalityComparer)
            : this(1, 1, equalityComparer)
        {
        }

        public Hashtable(int capacity, IEqualityComparer equalityComparer)
            : this(capacity, 1, equalityComparer)
        {
        }

        public Hashtable(int capacity, int loadFactor, IEqualityComparer equalityComparer)
            : this(capacity, loadFactor)
        {
            this.equalityComparer = equalityComparer;
        }

        //
        // Properties
        //

        //[Obsolete ("Please use EqualityComparer property.")]
        protected IComparer comparer
        {
            set
            {
                comparerRef = value;
            }
            get
            {
                return comparerRef;
            }
        }

        //[Obsolete ("Please use EqualityComparer property.")]
        protected IHashCodeProvider hcp
        {
            set
            {
                hcpRef = value;
            }
            get
            {
                return hcpRef;
            }
        }

        protected IEqualityComparer EqualityComparer
        {
            get
            {
                return equalityComparer;
            }
        }

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

        public virtual System.Object SyncRoot
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

        public virtual ICollection Keys
        {
            get
            {
                if (this.hashKeys == null)
                    this.hashKeys = new HashKeys(this);
                return this.hashKeys;
            }
        }

        public virtual ICollection Values
        {
            get
            {
                if (this.hashValues == null)
                    this.hashValues = new HashValues(this);
                return this.hashValues;
            }
        }

        public virtual System.Object this[System.Object key]
        {
            get
            {
                if (key == null)
                {
                    throw new System.ArgumentNullException("key", "null key");
                }

                Slot[] table = this.table;
                int[] hashes = this.hashes;
                uint size = (uint)table.Length;
                int h = this.GetHash(key) & Int32.MaxValue;
                uint indx = (uint)h;
                uint step = (uint)((h >> 5) + 1) % (size - 1) + 1;


                for (uint i = size; i > 0; i--)
                {
                    indx %= size;
                    Slot entry = table[indx];
                    int hashMix = hashes[indx];
                    System.Object k = entry.key;
                    if (k == null)
                        break;

                    if (k == key || ((hashMix & Int32.MaxValue) == h
                        && this.KeyEquals(key, k)))
                    {
                        return entry.value;
                    }

                    if ((hashMix & CHAIN_MARKER) == 0)
                        break;

                    indx += step;
                }

                return null;
            }

            set
            {
                PutImpl(key, value, true);
            }
        }

        //
        // Interface methods
        //


        // IEnumerable
        IEnumerator IEnumerable.GetEnumerator()
        {
            return new Enumerator(this, EnumeratorMode.ENTRY_MODE);
        }


        // ICollection
        public virtual void CopyTo(System.Array array, int arrayIndex)
        {
            if (null == array)
            {
                throw new System.ArgumentNullException("array");
            }

            if (arrayIndex < 0)
            {
                throw new System.ArgumentOutOfRangeException("arrayIndex");
            }
            if (array.Rank > 1)
            {
                throw new System.ArgumentException("array is multidimensional");
            }

            if ((array.Length > 0) && (arrayIndex >= array.Length))
            {
                throw new System.ArgumentException("arrayIndex is equal to or greater than array.Length");
            }

            if (arrayIndex + this.inUse > array.Length)
            {
                throw new System.ArgumentException("Not enough room from arrayIndex to end of array for this Hashtable");
            }

            IDictionaryEnumerator it = GetEnumerator();
            int i = arrayIndex;

            while (it.MoveNext())
            {
                array.SetValue(it.Entry, i++);
            }
        }


        // IDictionary
        public virtual void Add(System.Object key, System.Object value)
        {
            PutImpl(key, value, false);
        }

        public virtual void Clear()
        {
            for (int i = 0; i < table.Length; i++)
            {
                table[i].key = null;
                table[i].value = null;
                hashes[i] = 0;
            }

            inUse = 0;
            modificationCount++;
        }

        public virtual bool Contains(System.Object key)
        {
            return (Find(key) >= 0);
        }

        public virtual IDictionaryEnumerator GetEnumerator()
        {
            return new Enumerator(this, EnumeratorMode.ENTRY_MODE);
        }

        public virtual void Remove(System.Object key)
        {
            int i = Find(key);
            if (i >= 0)
            {
                Slot[] table = this.table;
                int h = hashes[i];
                h &= CHAIN_MARKER;
                hashes[i] = h;
                table[i].key = (h != 0)
                              ? KeyMarker.Removed
                              : null;
                table[i].value = null;
                --inUse;
                ++modificationCount;
            }
        }

        public virtual bool ContainsKey(System.Object key)
        {
            return Contains(key);
        }

        public virtual bool ContainsValue(System.Object value)
        {
            int size = this.table.Length;
            Slot[] table = this.table;
            if (value == null)
            {
                for (int i = 0; i < size; i++)
                {
                    Slot entry = table[i];
                    if (entry.key != null && (!entry.key.Equals(KeyMarker.Removed))
                        && entry.value == null)
                    {
                        return true;
                    }
                }
            }
            else
            {
                for (int i = 0; i < size; i++)
                {
                    Slot entry = table[i];
                    if (entry.key != null && (!entry.key.Equals(KeyMarker.Removed))
                        && value.Equals(entry.value))
                    {
                        return true;
                    }
                }
            }
            return false;
        }


        // ICloneable

        public virtual object Clone()
        {
            return new Hashtable(this);
        }

        //Serialization:


        //        public virtual void GetObjectData (SerializationInfo info, StreamingContext context)
        //        {
        //            if (info == null)
        //                throw new ArgumentNullException ("info");

        //            info.AddValue ("LoadFactor", loadFactor);
        //            info.AddValue ("Version", modificationCount);
        //            if (equalityComparer != null)
        //                info.AddValue ("KeyComparer", equalityComparer);
        //            else
        //                info.AddValue ("Comparer", comparerRef);
        //            if (hcpRef != null)
        //                info.AddValue ("HashCodeProvider", hcpRef);
        //            info.AddValue ("HashSize", this.table.Length);
        //// Create Keys
        //                        Object [] keys = new Object[inUse];
        //            CopyToArray(keys, 0, EnumeratorMode.KEY_MODE);

        //// Create Values
        //                        Object [] values = new Object[inUse];
        //            CopyToArray(values, 0, EnumeratorMode.VALUE_MODE);

        //            info.AddValue ("Keys", keys);
        //            info.AddValue ("Values", values);

        //            info.AddValue ("equalityComparer", equalityComparer);
        //        }

        //        public virtual void OnDeserialization (object sender)
        //        {
        //            if (serializationInfo == null) return;

        //            loadFactor = (float) serializationInfo.GetValue ("LoadFactor", typeof(float));
        //            modificationCount = (int) serializationInfo.GetValue ("Version", typeof(int));
        //            try {
        //                equalityComparer = (IEqualityComparer) serializationInfo.GetValue ("KeyComparer", typeof (object));
        //            } catch {
        //                // If not found, try to get "Comparer"
        //            }

        //            if (equalityComparer == null)
        //                comparerRef = (IComparer) serializationInfo.GetValue ("Comparer", typeof (object));
        //            try {
        //                hcpRef = (IHashCodeProvider) serializationInfo.GetValue ("HashCodeProvider", typeof (object));
        //            } catch {} // Missing value => null
        //            int size = (int) serializationInfo.GetValue ("HashSize", typeof(int));

        //            Object [] keys = (Object []) serializationInfo.GetValue("Keys", typeof(Object [] ));
        //            Object [] values = (Object []) serializationInfo.GetValue("Values", typeof(Object [] ));

        //            if (keys.Length != values.Length)
        //              throw new SerializationException("Keys and values of uneven size");

        //            size = ToPrime (size);
        //            this.SetTable (new Slot [size], new int [size]);

        //            for(int i=0;i<keys.Length;i++)
        //                Add(keys[i], values[i]);

        //            AdjustThreshold();

        //            serializationInfo = null;
        //        }

        /// <summary>
        ///  Returns a synchronized (thread-safe)
        ///  wrapper for the Hashtable.
        /// </summary>
        public static Hashtable Synchronized(Hashtable table)
        {
            if (table == null)
            {
                throw new System.ArgumentNullException("table");
            }
            return new SyncHashtable(table);
        }



        //
        // Protected instance methods
        //

        /// <summary>Returns the hash code for the specified key.</summary>
        protected virtual int GetHash(System.Object key)
        {
            if (equalityComparer != null)
                return equalityComparer.GetHashCode(key);
            if (hcpRef == null)
                return key.GetHashCode();

            return hcpRef.GetHashCode(key);
        }

        /// <summary>
        ///  Compares a specific Object with a specific key
        ///  in the Hashtable.
        /// </summary>
        protected virtual bool KeyEquals(System.Object item, System.Object key)
        {
            if (key == KeyMarker.Removed)
                return false;
            if (equalityComparer != null)
                return equalityComparer.Equals(item, key);
            if (comparerRef == null)
                return item.Equals(key);

            return comparerRef.Compare(item, key) == 0;
        }



        //
        // Private instance methods
        //

        private void AdjustThreshold()
        {
            int size = table.Length;

            threshold = (int)(size * loadFactor);
            if (this.threshold >= size)
                threshold = size - 1;
        }

        private void SetTable(Slot[] table, int[] hashes)
        {
            if (table == null)
            {
                throw new System.ArgumentNullException ("table");
            }

            this.table = table;
            this.hashes = hashes;
            AdjustThreshold();
        }

        private int Find(System.Object key)
        {
            if (key == null)
            {
                throw new System.ArgumentNullException("key", "null key");
            }

            Slot[] table = this.table;
            int[] hashes = this.hashes;
            uint size = (uint)table.Length;
            int h = this.GetHash(key) & Int32.MaxValue;
            uint indx = (uint)h;
            uint step = (uint)((h >> 5) + 1) % (size - 1) + 1;


            for (uint i = size; i > 0; i--)
            {
                indx %= size;
                Slot entry = table[indx];
                int hashMix = hashes[indx];
                System.Object k = entry.key;
                if (k == null)
                    break;

                if (k == key || ((hashMix & Int32.MaxValue) == h
                    && this.KeyEquals(key, k)))
                {
                    return (int)indx;
                }

                if ((hashMix & CHAIN_MARKER) == 0)
                    break;

                indx += step;
            }
            return -1;
        }


        private void Rehash()
        {
            int oldSize = this.table.Length;

            // From the SDK docs:
            //   Hashtable is automatically increased
            //   to the smallest prime number that is larger
            //   than twice the current number of Hashtable buckets
            uint newSize = (uint)ToPrime((oldSize << 1) | 1);


            Slot[] newTable = new Slot[newSize];
            Slot[] table = this.table;
            int[] newHashes = new int[newSize];
            int[] hashes = this.hashes;

            for (int i = 0; i < oldSize; i++)
            {
                Slot s = table[i];
                if (s.key != null)
                {
                    int h = hashes[i] & Int32.MaxValue;
                    uint spot = (uint)h;
                    uint step = ((uint)(h >> 5) + 1) % (newSize - 1) + 1;
                    for (uint j = spot % newSize; ; spot += step, j = spot % newSize)
                    {
                        // No check for KeyMarker.Removed here,
                        // because the table is just allocated.
                        if (newTable[j].key == null)
                        {
                            newTable[j].key = s.key;
                            newTable[j].value = s.value;
                            newHashes[j] |= h;
                            break;
                        }
                        else
                        {
                            newHashes[j] |= CHAIN_MARKER;
                        }
                    }
                }
            }

            ++this.modificationCount;

            this.SetTable(newTable, newHashes);
        }


        private void PutImpl(System.Object key, System.Object value, bool overwrite)
        {
            if (key == null)
            {
                throw new System.ArgumentNullException("key", "null key");
            }

            if (this.inUse >= this.threshold)
                this.Rehash();

            uint size = (uint)this.table.Length;

            int h = this.GetHash(key) & Int32.MaxValue;
            uint spot = (uint)h;
            uint step = (uint)((spot >> 5) + 1) % (size - 1) + 1;
            Slot[] table = this.table;
            int[] hashes = this.hashes;
            Slot entry;

            int freeIndx = -1;
            for (int i = 0; i < size; i++)
            {
                int indx = (int)(spot % size);
                entry = table[indx];
                int hashMix = hashes[indx];

                if (freeIndx == -1
                    && KeyMarker.Removed.Equals(entry.key)
                    && (hashMix & CHAIN_MARKER) != 0)
                    freeIndx = indx;

                if (entry.key == null ||
                    (KeyMarker.Removed.Equals(entry.key)
                     && (hashMix & CHAIN_MARKER) == 0))
                {

                    if (freeIndx == -1)
                        freeIndx = indx;
                    break;
                }

                if ((hashMix & Int32.MaxValue) == h && KeyEquals(key, entry.key))
                {
                    if (overwrite)
                    {
                        table[indx].value = value;
                        ++this.modificationCount;
                    }
                    else
                    {
                        // Handle Add ():
                        // An entry with the same key already exists in the Hashtable.

                        throw new System.ArgumentException ("Key duplication when adding: " + key);
                    }
                    return;
                }

                if (freeIndx == -1)
                {
                    hashes[indx] |= CHAIN_MARKER;
                }

                spot += step;

            }

            if (freeIndx != -1)
            {
                table[freeIndx].key = key;
                table[freeIndx].value = value;
                hashes[freeIndx] |= h;

                ++this.inUse;
                ++this.modificationCount;
            }

        }

        private void CopyToArray(System.Array arr, int i,
                       EnumeratorMode mode)
        {
            IEnumerator it = new Enumerator(this, mode);

            while (it.MoveNext())
            {
                arr.SetValue(it.Current, i++);
            }
        }



        //
        // Private static methods
        //
        internal static bool TestPrime(int x)
        {
            if ((x & 1) != 0)
            {
                int top = (int)System.Math.Sqrt(x);

                for (int n = 3; n < top; n += 2)
                {
                    if ((x % n) == 0)
                        return false;
                }
                return true;
            }
            // There is only one even prime - 2.
            return (x == 2);
        }

        internal static int CalcPrime(int x)
        {
            for (int i = (x & (~1)) - 1; i < Int32.MaxValue; i += 2)
            {
                if (TestPrime(i)) return i;
            }
            return x;
        }

        internal static int ToPrime(int x)
        {
            for (int i = 0; i < primeTbl.Length; i++)
            {
                if (x <= primeTbl[i])
                    return primeTbl[i];
            }
            return CalcPrime(x);
        }







        //
        // Inner classes
        //

        private enum EnumeratorMode : int { KEY_MODE = 0, VALUE_MODE, ENTRY_MODE };

        private sealed class Enumerator : IDictionaryEnumerator, IEnumerator
        {

            private Hashtable host;
            private System.Object currentKey;
            private System.Object currentValue;

            private int stamp;
            private int pos;
            private int size;
            private EnumeratorMode mode;

            private readonly static string xstr = "Hashtable.Enumerator: snapshot out of sync.";

            public Enumerator(Hashtable host, EnumeratorMode mode)
            {
                this.host = host;
                stamp = host.modificationCount;
                size = host.table.Length;
                this.mode = mode;
                Reset();
            }

            private void FailFast()
            {
                if (host.modificationCount != stamp)
                {
                    throw new System.InvalidOperationException(xstr);
                }
            }

            public void Reset()
            {
                FailFast();

                pos = -1;
                currentKey = null;
                currentValue = null;
            }

            public bool MoveNext()
            {
                FailFast();

                if (pos < size)
                {
                    while (++pos < size)
                    {
                        Slot entry = host.table[pos];

                        if (entry.key != null && (!entry.key.Equals(KeyMarker.Removed)))
                        {
                            currentKey = entry.key;
                            currentValue = entry.value;
                            return true;
                        }
                    }
                }

                currentKey = null;
                currentValue = null;
                return false;
            }

            public DictionaryEntry Entry
            {
                get
                {
                    if (currentKey == null)
                    {
                        throw new System.InvalidOperationException();
                    }
                    FailFast();
                    return new DictionaryEntry(currentKey, currentValue);
                }
            }

            public System.Object Key
            {
                get
                {
                    if (currentKey == null)
                    {
                        throw new System.InvalidOperationException();
                    }
                    FailFast();
                    return currentKey;
                }
            }

            public System.Object Value
            {
                get
                {
                    if (currentKey == null)
                    {
                        throw new System.InvalidOperationException();
                    }
                    FailFast();
                    return currentValue;
                }
            }

            public System.Object Current
            {
                get
                {
                    if (currentKey == null)
                    {
                        throw new System.InvalidOperationException();
                    }
                    if (mode == EnumeratorMode.KEY_MODE)
                    {
                        return currentKey;
                    }
                    else if (mode == EnumeratorMode.VALUE_MODE)
                    {
                        return currentValue;
                    }
                    else if (mode == EnumeratorMode.ENTRY_MODE)
                    {
                        return new DictionaryEntry(currentKey, currentValue);
                    }

                    //switch (mode)
                    //{
                    //    case EnumeratorMode.KEY_MODE:
                    //        return currentKey;
                    //    case EnumeratorMode.VALUE_MODE:
                    //        return currentValue;
                    //    case EnumeratorMode.ENTRY_MODE:
                    //        return new DictionaryEntry(currentKey, currentValue);
                    //}
                    return null;
                    //throw new Exception ("should never happen");
                }
            }
        }


        private class HashKeys : ICollection, IEnumerable
        {

            private Hashtable host;

            public HashKeys(Hashtable host)
            {
                if (host == null)
                {
                    throw new System.ArgumentNullException();
                }

                this.host = host;
            }

            // ICollection

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

            public virtual System.Object SyncRoot
            {
                get { return host.SyncRoot; }
            }

            public virtual void CopyTo(System.Array array, int arrayIndex)
            {
                if (array == null)
                {
                    throw new System.ArgumentNullException("array");
                }
                if (array.Rank != 1)
                {
                    throw new System.ArgumentException("array");
                }
                if (arrayIndex < 0)
                {
                    throw new System.ArgumentOutOfRangeException("arrayIndex");
                }
                if (array.Length - arrayIndex < Count)
                {
                    throw new System.ArgumentException("not enough space");
                }

                host.CopyToArray(array, arrayIndex, EnumeratorMode.KEY_MODE);
            }

            // IEnumerable

            public virtual IEnumerator GetEnumerator()
            {
                return new Hashtable.Enumerator(host, EnumeratorMode.KEY_MODE);
            }
        }

        private class HashValues : ICollection, IEnumerable
        {

            private Hashtable host;

            public HashValues(Hashtable host)
            {
                if (host == null)
                {
                    throw new System.ArgumentNullException();
                }

                this.host = host;
            }

            // ICollection

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

            public virtual System.Object SyncRoot
            {
                get
                {
                    return host.SyncRoot;
                }
            }

            public virtual void CopyTo(System.Array array, int arrayIndex)
            {
                if (array == null)
                {
                    throw new System.ArgumentNullException ("array");
                }
                if (array.Rank != 1)
                {
                    throw new System.ArgumentException("array");
                }
                if (arrayIndex < 0)
                {
                    throw new System.ArgumentOutOfRangeException ("arrayIndex");
                }
                if (array.Length - arrayIndex < Count)
                {
                    throw new System.ArgumentException("not enough space");
                }

                host.CopyToArray(array, arrayIndex, EnumeratorMode.VALUE_MODE);
            }

            // IEnumerable

            public virtual IEnumerator GetEnumerator()
            {
                return new Hashtable.Enumerator(host, EnumeratorMode.VALUE_MODE);
            }
        }


        private class SyncHashtable : Hashtable, IEnumerable
        {

            private Hashtable host;

            public SyncHashtable(Hashtable host)
            {
                if (host == null)
                {
                    throw new System.ArgumentNullException();
                }

                this.host = host;
            }

            //internal SyncHashtable (SerializationInfo info, StreamingContext context)
            //{
            //    host = (Hashtable) info.GetValue("ParentTable", typeof(Hashtable));
            //}

            //public override void GetObjectData (SerializationInfo info, StreamingContext context)
            //{
            //    info.AddValue ("ParentTable", host);
            //}

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

            public override System.Object SyncRoot
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

            public override ICollection Keys
            {
                get
                {
                    ICollection keys = null;
                    lock (host.SyncRoot)
                    {
                        keys = host.Keys;
                    }
                    return keys;
                }
            }

            public override ICollection Values
            {
                get
                {
                    ICollection vals = null;
                    lock (host.SyncRoot)
                    {
                        vals = host.Values;
                    }
                    return vals;
                }
            }



            public override System.Object this[System.Object key]
            {
                get
                {
                    return host[key];
                }
                set
                {
                    lock (host.SyncRoot)
                    {
                        host[key] = value;
                    }
                }
            }

            // IEnumerable

            IEnumerator IEnumerable.GetEnumerator()
            {
                return new Enumerator(host, EnumeratorMode.ENTRY_MODE);
            }




            // ICollection

            public override void CopyTo(System.Array array, int arrayIndex)
            {
                host.CopyTo(array, arrayIndex);
            }


            // IDictionary

            public override void Add(System.Object key, System.Object value)
            {
                lock (host.SyncRoot)
                {
                    host.Add(key, value);
                }
            }

            public override void Clear()
            {
                lock (host.SyncRoot)
                {
                    host.Clear();
                }
            }

            public override bool Contains(System.Object key)
            {
                return (host.Find(key) >= 0);
            }

            public override IDictionaryEnumerator GetEnumerator()
            {
                return new Enumerator(host, EnumeratorMode.ENTRY_MODE);
            }

            public override void Remove(System.Object key)
            {
                lock (host.SyncRoot)
                {
                    host.Remove(key);
                }
            }



            public override bool ContainsKey(System.Object key)
            {
                return host.Contains(key);
            }

            public override bool ContainsValue(System.Object value)
            {
                return host.ContainsValue(value);
            }


            // ICloneable

            public override object Clone()
            {
                lock (host.SyncRoot)
                {
                    return new SyncHashtable((Hashtable)host.Clone());
                }
            }

        } // SyncHashtable


    } // Hashtable

}
