namespace StoryWriter
{
    partial class DocumentForm
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
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.details = new System.Windows.Forms.TextBox();
            this.splitContainer3 = new System.Windows.Forms.SplitContainer();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.scenestab = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabControl2 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.chapters = new StoryWriter.GridView();
            this.sceneView = new StoryWriter.GridView();
            this.scene_viewpoint = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.scene_words = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.scene_scene = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.scene_status = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.title = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.words = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.scenes = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.chapter_wordaccu = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.description = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).BeginInit();
            this.splitContainer3.Panel1.SuspendLayout();
            this.splitContainer3.Panel2.SuspendLayout();
            this.splitContainer3.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.scenestab.SuspendLayout();
            this.tabControl2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chapters)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.sceneView)).BeginInit();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.splitContainer2);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer3);
            this.splitContainer1.Size = new System.Drawing.Size(1013, 589);
            this.splitContainer1.SplitterDistance = 250;
            this.splitContainer1.TabIndex = 0;
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.chapters);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.details);
            this.splitContainer2.Size = new System.Drawing.Size(250, 589);
            this.splitContainer2.SplitterDistance = 400;
            this.splitContainer2.TabIndex = 0;
            // 
            // details
            // 
            this.details.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.details.Location = new System.Drawing.Point(3, 3);
            this.details.Multiline = true;
            this.details.Name = "details";
            this.details.Size = new System.Drawing.Size(244, 179);
            this.details.TabIndex = 0;
            // 
            // splitContainer3
            // 
            this.splitContainer3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer3.Location = new System.Drawing.Point(0, 0);
            this.splitContainer3.Name = "splitContainer3";
            this.splitContainer3.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer3.Panel1
            // 
            this.splitContainer3.Panel1.Controls.Add(this.tabControl1);
            // 
            // splitContainer3.Panel2
            // 
            this.splitContainer3.Panel2.Controls.Add(this.tabControl2);
            this.splitContainer3.Size = new System.Drawing.Size(759, 589);
            this.splitContainer3.SplitterDistance = 350;
            this.splitContainer3.TabIndex = 0;
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.scenestab);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Location = new System.Drawing.Point(3, 3);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(756, 344);
            this.tabControl1.TabIndex = 0;
            // 
            // scenestab
            // 
            this.scenestab.Controls.Add(this.sceneView);
            this.scenestab.Location = new System.Drawing.Point(4, 25);
            this.scenestab.Name = "scenestab";
            this.scenestab.Padding = new System.Windows.Forms.Padding(3);
            this.scenestab.Size = new System.Drawing.Size(748, 315);
            this.scenestab.TabIndex = 0;
            this.scenestab.Text = "Scenes";
            this.scenestab.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Location = new System.Drawing.Point(4, 25);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(774, 315);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "tabPage2";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // tabControl2
            // 
            this.tabControl2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl2.Controls.Add(this.tabPage1);
            this.tabControl2.Controls.Add(this.tabPage3);
            this.tabControl2.Location = new System.Drawing.Point(3, 3);
            this.tabControl2.Name = "tabControl2";
            this.tabControl2.SelectedIndex = 0;
            this.tabControl2.Size = new System.Drawing.Size(756, 220);
            this.tabControl2.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Location = new System.Drawing.Point(4, 25);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(748, 191);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "tabPage1";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage3
            // 
            this.tabPage3.Location = new System.Drawing.Point(4, 25);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(774, 191);
            this.tabPage3.TabIndex = 1;
            this.tabPage3.Text = "tabPage3";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // chapters
            // 
            this.chapters.AllowUserToAddRows = false;
            this.chapters.AllowUserToDeleteRows = false;
            this.chapters.AllowUserToResizeRows = false;
            this.chapters.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.chapters.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.chapters.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.title,
            this.words,
            this.scenes,
            this.chapter_wordaccu,
            this.description});
            this.chapters.GridColor = System.Drawing.SystemColors.Window;
            this.chapters.Location = new System.Drawing.Point(3, 3);
            this.chapters.Name = "chapters";
            this.chapters.ReadOnly = true;
            this.chapters.RowHeadersVisible = false;
            this.chapters.RowTemplate.Height = 24;
            this.chapters.ShowGrid = false;
            this.chapters.Size = new System.Drawing.Size(244, 394);
            this.chapters.TabIndex = 0;
            // 
            // sceneView
            // 
            this.sceneView.AllowUserToAddRows = false;
            this.sceneView.AllowUserToResizeRows = false;
            this.sceneView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.sceneView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.sceneView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.scene_viewpoint,
            this.scene_words,
            this.scene_scene,
            this.scene_status});
            this.sceneView.GridColor = System.Drawing.SystemColors.Control;
            this.sceneView.Location = new System.Drawing.Point(0, 0);
            this.sceneView.Name = "sceneView";
            this.sceneView.ReadOnly = true;
            this.sceneView.RowHeadersVisible = false;
            this.sceneView.RowTemplate.Height = 24;
            this.sceneView.ShowGrid = true;
            this.sceneView.Size = new System.Drawing.Size(742, 309);
            this.sceneView.TabIndex = 0;
            // 
            // scene_viewpoint
            // 
            this.scene_viewpoint.HeaderText = "Viewpoint";
            this.scene_viewpoint.Name = "scene_viewpoint";
            this.scene_viewpoint.ReadOnly = true;
            // 
            // scene_words
            // 
            this.scene_words.HeaderText = "Words";
            this.scene_words.Name = "scene_words";
            this.scene_words.ReadOnly = true;
            this.scene_words.Width = 50;
            // 
            // scene_scene
            // 
            this.scene_scene.HeaderText = "Scene";
            this.scene_scene.Name = "scene_scene";
            this.scene_scene.ReadOnly = true;
            this.scene_scene.Width = 200;
            // 
            // scene_status
            // 
            this.scene_status.HeaderText = "Status";
            this.scene_status.Name = "scene_status";
            this.scene_status.ReadOnly = true;
            // 
            // title
            // 
            this.title.HeaderText = "Title";
            this.title.Name = "title";
            this.title.ReadOnly = true;
            // 
            // words
            // 
            this.words.HeaderText = "Words";
            this.words.Name = "words";
            this.words.ReadOnly = true;
            this.words.Width = 40;
            // 
            // scenes
            // 
            this.scenes.HeaderText = "Scenes";
            this.scenes.Name = "scenes";
            this.scenes.ReadOnly = true;
            this.scenes.Width = 30;
            // 
            // chapter_wordaccu
            // 
            this.chapter_wordaccu.HeaderText = "Words (Acc.)";
            this.chapter_wordaccu.Name = "chapter_wordaccu";
            this.chapter_wordaccu.ReadOnly = true;
            this.chapter_wordaccu.Width = 40;
            // 
            // description
            // 
            this.description.HeaderText = "Description";
            this.description.Name = "description";
            this.description.ReadOnly = true;
            // 
            // DocumentForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1013, 589);
            this.Controls.Add(this.splitContainer1);
            this.Name = "DocumentForm";
            this.Text = "Document";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.splitContainer3.Panel1.ResumeLayout(false);
            this.splitContainer3.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).EndInit();
            this.splitContainer3.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.scenestab.ResumeLayout(false);
            this.tabControl2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chapters)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.sceneView)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.SplitContainer splitContainer3;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage scenestab;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabControl tabControl2;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.TextBox details;
        private GridView chapters;
        private GridView sceneView;
        private System.Windows.Forms.DataGridViewTextBoxColumn scene_viewpoint;
        private System.Windows.Forms.DataGridViewTextBoxColumn scene_words;
        private System.Windows.Forms.DataGridViewTextBoxColumn scene_scene;
        private System.Windows.Forms.DataGridViewTextBoxColumn scene_status;
        private System.Windows.Forms.DataGridViewTextBoxColumn title;
        private System.Windows.Forms.DataGridViewTextBoxColumn words;
        private System.Windows.Forms.DataGridViewTextBoxColumn scenes;
        private System.Windows.Forms.DataGridViewTextBoxColumn chapter_wordaccu;
        private System.Windows.Forms.DataGridViewTextBoxColumn description;
    }
}