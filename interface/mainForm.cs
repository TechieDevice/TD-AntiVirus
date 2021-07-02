using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;
using Newtonsoft.Json;
using System.Security;

namespace @interface
{
    public partial class mainForm : Form
    {
        private List<string> detectedList = new List<string>();
        PermissionSet perms = new PermissionSet(null);
        string appSettings;
        string quarantine;
        string core;

        public mainForm()
        {
            var me = Directory.GetCurrentDirectory();

            appSettings = Path.Combine(me, "appSettings.json");
            quarantine = Path.Combine(me, "quarantine");
            core = Path.Combine(me, "core");

            if (!Directory.Exists(quarantine))
            {
                var dir = new FileInfo(quarantine);
                dir.Directory.CreateSubdirectory("quarantine");
            }

            if (!File.Exists(appSettings))
            {
                Aes aes = Aes.Create();
                aes.KeySize = 256;
                aes.GenerateKey();

                Key key = new Key
                {
                    key = aes.Key,
                    iv = aes.IV
                };

                using (StreamWriter file = File.CreateText(appSettings))
                {
                    JsonSerializer serializer = new JsonSerializer();
                    serializer.Serialize(file, key);
                }
            }

            InitializeComponent();
        }

        private void pathButton_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog.ShowDialog() == DialogResult.Cancel)
                return;
            string name = folderBrowserDialog.SelectedPath;
            pathText.Text = name;
        }

        private void startButton_Click(object sender, EventArgs e)
        {
            if (!Directory.Exists(pathText.Text))
            {
                MessageBox.Show("Указанная папка не существует");
                return;
            }

            if (!File.Exists(Path.Combine(core, "core.dll")))
            {
                MessageBox.Show("Ошибка: отсутствует библиотека core.dll");
                return;
            }

            try
            {
                detectedList = new List<string>();
                Core.AVstart(core, pathText.Text);
            }
            catch
            {
                MessageBox.Show("Ошибка работы библиотеки");
                return;
            }

            string detected = "";


            while (true)
            {
                byte[] byteString = new byte[128];

                Core.GetNextString(byteString);

                if (byteString[0] == 0) break;

                string dataString = Encoding.UTF8.GetString(byteString);

                int offset = dataString.IndexOf("\0");
                if (offset >= 0)
                    dataString = dataString.Substring(0, offset);

                if (dataString == "sign DB error")
                {
                    MessageBox.Show("Ошибка: отсутствует база сигнатур sign.sdb");
                    return;
                }
                if (dataString == "hash DB error")
                {
                    MessageBox.Show("Ошибка: отсутствует база сигнатур hash.hdb");
                    return;
                }
                if (dataString == "no viruses detected")
                {
                    MessageBox.Show("Вредоносных файлов не обнаружено");
                    return;
                }

                detected = detected + dataString + " - ";

                Core.GetNextString(byteString);

                if (byteString[0] == 0) break;

                dataString = Encoding.UTF8.GetString(byteString);
                offset = dataString.IndexOf("\0");
                if (offset >= 0)
                    dataString = dataString.Substring(0, offset);
                detected = detected + dataString + "\n";

                detectedList.Add(dataString);
            }

            textBox.Text = detected;
        }

        private void DeleteButton_Click(object sender, EventArgs e)
        {
            foreach (string path in detectedList)
            {
                if (!File.Exists(path)) continue;
                File.SetAttributes(path, FileAttributes.Normal);
                File.Delete(path);
            }
            detectedList.Clear();

            //Core.Delete();
        }

        private async void encryptButton_Click(object sender, EventArgs e)
        {
            if (!File.Exists(appSettings))
            {
                MessageBox.Show("Ошибка: отсутствует appSettings.json");
                return;
            }

            Key key = new Key();
            using (StreamReader file = File.OpenText(appSettings))
            {
                key = JsonConvert.DeserializeObject<Key>(file.ReadToEnd());
            }

            foreach (string path in detectedList)
            {
                if (!File.Exists(path)) continue;
                    
                byte[] encryptData;

                using (FileStream file = File.Open(path, FileMode.Open))
                {
                    int size = (int)file.Length;
                    byte[] filedata = new byte[size + 1];
                    await file.ReadAsync(filedata, 0, size);
                    encryptData = new byte[0];

                    int blocks = (size / 256) + 1;
                    byte[] encryptDataBlock;
                    for (int i = 1; i <= blocks; i++)
                    {
                        int blockSize;
                        if (i != (blocks)) blockSize = 256;
                        else blockSize = size % 256;
                        encryptDataBlock = new byte[blockSize + 1];

                        byte[] blockData = new byte[blockSize + 1];

                        for (int j = 0; j < blockSize; j++)
                        {
                            blockData[j] = filedata[i * (j)];
                        }

                        string unicodebyte = Encoding.Unicode.GetString(blockData);
                        encryptDataBlock = EncryptAes(unicodebyte, key.key, key.iv);

                        //string decryptDataBlockUnicode = DecryptAes(encryptDataBlock, key.key, key.iv);
                        //byte[] decryptDataBlock = Encoding.Unicode.GetBytes(decryptDataBlockUnicode);

                        encryptData = encryptData.Concat(encryptDataBlock).ToArray();
                    }
                }

                var oldPath = new FileInfo(path);
                oldPath.Attributes = FileAttributes.Normal;
                var newPath = quarantine + "/" + oldPath.Name;
                using (FileStream file = File.Open(newPath, FileMode.Create))
                {
                    using (BinaryWriter writer = new BinaryWriter(file))
                    {
                        writer.Write(encryptData);
                    }
                }

                File.Delete(oldPath.FullName);
            }

            //Core.Encrypt(key.key, key.iv);
        }

        private async void decryptButton_Click(object sender, EventArgs e)
        {
            if (!File.Exists(appSettings))
            {
                MessageBox.Show("Ошибка: отсутствует appSettings.json");
                return;
            }

            Key key = new Key();
            using (StreamReader file = File.OpenText(appSettings))
            {
                key = JsonConvert.DeserializeObject<Key>(file.ReadToEnd());
            }

            var files = Directory.GetFiles(quarantine);

            foreach (string path in files)
            {
                byte[] decryptData;
                using (FileStream file = File.Open(path, FileMode.Open))
                {
                    int size = (int)file.Length;
                    byte[] filedata = new byte[size + 1];
                    await file.ReadAsync(filedata, 0, size);
                    decryptData = new byte[0];

                    int blocks = (size / 256) + 1;
                    byte[] decryptDataBlock;
                    for (int i = 1; i <= blocks; i++)
                    {
                        int blockSize;
                        if (i != (blocks)) blockSize = 256;
                        else blockSize = size % 256;
                        decryptDataBlock = new byte[blockSize + 1];

                        byte[] blockData = new byte[blockSize];

                        for (int j = 0; j < blockSize; j++)
                        {
                            blockData[j] = filedata[i * (j)];
                        }

                        string decryptDataBlockUnicode = DecryptAes(blockData, key.key, key.iv);
                        decryptDataBlock = Encoding.Unicode.GetBytes(decryptDataBlockUnicode);

                        decryptData = decryptData.Concat(decryptDataBlock).ToArray();
                    }
                }

                var oldPath = new FileInfo(path);
                oldPath.Attributes = FileAttributes.Normal;
                var newPath = Path.Combine(quarantine, oldPath.Name);
                using (FileStream file = File.Open(newPath, FileMode.Create))
                {
                    using (BinaryWriter writer = new BinaryWriter(file))
                    {
                        writer.Write(decryptData);
                    }
                }
            }
        }


        private byte[] EncryptAes(string text, byte[] Key, byte[] IV)
        {
            byte[] data;
            using (Aes aesAlg = Aes.Create())
            {
                aesAlg.Key = Key;
                aesAlg.IV = IV;
                //byte[] UnicodeText = Encoding.Unicode.GetBytes(text);
                //string base64Text = Convert.ToBase64String(UnicodeText);

                ICryptoTransform encryptor = aesAlg.CreateEncryptor(aesAlg.Key, aesAlg.IV);

                // Create the streams used for encryption.
                using (MemoryStream msEncrypt = new MemoryStream())
                {
                    using (CryptoStream csEncrypt = new CryptoStream(msEncrypt, encryptor, CryptoStreamMode.Write))
                    {
                        using (StreamWriter swEncrypt = new StreamWriter(csEncrypt))
                        {
                            //Write all data to the stream.
                            swEncrypt.Write(text);
                        }
                        data = msEncrypt.ToArray();
                    }
                }
            }
            return data;
        }

        private string DecryptAes(byte[] text, byte[] Key, byte[] IV)
        {
            string data = null;
            using (Aes aesAlg = Aes.Create())
            {
                aesAlg.Key = Key;
                aesAlg.IV = IV;

                ICryptoTransform decryptor = aesAlg.CreateDecryptor(aesAlg.Key, aesAlg.IV);

                // Create the streams used for decryption.
                using (MemoryStream msDecrypt = new MemoryStream(text))
                {
                    using (CryptoStream csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Read))
                    {
                        using (StreamReader srDecrypt = new StreamReader(csDecrypt))
                        {

                            // Read the decrypted bytes from the decrypting stream
                            // and place them in a string.
                            data = srDecrypt.ReadToEnd();
                        }
                    }
                }
            }
            return data;
        }
    }

    public class Key
    {
        public byte[] key;
        public byte[] iv;
    }

    public static class Core
    {
        [DllImport("core/core.dll", EntryPoint = "AVstart")]
        public static extern void AVstart(string DBPath, string Path);

        [DllImport("core/core.dll", EntryPoint = "GetNextString")]
        public static extern void GetNextString(byte[] byteString);

        //[DllImport("core/core.dll", EntryPoint = "Delete")]
        //public static extern void Delete();

        //[DllImport("core/core.dll", EntryPoint = "Encrypt")]
        //public static extern void Encrypt(byte[] Key, byte[] IV);
    }   
}
