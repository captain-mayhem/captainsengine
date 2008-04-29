using System;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for ObjResponse.
	/// </summary>
	public class ObjResponse
	{
		public ObjResponse()
		{
      m_name = "Response";
		}

    public ObjResponse(String name){
      m_name = name;
    }

    public ObjResponse(BinaryReader reader) : this() 
    {
      m_name = reader.ReadString();
    }

    public void save(BinaryWriter writer)
    {
      writer.Write(m_name);
    }

    public String Name{
      get{return m_name;}
    }

    protected String m_name;
	}
}
