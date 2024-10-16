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

            var diffs = filePaths.Select(path =>
            {
                var image = SKImage.FromEncodedData(path);
                var bm = SKBitmap.FromImage(image);
                return new PixelDiff(context, bm.Bytes, bm.Width, bm.Height);
            });

            var first = diffs.FirstOrDefault();

            foreach (var diff in diffs.Skip(1))
            {
                var value = first.CalcMeanSquaredError(diff);
            }
        }
    }
}
