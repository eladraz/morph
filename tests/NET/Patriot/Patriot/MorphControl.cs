using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class MorphControl : IGameControl
    {


        public int GetAngle()
        {
            return Morph.UIX.GetPotentiometer();
        }

        public bool IsPressed()
        {
            return Morph.UIX.IsKeyPressed();
        }
    }
}
