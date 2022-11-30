using System;
using System.Collections.Generic;
using System.Text;

namespace Engine
{
    public static class LinqUtil
    {
        public static AABox ToAABox(this IEnumerable<Vector3> self)
            => AABox.Create(self);
    }
}
