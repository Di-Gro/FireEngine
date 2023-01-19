﻿using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System;
using System.Linq;
using System.Diagnostics;

using FireYaml;
using EngineDll;
using UI;

namespace Engine {
    public class CloseAttribute : Attribute { }

    public class OpenAttribute : Attribute { }

    public class ColorAttribute : Attribute { }
    
    public class SpaceAttribute : Attribute { }

    public class RangeAttribute : Attribute {
        public int imin, imax;
        public float fmin, fmax;

        public RangeAttribute(int min, int max) {
            imin = min;
            imax = max;
        }

        public RangeAttribute(float min, float max) {
            fmin = min;
            fmax = max;
        }
    }

}

namespace Engine {

    public class SerializerBase {

        public virtual bool NeedIncludeBase(Type type) => false;

        public virtual bool NeedSerialize(FieldInfo field) {
            if (field.IsPublic) {
                var closeAttribute = field.GetCustomAttribute<Engine.CloseAttribute>();
                if (closeAttribute != null)
                    return false;
            }
            if (field.IsPrivate) {
                var openAttribute = field.GetCustomAttribute<Engine.OpenAttribute>();
                if (openAttribute == null)
                    return false;
            }
            return true;
        }

        public virtual bool NeedSerialize(PropertyInfo prop) {
            bool canReadWrite = prop.CanRead && prop.CanWrite;
            if (!canReadWrite)
                return false;

            bool isPrivate = prop.GetGetMethod(true).IsPrivate || prop.GetSetMethod(true).IsPrivate;
            bool isPublic = !isPrivate;

            if (isPublic) {
                var closeAttribute = prop.GetCustomAttribute<Engine.CloseAttribute>();
                if (closeAttribute != null)
                    return false;
            }
            if (isPrivate) {
                var openAttribute = prop.GetCustomAttribute<Engine.OpenAttribute>();
                if (openAttribute == null)
                    return false;
            }
            return true;
        }

        public virtual void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) { }

        public virtual void OnDeserialize(FireYaml.FireReader deserializer, string selfPath, Type type, ref object instance) { }

        public virtual void OnDrawGui(Type type, ref object instance) { }
    }

    public class ActorSerializer : SerializerBase {

        private static LinkedList<WaitedComponent> m_waitedComponents = new LinkedList<WaitedComponent>();

        class WaitedComponent {
            public Actor actor;
            public Component component;
            public LinkedListNode<WaitedComponent> node;

            public void OnEndLoad() {
                Dll.Actor.InitComponent(actor.cppRef, component.cppRef);
                m_waitedComponents.Remove(node);
            }
        }

        public override void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var children = actor.GetChildren();
            var components = actor.GetComponentsList();

            serializer.AddField($"{selfPath}.m_children", children.GetType(), children);
            serializer.AddField($"{selfPath}.m_flags", typeof(ulong), (ulong)actor.Flags);

            m_AddComponents(serializer, selfPath, components);

            foreach (var child in children)
                serializer.CreateDocument(child.GetType(), child);

            foreach (var component in components) {
                if (!component.runtimeOnly)
                    serializer.CreateDocument(component.GetType(), component);
            }
        }

        private void m_AddComponents(FireYaml.FireWriter serializer, string selfPath, List<Component> components) {
            if (components.Count == 0) {
                serializer.AddField($"{selfPath}.m_components", components.GetType(), components);
                return;
            }
            int index = 0;
            foreach (var component in components) {
                if (!component.runtimeOnly)
                    serializer.AddField($"{selfPath}.m_components.{index++}", component.GetType(), component);
            }
        }

        public override void OnDeserialize(FireYaml.FireReader reader, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(reader, selfPath, type, ref instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var componentsPath = $"{selfPath}.m_components";
            var childrenPath = $"{selfPath}.m_children";
            var flagsPath = $"{selfPath}.m_flags";

            m_LoadComponents(reader, componentsPath, actor);
            m_LoadActorChildren(reader, childrenPath, actor);

            var flagsStr = reader.GetField(flagsPath);
            var str = flagsStr.GetValue(flagsPath, "0");
            var flags = ulong.Parse(str);
            actor.Flags = (Flag)flags;
        }

        private void m_LoadActorChildren(FireYaml.FireReader reader, string childrenPath, Engine.Actor actor) {
            var children = reader.GetField(childrenPath);

            var count = children.GetItemsCount(childrenPath);
            for (int i = 0; i < count; i++) {
                var yamlChild = children.GetValue($"{childrenPath}.{i}");
                var fullPath = $".{yamlChild.value}";

                if (!reader.HasFile(fullPath))
                    throw new Exception("Missing actor");

                if (yamlChild.type == YamlValue.Type.Null)
                    throw new Exception("Actor component can not be Null");

                object child = new Actor(actor);
                reader.LoadDocument(fullPath, ref child);
            }
        }

        private void m_LoadComponents(FireYaml.FireReader reader, string componentsPath, Engine.Actor actor) {
            var components = reader.GetField(componentsPath);

            var count = components.GetItemsCount(componentsPath);
            for (int i = 0; i < count; i++) {
                var yamlChild = components.GetValue($"{componentsPath}.{i}");
                var filePath = $".{yamlChild.value}";

                if (!reader.HasFile(filePath))
                    throw new Exception("Missing component");

                LoadComponent(reader, filePath, actor);
            }
        }

        public object LoadComponent(FireYaml.FireReader reader, string filePath, Engine.Actor actor) {
            var componentType = reader.GetTypeOf(filePath);

            /// Create
            var componentObj = Activator.CreateInstance(componentType);
            var component = componentObj as Component;
            var info = component.CppConstructor();

            /// Bind
            component.CsBindComponent(actor.csRef, info);
            Dll.Actor.BindComponent(actor.cppRef, component.cppRef);

            /// PostBind
            reader.LoadDocument(filePath, ref componentObj);

            /// Init (Delayed Init - wait for the end of load all documents)
            var waited = new WaitedComponent();
            waited.actor = actor;
            waited.component = component;
            waited.node = m_waitedComponents.AddLast(waited);

            reader.EndLoadEvent += waited.OnEndLoad;

            return componentObj;
        }

    }

    public class ComponentSerializer : SerializerBase {

        public override bool NeedIncludeBase(Type type) {
            bool isCsBased = type.BaseType == typeof(Engine.CSComponent);
            bool isCppBased = type.BaseType == typeof(Engine.CppComponent);

            return !isCsBased && !isCppBased;
        }

    }

    public class MeshComponentSerializer : ComponentSerializer {

        public override void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var mesh = meshComponent.mesh;
            var materials = m_GetMaterialsList(meshComponent);
            var staticMeshType = typeof(Engine.StaticMesh);
            var needSave = mesh != null && mesh.GetType() == staticMeshType;

            serializer.AddField($"{selfPath}.m_mesh", staticMeshType, needSave ? mesh : null);
            serializer.AddField($"{selfPath}.m_materials", materials.GetType(), materials);
        }

        public override void OnDeserialize(FireYaml.FireReader deserializer, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(deserializer, selfPath, type, ref instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var meshPath = $"{selfPath}.m_mesh";
            var materialsPath = $"{selfPath}.m_materials";
            
            m_LoadMesh(deserializer, meshPath, meshComponent);
            m_LoadMaterials(deserializer, materialsPath, meshComponent);

            Dll.MeshComponent.OnPreInit(meshComponent.cppRef);
        }

        private List<StaticMaterial> m_GetMaterialsList(MeshComponent meshComponent) {
            var list = new List<StaticMaterial>();

            for (int index = 0; index < meshComponent.MaterialCount; index++) {
                var material = meshComponent.GetMaterial((ulong)index);
                list.Add(material.IsDynamic ? null : material);
            }
            return list;
        }

        private void m_LoadMesh(FireYaml.FireReader deserializer, string meshPath, MeshComponent meshComponent) {
            
            var meshAssetPath = $"{meshPath}.assetId";
            var meshField = deserializer.GetField(meshPath);
        
            /// Если есть сам меш, значит равен нулю
            if(meshField.HasValue(meshPath))
                return;
                
            /// Если нет, значит меш не установлен
            if(!meshField.HasValue(meshAssetPath))
                return;
                
            var yamlValue = meshField.GetValue(meshAssetPath);
            var assetId = yamlValue.value;
            var isPath = m_IsPath(assetId);

            if (!isPath && !AssetStore.Instance.HasAssetPath(assetId))
                    throw new Exception($"Missing AssetId: {assetId}");

            if (yamlValue.type == YamlValue.Type.AssetId) {
                StaticMesh mesh = null;
                if (isPath)
                    mesh = new StaticMesh().LoadFromFile(assetId);
                else
                    mesh = new StaticMesh().LoadFromAsset(assetId);

                Dll.MeshComponent.SetPreInitMesh(meshComponent.cppRef, mesh.cppRef);
                return;
            }
        }

        private bool m_IsPath(string value) {
            return value.Contains('/') || value.Contains('\\') || value.Contains('.');
        }

        private void m_LoadMaterials(FireYaml.FireReader deserializer, string materialsPath, MeshComponent meshComponent) {
            var yamlObject = deserializer.GetField(materialsPath);

            var count = yamlObject.GetItemsCount(materialsPath);
            if (count == 0)
                return;

            var matRefs = new List<ulong>();
            var defaultMaterial = new StaticMaterial().LoadFromAsset(Assets.M_Default);

            for (int i = 0; i < count; i++) {
                var valuePath = $"{materialsPath}.{i}";

                /// Если есть, значит равен null
                if(!yamlObject.HasValue(valuePath)) {
                    var yamlValue = yamlObject.GetValue($"{valuePath}.assetId");
                    
                    if (yamlValue.type == YamlValue.Type.AssetId) {
                        var assetId = yamlValue.value;

                        if (!AssetStore.Instance.HasAssetPath(assetId))
                            throw new Exception($"Missing AssetId: {assetId}");

                        var material = new StaticMaterial().LoadFromAsset(assetId);
                        matRefs.Add(material.cppRef.value);
                        continue;
                    }
                }
                /// Null и другое
                matRefs.Add(defaultMaterial.cppRef.value);
            }
            Dll.MeshComponent.SetPreInitMaterials(meshComponent.cppRef, matRefs.ToArray(), matRefs.Count);
        }

        public override void OnDrawGui(Type type, ref object instance) {
            base.OnDrawGui(type, ref instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var mesh = meshComponent.mesh;
            int assetIdHash = 0;

            if(mesh != null)
                assetIdHash = mesh.IsDynamic ? -1 : ((StaticMesh)mesh).assetIdHash;

            GUI.Space();

            object changedMesh;
            if (GUI.DrawAsset("Mesh", typeof(StaticMesh), assetIdHash, out changedMesh))
                meshComponent.mesh = changedMesh as Mesh;
            
            var flags = ImGuiTreeNodeFlags_._Framed | ImGuiTreeNodeFlags_._DefaultOpen;
            if(GUI.CollapsingHeader("Materials", flags)) {

                var count = (ulong) meshComponent.MaterialCount;
                for (ulong index = 0; index < count; index++) {
                    var material = meshComponent.GetMaterial(index);

                    assetIdHash = material.IsDynamic ? -1 : material.assetIdHash;

                    object changedMaterial;
                    if (GUI.DrawAsset($"Material: {index}", typeof(StaticMaterial), assetIdHash, out changedMaterial))
                        meshComponent.SetMaterial(index, changedMaterial as StaticMaterial);

                    if(index < count - 1)
                        GUI.Space();
                }
            }
        }

    }

}
