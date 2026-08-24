using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Storage.Pickers;
using Windows.Storage;
using Windows.UI.ViewManagement;
using Microsoft.Gaming.XboxGameBar;
using Windows.UI.Xaml.Media.Imaging;
// https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x804 上介绍了“空白页”项模板

namespace App1
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private async void SelectPNG_Click(object sender, RoutedEventArgs e)
        {
            FileOpenPicker picker = new FileOpenPicker();
            picker.FileTypeFilter.Add(".png");
            StorageFile file = await picker.PickSingleFileAsync();
            if (file != null)
            {
                BitmapImage bitmapImage = new BitmapImage();
                using (var stream = await file.OpenAsync(Windows.Storage.FileAccessMode.Read))
                {

                    await bitmapImage.SetSourceAsync(stream);
                }
                CrosshairIMGplace.Source = bitmapImage;
                alertToSelect.Visibility = Visibility.Collapsed;

            }
        }
    }
}
