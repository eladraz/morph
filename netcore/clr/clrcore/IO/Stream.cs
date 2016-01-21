//
// System.IO.Stream.cs
//
// Authors:
//   Dietmar Maurer (dietmar@ximian.com)
//   Miguel de Icaza (miguel@ximian.com)
//   Gonzalo Paniagua Javier (gonzalo@ximian.com)
//   Marek Safar (marek.safar@gmail.com)
//
// (C) 2001, 2002 Ximian, Inc.  http://www.ximian.com
// (c) 2004 Novell, Inc. (http://www.novell.com)
// Copyright 2011 Xamarin, Inc (http://www.xamarin.com)
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


namespace Morph.IO
{
    public abstract class Stream  // : MarshalByRefObject, IDisposable
    {
        public static readonly Stream Null = new NullStream();

        //Func<byte[], int, int, int> async_read;
        //Action<byte[], int, int> async_write;
        //AutoResetEvent async_event;

        protected Stream()
        {
        }

        //TICKET#54
        public virtual bool CanRead
        {
            get { return true; }
        }

        //TICKET#54
        public virtual bool CanSeek
        {
            get { return true; }
        }

        //TICKET#54
        public virtual bool CanWrite
        {
            get { return true; }
        }

        public virtual bool CanTimeout
        {
            get
            {
                return false;
            }
        }

        //TICKET#54
        public virtual int Length
        {
            get { return 0; }
        }

        //TICKET#54
        public virtual int Position
        {
            get { return 0; }
            set {  }
        }


        public void Dispose()
        {
            Close();
        }

        //protected virtual void Dispose(bool disposing)
        //{
        //    if (async_event != null && disposing)
        //    {
        //        async_event.Close();
        //        async_event = null;
        //    }
        //}

        public virtual void Close()
        {
            //Dispose(true);
            //GC.SuppressFinalize(this);
        }

        public static Stream Synchronized(Stream stream)
        {
            return new SynchronizedStream(stream);
        }


        //TICKET#54
        public virtual void Flush() { }
        //TICKET#54
        public virtual int Read(byte[] buffer, int offset, int count) { return 0; }       //WAS: public abstract int Read([In, Out] byte[] buffer, int offset, int count);


        public virtual int ReadByte()
        {
            byte[] buffer = new byte[1];

            if (Read(buffer, 0, 1) == 1)
                return buffer[0];

            return -1;
        }
        //TICKET#54 //LONG NOT SUPPORTED
        public virtual int Seek(int offset, SeekOrigin origin) { return 0; }
        //TICKET#54
        public virtual void SetLength(int value) { }
        //TICKET#54
        public virtual void Write(byte[] buffer, int offset, int count) { }

        public virtual void WriteByte(byte value)
        {
            byte[] buffer = new byte[1];

            buffer[0] = value;

            Write(buffer, 0, 1);
        }


		public void CopyTo (Stream destination)
		{
			CopyTo (destination, 16*1024);
		}

		public void CopyTo (Stream destination, int bufferSize)
		{
			if (destination == null)
            {
                //throw new ArgumentNullException ("destination");
            }
			if (!CanRead)
            {
                //throw new NotSupportedException ("This stream does not support reading");
            }
			if (!destination.CanWrite)
            {
                //throw new NotSupportedException ("This destination stream does not support writing");
            }
            if (bufferSize <= 0)
            {
                //throw new ArgumentOutOfRangeException ("bufferSize");
            }

			byte[] buffer = new byte [bufferSize];
			int nread;
			while ((nread = Read (buffer, 0, bufferSize)) != 0)
				destination.Write (buffer, 0, nread);
		}
    }

    class NullStream : Stream
    {
        public override bool CanRead
        {
            get
            {
                return true;
            }
        }

        public override bool CanSeek
        {
            get
            {
                return true;
            }
        }

        public override bool CanWrite
        {
            get
            {
                return true;
            }
        }

        public override int Length
        {
            get
            {
                return 0;
            }
        }

        public override int Position
        {
            get
            {
                return 0;
            }
            set
            {
            }
        }

        public override void Flush()
        {
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            return 0;
        }

        public override int ReadByte()
        {
            return -1;
        }

        public override int Seek(int offset, SeekOrigin origin)
        {
            return 0;
        }

        public override void SetLength(int value)
        {
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
        }

        public override void WriteByte(byte value)
        {
        }
    }

    class SynchronizedStream : Stream
    {
        Stream source;
        //object slock;

        internal SynchronizedStream(Stream source)
        {
            this.source = source;
            //slock = new object();
        }

        public override bool CanRead
        {
            get
            {
                ////lock (slock)
                    return source.CanRead;
            }
        }

        public override bool CanSeek
        {
            get
            {
                //lock (slock)
                    return source.CanSeek;
            }
        }

        public override bool CanWrite
        {
            get
            {
                //lock (slock)
                    return source.CanWrite;
            }
        }

        public override int Length
        {
            get
            {
                //lock (slock)
                    return source.Length;
            }
        }

        public override int Position
        {
            get
            {
                //lock (slock)
                    return source.Position;
            }
            set
            {
                //lock (slock)
                    source.Position = value;
            }
        }

        public override void Flush()
        {
            //lock (slock)
                source.Flush();
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            //lock (slock)
                return source.Read(buffer, offset, count);
        }

        public override int ReadByte()
        {
            //lock (slock)
                return source.ReadByte();
        }

        public override int Seek(int offset, SeekOrigin origin)
        {
            //lock (slock)
                return source.Seek(offset, origin);
        }

        public override void SetLength(int value)
        {
            //lock (slock)
                source.SetLength(value);
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            //lock (slock)
                source.Write(buffer, offset, count);
        }

        public override void WriteByte(byte value)
        {
            //lock (slock)
                source.WriteByte(value);
        }
    }
}
