namespace StoryDesigner
{
    partial class RoomCtrlDlg
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
            this.scroller = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.zoomfactor = new System.Windows.Forms.ComboBox();
            this.doublewalkmap = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.roomwidth = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.roomheight = new System.Windows.Forms.NumericUpDown();
            this.objectgroup = new System.Windows.Forms.GroupBox();
            this.obj_state = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.drawstyle = new System.Windows.Forms.GroupBox();
            this.draw_front = new System.Windows.Forms.RadioButton();
            this.draw_middle = new System.Windows.Forms.RadioButton();
            this.draw_back = new System.Windows.Forms.RadioButton();
            this.obj_locked = new System.Windows.Forms.CheckBox();
            this.obj_editscript = new System.Windows.Forms.Button();
            this.obj_instname = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.charactergroup = new System.Windows.Forms.GroupBox();
            this.lookdir_panel = new System.Windows.Forms.Panel();
            this.look_right = new System.Windows.Forms.RadioButton();
            this.look_left = new System.Windows.Forms.RadioButton();
            this.look_up = new System.Windows.Forms.RadioButton();
            this.look_down = new System.Windows.Forms.RadioButton();
            this.snap_wm_button = new System.Windows.Forms.Button();
            this.chr_unmovable = new System.Windows.Forms.CheckBox();
            this.chr_locked = new System.Windows.Forms.CheckBox();
            this.chr_editscript = new System.Windows.Forms.Button();
            this.chr_instancename = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.room_editscript = new System.Windows.Forms.Button();
            this.light_color = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.scroller)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.roomwidth)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.roomheight)).BeginInit();
            this.objectgroup.SuspendLayout();
            this.drawstyle.SuspendLayout();
            this.charactergroup.SuspendLayout();
            this.lookdir_panel.SuspendLayout();
            this.SuspendLayout();
            // 
            // scroller
            // 
            this.scroller.BackColor = System.Drawing.SystemColors.ControlDark;
            this.scroller.Location = new System.Drawing.Point(16, 15);
            this.scroller.Margin = new System.Windows.Forms.Padding(4);
            this.scroller.Name = "scroller";
            this.scroller.Size = new System.Drawing.Size(173, 80);
            this.scroller.TabIndex = 0;
            this.scroller.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(217, 15);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(88, 17);
            this.label1.TabIndex = 1;
            this.label1.Text = "Zoom factor:";
            // 
            // zoomfactor
            // 
            this.zoomfactor.FormattingEnabled = true;
            this.zoomfactor.Location = new System.Drawing.Point(221, 34);
            this.zoomfactor.Margin = new System.Windows.Forms.Padding(4);
            this.zoomfactor.Name = "zoomfactor";
            this.zoomfactor.Size = new System.Drawing.Size(84, 24);
            this.zoomfactor.TabIndex = 2;
            this.zoomfactor.SelectedIndexChanged += new System.EventHandler(this.zoomfactor_SelectedIndexChanged);
            // 
            // doublewalkmap
            // 
            this.doublewalkmap.AutoSize = true;
            this.doublewalkmap.Location = new System.Drawing.Point(223, 74);
            this.doublewalkmap.Margin = new System.Windows.Forms.Padding(4);
            this.doublewalkmap.Name = "doublewalkmap";
            this.doublewalkmap.Size = new System.Drawing.Size(133, 21);
            this.doublewalkmap.TabIndex = 3;
            this.doublewalkmap.Text = "Double walkmap";
            this.doublewalkmap.UseVisualStyleBackColor = true;
            this.doublewalkmap.CheckedChanged += new System.EventHandler(this.doublewalkmap_CheckedChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(107, 111);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(48, 17);
            this.label2.TabIndex = 4;
            this.label2.Text = "Width:";
            // 
            // roomwidth
            // 
            this.roomwidth.Increment = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.roomwidth.Location = new System.Drawing.Point(157, 108);
            this.roomwidth.Margin = new System.Windows.Forms.Padding(4);
            this.roomwidth.Name = "roomwidth";
            this.roomwidth.Size = new System.Drawing.Size(73, 22);
            this.roomwidth.TabIndex = 5;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(239, 111);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(53, 17);
            this.label3.TabIndex = 6;
            this.label3.Text = "Height:";
            // 
            // roomheight
            // 
            this.roomheight.Increment = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.roomheight.Location = new System.Drawing.Point(299, 108);
            this.roomheight.Margin = new System.Windows.Forms.Padding(4);
            this.roomheight.Name = "roomheight";
            this.roomheight.Size = new System.Drawing.Size(73, 22);
            this.roomheight.TabIndex = 5;
            // 
            // objectgroup
            // 
            this.objectgroup.Controls.Add(this.obj_state);
            this.objectgroup.Controls.Add(this.label5);
            this.objectgroup.Controls.Add(this.drawstyle);
            this.objectgroup.Controls.Add(this.obj_locked);
            this.objectgroup.Controls.Add(this.obj_editscript);
            this.objectgroup.Controls.Add(this.obj_instname);
            this.objectgroup.Controls.Add(this.label4);
            this.objectgroup.Enabled = false;
            this.objectgroup.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.objectgroup.Location = new System.Drawing.Point(16, 151);
            this.objectgroup.Margin = new System.Windows.Forms.Padding(4);
            this.objectgroup.Name = "objectgroup";
            this.objectgroup.Padding = new System.Windows.Forms.Padding(4);
            this.objectgroup.Size = new System.Drawing.Size(347, 159);
            this.objectgroup.TabIndex = 7;
            this.objectgroup.TabStop = false;
            this.objectgroup.Text = "Object";
            // 
            // obj_state
            // 
            this.obj_state.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.obj_state.FormattingEnabled = true;
            this.obj_state.Location = new System.Drawing.Point(59, 100);
            this.obj_state.Margin = new System.Windows.Forms.Padding(4);
            this.obj_state.Name = "obj_state";
            this.obj_state.Size = new System.Drawing.Size(71, 25);
            this.obj_state.TabIndex = 6;
            this.obj_state.SelectedIndexChanged += new System.EventHandler(this.obj_state_SelectedIndexChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(8, 103);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(45, 17);
            this.label5.TabIndex = 5;
            this.label5.Text = "State:";
            // 
            // drawstyle
            // 
            this.drawstyle.Controls.Add(this.draw_front);
            this.drawstyle.Controls.Add(this.draw_middle);
            this.drawstyle.Controls.Add(this.draw_back);
            this.drawstyle.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.drawstyle.Location = new System.Drawing.Point(219, 39);
            this.drawstyle.Margin = new System.Windows.Forms.Padding(4);
            this.drawstyle.Name = "drawstyle";
            this.drawstyle.Padding = new System.Windows.Forms.Padding(4);
            this.drawstyle.Size = new System.Drawing.Size(113, 112);
            this.drawstyle.TabIndex = 4;
            this.drawstyle.TabStop = false;
            this.drawstyle.Text = "Drawstyle";
            // 
            // draw_front
            // 
            this.draw_front.AutoSize = true;
            this.draw_front.Location = new System.Drawing.Point(8, 80);
            this.draw_front.Margin = new System.Windows.Forms.Padding(4);
            this.draw_front.Name = "draw_front";
            this.draw_front.Size = new System.Drawing.Size(62, 21);
            this.draw_front.TabIndex = 2;
            this.draw_front.TabStop = true;
            this.draw_front.Text = "Front";
            this.draw_front.UseVisualStyleBackColor = true;
            this.draw_front.CheckedChanged += new System.EventHandler(this.draw_front_CheckedChanged);
            // 
            // draw_middle
            // 
            this.draw_middle.AutoSize = true;
            this.draw_middle.Location = new System.Drawing.Point(8, 52);
            this.draw_middle.Margin = new System.Windows.Forms.Padding(4);
            this.draw_middle.Name = "draw_middle";
            this.draw_middle.Size = new System.Drawing.Size(70, 21);
            this.draw_middle.TabIndex = 1;
            this.draw_middle.TabStop = true;
            this.draw_middle.Text = "Middle";
            this.draw_middle.UseVisualStyleBackColor = true;
            this.draw_middle.CheckedChanged += new System.EventHandler(this.draw_middle_CheckedChanged);
            // 
            // draw_back
            // 
            this.draw_back.AutoSize = true;
            this.draw_back.Location = new System.Drawing.Point(8, 23);
            this.draw_back.Margin = new System.Windows.Forms.Padding(4);
            this.draw_back.Name = "draw_back";
            this.draw_back.Size = new System.Drawing.Size(60, 21);
            this.draw_back.TabIndex = 0;
            this.draw_back.TabStop = true;
            this.draw_back.Text = "Back";
            this.draw_back.UseVisualStyleBackColor = true;
            this.draw_back.CheckedChanged += new System.EventHandler(this.draw_back_CheckedChanged);
            // 
            // obj_locked
            // 
            this.obj_locked.AutoSize = true;
            this.obj_locked.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.obj_locked.Location = new System.Drawing.Point(128, 57);
            this.obj_locked.Margin = new System.Windows.Forms.Padding(4);
            this.obj_locked.Name = "obj_locked";
            this.obj_locked.Size = new System.Drawing.Size(76, 21);
            this.obj_locked.TabIndex = 3;
            this.obj_locked.Text = "Locked";
            this.obj_locked.UseVisualStyleBackColor = true;
            this.obj_locked.CheckedChanged += new System.EventHandler(this.obj_locked_CheckedChanged);
            // 
            // obj_editscript
            // 
            this.obj_editscript.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.obj_editscript.Location = new System.Drawing.Point(12, 52);
            this.obj_editscript.Margin = new System.Windows.Forms.Padding(4);
            this.obj_editscript.Name = "obj_editscript";
            this.obj_editscript.Size = new System.Drawing.Size(100, 28);
            this.obj_editscript.TabIndex = 2;
            this.obj_editscript.Text = "Edit script";
            this.obj_editscript.UseVisualStyleBackColor = true;
            this.obj_editscript.Click += new System.EventHandler(this.obj_editscript_Click);
            // 
            // obj_instname
            // 
            this.obj_instname.AutoSize = true;
            this.obj_instname.Location = new System.Drawing.Point(119, 20);
            this.obj_instname.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.obj_instname.Name = "obj_instname";
            this.obj_instname.Size = new System.Drawing.Size(44, 17);
            this.obj_instname.TabIndex = 1;
            this.obj_instname.Text = "none";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(8, 20);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(100, 17);
            this.label4.TabIndex = 0;
            this.label4.Text = "Instancename:";
            // 
            // charactergroup
            // 
            this.charactergroup.Controls.Add(this.lookdir_panel);
            this.charactergroup.Controls.Add(this.snap_wm_button);
            this.charactergroup.Controls.Add(this.chr_unmovable);
            this.charactergroup.Controls.Add(this.chr_locked);
            this.charactergroup.Controls.Add(this.chr_editscript);
            this.charactergroup.Controls.Add(this.chr_instancename);
            this.charactergroup.Controls.Add(this.label6);
            this.charactergroup.Enabled = false;
            this.charactergroup.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.charactergroup.Location = new System.Drawing.Point(16, 318);
            this.charactergroup.Margin = new System.Windows.Forms.Padding(4);
            this.charactergroup.Name = "charactergroup";
            this.charactergroup.Padding = new System.Windows.Forms.Padding(4);
            this.charactergroup.Size = new System.Drawing.Size(347, 159);
            this.charactergroup.TabIndex = 7;
            this.charactergroup.TabStop = false;
            this.charactergroup.Text = "Character";
            // 
            // lookdir_panel
            // 
            this.lookdir_panel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lookdir_panel.Controls.Add(this.look_right);
            this.lookdir_panel.Controls.Add(this.look_left);
            this.lookdir_panel.Controls.Add(this.look_up);
            this.lookdir_panel.Controls.Add(this.look_down);
            this.lookdir_panel.Location = new System.Drawing.Point(243, 52);
            this.lookdir_panel.Margin = new System.Windows.Forms.Padding(4);
            this.lookdir_panel.Name = "lookdir_panel";
            this.lookdir_panel.Size = new System.Drawing.Size(95, 88);
            this.lookdir_panel.TabIndex = 5;
            // 
            // look_right
            // 
            this.look_right.AutoSize = true;
            this.look_right.Location = new System.Drawing.Point(69, 37);
            this.look_right.Margin = new System.Windows.Forms.Padding(4);
            this.look_right.Name = "look_right";
            this.look_right.Size = new System.Drawing.Size(17, 16);
            this.look_right.TabIndex = 0;
            this.look_right.TabStop = true;
            this.look_right.UseVisualStyleBackColor = true;
            this.look_right.CheckedChanged += new System.EventHandler(this.look_right_CheckedChanged);
            // 
            // look_left
            // 
            this.look_left.AutoSize = true;
            this.look_left.Location = new System.Drawing.Point(4, 37);
            this.look_left.Margin = new System.Windows.Forms.Padding(4);
            this.look_left.Name = "look_left";
            this.look_left.Size = new System.Drawing.Size(17, 16);
            this.look_left.TabIndex = 0;
            this.look_left.TabStop = true;
            this.look_left.UseVisualStyleBackColor = true;
            this.look_left.CheckedChanged += new System.EventHandler(this.look_left_CheckedChanged);
            // 
            // look_up
            // 
            this.look_up.AutoSize = true;
            this.look_up.Location = new System.Drawing.Point(39, 11);
            this.look_up.Margin = new System.Windows.Forms.Padding(4);
            this.look_up.Name = "look_up";
            this.look_up.Size = new System.Drawing.Size(17, 16);
            this.look_up.TabIndex = 0;
            this.look_up.TabStop = true;
            this.look_up.UseVisualStyleBackColor = true;
            this.look_up.CheckedChanged += new System.EventHandler(this.look_up_CheckedChanged);
            // 
            // look_down
            // 
            this.look_down.AutoSize = true;
            this.look_down.Location = new System.Drawing.Point(39, 59);
            this.look_down.Margin = new System.Windows.Forms.Padding(4);
            this.look_down.Name = "look_down";
            this.look_down.Size = new System.Drawing.Size(17, 16);
            this.look_down.TabIndex = 0;
            this.look_down.TabStop = true;
            this.look_down.UseVisualStyleBackColor = true;
            this.look_down.CheckedChanged += new System.EventHandler(this.look_down_CheckedChanged);
            // 
            // snap_wm_button
            // 
            this.snap_wm_button.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.snap_wm_button.Location = new System.Drawing.Point(12, 98);
            this.snap_wm_button.Margin = new System.Windows.Forms.Padding(4);
            this.snap_wm_button.Name = "snap_wm_button";
            this.snap_wm_button.Size = new System.Drawing.Size(100, 42);
            this.snap_wm_button.TabIndex = 4;
            this.snap_wm_button.Text = "Snap to Walkmap";
            this.snap_wm_button.UseVisualStyleBackColor = true;
            // 
            // chr_unmovable
            // 
            this.chr_unmovable.AutoSize = true;
            this.chr_unmovable.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.chr_unmovable.Location = new System.Drawing.Point(128, 85);
            this.chr_unmovable.Margin = new System.Windows.Forms.Padding(4);
            this.chr_unmovable.Name = "chr_unmovable";
            this.chr_unmovable.Size = new System.Drawing.Size(101, 21);
            this.chr_unmovable.TabIndex = 3;
            this.chr_unmovable.Text = "Unmovable";
            this.chr_unmovable.UseVisualStyleBackColor = true;
            this.chr_unmovable.CheckedChanged += new System.EventHandler(this.chr_unmovable_CheckedChanged);
            // 
            // chr_locked
            // 
            this.chr_locked.AutoSize = true;
            this.chr_locked.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.chr_locked.Location = new System.Drawing.Point(128, 57);
            this.chr_locked.Margin = new System.Windows.Forms.Padding(4);
            this.chr_locked.Name = "chr_locked";
            this.chr_locked.Size = new System.Drawing.Size(76, 21);
            this.chr_locked.TabIndex = 3;
            this.chr_locked.Text = "Locked";
            this.chr_locked.UseVisualStyleBackColor = true;
            this.chr_locked.CheckedChanged += new System.EventHandler(this.chr_locked_CheckedChanged);
            // 
            // chr_editscript
            // 
            this.chr_editscript.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.chr_editscript.Location = new System.Drawing.Point(12, 52);
            this.chr_editscript.Margin = new System.Windows.Forms.Padding(4);
            this.chr_editscript.Name = "chr_editscript";
            this.chr_editscript.Size = new System.Drawing.Size(100, 28);
            this.chr_editscript.TabIndex = 2;
            this.chr_editscript.Text = "Edit script";
            this.chr_editscript.UseVisualStyleBackColor = true;
            this.chr_editscript.Click += new System.EventHandler(this.chr_editscript_Click);
            // 
            // chr_instancename
            // 
            this.chr_instancename.AutoSize = true;
            this.chr_instancename.Location = new System.Drawing.Point(119, 20);
            this.chr_instancename.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.chr_instancename.Name = "chr_instancename";
            this.chr_instancename.Size = new System.Drawing.Size(44, 17);
            this.chr_instancename.TabIndex = 1;
            this.chr_instancename.Text = "none";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(8, 20);
            this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(100, 17);
            this.label6.TabIndex = 0;
            this.label6.Text = "Instancename:";
            // 
            // room_editscript
            // 
            this.room_editscript.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.room_editscript.Location = new System.Drawing.Point(16, 108);
            this.room_editscript.Margin = new System.Windows.Forms.Padding(4);
            this.room_editscript.Name = "room_editscript";
            this.room_editscript.Size = new System.Drawing.Size(83, 28);
            this.room_editscript.TabIndex = 8;
            this.room_editscript.Text = "Edit script";
            this.room_editscript.UseVisualStyleBackColor = true;
            this.room_editscript.Click += new System.EventHandler(this.room_editscript_Click);
            // 
            // light_color
            // 
            this.light_color.BackColor = System.Drawing.Color.Lime;
            this.light_color.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.light_color.Location = new System.Drawing.Point(332, 34);
            this.light_color.Margin = new System.Windows.Forms.Padding(4);
            this.light_color.Name = "light_color";
            this.light_color.Size = new System.Drawing.Size(31, 28);
            this.light_color.TabIndex = 9;
            this.light_color.UseVisualStyleBackColor = false;
            this.light_color.Click += new System.EventHandler(this.border_color_Click);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(304, 15);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(75, 17);
            this.label7.TabIndex = 10;
            this.label7.Text = "Room light";
            // 
            // RoomCtrlDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(391, 532);
            this.ControlBox = false;
            this.Controls.Add(this.label7);
            this.Controls.Add(this.light_color);
            this.Controls.Add(this.room_editscript);
            this.Controls.Add(this.charactergroup);
            this.Controls.Add(this.objectgroup);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.roomheight);
            this.Controls.Add(this.roomwidth);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.doublewalkmap);
            this.Controls.Add(this.zoomfactor);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.scroller);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "RoomCtrlDlg";
            this.Text = "Room Control Panel";
            ((System.ComponentModel.ISupportInitialize)(this.scroller)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.roomwidth)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.roomheight)).EndInit();
            this.objectgroup.ResumeLayout(false);
            this.objectgroup.PerformLayout();
            this.drawstyle.ResumeLayout(false);
            this.drawstyle.PerformLayout();
            this.charactergroup.ResumeLayout(false);
            this.charactergroup.PerformLayout();
            this.lookdir_panel.ResumeLayout(false);
            this.lookdir_panel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox scroller;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox zoomfactor;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown roomwidth;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown roomheight;
        private System.Windows.Forms.GroupBox objectgroup;
        private System.Windows.Forms.Label obj_instname;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button obj_editscript;
        private System.Windows.Forms.CheckBox obj_locked;
        private System.Windows.Forms.GroupBox charactergroup;
        private System.Windows.Forms.CheckBox chr_locked;
        private System.Windows.Forms.Button chr_editscript;
        private System.Windows.Forms.Label chr_instancename;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.GroupBox drawstyle;
        private System.Windows.Forms.RadioButton draw_back;
        private System.Windows.Forms.RadioButton draw_front;
        private System.Windows.Forms.RadioButton draw_middle;
        private System.Windows.Forms.Button snap_wm_button;
        private System.Windows.Forms.Panel lookdir_panel;
        private System.Windows.Forms.CheckBox chr_unmovable;
        private System.Windows.Forms.RadioButton look_right;
        private System.Windows.Forms.RadioButton look_left;
        private System.Windows.Forms.RadioButton look_up;
        private System.Windows.Forms.RadioButton look_down;
        private System.Windows.Forms.ComboBox obj_state;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.CheckBox doublewalkmap;
        private System.Windows.Forms.Button room_editscript;
        private System.Windows.Forms.Button light_color;
        private System.Windows.Forms.Label label7;
    }
}