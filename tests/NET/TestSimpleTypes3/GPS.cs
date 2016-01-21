using System;

namespace TestSimpleTypes3
{
    /// <summary>
    /// The GPS coordinate. And yes, I know GPS has 6 coordinate but I don't
    /// have time for that. 3 coordinate are good enough
    /// </summary>
    struct GPS
    {
        /// <summary>
        /// Constructor. Generate a new space.
        /// </summary>
        /// <param name="x">The x's axis coordinate</param>
        /// <param name="y">The y's axis coordinate</param>
        /// <param name="z">The z's axis coordinate</param>
        public GPS(int x, int y, int z)
        {
            this.m_x = x;
            this.m_y = y;
            this.m_z = z;
        }

        /// <summary>
        /// Override the default behavior of the string to print out the
        /// positions of the object normally
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return "[" + m_x.ToString() + ", " +
                         m_y.ToString() + ", " +
                         m_z.ToString() + "]" ;
        }

        /// <summary> The X's coordinate </summary>
        public int m_x;
        /// <summary> The Y's coordinate </summary>
        public int m_y;
        /// <summary> The Z's coordinate </summary>
        public int m_z;
    }
}
