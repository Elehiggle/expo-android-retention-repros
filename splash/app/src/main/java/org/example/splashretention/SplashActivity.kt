package org.example.splashretention

import android.app.Activity
import android.os.Bundle
import android.widget.Button
import expo.modules.splashscreen.SplashScreenManager
import expo.modules.splashscreen.SplashScreenOptions

class SplashActivity : Activity() {
  override fun onCreate(savedInstanceState: Bundle?) {
    SplashScreenManager.registerOnActivity(this)
    super.onCreate(savedInstanceState)
    repeat(3) { SplashScreenManager.setSplashScreenOptions(SplashScreenOptions()) }
    setContentView(Button(this).apply {
      text = "Finish activity"
      setOnClickListener { finish() }
    })
    SplashScreenManager.hide()
  }
}
