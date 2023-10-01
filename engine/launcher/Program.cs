
using System.IO;
using System.Diagnostics;

class Launcher {

    static string msBuildExe = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";

    static string workspaceFolder = Path.GetFullPath("../../../../../");
    static string launchArg = "";

    static void Main(string[] args) {

        if (args.Length > 0)
            workspaceFolder = args[0];

        if (args.Length > 1) {
            launchArg = args[1];

            if(launchArg == "Launch:Build") {
                Build();
                return;
            }
            if (launchArg == "Launch:Run") {
                if (Build())
                    Run();
                return;
            }
            if (launchArg == "Launch:Debug") {
                if (Build())
                    Run("FireMode:WaitDebugger");
                return;
            }
        }
        Run();
    }

    static bool Build() {
        var buildArgs = $"{workspaceFolder}/FireProject.sln";

        Process cmd = new Process();
        cmd.StartInfo.FileName = $"\"{msBuildExe}\"";
        cmd.StartInfo.Arguments = $"{buildArgs} /clp:ErrorsOnly;Summary";
        cmd.StartInfo.RedirectStandardInput = true;
        cmd.StartInfo.RedirectStandardOutput = false;
        cmd.StartInfo.CreateNoWindow = false;
        cmd.StartInfo.UseShellExecute = false;
        cmd.Start();
        cmd.WaitForExit();

        return cmd.ExitCode == 0;
    }

    static void Run(string args = "") {
        var path = @"C:\Program Files\Mono\bin;" + Environment.GetEnvironmentVariable("PATH");

        Process cmd = new Process();
        cmd.StartInfo.FileName = $"\"{workspaceFolder}/x64/Debug/Core.exe\"";
        cmd.StartInfo.Arguments = args;
        cmd.StartInfo.RedirectStandardInput = true;
        cmd.StartInfo.RedirectStandardOutput = false;
        cmd.StartInfo.CreateNoWindow = false;
        cmd.StartInfo.UseShellExecute = false;
        cmd.StartInfo.EnvironmentVariables["PATH"] = path;
        cmd.StartInfo.WorkingDirectory = $"{workspaceFolder}\\engine\\core";
        cmd.Start();
        cmd.WaitForExit();
    }
        
}


