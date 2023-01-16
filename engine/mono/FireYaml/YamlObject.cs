using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;

namespace FireYaml {
    public class YamlObject {
        public enum Style { OneLine, MultiLine }

        public YamlScriptId scriptId = null;
        public YamlRef baseRef = null;
        public YamlRef docRef = null;
        public string selfDoc = "";

        public string name = "";
        public string tag = "";
        public string value = "";

        public List<YamlObject> values = new List<YamlObject>();

        public bool IsScalar => !IsComposite && !IsList && scriptId == null;
        public bool IsObject => IsComposite && !IsList;
        public bool IsComposite => IsNullObject || values.Count > 0;
        public bool IsList { get; set; }
        public bool IsNullObject { get; set; }
        public bool IsDocument => selfDoc != "";

        public object Convert(Type targetType) {
            if (targetType == typeof(float) || targetType == typeof(double))
                return System.Convert.ChangeType(value, targetType, CultureInfo.InvariantCulture);

            if (targetType.IsEnum)
                return Enum.Parse(targetType, value);

            return System.Convert.ChangeType(value, targetType);
        }

        public int TextLength() {
            int length = value.Length;

            foreach (var child in values)
                length += child.TextLength();

            return length;
        }

        public void WriteValues(YamlValues yamlValues, YamlObject parent, string parentPath = "") {

            bool isItem = parent != null ? parent.IsList : false;
            string nameStr = IsDocument ? selfDoc.Replace("!", "") : name;

            if (docRef != null) {
                var yamlValue = new YamlValue() {
                    value = $"{docRef.Tag}",
                    type = YamlValue.Type.Ref
                };
                yamlValues.AddValue($"{parentPath}", yamlValue);
            }
            else if (IsList) {
                for (int i = 0; i < values.Count; i++) {
                    var child = values[i];
                    child.WriteValues(yamlValues, this, $"{parentPath}.{nameStr}.{i}");
                }
            }
            else if (IsObject) {
                for (int i = 0; i < values.Count; i++) {
                    var child = values[i];
                    child.WriteValues(yamlValues, this, isItem ? $"{parentPath}" : $"{parentPath}.{nameStr}");
                }
            }
            else if (IsScalar) {
                var path = isItem ? $"{parentPath}" : $"{parentPath}.{nameStr}";
                var yamlValue = new YamlValue() {
                    value = $"{value}",
                    type = YamlValue.Type.Var
                };
                yamlValues.AddValue(path, yamlValue);
            }
        }

        public string ToString(YamlObject parent, Style style, int indent = 0) {
            string text = "";
            string path = "";

            bool isOneLine = style == Style.OneLine;
            bool isItem = parent != null ? parent.IsList : false;

            string tagStr = tag != "" ? $"{tag} " : "";
            string itemStr = isItem ? "- " : "";
            string indentStr = isOneLine ? "" : new string(' ', indent);
            string childIndent = isOneLine ? "" : new string(' ', indent + 4);
            string newLine = isOneLine ? "" : "\n";

            text += $"{indentStr}{itemStr}{tagStr}{name}: ";

            if (IsNullObject || (IsList && values.Count == 0)) {
                text += IsList ? "[]" : "{}";
                return text;
            }

            text += IsComposite && isOneLine ? (IsList ? "[" : "{") : "";

            if (scriptId != null) {
                text += $"{newLine}{childIndent}{FireYaml.FireWriter.TagMeta} scriptId: {{assetId: {scriptId.assetId}}}";
                text += isOneLine ? ", " : "";
            }
            if (baseRef != null) {
                text += $"{newLine}{childIndent}{FireYaml.FireWriter.TagMeta} baseRef: {{lref: {baseRef.lref}}}";
                text += isOneLine ? ", " : "";
            }
            if (docRef != null) {
                text += $"{{lref: {docRef.lref}}} ";
            }
            else if (IsScalar) {
                text += $"{value}";
            }
            else if (IsComposite) {
                for (int i = 0; i < values.Count; i++) {
                    var child = values[i];

                    if (isOneLine) {
                        text += child.ToString(this, style);

                        if (i != values.Count - 1)
                            text += ", ";
                    }
                    else {
                        text += "\n";
                        if (child.IsObject && child.TextLength() < 40)
                            text += childIndent + child.ToString(this, Style.OneLine, indent + 4);
                        else
                            text += child.ToString(this, style, indent + 4);
                    }
                }
            }
            text += IsComposite && isOneLine ? (IsList ? "]" : "}") : "";
            return text;
        }
    }
}
