using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace MuInject
{
    public partial class MainForm : Form
    {
        private ComboBox processComboBox;
        private Button injectButton;
        private Button importLicenseButton;
        private TextBox licenseStatusTextBox;
        private Label statusLabel;
        private Label creditsLabel;
        private Timer processRefreshTimer;
        
        private LicenseManager licenseManager;
        private bool isLicenseValid = false;

        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool CloseHandle(IntPtr hObject);

        const int PROCESS_CREATE_THREAD = 0x0002;
        const int PROCESS_QUERY_INFORMATION = 0x0400;
        const int PROCESS_VM_OPERATION = 0x0008;
        const int PROCESS_VM_WRITE = 0x0020;
        const int PROCESS_VM_READ = 0x0010;

        const uint MEM_COMMIT = 0x00001000;
        const uint MEM_RESERVE = 0x00002000;
        const uint PAGE_READWRITE = 4;

        public MainForm()
        {
            InitializeComponent();
            licenseManager = new LicenseManager();
            CheckExistingLicense();
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();

            // Form properties
            this.Text = "MuInject - MuBot Injector";
            this.Size = new Size(500, 400);
            this.BackColor = Color.FromArgb(25, 25, 25);
            this.ForeColor = Color.White;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.StartPosition = FormStartPosition.CenterScreen;

            // Process selection
            var processLabel = new Label
            {
                Text = "Selecionar Processo:",
                Location = new Point(20, 20),
                Size = new Size(150, 20),
                ForeColor = Color.White
            };
            this.Controls.Add(processLabel);

            processComboBox = new ComboBox
            {
                Location = new Point(20, 45),
                Size = new Size(300, 25),
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                DropDownStyle = ComboBoxStyle.DropDownList
            };
            this.Controls.Add(processComboBox);

            var refreshButton = new Button
            {
                Text = "Atualizar",
                Location = new Point(330, 45),
                Size = new Size(80, 25),
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat
            };
            refreshButton.Click += RefreshProcesses_Click;
            this.Controls.Add(refreshButton);

            // License section
            var licenseLabel = new Label
            {
                Text = "Licença:",
                Location = new Point(20, 90),
                Size = new Size(100, 20),
                ForeColor = Color.White
            };
            this.Controls.Add(licenseLabel);

            importLicenseButton = new Button
            {
                Text = "Importar app.licenca",
                Location = new Point(20, 115),
                Size = new Size(150, 30),
                BackColor = Color.FromArgb(80, 80, 80),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat
            };
            importLicenseButton.Click += ImportLicense_Click;
            this.Controls.Add(importLicenseButton);

            licenseStatusTextBox = new TextBox
            {
                Location = new Point(20, 155),
                Size = new Size(440, 80),
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                Multiline = true,
                ReadOnly = true,
                Text = "Nenhuma licença carregada"
            };
            this.Controls.Add(licenseStatusTextBox);

            // Inject button
            injectButton = new Button
            {
                Text = "INJETAR",
                Location = new Point(20, 250),
                Size = new Size(440, 50),
                BackColor = Color.FromArgb(150, 50, 50),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Font = new Font("Arial", 14, FontStyle.Bold),
                Enabled = false
            };
            injectButton.Click += InjectButton_Click;
            this.Controls.Add(injectButton);

            // Status label
            statusLabel = new Label
            {
                Text = "Status: Aguardando licença válida",
                Location = new Point(20, 315),
                Size = new Size(440, 20),
                ForeColor = Color.Yellow
            };
            this.Controls.Add(statusLabel);

            // Credits
            creditsLabel = new Label
            {
                Text = "By CarlosNTC",
                Location = new Point(380, 350),
                Size = new Size(100, 15),
                ForeColor = Color.Gray,
                Font = new Font("Arial", 8)
            };
            this.Controls.Add(creditsLabel);

            // Timer for process refresh
            processRefreshTimer = new Timer
            {
                Interval = 5000 // 5 seconds
            };
            processRefreshTimer.Tick += (s, e) => RefreshProcesses();
            processRefreshTimer.Start();

            this.ResumeLayout(false);
            
            RefreshProcesses();
        }

        private void CheckExistingLicense()
        {
            string licensePath = Path.Combine(Application.StartupPath, "app.licenca");
            if (File.Exists(licensePath))
            {
                LoadLicense(licensePath);
            }
        }

        private void RefreshProcesses_Click(object sender, EventArgs e)
        {
            RefreshProcesses();
        }

        private void RefreshProcesses()
        {
            processComboBox.Items.Clear();
            
            var processes = Process.GetProcesses()
                .Where(p => p.ProcessName.ToLower().Contains("main") || 
                           p.ProcessName.ToLower().Contains("mu"))
                .OrderBy(p => p.ProcessName)
                .ToList();

            foreach (var process in processes)
            {
                try
                {
                    processComboBox.Items.Add($"{process.ProcessName} (PID: {process.Id})");
                }
                catch
                {
                    // Ignore processes we can't access
                }
            }

            if (processComboBox.Items.Count > 0)
            {
                processComboBox.SelectedIndex = 0;
            }
        }

        private void ImportLicense_Click(object sender, EventArgs e)
        {
            using (var openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "License files (*.licenca)|*.licenca|All files (*.*)|*.*";
                openFileDialog.Title = "Selecionar arquivo de licença";

                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    LoadLicense(openFileDialog.FileName);
                }
            }
        }

        private void LoadLicense(string filePath)
        {
            try
            {
                var licenseInfo = licenseManager.ValidateLicense(filePath);
                
                if (licenseInfo != null)
                {
                    isLicenseValid = true;
                    
                    var daysRemaining = (licenseInfo.Validade - DateTime.Now).Days;
                    var statusColor = daysRemaining < 5 ? Color.Red : Color.Green;
                    
                    licenseStatusTextBox.Text = $"Usuário: {licenseInfo.Usuario}\n" +
                                              $"Validade: {licenseInfo.Validade:dd/MM/yyyy}\n" +
                                              $"Dias restantes: {daysRemaining}\n" +
                                              $"PCs permitidos: {licenseInfo.QuantidadePCs}\n" +
                                              $"Status: VÁLIDA";
                    licenseStatusTextBox.ForeColor = statusColor;
                    
                    statusLabel.Text = "Status: Licença válida - Pronto para injetar";
                    statusLabel.ForeColor = Color.Green;
                    
                    injectButton.Enabled = true;
                    injectButton.BackColor = Color.FromArgb(50, 150, 50);
                    
                    // Copy license to application directory
                    string targetPath = Path.Combine(Application.StartupPath, "app.licenca");
                    if (filePath != targetPath)
                    {
                        File.Copy(filePath, targetPath, true);
                    }
                }
                else
                {
                    ShowLicenseError("Licença inválida ou expirada");
                }
            }
            catch (Exception ex)
            {
                ShowLicenseError($"Erro ao carregar licença: {ex.Message}");
            }
        }

        private void ShowLicenseError(string message)
        {
            isLicenseValid = false;
            licenseStatusTextBox.Text = message;
            licenseStatusTextBox.ForeColor = Color.Red;
            statusLabel.Text = "Status: Erro na licença";
            statusLabel.ForeColor = Color.Red;
            injectButton.Enabled = false;
            injectButton.BackColor = Color.FromArgb(150, 50, 50);
        }

        private void InjectButton_Click(object sender, EventArgs e)
        {
            if (!isLicenseValid)
            {
                MessageBox.Show("Licença inválida!", "Erro", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (processComboBox.SelectedItem == null)
            {
                MessageBox.Show("Selecione um processo!", "Erro", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            string dllPath = Path.Combine(Application.StartupPath, "syslib.dll");
            if (!File.Exists(dllPath))
            {
                MessageBox.Show("syslib.dll não encontrada!", "Erro", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                string selectedProcess = processComboBox.SelectedItem.ToString();
                int pid = int.Parse(selectedProcess.Split('(')[1].Split(')')[0].Replace("PID: ", ""));
                
                if (InjectDLL(pid, dllPath))
                {
                    statusLabel.Text = "Status: DLL injetada com sucesso!";
                    statusLabel.ForeColor = Color.Green;
                    MessageBox.Show("Injeção realizada com sucesso!\nPressione F5 no jogo para abrir o menu.", 
                                  "Sucesso", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    statusLabel.Text = "Status: Falha na injeção";
                    statusLabel.ForeColor = Color.Red;
                    MessageBox.Show("Falha ao injetar DLL!", "Erro", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Erro durante injeção: {ex.Message}", "Erro", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private bool InjectDLL(int processId, string dllPath)
        {
            IntPtr hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
                                        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, 
                                        false, processId);
            
            if (hProcess == IntPtr.Zero)
                return false;

            try
            {
                IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
                if (loadLibraryAddr == IntPtr.Zero)
                    return false;

                byte[] dllPathBytes = Encoding.ASCII.GetBytes(dllPath);
                IntPtr allocMemAddress = VirtualAllocEx(hProcess, IntPtr.Zero, (uint)dllPathBytes.Length, 
                                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                
                if (allocMemAddress == IntPtr.Zero)
                    return false;

                if (!WriteProcessMemory(hProcess, allocMemAddress, dllPathBytes, (uint)dllPathBytes.Length, out _))
                    return false;

                IntPtr hThread = CreateRemoteThread(hProcess, IntPtr.Zero, 0, loadLibraryAddr, 
                                                  allocMemAddress, 0, IntPtr.Zero);
                
                return hThread != IntPtr.Zero;
            }
            finally
            {
                CloseHandle(hProcess);
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            processRefreshTimer?.Stop();
            processRefreshTimer?.Dispose();
            base.OnFormClosing(e);
        }
    }
}