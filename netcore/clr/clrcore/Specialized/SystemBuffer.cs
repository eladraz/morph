using System;
using System.Collections.Generic;
using System.Text;

namespace Morph.Specialized
{

    /// <summary>
    /// Used for efficient memory buffering activities, with minimal memcopy operations
    /// </summary>
    public unsafe class SystemBuffer
    {
        private byte* m_pdata;
        private int m_totalLength;
        private int m_length;
        private bool m_shouldFree = false;
        private SystemBuffer next = null;
        private SystemBuffer prev = null;

        private ShortIndexer m_shortIndex;
        private UShortIndexer m_ushortIndex;
        private IntIndexer m_intIndex;
        private UIntIndexer m_uintIndex;

        /// <summary>
        /// Represents Endianity type
        /// </summary>
        public enum EndianessEnum
        {
            BigEndian,
            LittleEndian
        }

        /// <summary>
        /// Repersents the Endianity of the buffer, default is BigEndian.
        /// When array is joined to another, it accepts the endianity of the header.
        /// </summary>
        public EndianessEnum Endianess = EndianessEnum.BigEndian;

        public int TotalLength
        {
            get { return m_totalLength; }
        }
        public int PartLength
        {
            get { return m_length; }
        }


        #region constructors
        public SystemBuffer(int length, int size, EndianessEnum Endianness)
        {
            m_totalLength = m_length = length * size;
            Endianess = Endianness;
            m_pdata = (byte*) Imports.allocate((uint)m_length);
            m_shouldFree = true;
            internalInit();
        }

        public SystemBuffer(byte[] data, EndianessEnum Endianness)
        {
            m_totalLength = m_length = data.Length;
            m_pdata = (byte*)Imports.allocate((uint)m_length);
            unsafe
            {
                m_pdata = (byte*)Imports.convert(data);
            }
            internalInit();
        }

        internal SystemBuffer(byte* data, int length, EndianessEnum Endianness)
        {
            m_totalLength = m_length = length;
            m_pdata = (byte*)Imports.allocate((uint)m_length);
            unsafe
            {
                m_pdata = data;

            }
            internalInit();
        }
        private void internalInit()
        {
            m_shortIndex = new ShortIndexer(this);
            m_ushortIndex = new UShortIndexer(this);
            m_intIndex = new IntIndexer(this);
            m_uintIndex = new UIntIndexer(this);
        }
        #endregion constructors

        public void Join(SystemBuffer buffer)
        {
            this.next = buffer;
            buffer.prev = this;
        }
        public void JoinHeader(SystemBuffer buffer)
        {
            this.prev = buffer;
            buffer.next = this;
        }

        public int TotalByteLength { get {return m_totalLength; } }
        public int SegmentByteLength { get {return m_length; } }

        private byte* getByte(int index)
        {
            if (index < m_length)
            {
                return m_pdata;
            }
            if (next != null)
            {
                return next.getByte(index - m_length);
            }
            //next is null and we are off-bounds
            throw new System.IndexOutOfRangeException();
        }


        /// <summary>
        /// This function is the main function for placement of bytes, and is used such internally as well
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        [System.Runtime.CompilerServices.IndexerName("Bytes")]
        public byte this[int index]
        {
            get
            {
                throw new System.NotImplementedException();
            }
            set
            {
                throw new System.NotImplementedException();
            }
        }

        public void SetByte(int index, byte value)
        {
            throw new System.NotImplementedException();
        }
        public void GetByte(int index)
        {
            throw new System.NotImplementedException();
        }

        public void SetShort(int index, short value)
        {
            throw new System.NotImplementedException();
        }
        public short GetShort(int index)
        {
            throw new System.NotImplementedException();
        }

        public void SetUShort(int index, ushort value)
        {
            throw new System.NotImplementedException();
        }
        public ushort GetUShort(int index)
        {
            throw new System.NotImplementedException();
        }

        public void SetInt(int index, int value)
        {
            throw new System.NotImplementedException();
        }
        public int GetInt(int index)
        {
            throw new System.NotImplementedException();
        }

        public void SetUInt(int index, uint value)
        {
            throw new System.NotImplementedException();
        }
        public uint GetUInt(int index)
        {
            throw new System.NotImplementedException();
        }


        private void updateLength(int lengthOfFollowingBufs)
        {
            m_totalLength = m_length + lengthOfFollowingBufs;
            if (prev != null)
            {
                prev.updateLength(m_totalLength);
            }
        }



        public ShortIndexer Short
        {
            get
            {
                return m_shortIndex;
            }
        }
        public UShortIndexer UShort
        {
            get
            {
                return m_ushortIndex;
            }
        }
        public IntIndexer Int
        {
            get
            {
                return m_intIndex;
            }
        }
        public UIntIndexer UInt
        {
            get
            {
                return m_uintIndex;
            }
        }


        #region indexer-classes
        public class ShortIndexer
        {
            private SystemBuffer m_buffer;
            internal ShortIndexer(SystemBuffer buf)
            {
                m_buffer = buf;
            }
            public short this[int index]
            {
                get
                {
                    return m_buffer.GetShort(index);
                }
                set
                {
                    m_buffer.SetShort(index, value);
                }
            }

        }

        public class UShortIndexer
        {
            private SystemBuffer m_buffer;
            internal UShortIndexer(SystemBuffer buf)
            {
                m_buffer = buf;
            }
            public ushort this[int index]
            {
                get
                {
                    return m_buffer.GetUShort(index);
                }
                set
                {
                    m_buffer.SetUShort(index, value);
                }
            }
        }

                public class IntIndexer
        {
            private SystemBuffer m_buffer;
            internal IntIndexer(SystemBuffer buf)
            {
                m_buffer = buf;
            }
            public int this[int index]
            {
                get
                {
                    return m_buffer.GetInt(index);
                }
                set
                {
                    m_buffer.SetInt(index, value);
                }
            }

        }

        public class UIntIndexer
        {
            private SystemBuffer m_buffer;
            internal UIntIndexer(SystemBuffer buf)
            {
                m_buffer = buf;
            }
            public uint this[int index]
            {
                get
                {
                    return m_buffer.GetUInt(index);
                }
                set
                {
                    m_buffer.SetUInt(index, value);
                }
            }
        }
        #endregion indexer-classes
    }
}
