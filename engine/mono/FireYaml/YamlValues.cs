using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Text.RegularExpressions;

namespace FireYaml {

    public class YamlTable {

        public static bool ReadLine(string text, out string key, out string value) {
            key = "";
            value = "";

            var document = new YamlDocumentReader().ReadOne(text);

            foreach (var docPair in document.objects) {
                var yamlDoc = docPair.Value;

                key = yamlDoc.name;
                value = yamlDoc.value;

                return true;
            }
            return false;
        }
    }

    public class YamlValues {

        private Dictionary<string, YamlValue> m_values;

        public YamlValues() {
            m_values = new Dictionary<string, YamlValue>();
        }

        public YamlValues(IEnumerable<KeyValuePair<string, YamlValue>> enumerable) {
            m_values = new Dictionary<string, YamlValue>(enumerable);
        }

        public YamlValues LoadFromText(string text) {
            var document = new YamlDocumentReader().ReadAll(text);
            return LoadFromDocument(document);
        }

        public YamlValues LoadFromDocument(YamlDocument document) {

            foreach (var docPair in document.objects) {
                var docName = docPair.Key;
                var yamlDoc = docPair.Value;

                foreach (var yamlObj in yamlDoc.values) {
                    var path = yamlObj.name;
                    var typeStr = yamlObj.values[0].name;
                    var value = yamlObj.values[0].value;
                    var type = Enum.Parse<YamlValue.Type>(typeStr);

                    AddValue(path, new YamlValue(type, value));
                }
            }
            return this;
        }

        public int Count => m_values.Count;

        public bool HasValue(string path) => m_values.ContainsKey(path);

        public YamlValue GetValue(string path) => m_values[path];

        public string GetValue(string path, string defaultValue) {
            return HasValue(path) ? GetValue(path).value : defaultValue;
        }

        public int GetItemsCount(string fieldPath) {
            var childPath = $"{fieldPath}.";

            int maxIndex = -1;
            foreach (var pair in m_values) {
                if (pair.Key.StartsWith(childPath)) {
                    int index;
                    var section = m_GetPathSection(pair.Key, childPath.Length);
                    if (int.TryParse(section, out index))
                        maxIndex = index > maxIndex ? index : maxIndex;
                }
            }
            return maxIndex >= 0 ? maxIndex + 1 : 0;
        }

        public List<string> GetFiles() {
            var filePath = ".file";
            var ids = new SortedSet<int>();

            foreach (var pair in m_values) {
                if (pair.Key.StartsWith(filePath)) {
                    var fileName = m_GetPathSection(pair.Key, 1);
                    ids.Add(new YamlRef(fileName).Index);
                }
            }

            var files = new List<string>();
            foreach (var id in ids)
                files.Add($"file{id}");

            return files;
        }

        private static string m_GetPathSection(string path, int startPos) {
            var nextDotIndex = path.IndexOf('.', startPos);
            nextDotIndex = nextDotIndex < 0 ? path.Length : nextDotIndex;

            var nextAtIndex = path.IndexOf('!', startPos);
            nextAtIndex = nextAtIndex < 0 ? path.Length : nextAtIndex;

            nextDotIndex = Math.Min(nextDotIndex, nextAtIndex);

            var length = nextDotIndex - startPos;

            return path.Substring(startPos, length);
        }

        public bool HasChildren(string path) => GetFirstChildPath(path) != "";

        public string GetFirstChildPath(string path) {
            var childPath = $"{path}.";
            foreach (var pair in m_values) {
                if (pair.Key.StartsWith(childPath))
                    return pair.Key;
            }
            return "";
        }

        public YamlValues GetObject(string path) {
            var selected = from pair in m_values
                           where 
                           pair.Key.StartsWith($"{path}:") || 
                           pair.Key.StartsWith($"{path}.") || 
                           pair.Key.StartsWith($"{path}!")
                           select pair;

            return new YamlValues(selected);
        }

        public YamlValues GetChildren(string path) {
            var selected = from pair in m_values
                           where pair.Key.StartsWith($"{path}.")
                           select pair;

            return new YamlValues(selected);
        }

        public bool IsList(string path) {
            var obj = GetObject(path);

            if (obj.Count == 0)
                return false;

            var firtItem = $"{path}.0";
            foreach (var pair in obj.m_values) {
                if (pair.Key.StartsWith(firtItem))
                    return true;
            }
            return false;
        }

        public void AddValue(string path, YamlValue value) {
            m_values.Add(path, value);
        }

        public void SetValue(string path, YamlValue value) {
            m_values[path] = value;
        }

        public void RemoveValue(string path) {
            m_values.Remove(path);
        }

        public void ResetValue(string path, YamlValue value) {

            if (!m_RemoveAncestor(path)) {
                foreach (var pair in m_values) {
                    if (pair.Key.StartsWith(path))
                        m_values.Remove(pair.Key);
                }
            }
            m_values.Add(path, value);
        }

        public void Clear() => m_values.Clear();

        public string ToSortedText() {
            string text = "";

            var files = GetFiles();
            foreach (var file in files) {
                var obj = GetObject($".{file}");
                text += obj.ToText();
                text += "\n";
            }
            return text;
        }

        public string ToText() {
            string text = "";

            foreach (var pair in m_values) {
                var path = pair.Key;
                var value = pair.Value;
                text += $"{path}: {{{value.type}: {value.value}}}\n";
            }
            return text;
        }

        public void Print() {
            foreach (var pair in m_values) {
                var path = pair.Key;
                var value = pair.Value;

                Console.WriteLine($"{path}: {{{value.type}: {value.value}}}");
            }
        }

        private bool m_RemoveAncestor(string path) {
            var ancestors = GetAllAncestors(path);

            for (int i = 1; i < ancestors.Length; i++) {
                var ancestor = ancestors[i];
                if (HasValue(ancestor)) {
                    RemoveValue(ancestor);
                    return true;
                }
            }
            return false;
        }

        public static YamlValues Mods(YamlValues source, YamlValues modified) {
            var mods = new YamlValues();

            foreach (var pair in modified.m_values) {
                var path = pair.Key;
                var value = pair.Value;

                var hasInOther = source.HasValue(path);
                if (hasInOther) {
                    var otherValue = source.GetValue(path);
                    if (value != otherValue) {
                        if (!IsEqualLinks(path, value, otherValue))
                            mods.AddValue(path, value);
                    }
                }
                else {
                    mods.AddValue(path, value);
                }
            }
            return mods;
        }

        private static bool IsEqualLinks(string path, YamlValue left, YamlValue right) {
            if (left.type == right.type && left.type == YamlValue.Type.Ref) {
                var unpackedLeft = UnpackLink(path, left.value);
                return unpackedLeft == right.value;
            }
            return false;
        }

        public static YamlValues Merge(YamlValues source, YamlValues target, string rootPath, bool packLinks = true) {
            var merged = new YamlValues();

            foreach (var pair in source.m_values) {
                var path = pair.Key;
                var value = pair.Value;

                if (packLinks && value.type == YamlValue.Type.Ref)
                    value.value = $"{rootPath.Substring(1)}.{value.value}";

                merged.m_values.Add($"{rootPath}{path}", value);
            }
            foreach (var pair in target.m_values) {
                var path = pair.Key;
                var value = pair.Value;

                merged.ResetValue(path, value);
            }

            return merged;
        }

        public void Append(YamlValues other, string rootPath) {
            foreach (var pair in other.m_values)
                AddValue($"{rootPath}{pair.Key}", pair.Value);
        }

        public static string[] SplitPath(string path) {
            var rx = new Regex(@"[\.]");
            var fixedPath = path.Replace("!.", ".!");
            return rx.Split(fixedPath, path.Length, 1);
        }

        public static string UnpackLink(string path, string link) {
            var rx = new Regex(@"[\.]");
            var splitedPath = rx.Split(path.Substring(1), path.Length, 1);
            var splitedLink = rx.Split(link, link.Length, 1);

            for (int i = 0; i < splitedPath.Length && i < splitedLink.Length; i++) {
                if (splitedLink[i].EndsWith("!")) {
                    if (splitedLink[i] == splitedPath[i])
                        splitedLink[i] = "";
                }
            }
            var localLink = "";
            for (int i = 0; i < splitedLink.Length; i++) {
                localLink += splitedLink[i];
                if (splitedLink[i] != "" && i < splitedLink.Length - 1)
                    localLink += ".";
            }
            return localLink;
        }

        public static YamlValues PackLins(YamlValues source, string rootPath) {
            var packed = new YamlValues();

            foreach (var pair in source.m_values) {
                var path = pair.Key;
                var value = pair.Value;

                if (value.type == YamlValue.Type.Ref)
                    value.value = $"{rootPath.Substring(1)}.{value.value}";

                packed.AddValue(path, value);
            }
            return packed;
        }

        public static string[] GetAllAncestors(string path) {
            var splited = SplitPath(path);
            var ancestors = new string[splited.Length];

            for (int i = 0; i < splited.Length; i++) {
                string ancestor = "";
                for (int j = 0; j < splited.Length - i; j++) {
                    ancestor += splited[j];
                    if (j < splited.Length - i - 1)
                        ancestor += ".";
                }
                ancestors[i] = ancestor.Replace(".!", "!."); ;
            }
            return ancestors;
        }

    }
}
