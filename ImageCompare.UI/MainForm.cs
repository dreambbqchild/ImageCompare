using SkiaSharp;
using System;
using System.Linq;
using System.Windows.Forms;

namespace ImageCompare.UI
{
    public partial class MainForm : Form
    {
        private readonly PixelDiffConvertContext context = new PixelDiffConvertContext();

        public MainForm()
        {
            InitializeComponent();
        }

        private void lblDropZone_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
                e.Effect = DragDropEffects.Copy;
        }

        private void lblDropZone_DragDrop(object sender, DragEventArgs e)
        {
            if(!(e.Data.GetData(DataFormats.FileDrop) is string[] filePaths))
                return;

            filmStrip.Clear();
            PixelDiff referenceDiff = null;
            foreach (var path in filePaths)
            {
                var image = SKImage.FromEncodedData(path);
                var bitmap = SKBitmap.FromImage(image);
                var pixelDiff = new PixelDiff(context, bitmap.Bytes, bitmap.Width, bitmap.Height);

                if (referenceDiff is null)
                {
                    referenceDiff = pixelDiff;
                    filmStrip.AddPicture(bitmap, 0.0f);
                    continue;
                }

                var value = referenceDiff.CalcMeanSquaredError(pixelDiff);
                filmStrip.AddPicture(bitmap, value);
            }
        }
    }
}
