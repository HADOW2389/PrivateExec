using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using Microsoft.Win32;
using PrivateExec.Bridge;

namespace PrivateExec {

    public partial class MainWindow : Window {

        private readonly ExecBridge _bridge = new();
        private readonly AppSettings _settings = AppSettings.Load();
        private CancellationTokenSource _watcherCts = new();

        // Currently attached process info
        private int  _attachedPid = 0;
        private bool _injected    = false;

        // Tab management
        private readonly List<TabEntry> _tabs = new();
        private int _activeTab = -1;

        record TabEntry(string Title, string Content);

        // ── Init ─────────────────────────────────────────────────────────────
        public MainWindow() {
            InitializeComponent();
            ApplySettings();
            AddTab("Script 1", "-- PrivateExec | version-8884371d30284041\nprint(\"Hello!\")");
            StartProcessWatcher();

            // Make sure workspace folder exists
            Directory.CreateDirectory(_bridge.WorkspaceFolder);
            RefreshScriptList();
        }

        // ── Titlebar ─────────────────────────────────────────────────────────
        private void Titlebar_MouseDown(object s, MouseButtonEventArgs e) {
            if (e.ChangedButton == MouseButton.Left) DragMove();
        }

        private void MinBtn_Click(object s, RoutedEventArgs e)
            => WindowState = WindowState.Minimized;

        private void CloseBtn_Click(object s, RoutedEventArgs e) {
            _settings.Save();
            Application.Current.Shutdown();
        }

        // ── Sidebar navigation ────────────────────────────────────────────────
        private void ShowPage(UIElement page) {
            PageEditor.Visibility   = Visibility.Collapsed;
            PageScripts.Visibility  = Visibility.Collapsed;
            PageSettings.Visibility = Visibility.Collapsed;
            page.Visibility = Visibility.Visible;
        }

        private void SideHome_Click(object s, MouseButtonEventArgs e)    => ShowPage(PageEditor);
        private void SideScripts_Click(object s, MouseButtonEventArgs e) { RefreshScriptList(); ShowPage(PageScripts); }
        private void SideSettings_Click(object s, MouseButtonEventArgs e) => ShowPage(PageSettings);

        // ── Tabs ─────────────────────────────────────────────────────────────
        private void AddTab(string title, string content = "") {
            int idx = _tabs.Count;
            _tabs.Add(new TabEntry(title, content));

            var btn = new Border {
                Background   = idx == 0 ? new SolidColorBrush(Color.FromRgb(30, 30, 50))
                                        : Brushes.Transparent,
                CornerRadius = new CornerRadius(5),
                Padding      = new Thickness(14, 0, 14, 0),
                Margin       = new Thickness(2, 6, 0, 6),
                Cursor       = Cursors.Hand,
                Tag          = idx,
                Child        = new TextBlock {
                    Text         = title,
                    Foreground   = Brushes.Silver,
                    FontSize     = 12,
                    VerticalAlignment = VerticalAlignment.Center,
                }
            };
            btn.MouseLeftButtonDown += (_, _) => SwitchTab((int)btn.Tag);
            TabBar.Children.Add(btn);

            SwitchTab(idx);
        }

        private void SwitchTab(int idx) {
            if (idx < 0 || idx >= _tabs.Count) return;
            if (_activeTab >= 0 && _activeTab < _tabs.Count)
                _tabs[_activeTab] = _tabs[_activeTab] with { Content = ScriptEditor.Text };

            _activeTab = idx;
            ScriptEditor.Text = _tabs[idx].Content;

            // Highlight active tab border
            for (int i = 0; i < TabBar.Children.Count; i++) {
                var b = (Border)TabBar.Children[i];
                b.Background = i == idx
                    ? new SolidColorBrush(Color.FromRgb(30, 30, 50))
                    : Brushes.Transparent;
            }
        }

        // ── Editor action buttons ─────────────────────────────────────────────
        private void ClearBtn_Click(object s, RoutedEventArgs e)
            => ScriptEditor.Clear();

        private void OpenFileBtn_Click(object s, RoutedEventArgs e) {
            var dlg = new OpenFileDialog {
                Filter = "Lua scripts (*.lua;*.txt)|*.lua;*.txt|All files (*.*)|*.*"
            };
            if (dlg.ShowDialog() == true) {
                ScriptEditor.Text = File.ReadAllText(dlg.FileName);
                if (_activeTab >= 0)
                    _tabs[_activeTab] = _tabs[_activeTab] with {
                        Title   = Path.GetFileName(dlg.FileName),
                        Content = ScriptEditor.Text
                    };
            }
        }

        private void SaveBtn_Click(object s, RoutedEventArgs e) {
            var dlg = new SaveFileDialog {
                Filter   = "Lua script (*.lua)|*.lua|Text file (*.txt)|*.txt",
                FileName = _tabs.Count > 0 ? _tabs[_activeTab].Title : "script.lua"
            };
            if (dlg.ShowDialog() == true)
                File.WriteAllText(dlg.FileName, ScriptEditor.Text);
        }

        // ── Inject ────────────────────────────────────────────────────────────
        private async void InjectBtn_Click(object s, RoutedEventArgs e) {
            InjectBtn.IsEnabled = false;
            SetStatus("Injecting...", "#FAA61A");

            int pid = FindRobloxPid();
            if (pid == 0) {
                SetStatus("Roblox not found", "#ED4245");
                InjectBtn.IsEnabled = true;
                return;
            }

            bool ok = await Task.Run(() => _bridge.Inject(pid));
            if (ok) {
                _attachedPid = pid;
                _injected    = true;
                SetStatus($"Injected → PID {pid}", "#3BA55D");
                ProcInfo.Text = $"RobloxPlayerBeta  PID {pid}";
                ExecBtn.IsEnabled = true;
            } else {
                SetStatus("Injection failed", "#ED4245");
                InjectBtn.IsEnabled = true;
            }
        }

        // ── Execute ───────────────────────────────────────────────────────────
        private async void ExecBtn_Click(object s, RoutedEventArgs e) {
            if (!_injected || _attachedPid == 0) {
                SetStatus("Not injected", "#ED4245");
                return;
            }

            // Save current tab content
            if (_activeTab >= 0)
                _tabs[_activeTab] = _tabs[_activeTab] with { Content = ScriptEditor.Text };

            string script = ScriptEditor.Text;
            if (string.IsNullOrWhiteSpace(script)) return;

            ExecBtn.IsEnabled = false;
            bool ok = await Task.Run(() => _bridge.Execute(script, _attachedPid));
            ExecBtn.IsEnabled = true;

            if (!ok) SetStatus("Execute failed — pipe error", "#ED4245");
        }

        // ── Script list (workspace folder) ────────────────────────────────────
        private void RefreshScriptList() {
            ScriptList.Children.Clear();
            var folder = _bridge.WorkspaceFolder;
            if (!Directory.Exists(folder)) return;

            foreach (var file in Directory.EnumerateFiles(folder, "*.lua")) {
                var name = Path.GetFileName(file);
                var card = new Border {
                    Background   = new SolidColorBrush(Color.FromRgb(17, 17, 17)),
                    BorderBrush  = new SolidColorBrush(Color.FromRgb(30, 30, 30)),
                    BorderThickness = new Thickness(1),
                    CornerRadius = new CornerRadius(8),
                    Padding      = new Thickness(14, 10, 14, 10),
                    Cursor       = Cursors.Hand,
                };
                var row = new Grid();
                row.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
                row.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });

                var lbl = new TextBlock {
                    Text       = name,
                    Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
                    FontSize   = 13,
                    VerticalAlignment = VerticalAlignment.Center,
                };
                row.Children.Add(lbl);

                var execBtn = new Button {
                    Content         = "Execute",
                    Padding         = new Thickness(10, 5, 10, 5),
                    FontSize        = 11,
                    Foreground      = Brushes.White,
                    Background      = new SolidColorBrush(Color.FromRgb(88, 101, 242)),
                    BorderThickness = new Thickness(0),
                    Cursor          = Cursors.Hand,
                    Tag             = file,
                };
                Grid.SetColumn(execBtn, 1);
                execBtn.Click += (_, _) => {
                    var src = File.ReadAllText((string)execBtn.Tag);
                    ScriptEditor.Text = src;
                    ShowPage(PageEditor);
                    if (_injected) _bridge.Execute(src, _attachedPid);
                };
                row.Children.Add(execBtn);

                card.Child = row;
                ScriptList.Children.Add(card);
            }
        }

        // ── Settings toggles ──────────────────────────────────────────────────
        private void ApplySettings() {
            ToggleFPS.IsChecked        = _settings.UnlockFPS;
            ToggleAutoInject.IsChecked = _settings.AutoInject;
            ToggleTopmost.IsChecked    = _settings.AlwaysOnTop;
            Topmost = _settings.AlwaysOnTop;
            ExecBtn.IsEnabled = false;
        }

        private void ToggleFPS_Changed(object s, RoutedEventArgs e) {
            _settings.UnlockFPS = ToggleFPS.IsChecked == true;
            _settings.Save();
            if (_settings.UnlockFPS && _injected)
                _bridge.Execute("-- FPS unlock applied via offsets in DLL", _attachedPid);
        }

        private void ToggleAutoInject_Changed(object s, RoutedEventArgs e) {
            _settings.AutoInject = ToggleAutoInject.IsChecked == true;
            _settings.Save();
        }

        private void ToggleTopmost_Changed(object s, RoutedEventArgs e) {
            _settings.AlwaysOnTop = ToggleTopmost.IsChecked == true;
            Topmost = _settings.AlwaysOnTop;
            _settings.Save();
        }

        // ── Process watcher ───────────────────────────────────────────────────
        private void StartProcessWatcher() {
            var token = _watcherCts.Token;
            Task.Run(async () => {
                while (!token.IsCancellationRequested) {
                    await Task.Delay(2000, token);
                    int pid = FindRobloxPid();

                    Dispatcher.Invoke(() => {
                        if (pid == 0) {
                            if (_injected) {
                                _injected    = false;
                                _attachedPid = 0;
                                SetStatus("Roblox closed", "#808080");
                                ProcInfo.Text     = "";
                                ExecBtn.IsEnabled = false;
                                InjectBtn.IsEnabled = true;
                            }
                        } else {
                            if (!_injected) {
                                ProcInfo.Text = $"RobloxPlayerBeta  PID {pid}";
                                if (_settings.AutoInject) {
                                    SetStatus("Auto-injecting...", "#FAA61A");
                                    _ = Task.Run(() => {
                                        bool ok = _bridge.Inject(pid);
                                        Dispatcher.Invoke(() => {
                                            if (ok) {
                                                _attachedPid = pid;
                                                _injected    = true;
                                                SetStatus($"Auto-injected → PID {pid}", "#3BA55D");
                                                ExecBtn.IsEnabled = true;
                                            } else {
                                                SetStatus("Auto-inject failed", "#ED4245");
                                            }
                                        });
                                    });
                                }
                            }
                        }
                    });
                }
            }, token);
        }

        // ── Status bar helpers ────────────────────────────────────────────────
        private void SetStatus(string text, string colorHex) {
            StatusText.Text = text;
            StatusDotColor.Color = (Color)ColorConverter.ConvertFromString(colorHex);
        }

        private static int FindRobloxPid() {
            var procs = Process.GetProcessesByName("RobloxPlayerBeta");
            return procs.Length > 0 ? procs[0].Id : 0;
        }

        protected override void OnClosed(EventArgs e) {
            _watcherCts.Cancel();
            _settings.Save();
            base.OnClosed(e);
        }
    }

    // ── App settings ─────────────────────────────────────────────────────────
    public class AppSettings {
        private static readonly string Path =
            System.IO.Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "PrivateExec", "settings.json");

        public bool UnlockFPS    { get; set; } = true;
        public bool AutoInject   { get; set; } = false;
        public bool AlwaysOnTop  { get; set; } = false;

        public static AppSettings Load() {
            try {
                if (File.Exists(Path))
                    return System.Text.Json.JsonSerializer.Deserialize<AppSettings>(File.ReadAllText(Path))!;
            } catch { }
            return new AppSettings();
        }

        public void Save() {
            try {
                Directory.CreateDirectory(System.IO.Path.GetDirectoryName(Path)!);
                File.WriteAllText(Path, System.Text.Json.JsonSerializer.Serialize(this,
                    new System.Text.Json.JsonSerializerOptions { WriteIndented = true }));
            } catch { }
        }
    }
}