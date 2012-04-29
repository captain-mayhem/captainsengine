package de.captain.online;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class ListAdventuresActivity extends ListActivity{
	public void onCreate(Bundle icicle){
		super.onCreate(icicle);
		String[] values = new String[] {"testadv", "Alühn", "functiondemo"};
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(
				this, android.R.layout.simple_list_item_1, values);
		setListAdapter(adapter);
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id){
		String item = (String) getListAdapter().getItem(position);
		Toast.makeText(this, item + " selected", Toast.LENGTH_LONG).show();
		Intent i = new Intent(this, AdventureActivity.class);
		//Intent i = new Intent();
		//i.setAction("de.captain.online.AdventureActivity.START");
		Bundle bundle = new Bundle();
		bundle.putString("adventure", item);
		i.putExtras(bundle);
		startActivity(i);
	}
}
