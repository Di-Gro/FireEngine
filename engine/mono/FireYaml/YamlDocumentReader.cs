using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System.Diagnostics;
using System.IO;

namespace FireYaml {

    public class YamlDocumentReader {

        public YamlDocument ReadOne(string text) {
            var document = new YamlDocument();

            var sr = new StringReader(text);
            var stream = new YamlStream();
            stream.Load(sr);

            foreach (var doc in stream.Documents) {
                var nodes = new List<YamlNode>(doc.AllNodes);

                /// Document Tag
                var docTag = "";
                if (!nodes[0].Tag.IsEmpty)
                    docTag = nodes[0].Tag.Value;

                /// Root Object
                var obj = new YamlObject();
                obj.selfDoc = docTag;
                obj.name = nodes[1].ToString();

                if (!nodes[1].Tag.IsEmpty)
                    obj.tag = nodes[1].Tag.Value;

                var size = VisitObject(ref obj, nodes[2]);

                /// Append Object
                document.objects[docTag] = obj;
            }
            return document;
        }

        public YamlDocument ReadAll(string text) {
            var document = new YamlDocument();

            var sr = new StringReader(text);
            var stream = new YamlStream();
            stream.Load(sr);

            foreach (var doc in stream.Documents) {
                var nodes = new List<YamlNode>(doc.AllNodes);

                /// Document Tag
                var docTag = "";
                if (!nodes[0].Tag.IsEmpty)
                    docTag = nodes[0].Tag.Value;

                /// Root Object
                var obj = new YamlObject();
                obj.selfDoc = docTag;
                obj.name = nodes[1].ToString();

                if (!nodes[1].Tag.IsEmpty)
                    obj.tag = nodes[1].Tag.Value;

                var size = VisitObject(ref obj, nodes[0]);

                /// Append Object
                document.objects[docTag] = obj;
            }
            return document;
        }

        public int VisitObject(ref YamlObject self, YamlNode valueNode) {
            var nodes = new List<YamlNode>(valueNode.AllNodes);
            var nodesCount = nodes.Count;

            self.IsList = valueNode.NodeType == YamlNodeType.Sequence;

            /// Consume Raw Data
            nodes.RemoveAt(0);

            if (nodes.Count == 0 && valueNode.NodeType == YamlNodeType.Scalar) {
                self.value = valueNode.ToString();
                return nodesCount;
            }

            /// Consume Objects
            while (nodes.Count > 0) {
                if (self.IsList)
                    nodes.RemoveAt(0);

                var child = new YamlObject();
                child.name = nodes[0].ToString();

                if (!nodes[0].Tag.IsEmpty)
                    child.tag = nodes[0].Tag.Value;

                nodes.RemoveAt(0);

                var size = VisitObject(ref child, nodes[0]);
                nodes.RemoveRange(0, size);

                if (child.tag == "!M") {
                    switch (child.name) {
                        case "scriptId":
                            self.scriptId = new YamlScriptId();
                            self.scriptId.assetId = child.values[0].value;
                            //var scriptIdObj = (object)self.scriptId;
                            //LoadObject(child, typeof(YamlScriptId), ref scriptIdObj);
                            break;

                        case "baseRef":
                            self.baseRef = new YamlRef();
                            self.baseRef.lref = child.values[0].value;
                            //var baseRefObj = (object)self.baseRef;
                            //LoadObject(child, typeof(YamlRef), ref baseRefObj);
                            break;
                    }
                    continue;
                }
                if (child.tag == "!R") {
                    child.docRef = new YamlRef();
                    child.docRef.lref = child.values[0].value;
                    //var docRefObj = (object)child.docRef;
                    //LoadObject(child, typeof(YamlRef), ref docRefObj);
                }
                self.values.Add(child);
            }

            return nodesCount;
        }
    }
}
