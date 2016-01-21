using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    public interface IGameControl
    {
        int GetAngle();
        bool IsPressed();
    }
}
