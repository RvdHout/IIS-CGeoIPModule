namespace CGeoIPModule
{
    partial class AddExceptionRuleForm
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
            this.BOk = new System.Windows.Forms.Button();
            this.BCancel = new System.Windows.Forms.Button();
            this.RBSpecificIpAddress = new System.Windows.Forms.RadioButton();
            this.LCaption = new System.Windows.Forms.Label();
            this.TBSpecificIpAddress = new System.Windows.Forms.TextBox();
            this.RBAddressRange = new System.Windows.Forms.RadioButton();
            this.TBAddressRange = new System.Windows.Forms.TextBox();
            this.LMask = new System.Windows.Forms.Label();
            this.TBMask = new System.Windows.Forms.TextBox();
            this.ipv4Checkbox = new System.Windows.Forms.CheckBox();
            this.ipv6Checkbox = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // BOk
            // 
            this.BOk.Enabled = false;
            this.BOk.Location = new System.Drawing.Point(291, 253);
            this.BOk.Margin = new System.Windows.Forms.Padding(4);
            this.BOk.Name = "BOk";
            this.BOk.Size = new System.Drawing.Size(100, 28);
            this.BOk.TabIndex = 5;
            this.BOk.Text = "OK";
            this.BOk.UseVisualStyleBackColor = true;
            this.BOk.Click += new System.EventHandler(this.BOk_Click);
            // 
            // BCancel
            // 
            this.BCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.BCancel.Location = new System.Drawing.Point(399, 253);
            this.BCancel.Margin = new System.Windows.Forms.Padding(4);
            this.BCancel.Name = "BCancel";
            this.BCancel.Size = new System.Drawing.Size(100, 28);
            this.BCancel.TabIndex = 6;
            this.BCancel.Text = "Cancel";
            this.BCancel.UseVisualStyleBackColor = true;
            // 
            // RBSpecificIpAddress
            // 
            this.RBSpecificIpAddress.AutoSize = true;
            this.RBSpecificIpAddress.Checked = true;
            this.RBSpecificIpAddress.Location = new System.Drawing.Point(21, 77);
            this.RBSpecificIpAddress.Margin = new System.Windows.Forms.Padding(4);
            this.RBSpecificIpAddress.Name = "RBSpecificIpAddress";
            this.RBSpecificIpAddress.Size = new System.Drawing.Size(132, 20);
            this.RBSpecificIpAddress.TabIndex = 2;
            this.RBSpecificIpAddress.TabStop = true;
            this.RBSpecificIpAddress.Text = "Specific address:";
            this.RBSpecificIpAddress.UseVisualStyleBackColor = true;
            this.RBSpecificIpAddress.CheckedChanged += new System.EventHandler(this.RBSpecificIpAddress_CheckedChanged);
            // 
            // LCaption
            // 
            this.LCaption.AutoSize = true;
            this.LCaption.Location = new System.Drawing.Point(27, 9);
            this.LCaption.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.LCaption.Name = "LCaption";
            this.LCaption.Size = new System.Drawing.Size(325, 16);
            this.LCaption.TabIndex = 3;
            this.LCaption.Text = "Allow/Deny access for the following address or range:";
            // 
            // TBSpecificIpAddress
            // 
            this.TBSpecificIpAddress.Location = new System.Drawing.Point(46, 106);
            this.TBSpecificIpAddress.Margin = new System.Windows.Forms.Padding(4);
            this.TBSpecificIpAddress.Name = "TBSpecificIpAddress";
            this.TBSpecificIpAddress.Size = new System.Drawing.Size(345, 22);
            this.TBSpecificIpAddress.TabIndex = 1;
            this.TBSpecificIpAddress.TextChanged += new System.EventHandler(this.SetOkButtonStatus);
            // 
            // RBAddressRange
            // 
            this.RBAddressRange.AutoSize = true;
            this.RBAddressRange.Location = new System.Drawing.Point(21, 145);
            this.RBAddressRange.Margin = new System.Windows.Forms.Padding(4);
            this.RBAddressRange.Name = "RBAddressRange";
            this.RBAddressRange.Size = new System.Drawing.Size(120, 20);
            this.RBAddressRange.TabIndex = 2;
            this.RBAddressRange.Text = "Address range:";
            this.RBAddressRange.UseVisualStyleBackColor = true;
            this.RBAddressRange.CheckedChanged += new System.EventHandler(this.RBAddressRange_CheckedChanged);
            // 
            // TBAddressRange
            // 
            this.TBAddressRange.Enabled = false;
            this.TBAddressRange.Location = new System.Drawing.Point(46, 173);
            this.TBAddressRange.Margin = new System.Windows.Forms.Padding(4);
            this.TBAddressRange.Name = "TBAddressRange";
            this.TBAddressRange.Size = new System.Drawing.Size(345, 22);
            this.TBAddressRange.TabIndex = 3;
            this.TBAddressRange.TextChanged += new System.EventHandler(this.SetOkButtonStatus);
            // 
            // LMask
            // 
            this.LMask.AutoSize = true;
            this.LMask.Location = new System.Drawing.Point(42, 204);
            this.LMask.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.LMask.Name = "LMask";
            this.LMask.Size = new System.Drawing.Size(190, 16);
            this.LMask.TabIndex = 7;
            this.LMask.Text = "subnet mask (or prefix for ipv6):";
            // 
            // TBMask
            // 
            this.TBMask.Enabled = false;
            this.TBMask.Location = new System.Drawing.Point(46, 223);
            this.TBMask.Margin = new System.Windows.Forms.Padding(4);
            this.TBMask.Name = "TBMask";
            this.TBMask.Size = new System.Drawing.Size(345, 22);
            this.TBMask.TabIndex = 4;
            this.TBMask.TextChanged += new System.EventHandler(this.SetOkButtonStatus);
            // 
            // ipv4Checkbox
            // 
            this.ipv4Checkbox.AutoSize = true;
            this.ipv4Checkbox.Checked = true;
            this.ipv4Checkbox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ipv4Checkbox.Location = new System.Drawing.Point(30, 40);
            this.ipv4Checkbox.Name = "ipv4Checkbox";
            this.ipv4Checkbox.Size = new System.Drawing.Size(55, 20);
            this.ipv4Checkbox.TabIndex = 8;
            this.ipv4Checkbox.Text = "IPv4";
            this.ipv4Checkbox.UseVisualStyleBackColor = true;
            this.ipv4Checkbox.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // ipv6Checkbox
            // 
            this.ipv6Checkbox.AutoSize = true;
            this.ipv6Checkbox.Location = new System.Drawing.Point(91, 40);
            this.ipv6Checkbox.Name = "ipv6Checkbox";
            this.ipv6Checkbox.Size = new System.Drawing.Size(55, 20);
            this.ipv6Checkbox.TabIndex = 9;
            this.ipv6Checkbox.Text = "IPv6";
            this.ipv6Checkbox.UseVisualStyleBackColor = true;
            this.ipv6Checkbox.CheckedChanged += new System.EventHandler(this.checkBox2_CheckedChanged);
            // 
            // AddExceptionRuleForm
            // 
            this.AcceptButton = this.BOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.BCancel;
            this.ClientSize = new System.Drawing.Size(533, 297);
            this.Controls.Add(this.ipv6Checkbox);
            this.Controls.Add(this.ipv4Checkbox);
            this.Controls.Add(this.TBMask);
            this.Controls.Add(this.LMask);
            this.Controls.Add(this.TBAddressRange);
            this.Controls.Add(this.RBAddressRange);
            this.Controls.Add(this.TBSpecificIpAddress);
            this.Controls.Add(this.LCaption);
            this.Controls.Add(this.RBSpecificIpAddress);
            this.Controls.Add(this.BCancel);
            this.Controls.Add(this.BOk);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AddExceptionRuleForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Add Exception Rule";
            this.Load += new System.EventHandler(this.AddExceptionRuleForm_Load);
            this.Shown += new System.EventHandler(this.AddExceptionRuleForm_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button BOk;
        private System.Windows.Forms.Button BCancel;
        private System.Windows.Forms.RadioButton RBSpecificIpAddress;
        private System.Windows.Forms.Label LCaption;
        private System.Windows.Forms.TextBox TBSpecificIpAddress;
        private System.Windows.Forms.RadioButton RBAddressRange;
        private System.Windows.Forms.TextBox TBAddressRange;
        private System.Windows.Forms.Label LMask;
        private System.Windows.Forms.TextBox TBMask;
        private System.Windows.Forms.CheckBox ipv4Checkbox;
        private System.Windows.Forms.CheckBox ipv6Checkbox;
    }
}