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
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.statLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.cmbConvertContext = new System.Windows.Forms.ComboBox();
            this.statusStrip1.SuspendLayout();
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
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statLabel});
            this.statusStrip1.Location = new System.Drawing.Point(0, 851);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1298, 24);
            this.statusStrip1.TabIndex = 3;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // statLabel
            // 
            this.statLabel.Font = new System.Drawing.Font("Segoe UI", 13.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.statLabel.Name = "statLabel";
            this.statLabel.Size = new System.Drawing.Size(0, 18);
            // 
            // cmbConvertContext
            // 
            this.cmbConvertContext.Font = new System.Drawing.Font("Segoe UI", 16.2F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.cmbConvertContext.FormattingEnabled = true;
            this.cmbConvertContext.Items.AddRange(new object[] {
            "CPU",
            "SSE41",
            "AVX2",
            "Compute Shader"});
            this.cmbConvertContext.Location = new System.Drawing.Point(157, 23);
            this.cmbConvertContext.Name = "cmbConvertContext";
            this.cmbConvertContext.Size = new System.Drawing.Size(289, 45);
            this.cmbConvertContext.TabIndex = 4;
            this.cmbConvertContext.SelectedIndexChanged += new System.EventHandler(this.cmbConvertContext_SelectedIndexChanged);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1298, 875);
            this.Controls.Add(this.cmbConvertContext);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.filmStrip);
            this.Controls.Add(this.lblDropZone);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "MainForm";
            this.Text = "Image Grouper";
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblDropZone;
        private FilmStrip filmStrip;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel statLabel;
        private System.Windows.Forms.ComboBox cmbConvertContext;
    }
}

