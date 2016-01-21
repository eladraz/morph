using System;

namespace TestSimpleTypes3
{
    /// <summary>
    /// Interface for all creator being
    /// </summary>
    interface Creator
    {
        /// <summary>
        /// Eat some food
        /// </summary>
        void eat();
        /// <returns>Return the number of years the creator is living</returns>
        int getAge();
        /// <returns>Return the name of the creator</returns>
        string getName();
    }

    /// <summary>
    /// Return the object position in the space
    /// </summary>
    interface ObjectPosition
    {
        /// <returns> Return the object position </returns>
        GPS getPosition();
    }
}
