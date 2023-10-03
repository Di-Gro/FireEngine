using System;
using System.Collections.Generic;
using System.Reflection;

namespace Engine {

    [System.AttributeUsage 
        (
            System.AttributeTargets.Class | System.AttributeTargets.Struct, 
            Inherited = false, 
            AllowMultiple = false
        )
    ]
    public sealed class GUIDAttribute : System.Attribute {

        public static Dictionary<int, Type> types = new Dictionary<int, Type>();

        public readonly string guid;

        public static void CollectTypes() {
            types.Clear();

            var currentDomain = AppDomain.CurrentDomain;
            var assemblies = currentDomain.GetAssemblies();

            foreach (var assembly in assemblies) {
                var types = assembly.GetTypes();

                foreach (var type in types) {
                    if (GUIDAttribute.HasGuid(type))
                        GUIDAttribute.types.Add(GUIDAttribute.GetGuidHash(type), type);
                }
            }
        }

        public GUIDAttribute(string guid, Type type = null) {
            this.guid = guid;

            var hash = guid.GetHashCode();
        }

        public static Type GetTypeByGuid(string guid) {
            return GetTypeByHash(guid.GetHashCode());
        }

        public static Type GetTypeByHash(int guidHash) {
            if (types.ContainsKey(guidHash))
                return types[guidHash];
            return null;
        }

        public static bool HasTypeByFullName(string fullName) {
            var type = Type.GetType(fullName);
            return type != null ? HasGuid(type) : false; ;
        }

        public static int GetGuidHash(Type type) {
            var attr = type.GetCustomAttribute<GUIDAttribute>();
            var hash = attr == null ? 0 : attr.guid.GetHashCode();
            return hash;
        }

        public static string GetGuid(Type type) {
            var attr = type.GetCustomAttribute<GUIDAttribute>();
            return attr == null ? "" : attr.guid;
        }

        public static bool HasGuid(Type type) {
            return type.GetCustomAttribute<GUIDAttribute>() != null;
        }

        public static bool IsGuid(string guid) {
            Guid parsed;
            return Guid.TryParse(guid, out parsed);
        }

    }
}