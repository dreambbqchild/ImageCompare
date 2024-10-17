using SkiaSharp;
using System.Windows.Forms;

namespace ImageCompare.UI
{
    public partial class MainForm : Form
    {
        private readonly PixelDiffConvertContext context = new PixelDiffConvertContext(ConvertContext.ComputeShader);

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
                var pixelDiff = new PixelDiff(context, bitmap.Bytes, bitmap.Width, bitmap.Height, bitmap.BytesPerPixel);

                if (referenceDiff is null)
                {
                    referenceDiff = pixelDiff;
                    filmStrip.AddPicture(bitmap, 0.0f);
                    continue;
                }

                var value = referenceDiff.CalcMeanSquaredError(pixelDiff);
                filmStrip.AddPicture(bitmap, value);

                referenceDiff = pixelDiff;
            }
        }
    }
}
