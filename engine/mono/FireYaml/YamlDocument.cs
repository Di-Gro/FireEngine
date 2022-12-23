using System;
using System.Collections.Generic;
using System.Text;

namespace FireYaml {
    public class YamlDocument {

        public Dictionary<string, YamlObject> objects = new Dictionary<string, YamlObject>();

        public YamlObject this[string key] {
            get => objects[key];
            set => objects[key] = value;
        }

        public string CreateText() {
            string text = "";

            foreach (var pair in objects) {
                var docRef = pair.Key;
                var obj = pair.Value;

                text += $"\n--- {docRef} \n";
                text += obj.ToString(null, YamlObject.Style.MultiLine);
                text += "\n";
            }
            return text;
        }

    }
}
