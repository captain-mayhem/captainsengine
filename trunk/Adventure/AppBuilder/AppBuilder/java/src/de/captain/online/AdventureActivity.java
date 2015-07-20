
package de.captain.online;

import android.app.Activity;
import android.os.Bundle;

public class AdventureActivity extends Activity {
	
	AdventureView mView;

	@Override protected void onCreate(Bundle icicle){
		super.onCreate(icicle);
		Bundle arguments = getIntent().getExtras();
		String adventure = arguments.getString("adventure");
		mView = new AdventureView(this, adventure);
		mView.setFocusableInTouchMode(true);
		mView.requestFocus();
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
