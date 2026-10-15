using Microsoft.Gaming.XboxGameBar;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;
using App1;
using Windows.UI.ViewManagement;
using Windows.UI;
using System.Drawing;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace WidgetSampleCS
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Widget1 : Page
    {
        
        public Widget1()
        {
            this.InitializeComponent();
            
        }
        private async void SelectPNG_Click(Object sender, RoutedEventArgs e)
        {
            FileOpenPicker picker = new FileOpenPicker();
            picker.FileTypeFilter.Add(".png");
            StorageFile file = await picker.PickSingleFileAsync();
            if (file != null)
            {
                setCrosshairImg(file);
                ///缓存图片
                StorageFolder localFolder = ApplicationData.Current.LocalFolder;
                StorageFile imgCache = await localFolder.CreateFileAsync("imgCache.png", CreationCollisionOption.ReplaceExisting);
                await file.CopyAndReplaceAsync(imgCache);
            }
        }

        public async void hideOrShow(bool whetherToShow)
        {
            if (!whetherToShow)
            {
                try
                {

                    await Dispatcher.RunAsync(
                        Windows.UI.Core.CoreDispatcherPriority.Normal,
                        () =>
                        {
                            menubar.Visibility = Visibility.Collapsed;
                            centerScreenButton.Visibility = Visibility.Collapsed;
                            foundationGrid.Background = null;
                            tipBar.Visibility = Visibility.Collapsed;
                            settingsButton.Visibility = Visibility.Collapsed;
                            settingsPanel.Visibility = Visibility.Collapsed;

                        });

                }
                catch (Exception ex)
                {
                    await Log(ex.ToString());
                }
            }
            else
            {
                try
                {

                    await Dispatcher.RunAsync(
                        Windows.UI.Core.CoreDispatcherPriority.Normal,
                        () =>
                        {
                            menubar.Visibility = Visibility.Visible;
                            centerScreenButton.Visibility = Visibility.Visible;
                            foundationGrid.Background = new SolidColorBrush(Windows.UI.Colors.White);
                            tipBar.Visibility = Visibility.Visible;
                            settingsButton.Visibility = Visibility.Visible;
                        });

                }
                catch (Exception ex)
                {
                    await Log(ex.ToString());
                }
            }
            
        }
        private async Task Log(string message)
        {
                StorageFolder folder =
                    ApplicationData.Current.LocalFolder;

                StorageFile file =
                    await folder.CreateFileAsync(
                        "log.txt",
                        CreationCollisionOption.OpenIfExists);

                await FileIO.AppendTextAsync(
                    file,
                    DateTime.Now.ToString("HH:mm:ss.fff")
                    + " "
                    + message
                    + "\r\n");
            
            
        }

        private async void centerScreenButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var currentApp = Application.Current as App;
                await Dispatcher.RunAsync(
                       Windows.UI.Core.CoreDispatcherPriority.Normal,
                       async () =>
                       {
                           await currentApp.setAsCenter();
                       });
            }
            catch (Exception ex)
            {
                await Log(ex.ToString());
            }
        }

        // Settings remain available when returning to the main view.
        public int UserFov => (int)Math.Round(fovSlider.Value);
        public Windows.Foundation.Size UserScreenResolution
        {
            get
            {
                switch (resolutionSelector.SelectedIndex)
                {
                    case 0: return new Windows.Foundation.Size(3840, 2160);
                    case 1: return new Windows.Foundation.Size(2560, 1440);
                    default: return new Windows.Foundation.Size(1920, 1080);
                }
            }
        }
        public int UserMagnification => int.Parse(((ComboBoxItem)magnificationSelector.SelectedItem).Content.ToString());
        public int UserZeroingDistance => int.Parse(((ComboBoxItem)zeroingSelector.SelectedItem).Content.ToString());

        private void settingsButton_Click(object sender, RoutedEventArgs e)
        {
            settingsPanel.Visibility = Visibility.Visible;
        }

        private void SettingsBack_Click(object sender, RoutedEventArgs e)
        {
            settingsPanel.Visibility = Visibility.Collapsed;
        }

        public async void setCrosshairImg(StorageFile crosshairFile)
        {
            if (crosshairFile == null) return;
            await Dispatcher.RunAsync(
                        Windows.UI.Core.CoreDispatcherPriority.Normal,
                        async () =>
                        {
                            BitmapImage bitmapImage = new BitmapImage();
                            using (var stream = await crosshairFile.OpenAsync(Windows.Storage.FileAccessMode.Read))
                            {

                                await bitmapImage.SetSourceAsync(stream);
                            }
                            CrosshairIMGplace.Source = bitmapImage;
                            var imgSize = new Windows.Foundation.Size(bitmapImage.PixelWidth, bitmapImage.PixelHeight);
                            alertToSelect.Visibility = Visibility.Collapsed;
                            var appObject = Application.Current as App;
                            appObject.resizeWidget(imgSize);
                        });
            
        }
    }
}
