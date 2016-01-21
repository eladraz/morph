//using System;
//using System.Collections.Generic;
//using System.Text;

//namespace Morph
//{
//    public struct UInt64
//    {
//        public const ulong MaxValue = 0xffffffffffffffff;
//        public const ulong MinValue = 0;

//        internal int m_value;

//        public int CompareTo(object value)
//        {
//            if (value == null)
//                return 1;

//            if (!(value is System.UInt64))
//            {
//                //throw new ArgumentException(Locale.GetText("Value is not a System.UInt64."));
//                return 1;
//            }

//            return this.CompareTo((ulong)value);
//        }

//        public override bool Equals(object obj)
//        {
//            if (!(obj is System.UInt64))
//                return false;

//            return this.Equals((ulong)obj);
//        }

//        public override int GetHashCode()
//        {
//            return (int)(m_value & 0xffffffff) ^ (int)(m_value >> 32);
//        }

//        public int CompareTo(ulong value)
//        {
//            if (m_value == value)
//                return 0;
//            if (m_value > value)
//                return 1;
//            else
//                return -1;
//        }

//        public bool Equals(ulong obj)
//        {
//            return obj == m_value;
//        }

//        public static ulong Parse(string s)
//        {
//            ulong result = 0;
//            int i = 0, len = s.Length;
//            char c;

//            //if (s == null)
//            //    throw new ArgumentNullException();

//            //if (len > 20)
//            //    throw new Exception("Too big number");

//            for (; i < len; i++)
//            {
//                c = s[i];

//                if (c >= '0' && c <= '9')
//                {
//                    byte d = (byte)(c - '0');
//                    //if (result > (ulong.MaxValue / 10))
//                    //    throw new Exception("Too big number");

//                    if (result == (ulong.MaxValue / 10))
//                    {
//                        // if (d > (ulong.MaxValue % 10))
//                        //     throw new Exception("Too big number");

//                        result = result * 10 + d;
//                    }
//                    else
//                        result = result * 10 + d;
//                }
//                //else
//                //throw new Exception("type not match");
//            }

//            return result;
//        }

//        public static bool TryParse(string s, out ulong result)
//        {
//            result = 0;
//            int i = 0, len = s.Length;
//            char c;

//            if (s == null)
//                return false;

//            if (len > 20)
//                return false;

//            for (; i < len; i++)
//            {
//                c = s[i];

//                if (c >= '0' && c <= '9')
//                {
//                    byte d = (byte)(c - '0');
//                    if (result > (ulong.MaxValue / 10))
//                        return false;

//                    if (result == (ulong.MaxValue / 10))
//                    {
//                        if (d > (ulong.MaxValue % 10))
//                            return false;

//                        result = result * 10 + d;
//                    }
//                    else
//                        result = result * 10 + d;
//                }
//                else
//                    return false;
//            }

//            return true;
//        }

//        public override string ToString()
//        {
//            return ((uint)(m_value / 1000000000000000000)).ToString() + ((uint)((m_value / 1000000000) % 1000000000)).ToString() + ((uint)(m_value % 1000000000)).ToString();
//        }
//    }
//}
