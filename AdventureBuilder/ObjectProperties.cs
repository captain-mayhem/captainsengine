using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for ObjectProperties.
	/// </summary>
  public class ObjectProperties : System.Windows.Forms.Form
  {
    private System.Windows.Forms.TextBox ObjectName;
    private System.Windows.Forms.ListBox Properties;
    private System.Windows.Forms.ListBox Value;
    private System.Windows.Forms.ListBox Response;
    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.Label label3;
    private System.Windows.Forms.Label label4;
    private System.Windows.Forms.Button button4;
    private System.Windows.Forms.Button OK;
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.Container components = null;
    private GraphNode m_node;
    private System.Windows.Forms.Button button_property;
    private System.Windows.Forms.Button button_value;
    private System.Windows.Forms.Button button_response;
    private System.Windows.Forms.TextBox textbox_add;
    private System.Windows.Forms.Label label5;
    private System.Windows.Forms.ComboBox responseType;
    private System.Windows.Forms.TextBox responsefield;
    private ArrayList m_properties;

    public ObjectProperties(GraphNode node)
    {
      //
      // Required for Windows Form Designer support
      //
      InitializeComponent();

      m_properties = new ArrayList();
      m_node = node;
      if (m_node != null){
        getValuesFromNode();
      }
    }

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    protected override void Dispose( bool disposing )
    {
      if( disposing )
      {
        if(components != null)
        {
          components.Dispose();
        }
      }
      base.Dispose( disposing );
    }

    #region Windows Form Designer generated code
    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      this.ObjectName = new System.Windows.Forms.TextBox();
      this.Properties = new System.Windows.Forms.ListBox();
      this.Value = new System.Windows.Forms.ListBox();
      this.Response = new System.Windows.Forms.ListBox();
      this.label1 = new System.Windows.Forms.Label();
      this.label2 = new System.Windows.Forms.Label();
      this.label3 = new System.Windows.Forms.Label();
      this.label4 = new System.Windows.Forms.Label();
      this.button_property = new System.Windows.Forms.Button();
      this.button_value = new System.Windows.Forms.Button();
      this.button_response = new System.Windows.Forms.Button();
      this.textbox_add = new System.Windows.Forms.TextBox();
      this.OK = new System.Windows.Forms.Button();
      this.button4 = new System.Windows.Forms.Button();
      this.label5 = new System.Windows.Forms.Label();
      this.responseType = new System.Windows.Forms.ComboBox();
      this.responsefield = new System.Windows.Forms.TextBox();
      this.SuspendLayout();
      // 
      // ObjectName
      // 
      this.ObjectName.Location = new System.Drawing.Point(576, 16);
      this.ObjectName.Name = "ObjectName";
      this.ObjectName.Size = new System.Drawing.Size(192, 20);
      this.ObjectName.TabIndex = 0;
      this.ObjectName.Text = "ObjectName";
      // 
      // Properties
      // 
      this.Properties.Location = new System.Drawing.Point(24, 72);
      this.Properties.Name = "Properties";
      this.Properties.Size = new System.Drawing.Size(120, 160);
      this.Properties.TabIndex = 2;
      this.Properties.SelectedIndexChanged += new System.EventHandler(this.Properties_SelectedIndexChanged);
      // 
      // Value
      // 
      this.Value.Location = new System.Drawing.Point(152, 72);
      this.Value.Name = "Value";
      this.Value.Size = new System.Drawing.Size(80, 160);
      this.Value.TabIndex = 3;
      this.Value.SelectedIndexChanged += new System.EventHandler(this.Value_SelectedIndexChanged);
      // 
      // Response
      // 
      this.Response.Location = new System.Drawing.Point(240, 72);
      this.Response.Name = "Response";
      this.Response.Size = new System.Drawing.Size(528, 160);
      this.Response.TabIndex = 4;
      // 
      // label1
      // 
      this.label1.Location = new System.Drawing.Point(64, 48);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(48, 16);
      this.label1.TabIndex = 5;
      this.label1.Text = "Property";
      // 
      // label2
      // 
      this.label2.Location = new System.Drawing.Point(176, 48);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(40, 16);
      this.label2.TabIndex = 6;
      this.label2.Text = "Value";
      // 
      // label3
      // 
      this.label3.Location = new System.Drawing.Point(264, 48);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(56, 16);
      this.label3.TabIndex = 7;
      this.label3.Text = "Response";
      // 
      // label4
      // 
      this.label4.Location = new System.Drawing.Point(528, 16);
      this.label4.Name = "label4";
      this.label4.Size = new System.Drawing.Size(40, 16);
      this.label4.TabIndex = 8;
      this.label4.Text = "Name";
      // 
      // button_property
      // 
      this.button_property.Location = new System.Drawing.Point(64, 16);
      this.button_property.Name = "button_property";
      this.button_property.TabIndex = 9;
      this.button_property.Text = "Add";
      this.button_property.Click += new System.EventHandler(this.button_property_Click);
      // 
      // button_value
      // 
      this.button_value.Location = new System.Drawing.Point(160, 16);
      this.button_value.Name = "button_value";
      this.button_value.TabIndex = 10;
      this.button_value.Text = "Add";
      this.button_value.Click += new System.EventHandler(this.button_value_Click);
      // 
      // button_response
      // 
      this.button_response.Location = new System.Drawing.Point(256, 16);
      this.button_response.Name = "button_response";
      this.button_response.TabIndex = 11;
      this.button_response.Text = "Add";
      this.button_response.Click += new System.EventHandler(this.button_response_Click);
      // 
      // textbox_add
      // 
      this.textbox_add.Location = new System.Drawing.Point(344, 16);
      this.textbox_add.Name = "textbox_add";
      this.textbox_add.TabIndex = 12;
      this.textbox_add.Text = "";
      // 
      // OK
      // 
      this.OK.Location = new System.Drawing.Point(600, 528);
      this.OK.Name = "OK";
      this.OK.TabIndex = 13;
      this.OK.Text = "OK";
      this.OK.Click += new System.EventHandler(this.OK_Click);
      // 
      // button4
      // 
      this.button4.Location = new System.Drawing.Point(696, 528);
      this.button4.Name = "button4";
      this.button4.TabIndex = 14;
      this.button4.Text = "Cancel";
      this.button4.Click += new System.EventHandler(this.button4_Click);
      // 
      // label5
      // 
      this.label5.Location = new System.Drawing.Point(40, 264);
      this.label5.Name = "label5";
      this.label5.Size = new System.Drawing.Size(88, 16);
      this.label5.TabIndex = 15;
      this.label5.Text = "Response type:";
      // 
      // responseType
      // 
      this.responseType.Items.AddRange(new object[] {
                                                      "none",
                                                      "textout"});
      this.responseType.Location = new System.Drawing.Point(136, 264);
      this.responseType.Name = "responseType";
      this.responseType.Size = new System.Drawing.Size(121, 21);
      this.responseType.TabIndex = 16;
      this.responseType.Text = "none";
      // 
      // responsefield
      // 
      this.responsefield.Location = new System.Drawing.Point(24, 296);
      this.responsefield.Multiline = true;
      this.responsefield.Name = "responsefield";
      this.responsefield.Size = new System.Drawing.Size(744, 208);
      this.responsefield.TabIndex = 17;
      this.responsefield.Text = "";
      // 
      // ObjectProperties
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(792, 566);
      this.Controls.Add(this.responsefield);
      this.Controls.Add(this.responseType);
      this.Controls.Add(this.label5);
      this.Controls.Add(this.button4);
      this.Controls.Add(this.OK);
      this.Controls.Add(this.textbox_add);
      this.Controls.Add(this.button_response);
      this.Controls.Add(this.button_value);
      this.Controls.Add(this.button_property);
      this.Controls.Add(this.label4);
      this.Controls.Add(this.label3);
      this.Controls.Add(this.label2);
      this.Controls.Add(this.label1);
      this.Controls.Add(this.Response);
      this.Controls.Add(this.Value);
      this.Controls.Add(this.Properties);
      this.Controls.Add(this.ObjectName);
      this.Name = "ObjectProperties";
      this.Text = "ObjectProperties";
      this.ResumeLayout(false);

    }
    #endregion

    private void Properties_SelectedIndexChanged(object sender, System.EventArgs e)
    {
      ObjProperty prop = (ObjProperty)m_properties[Properties.SelectedIndex];
      Response.Items.Clear();
      Value.Items.Clear();
      foreach(ObjValue val in prop.Values){
        Value.Items.Add(val.Value);
      }
    }

    public void addProperty(String property, int value, String response)
    {
      Properties.Items.Add(property);
      Value.Items.Add(value);
      Response.Items.Add(response);
    }

    private void button4_Click(object sender, System.EventArgs e)
    {
      Close();
    }

    private void OK_Click(object sender, System.EventArgs e)
    {
      setValuesToNode();
      Close();
    }

    private void getValuesFromNode()
    {
      ObjectName.Text = m_node.Name;
      m_properties = m_node.Properties;
      foreach(ObjProperty prop in m_properties)
      {
        Properties.Items.Add(prop.Name);
      }
      if (m_properties.Count > 0)
      {
        ObjProperty prop = (ObjProperty)m_properties[0];
        foreach(ObjValue val in prop.Values){
          Value.Items.Add(val.Value);
        }
        if (prop.Values.Count > 0){
          ObjValue val = (ObjValue)prop.Values[0];
          foreach (ObjResponse resp in val.Responses)
          {
            Response.Items.Add(resp.Name);
          }
        }
      }
    }

    private void setValuesToNode()
    {
      m_node.Name = ObjectName.Text;
      m_node.Properties = m_properties;
    }

    private void button_property_Click(object sender, System.EventArgs e)
    {
      if (textbox_add.Text.Length == 0)
        return;
      ObjProperty prop = new ObjProperty(textbox_add.Text);
      m_properties.Add(prop);
      Properties.Items.Add(prop.Name);
    }

    private void button_value_Click(object sender, System.EventArgs e)
    {
      if (textbox_add.Text.Length == 0)
        return;
      if (Properties.SelectedIndex == -1){
        MessageBox.Show(this, "You need to select a property first.");
        return;
      }
      int x = 0;
      try
      {
        x = System.Convert.ToInt32(textbox_add.Text);
      }
      catch (System.FormatException)
      {
        MessageBox.Show(this, "You must enter a numeric value");
      	return;
      }
      ObjValue val = new ObjValue(x);
      ObjProperty prop = (ObjProperty)m_properties[Properties.SelectedIndex];
      prop.Values.Add(val);
      Value.Items.Add(val.Value);
    }

    private void button_response_Click(object sender, System.EventArgs e)
    {
      if (textbox_add.Text.Length == 0)
        return;
      if (Value.SelectedIndex == -1)
      {
        MessageBox.Show(this, "You need to select a value first.");
        return;
      }
      ObjResponse resp = new ObjResponse(textbox_add.Text);
      ObjProperty prop = (ObjProperty)m_properties[Properties.SelectedIndex];
      ObjValue val = (ObjValue)prop.Values[Value.SelectedIndex];
      val.Responses.Add(resp);
      Response.Items.Add(resp.Name);
    }

    private void Value_SelectedIndexChanged(object sender, System.EventArgs e)
    {
      if (Properties.SelectedIndex == -1){
        MessageBox.Show(this, "You need to select a property first.");
        return;
      }
      ObjProperty prop = (ObjProperty)m_properties[Properties.SelectedIndex];
      ObjValue val = (ObjValue)prop.Values[Value.SelectedIndex];
      Response.Items.Clear();
      foreach(ObjResponse resp in val.Responses)
      {
        Response.Items.Add(resp.Name);
      }
    }
	}
}
