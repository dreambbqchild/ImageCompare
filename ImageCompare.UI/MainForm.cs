using SkiaSharp;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ImageCompare.UI
{
    public partial class MainForm : Form
    {
        private PixelDiffConvertContext context = new PixelDiffConvertContext();

        public MainForm()
        {
            InitializeComponent();

            cmbConvertContext.SelectedIndex = (int)context.Context;
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
            Task.Run(() =>
            {
                var stopWatch = System.Diagnostics.Stopwatch.StartNew();
                PixelDiff referenceDiff = null;
                foreach (var path in filePaths)
                {
                    var image = SKImage.FromEncodedData(path);
                    var bitmap = SKBitmap.FromImage(image);
                    var pixelDiff = new PixelDiff(context, bitmap.Bytes, bitmap.Width, bitmap.Height, bitmap.BytesPerPixel);

                    if (referenceDiff is null)
                    {
                        referenceDiff = pixelDiff;
                        Invoke(() => filmStrip.AddPicture(bitmap, 0.0f));
                        continue;
                    }

                    var value = referenceDiff.CalcMeanSquaredError(pixelDiff);
                    if (value > 3000) //Would be good if this were on a silder or something.
                    {
                        filmStrip.NewRow();
                        referenceDiff = pixelDiff;
                        value = 0;
                    }

                    Invoke(() => filmStrip.AddPicture(bitmap, value));
                }

                stopWatch.Stop();
                Invoke(() => statLabel.Text = $"Processing Time: {stopWatch.Elapsed} ({cmbConvertContext.Text})");
            });
        }

        private void cmbConvertContext_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            context = new PixelDiffConvertContext((ConvertContext)cmbConvertContext.SelectedIndex);
        }
    }
}
