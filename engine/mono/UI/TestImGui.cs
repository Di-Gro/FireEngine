using System;
using System.Collections.Generic;
using System.Text;
using System.Numerics;
using System.Runtime.InteropServices;


namespace UI {
    class Pin : IDisposable {
        GCHandle m_handle;

        public IntPtr Ptr => m_handle.AddrOfPinnedObject();

        public static implicit operator IntPtr(Pin pin) => pin.Ptr;

        public void Dispose() {
            m_handle.Free();
        }

        public static Pin Create<T>(in T[] value) where T : struct {
            return new Pin {
                m_handle = GCHandle.Alloc(value, GCHandleType.Pinned)
            };
        }
    }

    class TestImGui : Engine.CSComponent {

        private float f = 0.0f;
        private int counter = 0;

        private bool[] show_demo_window = new[] { true };
        private bool[] show_another_window = new[] { false };
        private Vector4 clear_color = new Vector4(0.45f, 0.55f, 0.60f, 1.00f);

        private float m_timeToUpdate = 0;
        private string m_text = "";


        public override void OnUpdate() {
            if (m_timeToUpdate > 0)
                m_timeToUpdate -= Engine.Game.DeltaTime;

            if (Engine.Input.GetButton(Engine.Key.Tilda)) {
                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
                {
                    ImGui.Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ImGui.Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui.Checkbox("Demo Window", ref show_demo_window[0]);      // Edit bools storing our window open/close state
                    ImGui.Checkbox("Another Window", ref show_another_window[0]);

                    ImGui.SliderFloat("float", ref f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui.ColorEdit3("clear color", ref clear_color.X); // Edit 3 floats representing a color

                    if (ImGui.Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ImGui.SameLine();
                    ImGui.Text($"counter = {counter}");

                    var p = ImGui.GetIO();
                    var i = (ImGuiIO)Marshal.PtrToStructure(p, typeof(ImGuiIO));
                    ImGui.Text($"Application average {1000.0f / i.Framerate:F3} ms/frame ({i.Framerate:F1} FPS)");
                    ImGui.End();
                }

                // 3. Show another simple window.
                if (show_another_window[0]) {
                    using var pin = Pin.Create(show_another_window);
                    ImGui.Begin("Another Window", pin);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui.Text("Hello from another window!");
                    if (ImGui.Button("Close Me"))
                        show_another_window[0] = false;
                    ImGui.End();
                }

                // Иерархия сцены
                if (m_timeToUpdate <= 0) {
                    m_timeToUpdate = 1.0f;
                    m_text = "";
                    var actors = Engine.Game.GetRootActors();
                    foreach (var actor in actors)
                        AddActorText(ref m_text, actor, 0);
                }
                ImGui.Begin("Root Actors");
                ImGui.Text(m_text);
                ImGui.End();
            }

        }

        private void AddActorText(ref string text, Engine.Actor actor, int level) {
            for (int i = 0; i < level; i++)
                text += "-> ";

            text += $"{actor.Name}\n";

            var childrenCount = actor.GetChildrenCount();
            if (childrenCount > 0) {
                for (int c = 0; c < childrenCount; c++)
                    AddActorText(ref text, actor.GetChild(c), level + 1);
            }
        }
    }
}