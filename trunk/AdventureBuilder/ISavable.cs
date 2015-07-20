using System;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for ISavable.
	/// </summary>
	public interface ISavable
	{
    void save(BinaryWriter writer);
	}
}
