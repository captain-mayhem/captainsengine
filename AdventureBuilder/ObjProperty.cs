using System;
using System.Collections;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Properties.
	/// </summary>
	public class ObjProperty
	{
    public ObjProperty(){
      m_values = new ArrayList();
      m_name = "Property";
    }

		public ObjProperty(String name) : this()
		{
      m_name = name;
		}

    public ObjProperty(BinaryReader reader) : this() 
    {
      int count = reader.ReadInt32();
      for (int i = 0; i < count; ++i)
      {
        m_values.Add(new ObjValue(reader));
      }
      m_name = reader.ReadString();
    }

    public void save(BinaryWriter writer)
    {
      writer.Write(m_values.Count);
      foreach (ObjValue val in m_values)
      {
        val.save(writer);
      }
      writer.Write(m_name);
    }

    public ArrayList Values{
      get{return m_values;}
    }

    public String Name{
      get{return m_name;}
    }

    protected String m_name;
    protected ArrayList m_values;
	}
}
