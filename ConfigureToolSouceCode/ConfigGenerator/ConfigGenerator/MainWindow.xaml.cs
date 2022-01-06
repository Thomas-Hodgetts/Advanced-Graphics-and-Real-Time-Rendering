using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ConfigGenerator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        void WriteFile(string text)
        {
            if (!System.IO.File.Exists("Settings.txt"))
            {
                System.IO.FileStream fs = System.IO.File.Create("Settings.txt");
                Byte[] title = new UTF8Encoding(true).GetBytes(text);
                fs.Write(title, 0, title.Length);
            }
            else
            {
                 File.Delete("Settings.txt");
                WriteFile(text);
            }
        }

        public MainWindow()
        {
            InitializeComponent();
            SelectBox.Items.Add("Normal Mapping");
            SelectBox.Items.Add("Parallax Mapping");
            SelectBox.Items.Add("Render to Texture");
            SelectBox.Items.Add("Shadow Mapping");
        }
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if(SelectBox.SelectedItem.ToString() == "Normal Mapping")
            {
                WriteFile("NormalMapping");
            }
            if (SelectBox.SelectedItem.ToString() == "Parallax Mapping")
            {
                WriteFile("ParallaxMapping");
            }
            if (SelectBox.SelectedItem.ToString() == "Render to Texture")
            {
                WriteFile("RenderToTexture");
            }
            if (SelectBox.SelectedItem.ToString() == "Shadow Mapping")
            {
                WriteFile("ShadowMapping");
            }
        }
    }
}
