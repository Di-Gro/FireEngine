using System;
using System.Collections.Generic;
using System.Text;

namespace FireYaml {
    public class YamlValue {

        public enum Type { Var, Ref, CsRef, AssetId, Null }

        public string value;
        public Type type;

        public YamlValue() {
            type = Type.Null;
            value = "";
        }

        public YamlValue(Type type, string value = "") {
            this.type = type;
            this.value = value;
        }

        public override string ToString() => $"{{{type}: {value}}}";

        public static bool operator ==(YamlValue left, YamlValue right) {
            return left.type == right.type && left.value == right.value;
        }

        public static bool operator !=(YamlValue left, YamlValue right) {
            return left.type != right.type || left.value != right.value;
        }

    }
}
