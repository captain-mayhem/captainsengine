using System;
using System.IO;
using System.Collections;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for ObjValues.
	/// </summary>
	public class ObjValue
	{
		public ObjValue()
		{
      m_responses = new ArrayList();
      m_value = 0;
		}

    public ObjValue(int value) : this() {
      m_value = value;
    }

    public ObjValue(BinaryReader reader) : this() 
    {
      int count = reader.ReadInt32();
      for (int i = 0; i < count; ++i)
      {
        m_responses.Add(new ObjResponse(reader));
      }
      m_value = reader.ReadInt32();
    }

    public void save(BinaryWriter writer)
    {
      writer.Write(m_responses.Count);
      foreach (ObjResponse resp in m_responses)
      {
        resp.save(writer);
      }
      writer.Write(m_value);
    }

    public int Value{
      get{return m_value;}
    }

    public ArrayList Responses{
      get{return m_responses;}
    }

    protected ArrayList m_responses;
    protected int m_value;
	}
}
