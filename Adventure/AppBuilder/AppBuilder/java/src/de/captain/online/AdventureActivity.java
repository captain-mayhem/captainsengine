
package de.captain.online;

import android.app.Activity;
import android.os.Bundle;

public class AdventureActivity extends Activity {
	
	AdventureView mView;

	@Override protected void onCreate(Bundle icicle){
		super.onCreate(icicle);
		mView = new AdventureView(getApplication());
		setContentView(mView);
	}
	
	@Override protected void onPause(){
		super.onPause();
		mView.onPause();
	}
	
	@Override protected void onResume(){
		super.onResume();
		mView.onResume();
	}
}
