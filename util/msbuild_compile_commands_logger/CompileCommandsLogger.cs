// The MIT License (MIT)
//
// Copyright (c) 2024-present Bartek Kryza <bkryza@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
using System.Web;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

/// <summary>
/// MSBuild logger plugin which generates compile_commands.json file.
/// </summary>
/// <remarks>
/// Based on https://github.com/0xabu/MsBuildCompileCommandsJson, but significantly simplified and customized
/// for clang-uml.
/// </remarks>
public class CompileCommandsLogger : Logger
{
    private static readonly string CCompilerFrontend = "clang.exe";
    private static readonly string CPPCompilerFrontend = "clang++.exe";

    private static readonly string[] SourceFileTypeQualifierFlags = {"/Tc", "/Tp"};

    private static readonly string[] CppExtensions = { ".cpp", ".cxx", ".cc", ".c" };

    public override void Initialize(IEventSource eventSource)
    {
        string outputFilePath = "compile_commands.json.tmp";

        try
        {
            const bool append = false;
            Encoding utf8WithoutBom = new UTF8Encoding(false);
            this.streamWriter = new StreamWriter(outputFilePath, append, utf8WithoutBom);
            this.firstLine = true;
            streamWriter.WriteLine("[");
        }
        catch (Exception ex)
        {
            if (ex is UnauthorizedAccessException
                || ex is ArgumentNullException
                || ex is PathTooLongException
                || ex is DirectoryNotFoundException
                || ex is NotSupportedException
                || ex is ArgumentException
                || ex is SecurityException
                || ex is IOException)
            {
                throw new LoggerException("Failed to create " + outputFilePath + ": " + ex.Message);
            }
            else
            {
                // Unexpected failure
                throw;
            }
        }

        eventSource.AnyEventRaised += EventSource_AnyEventRaised;
    }

    private void EventSource_AnyEventRaised(object sender, BuildEventArgs args)
    {
        if (args is TaskCommandLineEventArgs taskArgs && taskArgs.TaskName == "CL")
        {
            const string clExe = "cl.exe ";
            int clExeIndex = taskArgs.CommandLine.ToLower().IndexOf(clExe);
            if (clExeIndex == -1)
            {
                throw new LoggerException("Unexpected lack of CL.exe in " + taskArgs.CommandLine);
            }

            string compilerPath = taskArgs.CommandLine.Substring(0, clExeIndex + clExe.Length - 1).Replace("\\", "/");
            string argsString = taskArgs.CommandLine.Substring(clExeIndex + clExe.Length).TrimStart();
            string[] cmdArgs = CommandLineToArgs(argsString);

            List<string> filenames = new List<string>();
            List<string> compileFlags = new List<string>();

            // Here we assume that source files are at the end of the command and are absolute
            int argIndex = cmdArgs.Length - 1;
            for(;argIndex >= 0; argIndex--) {
                if(Array.IndexOf(SourceFileTypeQualifierFlags, cmdArgs[argIndex]) > -1) {
                    continue;
                }
                else if(IsCPPSourcePath(cmdArgs[argIndex])) {
                    filenames.Add(cmdArgs[argIndex]);
                }
                else {
                    break;
                }
            }

            for(int i = 0; i < argIndex; i++) {
                if(cmdArgs[i] == "/I") {
                    compileFlags.Add("-I");
                    compileFlags.Add(NormalizeCommandArgument(cmdArgs[i+1]));
                    i++;

                    continue;
                }

                if(cmdArgs[i].StartsWith("/I")) {
                    compileFlags.Add("-I");
                    compileFlags.Add(NormalizeCommandArgument(cmdArgs[i].Substring(2)));

                    continue;
                }

                if(cmdArgs[i] == "/external:I") {
                    compileFlags.Add("-isystem");
                    compileFlags.Add(NormalizeCommandArgument(cmdArgs[i+1]));
                    i++;

                    continue;
                }

                if(cmdArgs[i].StartsWith("/external:I")) {
                    compileFlags.Add("-isystem");
                    compileFlags.Add(NormalizeCommandArgument(cmdArgs[i].Substring(2)));

                    continue;
                }

                if(cmdArgs[i] == "/D") {
                    compileFlags.Add("-D" + cmdArgs[i+1]);
                    i++;

                    continue;
                }

                if(cmdArgs[i].StartsWith("/D")) {
                    compileFlags.Add("-D" + cmdArgs[i].Substring(2));

                    continue;
                }

                if(cmdArgs[i] == "/std:c++14") {
                    compileFlags.Add("-std=c++14");
                    continue;
                }

                if(cmdArgs[i] == "/std:c++17") {
                    compileFlags.Add("-std=c++17");
                    continue;
                }

                if(cmdArgs[i] == "/std:c++20") {
                    compileFlags.Add("-std=c++20");
                    continue;
                }

                if(cmdArgs[i] == "/std:c++23") {
                    compileFlags.Add("-std=c++23");
                    continue;
                }
            }

            filenames.Reverse();

            // simplify the compile command to avoid .. etc.
            string dirname = Path.GetDirectoryName(taskArgs.ProjectFile);

            // For each source file, emit a JSON entry
            foreach (string filename in filenames)
            {
                // Terminate the preceding entry
                if (firstLine)
                {
                    firstLine = false;
                }
                else
                {
                    streamWriter.WriteLine(" ,");
                }

                string directoryPath = HttpUtility.JavaScriptStringEncode(dirname.Replace("\\", "/"));

                string filePath = HttpUtility.JavaScriptStringEncode(NormalizePath(filename));

                string compileCommand = HttpUtility.JavaScriptStringEncode(GetCompilerFrontend(filename) + " " + String.Join(" ", compileFlags));

                string compileCommandWithFilename = compileCommand + " " + filePath;

                // Write one entry
                streamWriter.WriteLine(" {");

                streamWriter.WriteLine(String.Format( "   \"directory\": \"{0}\",", directoryPath));

                streamWriter.Write("   \"arguments\": [");

                streamWriter.Write(string.Format("\"{0}\"", GetCompilerFrontend(filename)));
                foreach (string arg in compileFlags) {
                    streamWriter.Write(string.Format(", \"{0}\"", HttpUtility.JavaScriptStringEncode(arg)));
                }
                streamWriter.Write(string.Format(", \"{0}\"", filename.Replace("\\", "/")));

                streamWriter.WriteLine("],");

                streamWriter.Write(String.Format("   \"file\": \"{0}\"", filename.Replace("\\", "/")));

                streamWriter.WriteLine("");
                streamWriter.WriteLine(" }");
            }
        }
    }

    [DllImport("shell32.dll", SetLastError = true)]
    static extern IntPtr CommandLineToArgvW(
        [MarshalAs(UnmanagedType.LPWStr)] string lpCmdLine, out int pNumArgs);

    static string[] CommandLineToArgs(string commandLine)
    {
        int argc;
        var argv = CommandLineToArgvW(commandLine, out argc);
        if (argv == IntPtr.Zero)
            throw new System.ComponentModel.Win32Exception();
        try
        {
            var args = new string[argc];
            for (var i = 0; i < args.Length; i++)
            {
                var p = Marshal.ReadIntPtr(argv, i * IntPtr.Size);
                args[i] = Marshal.PtrToStringUni(p);
            }

            return args;
        }
        finally
        {
            Marshal.FreeHGlobal(argv);
        }
    }

    public static string GetCompilerFrontend(string filePath) {
        string extension = Path.GetExtension(filePath).ToLowerInvariant();
        if(extension == ".c") {
            return CCompilerFrontend;
        }

        return CPPCompilerFrontend;
    }

    public static bool IsCPPSourcePath(string filePath)
    {
        try {
            if (string.IsNullOrWhiteSpace(filePath))
            {
                return false;
            }

            if(!Path.IsPathRooted(filePath)) {
                return false;
            }

            if(!File.Exists(filePath)) {
                return false;
            }

            string extension = Path.GetExtension(filePath).ToLowerInvariant();
            foreach (string cppExt in CppExtensions)
            {
                if (extension == cppExt)
                {
                    return true;
                }
            }
        }
        catch(Exception) {
            // Ignore non-paths
        }

        return false;
    }

    public static string NormalizeCommandArgument(string input) {
        string result = input.Replace("\\", "/");

        if (string.IsNullOrEmpty(result))
        {
            return result;
        }

        return result;
    }

    public static string NormalizePath(string input)
    {
        string result = input.Replace("\\", "/");

        if (string.IsNullOrEmpty(result))
        {
            return result;
        }

        if (!result.StartsWith("\""))
        {
            result = "\"" + result;
        }

        if (!result.EndsWith("\""))
        {
            result = result + "\"";
        }

        return result;
    }

    public override void Shutdown()
    {
        streamWriter.WriteLine("]");
        streamWriter.Close();

        // Do not overwrite existing compile_commands.json
        if (!File.Exists("compile_commands.json"))
        {
            File.Move("compile_commands.json.tmp", "compile_commands.json");
        }

        base.Shutdown();
    }

    private StreamWriter streamWriter;
    private bool firstLine;
}
