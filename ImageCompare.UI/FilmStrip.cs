using SkiaSharp;
using System;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace ImageCompare.UI
{
    public partial class FilmStrip : UserControl
    {
        private readonly Font font = new Font("Segoe UI", 14.0f, FontStyle.Regular, GraphicsUnit.Point, ((byte)(0)));

        public FilmStrip()
        {
            InitializeComponent();
        }

        protected override void OnResize(EventArgs e)
        {
            if (DesignMode)
                return;

            base.OnResize(e);
        }

        public void Clear()
        {
            var pictureBoxes = Controls.OfType<PictureBox>().ToList();

            Controls.Clear();

            foreach (var pictureBox in pictureBoxes)
            {
                pictureBox.Image?.Dispose();
                pictureBox.Image = null;
            }
        }

        public void AddPicture(SKBitmap bmp, double value)
        {
            var lbl = new Label() { Text = value.ToString(), AutoSize = true, Font = font };
            Controls.Add(lbl);

            var thumbnailHeight = (Height / 3) - lbl.Height - 8 * 5;
            var thumbmailWidth = Convert.ToInt32(Math.Round((thumbnailHeight / (float)bmp.Height) * bmp.Width));

            using var thumbnail = new SKBitmap(thumbmailWidth * 2, thumbnailHeight * 2);
            bmp.ScalePixels(thumbnail, SKFilterQuality.High);

            var pictureBox = new PictureBox();

            pictureBox.Top = 8;
            pictureBox.Left = 8 + ((thumbmailWidth + 8) * (Controls.Count / 2));
            pictureBox.Width = thumbmailWidth;
            pictureBox.Height = thumbnailHeight;
            pictureBox.SizeMode = PictureBoxSizeMode.StretchImage;
            SetImage(pictureBox, SKImage.FromBitmap(thumbnail));
            Controls.Add(pictureBox);

            lbl.Top = pictureBox.Bottom + 8;
            lbl.Left = pictureBox.Left + (pictureBox.Width / 2) - (lbl.Width / 2);
        }

        private void SetImage(PictureBox pictureBox, SKImage image)
        {
            using var data = image.Encode();
            using var mStream = new System.IO.MemoryStream(data.ToArray());

            pictureBox.Image?.Dispose();
            pictureBox.Image = new Bitmap(mStream, false);
        }
    }
}
