
namespace Engine.AI {
    public struct Condition {
        public string target;
        public string field;
        public string value;

        public Condition(string target, string field, string value) {
            this.target = target;
            this.field = field;
            this.value = value;
        }

        public override string ToString() {
            return $"{target}, {field}, {value}";
        }
    }
}