using System;
using System.Collections.Generic;
using System.Text;

namespace FireYaml {
    public class YamlRef {
        public string lref = "";

        public YamlRef() { }
        public YamlRef(int value) => lref = value.ToString();
        public YamlRef(string value) {
            if (value.StartsWith("!file"))
                lref = value.Replace("!file", "");
            else if (value.StartsWith("file"))
                lref = value.Replace("file", "");
            else if (value.StartsWith(".file"))
                lref = value.Replace(".file", "");
            else
                lref = value;
        }

        public string Tag => lref == "" ? "" : $"!file{lref}";
        public string Name => lref == "" ? "" : $"file{lref}";
        public int Index => lref == "" ? -1 : int.Parse(lref);

        public override string ToString() {
            return $"{Index}: {Name}";
        }

    }
}
