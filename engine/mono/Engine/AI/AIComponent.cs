using System;
using System.Collections.Generic;
using System.Reflection;

using EngineDll;

namespace Engine {

    [GUID("3a378a5f-c500-48e2-9dcc-330cca2e22e1")]
    public class AIComponent : CppComponent {

        public List<AI.Field> m_fields = new List<AI.Field>();

        private Dictionary<string, AI.Decision> m_decisions = new Dictionary<string, AI.Decision>();

        private bool m_needDecide = false;

        public void Add(string target, string fieldName, string value) {
            Dll.AIComponent.AddWME(cppRef, target, fieldName, value);
        }

        public void Add(CppLinked target, string fieldName) {

            int index = Dll.AIComponent.AddWME(cppRef, $"{target.csRef}", fieldName, "null");

            var aiField = AI.Field.CreateField(index, fieldName, target);
            if (aiField == null)
                throw new Exception($"AIComponent: Field '{fieldName}' not found");

            m_fields.Add(aiField);
        }

        public void AddDecision(string decision, Action action) {
            m_decisions[decision] = new AI.Decision(cppRef, decision, action);
        }

        public void AddDecision(string decision) {
            m_decisions[decision] = new AI.Decision(cppRef, decision);
        }

        public AI.Decision this[string decision] {
            get => m_decisions[decision];
            set => m_decisions[decision] = value;
        }

        public void DecideAll() {
            foreach (var name_decision in m_decisions) {
                name_decision.Value.NeedDecide = true;
                m_needDecide = true;
            }
        }

        public void Decide(string decision) {
            m_decisions[decision].NeedDecide = true;
            m_needDecide = true;
        }

        public override void OnUpdate() {
            if (m_needDecide) {
                m_needDecide = false;

                m_UpdateMemory();
                m_MakeDecisions();
            }
        }

        private void m_UpdateMemory() {
            //TODO: нужно ли удалять невалидные объекты на стороне c++?
            // Сейчас ззначения невалидных объектов будет установлено как null
            // А поле этого объекта будет удалено из обновления
            bool need_update = false; 
            for (int i = m_fields.Count - 1; i >= 0; i--) {
                var field = m_fields[i];
                // Console.WriteLine($"filed: {field.name}");
                object value = null;
                need_update = true;
                bool isValid = field.GetValue(out value);

                 if (value != null && field.IsCppLinked)
                    value = ((CppLinked)value).csRef;

                var valueStr = value == null ? "null" : $"{value}";
                // Console.WriteLine($"valueStr: {valueStr}");
                Dll.AIComponent.SetWMEValue(cppRef, field.index, valueStr);

                if (!isValid)
                    m_fields.RemoveAt(i);
            }

            if (need_update)
            {
                Dll.AIComponent.UpdateRete(cppRef);
            }
        }

        private void m_MakeDecisions() {
            foreach (var name_decision in m_decisions) {
                var decision = name_decision.Value;
                decision.IsDecided = false;

                if (decision.NeedDecide) {
                    decision.NeedDecide = false;
                    decision.IsDecided = Dll.AIComponent.Decide(cppRef, decision.hash);
                    if (decision.IsDecided)
                        decision.action?.Invoke();
                }
            }
        }

        public override CppObjectInfo CppConstructor() {
            return Dll.AIComponent.Create(csRef);
        }
    }
}