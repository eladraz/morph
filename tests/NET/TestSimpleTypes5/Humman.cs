using System;

namespace TestSimpleTypes3
{
    class Humman : Creator, ObjectPosition
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        /// <param name="name"></param>
        public Humman(string name, int age)
        {
            m_age = age;
            m_name = name;
            System.Random rnd = new System.Random();
            m_position = new GPS(rnd.Next(), rnd.Next(), rnd.Next());
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~Humman()
        {
            System.Console.WriteLine("Humman " + m_name + " died at age " + m_age);
        }

        #region ObjectPosition Members
        public GPS getPosition()
        {
            return m_position;
        }

        #endregion

        #region Creator Members Methods
        public void eat()
        {
            m_age++;
        }

        public int getAge()
        {
            return m_age;
        }

        public string getName()
        {
            return m_name;
        }
        #endregion

        #region Creator Members
        /// <summary>
        ///  The age of the humman
        /// </summary>
        private int m_age;
        /// <summary>
        /// The name of the humman
        /// </summary>
        private string m_name;
        /// <summary>
        /// The humman position
        /// </summary>
        private GPS m_position;
        #endregion
    }
}
