using System;
using System.Collections.Generic;
using System.Reflection;

using EngineDll;


namespace Engine.AI {

    public class Decision {
        public CppRef aiComponentRef;
        public string name;
        public int hash;
        public Action action;

        public bool NeedDecide = false;
        public bool IsDecided = false;

        // Просто, чтобы можно было посмотреть в C# во время дебага
        private List<Condition> m_conditions = new List<Condition>();


        public Decision(CppRef aiComponentRef, string name, Action action = null) {
            this.aiComponentRef = aiComponentRef;
            this.name = name;
            this.hash = name.GetHashCode();
            this.action = action;
        }

        public void Add(string target, string field, string value) {
            m_conditions.Add(new Condition(target, field, value));

            var type1 = m_ConsumeType(ref target);
            var type2 = m_ConsumeType(ref field);
            var type3 = m_ConsumeType(ref value);

            Dll.AIComponent.AddCondition(
                aiComponentRef,
                hash,
                (int)type1, target,
                (int)type2, field,
                (int)type3, value
            );
        }

        public void Add(CppLinked target, string field, string value) {
            Add($"{target.csRef}", field, value);
        }

        public void Add(string target, string field, CppLinked value) {
            Add(target, field, $"{value.csRef}");
        }

        public void Add(CppLinked target, string field, CppLinked value) {
            Add($"{target.csRef}", field, $"{value.csRef}");
        }

        public string GetText() {
            if (!IsDecided)
                return "";

            var strptr = Dll.AIComponent.GetText(aiComponentRef, hash);
            var text = Assets.ReadCString(strptr);

            return text;
        }

        private ConstOrVar m_ConsumeType(ref string value) {

            if (value.StartsWith("#")) {
                value = value.Substring(1);
                return ConstOrVar.Var;
            }
            return ConstOrVar.Const;
        }

        public override string ToString() {
            return $"'{name}', cond: {m_conditions.Count}";
        }
    }
}