namespace ImageCompare.UI
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lblDropZone = new System.Windows.Forms.Label();
            this.filmStrip = new ImageCompare.UI.FilmStrip();
            this.SuspendLayout();
            // 
            // lblDropZone
            // 
            this.lblDropZone.AllowDrop = true;
            this.lblDropZone.AutoSize = true;
            this.lblDropZone.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lblDropZone.Font = new System.Drawing.Font("Segoe UI", 25.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblDropZone.Location = new System.Drawing.Point(464, 9);
            this.lblDropZone.Name = "lblDropZone";
            this.lblDropZone.Size = new System.Drawing.Size(371, 61);
            this.lblDropZone.TabIndex = 1;
            this.lblDropZone.Text = "Drop Images Here";
            this.lblDropZone.DragDrop += new System.Windows.Forms.DragEventHandler(this.lblDropZone_DragDrop);
            this.lblDropZone.DragEnter += new System.Windows.Forms.DragEventHandler(this.lblDropZone_DragEnter);
            // 
            // filmStrip
            // 
            this.filmStrip.AutoScroll = true;
            this.filmStrip.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.filmStrip.Location = new System.Drawing.Point(12, 85);
            this.filmStrip.Margin = new System.Windows.Forms.Padding(0);
            this.filmStrip.Name = "filmStrip";
            this.filmStrip.Size = new System.Drawing.Size(1277, 758);
            this.filmStrip.TabIndex = 2;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1298, 852);
            this.Controls.Add(this.filmStrip);
            this.Controls.Add(this.lblDropZone);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "MainForm";
            this.Text = "Image Compare";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblDropZone;
        private FilmStrip filmStrip;
    }
}

