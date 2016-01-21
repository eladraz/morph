using System;

namespace clrcore
{
    unsafe class Buffer
    {
        public enum DestructionMethod { None, Free };

        public Buffer(void* buffer, uint length, DestructionMethod destructionMethod)
        {
            m_buffer = buffer;
            m_length = length;
            m_destructionMethod = destructionMethod;
        }

        ~Buffer()
        {
            if (m_destructionMethod == DestructionMethod.Free)
            {
                Morph.Imports.free(m_buffer);
            }
        }

        public uint getLength()
        {
            return m_length;
        }

        public void* getBuffer()
        {
            return m_buffer;
        }

        // Buffer address and length
        private void* m_buffer;
        private uint  m_length;
        private DestructionMethod m_destructionMethod;
    }
}
