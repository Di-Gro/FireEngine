
using System.IO;
using System.Diagnostics;

class Launcher {

    static void Main(string[] args) {

        foreach(var arg in args)
            Console.WriteLine($"arg: {arg}");

        string workspaceFolder = Path.GetFullPath("../../../../../");
        string engineArgs = "";

        if (args.Length > 0)
            workspaceFolder = args[0];

        if (args.Length > 1) {
            engineArgs = args[1];

            if(engineArgs == "FireGUID") {
                Console.WriteLine(Guid.NewGuid().ToString());
                return;
            }
        }

        Process cmd = new Process();
        cmd.StartInfo.FileName = "cmd.exe";
        cmd.StartInfo.RedirectStandardInput = true;
        cmd.StartInfo.RedirectStandardOutput = false;
        cmd.StartInfo.CreateNoWindow = false;
        cmd.StartInfo.UseShellExecute = false;
        cmd.Start();

        cmd.StandardInput.WriteLine($"cd \"{workspaceFolder}/x64/Debug\"");
        cmd.StandardInput.Flush();

        cmd.StandardInput.WriteLine(@"SET PATH=C:\Program Files\Mono\bin");
        cmd.StandardInput.Flush();

        cmd.StandardInput.WriteLine($"\"Core.exe\" {engineArgs}");
        cmd.StandardInput.Flush();
        cmd.StandardInput.Close();

        cmd.WaitForExit();
    }
}


