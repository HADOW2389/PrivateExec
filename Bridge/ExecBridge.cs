using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading;

namespace PrivateExec.Bridge {

    // ExecBridge — thin wrapper around the injector EXE + named pipe protocol.
    // Mirrors what VelocityAPI does, but for our own injector + pipe name.

    public class ExecBridge {

        // Directory of the actual EXE (not the single-file temp extraction folder).
        // AppContext.BaseDirectory points to %TEMP%\.net\... for single-file apps.
        private static string ExeDir =>
            Path.GetDirectoryName(Environment.ProcessPath
                ?? System.Diagnostics.Process.GetCurrentProcess().MainModule!.FileName)!;

        // Injector EXE must be placed next to the UI executable
        private static string InjectorPath =>
            Path.Combine(ExeDir, "Injector.exe");

        // DLL to map — must also be next to the UI executable
        private static string PayloadPath =>
            Path.Combine(ExeDir, "Payload.dll");

        // Workspace folder exposed to UNC functions inside the DLL
        public string WorkspaceFolder { get; set; }

        public ExecBridge() {
            WorkspaceFolder = Path.Combine(ExeDir, "workspace");
        }

        // Path where Injector.exe writes its diagnostic log
        private static string LogPath =>
            Path.Combine(Path.GetTempPath(), "PrivateExec_inject.log");

        // Last log content — UI can read this to show the actual error
        public string LastInjectLog { get; private set; } = "";

        // ── Inject ────────────────────────────────────────────────────────────
        // Launches Injector.exe <pid> <dll_path> and waits up to 15 seconds.
        public bool Inject(int pid) {
            if (!File.Exists(InjectorPath)) {
                LastInjectLog = $"Injector not found:\n{InjectorPath}";
                Log(LastInjectLog);
                return false;
            }
            if (!File.Exists(PayloadPath)) {
                LastInjectLog = $"Payload not found:\n{PayloadPath}";
                Log(LastInjectLog);
                return false;
            }

            // Clear previous log
            try { File.Delete(LogPath); } catch { }

            try {
                var psi = new ProcessStartInfo(InjectorPath) {
                    Arguments       = $"{pid} \"{PayloadPath}\"",
                    UseShellExecute = true,
                    WindowStyle     = ProcessWindowStyle.Hidden,
                };
                using var proc = Process.Start(psi)!;
                proc.WaitForExit(15_000);

                // Read log written by Injector
                try {
                    LastInjectLog = File.Exists(LogPath)
                        ? File.ReadAllText(LogPath)
                        : $"(no log) exit={proc.ExitCode}";
                } catch {
                    LastInjectLog = $"exit={proc.ExitCode}";
                }

                Log($"Inject exit={proc.ExitCode}\n{LastInjectLog}");
                return proc.ExitCode == 0;
            } catch (Exception ex) {
                LastInjectLog = $"Launch failed: {ex.Message}";
                Log(LastInjectLog);
                return false;
            }
        }

        // ── Execute ───────────────────────────────────────────────────────────
        // Drops a Base64-encoded script into %TEMP%\PrivateExec_{pid}.b64.
        // The DLL polls that file every 300ms, reads it, deletes it, executes.
        // No named pipe needed — works even if Hyperion blocks CreateNamedPipe.
        public bool Execute(string script, int pid) {
            string cmdFile = Path.Combine(
                Path.GetTempPath(), $"PrivateExec_{pid}.b64");
            string b64 = Convert.ToBase64String(Encoding.UTF8.GetBytes(script));

            for (int i = 0; i < 8; i++) {
                try {
                    File.WriteAllText(cmdFile, b64);
                    Log($"Execute: wrote {cmdFile}");
                    return true;
                } catch (Exception ex) {
                    Log($"Execute write attempt {i}: {ex.Message}");
                    Thread.Sleep(200);
                }
            }
            Log("Execute: failed to write command file");
            return false;
        }

        // ── Set workspace folder on the DLL side ─────────────────────────────
        public bool SetWorkspaceFolder(string path, int pid) {
            // Protocol: send "setworkspacefolder: <path>" as Base64
            return Execute($"setworkspacefolder: {path}", pid);
        }

        private static void Log(string msg)
            => Debug.WriteLine($"[ExecBridge] {msg}");
    }
}