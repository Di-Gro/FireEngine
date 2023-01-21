using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {

    static class NavMesh { 

        public static Vector3[] GetPath(int slot) {
            ulong pointsPtr = 0;
            int count = 0;
            Dll.NavMesh.GethPath(Game.gameRef, ref pointsPtr, ref count, slot);

            if(count == 0)
                return null;

            var points = new Vector3[count];
            unsafe {
                Vector3* vec3_ptr = (Vector3*)pointsPtr;
                for (int i = 0; i < count; i++, vec3_ptr++)
                    points[i] = *vec3_ptr;
            }
            return points;
        }

        public static int FindPath(Vector3 startPos, Vector3 endPos, int pathSlot, int target) {
            return Dll.NavMesh.FindPath(Game.gameRef, startPos, endPos, pathSlot, target);
        }

        public static Vector3 RandomPoint() {
            return Dll.NavMesh.RandomPoint(Game.gameRef);
        }

    }

}