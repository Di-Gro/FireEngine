using System;
using System.Collections.Generic;
using System.Reflection;

using EngineDll;


namespace Engine.AI {
    public class Field {
        public int index;
        public string name;

        // public object Value => m_GetValue();
        public bool IsValidTarget => Target != null;

        public bool IsCppLinked { get; private set; }

        public object Target => CppLinked.GetObjectByRef(m_targetRef);

        private FieldInfo m_field = null;
        private PropertyInfo m_prop = null;

        private CsRef m_targetRef;

        public Field(int index, FieldInfo field, CppLinked target) : this(index, target, field.Name) {
            m_field = field;

            IsCppLinked = typeof(CppLinked).IsAssignableFrom(m_field.FieldType);
        }

        public Field(int index, PropertyInfo prop, CppLinked target) : this(index, target, prop.Name) {
            m_prop = prop;

            IsCppLinked = typeof(CppLinked).IsAssignableFrom(m_prop.PropertyType);
        }

        private Field(int index, CppLinked target, string fieldName) {
            this.index = index;
            name = fieldName;

            m_targetRef = target.csRef;
        }

        public static Field CreateField(int index, string name, CppLinked instance) {
            var flags =
                  BindingFlags.Instance | BindingFlags.Public |
                  BindingFlags.GetField | BindingFlags.GetProperty;

            var type = instance.GetType();

            var field = type.GetField(name, flags);
            var prop = type.GetProperty(name, flags);

            if (field != null)
                return new Field(index, field, instance);

            if (prop != null)
                return new Field(index, prop, instance);

            return null;
        }

        public bool GetValue(out object value) {
            value = null;

            if (Target == null)
                return false;

            if (m_field != null) {
                value = m_field.GetValue(Target);
                return true;
            } else if (m_prop != null) {
                value = m_prop.GetValue(Target);
                return true;
            }
            return false;
        }
    }
}