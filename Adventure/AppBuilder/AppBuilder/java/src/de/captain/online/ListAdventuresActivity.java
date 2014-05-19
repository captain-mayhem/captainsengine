package de.captain.online;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class ListAdventuresActivity extends ListActivity{
	String mDir;
	
	public void onCreate(Bundle icicle){
		Log.i("ListAdventures", "starting search");
		super.onCreate(icicle);
		mDir = getAdventureDir();
		Log.i("ListAdventures", "Dir is "+mDir);
		//String[] values = new String[] {"testadv", "Alühn", "functiondemo"};
		String[] values = getAdventures();
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
    	//String extdir = android.os.Environment.getExternalStorageDirectory().getAbsolutePath();
		String dir = mDir+"/"+item;
		bundle.putString("adventure", dir);
		i.putExtras(bundle);
		startActivity(i);
	}
	
	private String[] getAdventures(){
		File f = new File(mDir);
		File[] files = f.listFiles();
		if (files == null){
			Log.e("ListAdventures", "No adventures found!");
			return null;
		}
		String[] ret = new String[files.length];
		for (int i = 0; i < ret.length; ++i){
			ret[i] = files[i].getName();
			Log.i("ListAdventurs", ret[i]);
		}
		return ret;
	}
	
	private String getAdventureDir(){
		String extdir = Environment.getExternalStorageDirectory().getAbsolutePath();
		if (isCorrectDir(extdir))
			return extdir+"/adventure";
		String [] otherdirs = getExternalDir();
		for (int i = 0; i < otherdirs.length; ++i){
			if (isCorrectDir(otherdirs[i])){
				return otherdirs[i]+"/adventure";
			}
		}
		return "";
	}
	
	private boolean isCorrectDir(String path){
		Log.i("ListAdventures", "checking directory"+path+"/adventure for adventures");
		File file = new File(path+"/adventure");
		return file.isDirectory();
	}
	
	private String[] getExternalDir(){
		ArrayList<String> paths = new ArrayList<String>();
		File file = new File("/system/etc/vold.fstab");
		FileReader fr = null;
		BufferedReader br = null;

		try {
			fr = new FileReader(file);
		} catch (FileNotFoundException e) {
			 e.printStackTrace();
	    }
		try {
			if (fr != null) {
				br = new BufferedReader(fr);
				String s = br.readLine();
				while (s != null) {
					if (s.startsWith("dev_mount")) {
						String[] tokens = s.split("\\s");
						String path = tokens[2]; //mount_point
						if (!Environment.getExternalStorageDirectory().getAbsolutePath().equals(path)) {
							paths.add(path);
						}
					}
					s = br.readLine();
				}
			}           
		} catch (IOException e) {
		        e.printStackTrace();
	    } finally {
	    	try {
	    		if (fr != null) {
	            fr.close();
	  		}           
		    if (br != null) {
		    	br.close();
		    }
	     } catch (IOException e) {
	          e.printStackTrace();
	       }
	    }
		paths.add("/storage/extSdCard");//TODO
	    String[] ret = new String[paths.size()];
	    for (int i = 0; i < paths.size(); ++i){
	    	ret[i] = paths.get(i);
	    }
	    return ret;
	}
}
