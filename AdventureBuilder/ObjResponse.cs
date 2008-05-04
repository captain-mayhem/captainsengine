using System;
using System.IO;
using System.Collections;

namespace AdventureBuilder
{
  public enum Opcode
  {
    none,
    textout
  }
	/// <summary>
	/// Summary description for ObjResponse.
	/// </summary>
	public class ObjResponse
	{
		public ObjResponse()
		{
      m_name = "Response";
      m_opcode = Opcode.none;
      m_arguments = new ArrayList();
		}

    public ObjResponse(String name) : this(){
      m_name = name;
    }

    public ObjResponse(BinaryReader reader) : this() 
    {
      m_name = reader.ReadString();
      m_opcode = (Opcode)reader.ReadInt32();
      int size = reader.ReadInt32();
      for (int i = 0; i < size; ++i){
        m_arguments.Add(reader.ReadString());
      }
    }

    public void save(BinaryWriter writer)
    {
      writer.Write(m_name);
      writer.Write((int)m_opcode);
      writer.Write(m_arguments.Count);
      foreach(String str in m_arguments)
        writer.Write(str);
    }

    public String Name{
      get{return m_name;}
    }

    protected String m_name;
    protected Opcode m_opcode;
    protected ArrayList m_arguments;
	}
}
