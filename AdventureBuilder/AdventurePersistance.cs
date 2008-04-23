using System;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for AdventurePersistance.
	/// </summary>
	public class AdventurePersistance
	{
		public AdventurePersistance(Stream strm)
		{
      m_stream = strm;
		}

    public void save(ISavable what)
    {
      BinaryWriter writer = new BinaryWriter(m_stream);
      what.save(writer);
    }

    public object load()
    {
      BinaryReader reader = new BinaryReader(m_stream);
      return Factory.makeObject(reader);
    }

    private Stream m_stream;
	}
}
