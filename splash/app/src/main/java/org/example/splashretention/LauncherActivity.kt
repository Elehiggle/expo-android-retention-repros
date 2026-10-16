package org.example.splashretention

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.widget.Button

class LauncherActivity : Activity() {
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(Button(this).apply {
      text = "Open splash activity"
      setOnClickListener { startActivity(Intent(this@LauncherActivity, SplashActivity::class.java)) }
    })
  }
}
