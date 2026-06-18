using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Text;

namespace PrivateExec.Bridge {

    // ExecBridge — thin wrapper around the injector EXE + named pipe protocol.
    // Mirrors what VelocityAPI does, but for our own injector + pipe name.

    public class ExecBridge {

        // Injector EXE must be placed next to the UI executable
        private static string InjectorPath =>
            Path.Combine(AppContext.BaseDirectory, "Injector.exe");

        // DLL to map — must also be next to the UI executable
        private static string PayloadPath =>
            Path.Combine(AppContext.BaseDirectory, "Payload.dll");

        // Workspace folder exposed to UNC functions inside the DLL
        public string WorkspaceFolder { get; set; } =
            Path.Combine(AppContext.BaseDirectory, "workspace");

        // ── Inject ────────────────────────────────────────────────────────────
        // Launches Injector.exe <pid> <dll_path> and waits up to 15 seconds.
        public bool Inject(int pid) {
            if (!File.Exists(InjectorPath)) {
                Log($"Injector not found: {InjectorPath}");
                return false;
            }
            if (!File.Exists(PayloadPath)) {
                Log($"Payload not found: {PayloadPath}");
                return false;
            }

            try {
                var psi = new ProcessStartInfo(InjectorPath) {
                    Arguments       = $"{pid} \"{PayloadPath}\"",
                    UseShellExecute = false,
                    CreateNoWindow  = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError  = true,
                };
                using var proc = Process.Start(psi)!;
                proc.WaitForExit(15_000);
                return proc.ExitCode == 0;
            } catch (Exception ex) {
                Log($"Inject exception: {ex.Message}");
                return false;
            }
        }

        // ── Execute ───────────────────────────────────────────────────────────
        // Sends a Base64-encoded Lua script over our named pipe.
        // Pipe name: \\.\pipe\PrivateExec_{pid}
        public bool Execute(string script, int pid) {
            string pipeName = $"PrivateExec_{pid}";
            string b64      = Convert.ToBase64String(Encoding.UTF8.GetBytes(script));
            byte[] payload  = Encoding.UTF8.GetBytes(b64);

            try {
                using var pipe = new NamedPipeClientStream(
                    ".", pipeName, PipeDirection.Out,
                    PipeOptions.None);

                pipe.Connect(3000); // 3-second timeout
                pipe.Write(payload, 0, payload.Length);
                pipe.Flush();
                return true;
            } catch (TimeoutException) {
                Log("Pipe timeout — DLL not ready yet");
                return false;
            } catch (Exception ex) {
                Log($"Execute exception: {ex.Message}");
                return false;
            }
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