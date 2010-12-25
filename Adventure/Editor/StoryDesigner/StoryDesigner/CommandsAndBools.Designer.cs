namespace StoryDesigner
{
    partial class CommandsAndBools
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
            this.commandGrid = new System.Windows.Forms.DataGridView();
            this.command = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.text = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.label1 = new System.Windows.Forms.Label();
            this.bools = new System.Windows.Forms.CheckedListBox();
            this.label2 = new System.Windows.Forms.Label();
            this.add_bool = new System.Windows.Forms.Button();
            this.bool_delete = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.linktext = new System.Windows.Forms.TextBox();
            this.givelink = new System.Windows.Forms.TextBox();
            this.walktext = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.commandGrid)).BeginInit();
            this.SuspendLayout();
            // 
            // commandGrid
            // 
            this.commandGrid.AllowUserToResizeColumns = false;
            this.commandGrid.AllowUserToResizeRows = false;
            this.commandGrid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.commandGrid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.command,
            this.text});
            this.commandGrid.Location = new System.Drawing.Point(12, 25);
            this.commandGrid.Name = "commandGrid";
            this.commandGrid.Size = new System.Drawing.Size(243, 259);
            this.commandGrid.TabIndex = 0;
            // 
            // command
            // 
            this.command.HeaderText = "command";
            this.command.Name = "command";
            // 
            // text
            // 
            this.text.HeaderText = "text";
            this.text.Name = "text";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(62, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Commands:";
            // 
            // bools
            // 
            this.bools.FormattingEnabled = true;
            this.bools.Location = new System.Drawing.Point(310, 25);
            this.bools.Name = "bools";
            this.bools.Size = new System.Drawing.Size(212, 259);
            this.bools.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(307, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(36, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Bools:";
            // 
            // add_bool
            // 
            this.add_bool.Location = new System.Drawing.Point(528, 25);
            this.add_bool.Name = "add_bool";
            this.add_bool.Size = new System.Drawing.Size(44, 23);
            this.add_bool.TabIndex = 4;
            this.add_bool.Text = "+";
            this.add_bool.UseVisualStyleBackColor = true;
            this.add_bool.Click += new System.EventHandler(this.add_bool_Click);
            // 
            // bool_delete
            // 
            this.bool_delete.Location = new System.Drawing.Point(528, 54);
            this.bool_delete.Name = "bool_delete";
            this.bool_delete.Size = new System.Drawing.Size(44, 23);
            this.bool_delete.TabIndex = 4;
            this.bool_delete.Text = "-";
            this.bool_delete.UseVisualStyleBackColor = true;
            this.bool_delete.Click += new System.EventHandler(this.bool_delete_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 303);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Linktext:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(89, 303);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(51, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "Give link:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(175, 303);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(55, 13);
            this.label5.TabIndex = 7;
            this.label5.Text = "Walk text:";
            // 
            // linktext
            // 
            this.linktext.Location = new System.Drawing.Point(12, 319);
            this.linktext.Name = "linktext";
            this.linktext.Size = new System.Drawing.Size(74, 20);
            this.linktext.TabIndex = 8;
            // 
            // givelink
            // 
            this.givelink.Location = new System.Drawing.Point(92, 319);
            this.givelink.Name = "givelink";
            this.givelink.Size = new System.Drawing.Size(77, 20);
            this.givelink.TabIndex = 9;
            // 
            // walktext
            // 
            this.walktext.Location = new System.Drawing.Point(175, 319);
            this.walktext.Name = "walktext";
            this.walktext.Size = new System.Drawing.Size(80, 20);
            this.walktext.TabIndex = 10;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(373, 319);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 11;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // CommandsAndBools
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 362);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.walktext);
            this.Controls.Add(this.givelink);
            this.Controls.Add(this.linktext);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.bool_delete);
            this.Controls.Add(this.add_bool);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.bools);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.commandGrid);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "CommandsAndBools";
            this.RightToLeftLayout = true;
            this.Text = "Commands And Bools";
            ((System.ComponentModel.ISupportInitialize)(this.commandGrid)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView commandGrid;
        private System.Windows.Forms.DataGridViewTextBoxColumn command;
        private System.Windows.Forms.DataGridViewTextBoxColumn text;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckedListBox bools;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button add_bool;
        private System.Windows.Forms.Button bool_delete;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox linktext;
        private System.Windows.Forms.TextBox givelink;
        private System.Windows.Forms.TextBox walktext;
        private System.Windows.Forms.Button button1;
    }
}