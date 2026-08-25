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
                BitmapImage bitmapImage = new BitmapImage();
                using (var stream = await file.OpenAsync(Windows.Storage.FileAccessMode.Read))
                {

                    await bitmapImage.SetSourceAsync(stream);
                }
                CrosshairIMGplace.Source = bitmapImage;
                alertToSelect.Visibility = Visibility.Collapsed;

            }
        }

        public async void hideAllWhenBackground()
        {
            try
            {
                await Log("activate");
                await Dispatcher.RunAsync(
                    Windows.UI.Core.CoreDispatcherPriority.Normal, 
                    () => 
                    {
                        alertToSelect.Visibility = Visibility.Collapsed;
                    });
                
            }
            catch (Exception ex) 
            {
                await Log(ex.ToString());
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
    }
}
